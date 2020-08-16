#include <linux/trace_error.h>
#include <linux/sched.h>
#include <linux/preempt.h>

void set_last_err(const char *file, unsigned int line, int errno)
{
	struct last_err *last_err;

	if (!in_task())
		return;

	last_err = &current->last_err;

	last_err->file = file;
	last_err->line = line;
	last_err->errno = errno;
}

EXPORT_SYMBOL(set_last_err);
