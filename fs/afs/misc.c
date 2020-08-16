// SPDX-License-Identifier: GPL-2.0-or-later
/* miscellaneous bits
 *
 * Copyright (C) 2002, 2007 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include "internal.h"
#include "afs_fs.h"
#include "protocol_uae.h"

/*
 * convert an AFS abort code to a Linux error number
 */
int afs_abort_to_error(u32 abort_code)
{
	switch (abort_code) {
		/* Low errno codes inserted into abort namespace */
	case 13:		return -ERR(EACCES);
	case 27:		return -ERR(EFBIG);
	case 30:		return -ERR(EROFS);

		/* VICE "special error" codes; 101 - 111 */
	case VSALVAGE:		return -ERR(EIO);
	case VNOVNODE:		return -ERR(ENOENT);
	case VNOVOL:		return -ERR(ENOMEDIUM);
	case VVOLEXISTS:	return -ERR(EEXIST);
	case VNOSERVICE:	return -ERR(EIO);
	case VOFFLINE:		return -ERR(ENOENT);
	case VONLINE:		return -ERR(EEXIST);
	case VDISKFULL:		return -ERR(ENOSPC);
	case VOVERQUOTA:	return -ERR(EDQUOT);
	case VBUSY:		return -ERR(EBUSY);
	case VMOVED:		return -ERR(ENXIO);

		/* Volume Location server errors */
	case AFSVL_IDEXIST:		return -ERR(EEXIST);
	case AFSVL_IO:			return -ERR(EREMOTEIO);
	case AFSVL_NAMEEXIST:		return -ERR(EEXIST);
	case AFSVL_CREATEFAIL:		return -ERR(EREMOTEIO);
	case AFSVL_NOENT:		return -ERR(ENOMEDIUM);
	case AFSVL_EMPTY:		return -ERR(ENOMEDIUM);
	case AFSVL_ENTDELETED:		return -ERR(ENOMEDIUM);
	case AFSVL_BADNAME:		return -ERR(EINVAL);
	case AFSVL_BADINDEX:		return -ERR(EINVAL);
	case AFSVL_BADVOLTYPE:		return -ERR(EINVAL);
	case AFSVL_BADSERVER:		return -ERR(EINVAL);
	case AFSVL_BADPARTITION:	return -ERR(EINVAL);
	case AFSVL_REPSFULL:		return -ERR(EFBIG);
	case AFSVL_NOREPSERVER:		return -ERR(ENOENT);
	case AFSVL_DUPREPSERVER:	return -ERR(EEXIST);
	case AFSVL_RWNOTFOUND:		return -ERR(ENOENT);
	case AFSVL_BADREFCOUNT:		return -ERR(EINVAL);
	case AFSVL_SIZEEXCEEDED:	return -ERR(EINVAL);
	case AFSVL_BADENTRY:		return -ERR(EINVAL);
	case AFSVL_BADVOLIDBUMP:	return -ERR(EINVAL);
	case AFSVL_IDALREADYHASHED:	return -ERR(EINVAL);
	case AFSVL_ENTRYLOCKED:		return -ERR(EBUSY);
	case AFSVL_BADVOLOPER:		return -ERR(EBADRQC);
	case AFSVL_BADRELLOCKTYPE:	return -ERR(EINVAL);
	case AFSVL_RERELEASE:		return -ERR(EREMOTEIO);
	case AFSVL_BADSERVERFLAG:	return -ERR(EINVAL);
	case AFSVL_PERM:		return -ERR(EACCES);
	case AFSVL_NOMEM:		return -ERR(EREMOTEIO);

		/* Unified AFS error table */
	case UAEPERM:			return -ERR(EPERM);
	case UAENOENT:			return -ERR(ENOENT);
	case UAEACCES:			return -ERR(EACCES);
	case UAEBUSY:			return -ERR(EBUSY);
	case UAEEXIST:			return -ERR(EEXIST);
	case UAENOTDIR:			return -ERR(ENOTDIR);
	case UAEISDIR:			return -ERR(EISDIR);
	case UAEFBIG:			return -ERR(EFBIG);
	case UAENOSPC:			return -ERR(ENOSPC);
	case UAEROFS:			return -ERR(EROFS);
	case UAEMLINK:			return -ERR(EMLINK);
	case UAEDEADLK:			return -ERR(EDEADLK);
	case UAENAMETOOLONG:		return -ERR(ENAMETOOLONG);
	case UAENOLCK:			return -ERR(ENOLCK);
	case UAENOTEMPTY:		return -ERR(ENOTEMPTY);
	case UAELOOP:			return -ERR(ELOOP);
	case UAEOVERFLOW:		return -ERR(EOVERFLOW);
	case UAENOMEDIUM:		return -ERR(ENOMEDIUM);
	case UAEDQUOT:			return -ERR(EDQUOT);

		/* RXKAD abort codes; from include/rxrpc/packet.h.  ET "RXK" == 0x1260B00 */
	case RXKADINCONSISTENCY: return -ERR(EPROTO);
	case RXKADPACKETSHORT:	return -ERR(EPROTO);
	case RXKADLEVELFAIL:	return -ERR(EKEYREJECTED);
	case RXKADTICKETLEN:	return -ERR(EKEYREJECTED);
	case RXKADOUTOFSEQUENCE: return -ERR(EPROTO);
	case RXKADNOAUTH:	return -ERR(EKEYREJECTED);
	case RXKADBADKEY:	return -ERR(EKEYREJECTED);
	case RXKADBADTICKET:	return -ERR(EKEYREJECTED);
	case RXKADUNKNOWNKEY:	return -ERR(EKEYREJECTED);
	case RXKADEXPIRED:	return -ERR(EKEYEXPIRED);
	case RXKADSEALEDINCON:	return -ERR(EKEYREJECTED);
	case RXKADDATALEN:	return -ERR(EKEYREJECTED);
	case RXKADILLEGALLEVEL:	return -ERR(EKEYREJECTED);

	case RXGEN_OPCODE:	return -ERR(ENOTSUPP);

	default:		return -ERR(EREMOTEIO);
	}
}

/*
 * Select the error to report from a set of errors.
 */
void afs_prioritise_error(struct afs_error *e, int error, u32 abort_code)
{
	switch (error) {
	case 0:
		return;
	default:
		if (e->error == -ETIMEDOUT ||
		    e->error == -ETIME)
			return;
		/* Fall through */
	case -ETIMEDOUT:
	case -ETIME:
		if (e->error == -ENOMEM ||
		    e->error == -ENONET)
			return;
		/* Fall through */
	case -ENOMEM:
	case -ENONET:
		if (e->error == -ERFKILL)
			return;
		/* Fall through */
	case -ERFKILL:
		if (e->error == -EADDRNOTAVAIL)
			return;
		/* Fall through */
	case -EADDRNOTAVAIL:
		if (e->error == -ENETUNREACH)
			return;
		/* Fall through */
	case -ENETUNREACH:
		if (e->error == -EHOSTUNREACH)
			return;
		/* Fall through */
	case -EHOSTUNREACH:
		if (e->error == -EHOSTDOWN)
			return;
		/* Fall through */
	case -EHOSTDOWN:
		if (e->error == -ECONNREFUSED)
			return;
		/* Fall through */
	case -ECONNREFUSED:
		if (e->error == -ECONNRESET)
			return;
		/* Fall through */
	case -ECONNRESET: /* Responded, but call expired. */
		if (e->responded)
			return;
		e->error = error;
		return;

	case -ECONNABORTED:
		e->responded = true;
		e->error = afs_abort_to_error(abort_code);
		return;
	}
}
