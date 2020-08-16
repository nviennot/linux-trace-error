Error provenance in the Linux kernel
====================================

Problem
-------

We often use `strace` to understand why our programs fail. It is useful to
see which system calls fail, which which error. However, it is hard to
understand what would be required to fix certain errors at times.
For example, when a system call fails with `EPERM`, a number of factors can be at play,
such as namespaces configuration, capabilities, seccomp, selinux, or other
security mechanisms, it's hard and tedious to pinpoint origin the problem.

To determine the cause of an error, we typically start by fiddling with the
program invocation, sometimes trying with various elevated privileges.
When pocking in the dark doesn't work, we can read the kernel source code, go
through the syscall code path to figure out what could possibly go wrong. When
this fails, we can run the kernel in qemu, hook dbg, and start tracing.

This process can be frustrating and time consuming. The following describes a
simple and effective way to deal with this problem.

Solution
---------

To determine the cause of certain errors, we can modify the kernel to record
the provenance of all system call errors.

In the kernel source code, all error constants (like `EPERM`) are wrapped with
a macro `ERR(EPERM)`. This macro saves the corresponding file and line number
in the task struct of the current process. This information can then be
retrieved by reading `/proc/pid/last_error`. This makes it easy to integrate
in tools such as `strace`.

We cannot search and replace all error constants blindly in the codebase.
We must skip constants that are used in `if` or `switch/case` statements.
The compiler will complain with the latter that the `case` expression must be
constant. The semantic patching tool [Coccinelle](http://coccinelle.lip6.fr/)
is used to do the patching. The patching is done in the following situations:
1) variable initializations 2) variable assignments and 3) return statements.
See the applied patch [here](https://github.com/nviennot/linux-trace-error/commit/fd9e7310aaee716d265f2144fc355232a195dc5a#diff-6e5425acbc4ca30ccaa348e8e98fb1fe), it's quite simple and effective.
Using this patch, we instrument more than 40,000 errors in the kernel,
excluding the `drivers/` subtree.

Example
-------

The following shows an example of using a modified version of strace to
instrument a bash process trying to write to `/proc/sys/kernel/ns_last_pid`
without the proper privileges.

```bash
$ strace bash -c 'echo 123 > /proc/sys/kernel/ns_last_pid' |& grep '= -1'

access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory) fs/namei.c:1269
ioctl(-1, TIOCGPGRP, 0x7ffef4bc6a24)    = -1 EBADF (Bad file descriptor) fs/ioctl.c:745
stat("/sbin/bash", 0x7ffef4bc66a0)      = -1 ENOENT (No such file or directory) fs/namei.c:1269
connect(3, {sa_family=AF_UNIX, sun_path="/var/run/nscd/socket"}, 110) = -1 ENOENT (No such file or directory) fs/namei.c:1269
ioctl(2, TIOCGPGRP, 0x7ffef4bc68d4)     = -1 ENOTTY (Inappropriate ioctl for device) fs/ioctl.c:50
getpeername(0, 0x7ffef4bc6a20, [16])    = -1 ENOTSOCK (Socket operation on non-socket) net/socket.c:458
write(1, "123\n", 4)                    = -1 EPERM (Operation not permitted) kernel/pid_namespace.c:273
```

Each syscall error is shown with a kernel code location. This location is
the content of `/proc/pid/last_error` content, pointing us to the location
where the error is likely to be originating from. The following shows the
source code corresponding to the previous strace output.

```c
// fs/ioctl.c
741: 	struct fd f = fdget(fd);
742: 	int error;
743: 
744: 	if (!f.file)
745: 		return -ERR(EBADF);

// fs/ioctl.c
48:	error = filp->f_op->unlocked_ioctl(filp, cmd, arg);
49:	if (error == -ENOIOCTLCMD)
50:		error = -ERR(ENOTTY);

// net/socket.c
453: struct socket *sock_from_file(struct file *file, int *err)
454: {
455: 	if (file->f_op == &socket_file_ops)
456: 		return file->private_data;	/* set in sock_map_fd */
457:
458: 	*err = -ERR(ENOTSOCK);
459: 	return NULL;
460: }

// kernel/pid_namespace.c
272:	if (write && !ns_capable(pid_ns->user_ns, CAP_SYS_ADMIN))
273:		return -ERR(EPERM);
```

We can quickly see what is the root cause of the syscall errors.
For example, we see that the `EPERM` error returned from the `write()`
system call is caused by the missing `CAP_SYS_ADMIN` capability in the
current user namespace.

Error provenance can greatly facilitate the troubleshooting of system call
errors.

How to use
----------

There are two ways to prepare the kernel:
* Clone the current repo and use it as is. This kernel is based on Linux 5.8.
* Apply this [patch](https://github.com/nviennot/linux-trace-error/commit/fd9e7310aaee716d265f2144fc355232a195dc5a#diff-6e5425acbc4ca30ccaa348e8e98fb1fe) and run `spatch_errors.sh` on your kernel.

Once you have a running kernel, pair it with this [strace
fork](https://github.com/nviennot/strace-trace-error) and you should be all set.

Original idea by [Geoffrey Thomas](https://github.com/geofft)
