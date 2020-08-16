/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LAST_ERR_H
#define _LINUX_LAST_ERR_H

#ifdef CONFIG_TRACE_ERROR
#ifndef __ASSEMBLY__

struct last_err {
	const char	*file;
	unsigned int	line;
	int 		errno;
};

extern void set_last_err(const char *file, unsigned int line, int errno);

#define ERR(errno) ({					\
	set_last_err(__FILE__, __LINE__, errno);	\
	errno;						\
})

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_TRACE_ERROR */

#endif
