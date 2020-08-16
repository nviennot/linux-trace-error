// SPDX-License-Identifier: GPL-2.0-only
/*
 *  The NFC Controller Interface is the communication protocol between an
 *  NFC Controller (NFCC) and a Device Host (DH).
 *
 *  Copyright (C) 2011 Texas Instruments, Inc.
 *
 *  Written by Ilan Elias <ilane@ti.com>
 *
 *  Acknowledgements:
 *  This file is based on lib.c, which was written
 *  by Maxim Krasnyansky.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>

#include <net/nfc/nci.h>
#include <net/nfc/nci_core.h>

/* NCI status codes to Unix errno mapping */
int nci_to_errno(__u8 code)
{
	switch (code) {
	case NCI_STATUS_OK:
		return 0;

	case NCI_STATUS_REJECTED:
		return -ERR(EBUSY);

	case NCI_STATUS_RF_FRAME_CORRUPTED:
		return -ERR(EBADMSG);

	case NCI_STATUS_NOT_INITIALIZED:
		return -ERR(EHOSTDOWN);

	case NCI_STATUS_SYNTAX_ERROR:
	case NCI_STATUS_SEMANTIC_ERROR:
	case NCI_STATUS_INVALID_PARAM:
	case NCI_STATUS_RF_PROTOCOL_ERROR:
	case NCI_STATUS_NFCEE_PROTOCOL_ERROR:
		return -ERR(EPROTO);

	case NCI_STATUS_UNKNOWN_GID:
	case NCI_STATUS_UNKNOWN_OID:
		return -ERR(EBADRQC);

	case NCI_STATUS_MESSAGE_SIZE_EXCEEDED:
		return -ERR(EMSGSIZE);

	case NCI_STATUS_DISCOVERY_ALREADY_STARTED:
		return -ERR(EALREADY);

	case NCI_STATUS_DISCOVERY_TARGET_ACTIVATION_FAILED:
	case NCI_STATUS_NFCEE_INTERFACE_ACTIVATION_FAILED:
		return -ERR(ECONNREFUSED);

	case NCI_STATUS_RF_TRANSMISSION_ERROR:
	case NCI_STATUS_NFCEE_TRANSMISSION_ERROR:
		return -ERR(ECOMM);

	case NCI_STATUS_RF_TIMEOUT_ERROR:
	case NCI_STATUS_NFCEE_TIMEOUT_ERROR:
		return -ERR(ETIMEDOUT);

	case NCI_STATUS_FAILED:
	default:
		return -ERR(ENOSYS);
	}
}
EXPORT_SYMBOL(nci_to_errno);
