// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Anna Schumaker <Anna.Schumaker@Netapp.com>
 */
#ifndef __LINUX_FS_NFS_NFS4_2XDR_H
#define __LINUX_FS_NFS_NFS4_2XDR_H

#include "nfs42.h"

#define encode_fallocate_maxsz		(encode_stateid_maxsz + \
					 2 /* offset */ + \
					 2 /* length */)
#define NFS42_WRITE_RES_SIZE		(1 /* wr_callback_id size */ +\
					 XDR_QUADLEN(NFS4_STATEID_SIZE) + \
					 2 /* wr_count */ + \
					 1 /* wr_committed */ + \
					 XDR_QUADLEN(NFS4_VERIFIER_SIZE))
#define encode_allocate_maxsz		(op_encode_hdr_maxsz + \
					 encode_fallocate_maxsz)
#define decode_allocate_maxsz		(op_decode_hdr_maxsz)
#define encode_copy_maxsz		(op_encode_hdr_maxsz +          \
					 XDR_QUADLEN(NFS4_STATEID_SIZE) + \
					 XDR_QUADLEN(NFS4_STATEID_SIZE) + \
					 2 + 2 + 2 + 1 + 1 + 1 +\
					 1 + /* One cnr_source_server */\
					 1 + /* nl4_type */ \
					 1 + XDR_QUADLEN(NFS4_OPAQUE_LIMIT))
#define decode_copy_maxsz		(op_decode_hdr_maxsz + \
					 NFS42_WRITE_RES_SIZE + \
					 1 /* cr_consecutive */ + \
					 1 /* cr_synchronous */)
#define encode_offload_cancel_maxsz	(op_encode_hdr_maxsz + \
					 XDR_QUADLEN(NFS4_STATEID_SIZE))
#define decode_offload_cancel_maxsz	(op_decode_hdr_maxsz)
#define encode_copy_notify_maxsz	(op_encode_hdr_maxsz + \
					 XDR_QUADLEN(NFS4_STATEID_SIZE) + \
					 1 + /* nl4_type */ \
					 1 + XDR_QUADLEN(NFS4_OPAQUE_LIMIT))
#define decode_copy_notify_maxsz	(op_decode_hdr_maxsz + \
					 3 + /* cnr_lease_time */\
					 XDR_QUADLEN(NFS4_STATEID_SIZE) + \
					 1 + /* Support 1 cnr_source_server */\
					 1 + /* nl4_type */ \
					 1 + XDR_QUADLEN(NFS4_OPAQUE_LIMIT))
#define encode_deallocate_maxsz		(op_encode_hdr_maxsz + \
					 encode_fallocate_maxsz)
#define decode_deallocate_maxsz		(op_decode_hdr_maxsz)
#define encode_seek_maxsz		(op_encode_hdr_maxsz + \
					 encode_stateid_maxsz + \
					 2 /* offset */ + \
					 1 /* whence */)
#define decode_seek_maxsz		(op_decode_hdr_maxsz + \
					 1 /* eof */ + \
					 1 /* whence */ + \
					 2 /* offset */ + \
					 2 /* length */)
#define encode_io_info_maxsz		4
#define encode_layoutstats_maxsz	(op_decode_hdr_maxsz + \
					2 /* offset */ + \
					2 /* length */ + \
					encode_stateid_maxsz + \
					encode_io_info_maxsz + \
					encode_io_info_maxsz + \
					1 /* opaque devaddr4 length */ + \
					XDR_QUADLEN(PNFS_LAYOUTSTATS_MAXSIZE))
#define decode_layoutstats_maxsz	(op_decode_hdr_maxsz)
#define encode_device_error_maxsz	(XDR_QUADLEN(NFS4_DEVICEID4_SIZE) + \
					1 /* status */ + 1 /* opnum */)
#define encode_layouterror_maxsz	(op_decode_hdr_maxsz + \
					2 /* offset */ + \
					2 /* length */ + \
					encode_stateid_maxsz + \
					1 /* Array size */ + \
					encode_device_error_maxsz)
#define decode_layouterror_maxsz	(op_decode_hdr_maxsz)
#define encode_clone_maxsz		(encode_stateid_maxsz + \
					encode_stateid_maxsz + \
					2 /* src offset */ + \
					2 /* dst offset */ + \
					2 /* count */)
#define decode_clone_maxsz		(op_decode_hdr_maxsz)

#define NFS4_enc_allocate_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 encode_allocate_maxsz + \
					 encode_getattr_maxsz)
#define NFS4_dec_allocate_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 decode_allocate_maxsz + \
					 decode_getattr_maxsz)
#define NFS4_enc_copy_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 encode_savefh_maxsz + \
					 encode_putfh_maxsz + \
					 encode_copy_maxsz + \
					 encode_commit_maxsz)
#define NFS4_dec_copy_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 decode_savefh_maxsz + \
					 decode_putfh_maxsz + \
					 decode_copy_maxsz + \
					 decode_commit_maxsz)
#define NFS4_enc_offload_cancel_sz	(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 encode_offload_cancel_maxsz)
#define NFS4_dec_offload_cancel_sz	(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 decode_offload_cancel_maxsz)
#define NFS4_enc_copy_notify_sz		(compound_encode_hdr_maxsz + \
					 encode_putfh_maxsz + \
					 encode_copy_notify_maxsz)
#define NFS4_dec_copy_notify_sz		(compound_decode_hdr_maxsz + \
					 decode_putfh_maxsz + \
					 decode_copy_notify_maxsz)
#define NFS4_enc_deallocate_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 encode_deallocate_maxsz + \
					 encode_getattr_maxsz)
#define NFS4_dec_deallocate_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 decode_deallocate_maxsz + \
					 decode_getattr_maxsz)
#define NFS4_enc_seek_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 encode_seek_maxsz)
#define NFS4_dec_seek_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 decode_seek_maxsz)
#define NFS4_enc_layoutstats_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 PNFS_LAYOUTSTATS_MAXDEV * encode_layoutstats_maxsz)
#define NFS4_dec_layoutstats_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 PNFS_LAYOUTSTATS_MAXDEV * decode_layoutstats_maxsz)
#define NFS4_enc_layouterror_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 NFS42_LAYOUTERROR_MAX * \
					 encode_layouterror_maxsz)
#define NFS4_dec_layouterror_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 NFS42_LAYOUTERROR_MAX * \
					 decode_layouterror_maxsz)
#define NFS4_enc_clone_sz		(compound_encode_hdr_maxsz + \
					 encode_sequence_maxsz + \
					 encode_putfh_maxsz + \
					 encode_savefh_maxsz + \
					 encode_putfh_maxsz + \
					 encode_clone_maxsz + \
					 encode_getattr_maxsz)
#define NFS4_dec_clone_sz		(compound_decode_hdr_maxsz + \
					 decode_sequence_maxsz + \
					 decode_putfh_maxsz + \
					 decode_savefh_maxsz + \
					 decode_putfh_maxsz + \
					 decode_clone_maxsz + \
					 decode_getattr_maxsz)

static void encode_fallocate(struct xdr_stream *xdr,
			     const struct nfs42_falloc_args *args)
{
	encode_nfs4_stateid(xdr, &args->falloc_stateid);
	encode_uint64(xdr, args->falloc_offset);
	encode_uint64(xdr, args->falloc_length);
}

static void encode_allocate(struct xdr_stream *xdr,
			    const struct nfs42_falloc_args *args,
			    struct compound_hdr *hdr)
{
	encode_op_hdr(xdr, OP_ALLOCATE, decode_allocate_maxsz, hdr);
	encode_fallocate(xdr, args);
}

static void encode_nl4_server(struct xdr_stream *xdr,
			      const struct nl4_server *ns)
{
	encode_uint32(xdr, ns->nl4_type);
	switch (ns->nl4_type) {
	case NL4_NAME:
	case NL4_URL:
		encode_string(xdr, ns->u.nl4_str_sz, ns->u.nl4_str);
		break;
	case NL4_NETADDR:
		encode_string(xdr, ns->u.nl4_addr.netid_len,
			      ns->u.nl4_addr.netid);
		encode_string(xdr, ns->u.nl4_addr.addr_len,
			      ns->u.nl4_addr.addr);
		break;
	default:
		WARN_ON_ONCE(1);
	}
}

static void encode_copy(struct xdr_stream *xdr,
			const struct nfs42_copy_args *args,
			struct compound_hdr *hdr)
{
	encode_op_hdr(xdr, OP_COPY, decode_copy_maxsz, hdr);
	encode_nfs4_stateid(xdr, &args->src_stateid);
	encode_nfs4_stateid(xdr, &args->dst_stateid);

	encode_uint64(xdr, args->src_pos);
	encode_uint64(xdr, args->dst_pos);
	encode_uint64(xdr, args->count);

	encode_uint32(xdr, 1); /* consecutive = true */
	encode_uint32(xdr, args->sync);
	if (args->cp_src == NULL) { /* intra-ssc */
		encode_uint32(xdr, 0); /* no src server list */
		return;
	}
	encode_uint32(xdr, 1); /* supporting 1 server */
	encode_nl4_server(xdr, args->cp_src);
}

static void encode_offload_cancel(struct xdr_stream *xdr,
				  const struct nfs42_offload_status_args *args,
				  struct compound_hdr *hdr)
{
	encode_op_hdr(xdr, OP_OFFLOAD_CANCEL, decode_offload_cancel_maxsz, hdr);
	encode_nfs4_stateid(xdr, &args->osa_stateid);
}

static void encode_copy_notify(struct xdr_stream *xdr,
			       const struct nfs42_copy_notify_args *args,
			       struct compound_hdr *hdr)
{
	encode_op_hdr(xdr, OP_COPY_NOTIFY, decode_copy_notify_maxsz, hdr);
	encode_nfs4_stateid(xdr, &args->cna_src_stateid);
	encode_nl4_server(xdr, &args->cna_dst);
}

static void encode_deallocate(struct xdr_stream *xdr,
			      const struct nfs42_falloc_args *args,
			      struct compound_hdr *hdr)
{
	encode_op_hdr(xdr, OP_DEALLOCATE, decode_deallocate_maxsz, hdr);
	encode_fallocate(xdr, args);
}

static void encode_seek(struct xdr_stream *xdr,
			const struct nfs42_seek_args *args,
			struct compound_hdr *hdr)
{
	encode_op_hdr(xdr, OP_SEEK, decode_seek_maxsz, hdr);
	encode_nfs4_stateid(xdr, &args->sa_stateid);
	encode_uint64(xdr, args->sa_offset);
	encode_uint32(xdr, args->sa_what);
}

static void encode_layoutstats(struct xdr_stream *xdr,
			       const struct nfs42_layoutstat_args *args,
			       struct nfs42_layoutstat_devinfo *devinfo,
			       struct compound_hdr *hdr)
{
	__be32 *p;

	encode_op_hdr(xdr, OP_LAYOUTSTATS, decode_layoutstats_maxsz, hdr);
	p = reserve_space(xdr, 8 + 8);
	p = xdr_encode_hyper(p, devinfo->offset);
	p = xdr_encode_hyper(p, devinfo->length);
	encode_nfs4_stateid(xdr, &args->stateid);
	p = reserve_space(xdr, 4*8 + NFS4_DEVICEID4_SIZE + 4);
	p = xdr_encode_hyper(p, devinfo->read_count);
	p = xdr_encode_hyper(p, devinfo->read_bytes);
	p = xdr_encode_hyper(p, devinfo->write_count);
	p = xdr_encode_hyper(p, devinfo->write_bytes);
	p = xdr_encode_opaque_fixed(p, devinfo->dev_id.data,
			NFS4_DEVICEID4_SIZE);
	/* Encode layoutupdate4 */
	*p++ = cpu_to_be32(devinfo->layout_type);
	if (devinfo->ld_private.ops)
		devinfo->ld_private.ops->encode(xdr, args,
				&devinfo->ld_private);
	else
		encode_uint32(xdr, 0);
}

static void encode_clone(struct xdr_stream *xdr,
			 const struct nfs42_clone_args *args,
			 struct compound_hdr *hdr)
{
	__be32 *p;

	encode_op_hdr(xdr, OP_CLONE, decode_clone_maxsz, hdr);
	encode_nfs4_stateid(xdr, &args->src_stateid);
	encode_nfs4_stateid(xdr, &args->dst_stateid);
	p = reserve_space(xdr, 3*8);
	p = xdr_encode_hyper(p, args->src_offset);
	p = xdr_encode_hyper(p, args->dst_offset);
	xdr_encode_hyper(p, args->count);
}

static void encode_device_error(struct xdr_stream *xdr,
				const struct nfs42_device_error *error)
{
	__be32 *p;

	p = reserve_space(xdr, NFS4_DEVICEID4_SIZE + 2*4);
	p = xdr_encode_opaque_fixed(p, error->dev_id.data,
			NFS4_DEVICEID4_SIZE);
	*p++ = cpu_to_be32(error->status);
	*p = cpu_to_be32(error->opnum);
}

static void encode_layouterror(struct xdr_stream *xdr,
			       const struct nfs42_layout_error *args,
			       struct compound_hdr *hdr)
{
	__be32 *p;

	encode_op_hdr(xdr, OP_LAYOUTERROR, decode_layouterror_maxsz, hdr);
	p = reserve_space(xdr, 8 + 8);
	p = xdr_encode_hyper(p, args->offset);
	p = xdr_encode_hyper(p, args->length);
	encode_nfs4_stateid(xdr, &args->stateid);
	p = reserve_space(xdr, 4);
	*p = cpu_to_be32(1);
	encode_device_error(xdr, &args->errors[0]);
}

/*
 * Encode ALLOCATE request
 */
static void nfs4_xdr_enc_allocate(struct rpc_rqst *req,
				  struct xdr_stream *xdr,
				  const void *data)
{
	const struct nfs42_falloc_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->falloc_fh, &hdr);
	encode_allocate(xdr, args, &hdr);
	encode_getfattr(xdr, args->falloc_bitmask, &hdr);
	encode_nops(&hdr);
}

static void encode_copy_commit(struct xdr_stream *xdr,
			  const struct nfs42_copy_args *args,
			  struct compound_hdr *hdr)
{
	__be32 *p;

	encode_op_hdr(xdr, OP_COMMIT, decode_commit_maxsz, hdr);
	p = reserve_space(xdr, 12);
	p = xdr_encode_hyper(p, args->dst_pos);
	*p = cpu_to_be32(args->count);
}

/*
 * Encode COPY request
 */
static void nfs4_xdr_enc_copy(struct rpc_rqst *req,
			      struct xdr_stream *xdr,
			      const void *data)
{
	const struct nfs42_copy_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->src_fh, &hdr);
	encode_savefh(xdr, &hdr);
	encode_putfh(xdr, args->dst_fh, &hdr);
	encode_copy(xdr, args, &hdr);
	if (args->sync)
		encode_copy_commit(xdr, args, &hdr);
	encode_nops(&hdr);
}

/*
 * Encode OFFLOAD_CANEL request
 */
static void nfs4_xdr_enc_offload_cancel(struct rpc_rqst *req,
					struct xdr_stream *xdr,
					const void *data)
{
	const struct nfs42_offload_status_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->osa_seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->osa_seq_args, &hdr);
	encode_putfh(xdr, args->osa_src_fh, &hdr);
	encode_offload_cancel(xdr, args, &hdr);
	encode_nops(&hdr);
}

/*
 * Encode COPY_NOTIFY request
 */
static void nfs4_xdr_enc_copy_notify(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs42_copy_notify_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->cna_seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->cna_seq_args, &hdr);
	encode_putfh(xdr, args->cna_src_fh, &hdr);
	encode_copy_notify(xdr, args, &hdr);
	encode_nops(&hdr);
}

/*
 * Encode DEALLOCATE request
 */
static void nfs4_xdr_enc_deallocate(struct rpc_rqst *req,
				    struct xdr_stream *xdr,
				    const void *data)
{
	const struct nfs42_falloc_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->falloc_fh, &hdr);
	encode_deallocate(xdr, args, &hdr);
	encode_getfattr(xdr, args->falloc_bitmask, &hdr);
	encode_nops(&hdr);
}

/*
 * Encode SEEK request
 */
static void nfs4_xdr_enc_seek(struct rpc_rqst *req,
			      struct xdr_stream *xdr,
			      const void *data)
{
	const struct nfs42_seek_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->sa_fh, &hdr);
	encode_seek(xdr, args, &hdr);
	encode_nops(&hdr);
}

/*
 * Encode LAYOUTSTATS request
 */
static void nfs4_xdr_enc_layoutstats(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs42_layoutstat_args *args = data;
	int i;

	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	WARN_ON(args->num_dev > PNFS_LAYOUTSTATS_MAXDEV);
	for (i = 0; i < args->num_dev; i++)
		encode_layoutstats(xdr, args, &args->devinfo[i], &hdr);
	encode_nops(&hdr);
}

/*
 * Encode CLONE request
 */
static void nfs4_xdr_enc_clone(struct rpc_rqst *req,
			       struct xdr_stream *xdr,
			       const void *data)
{
	const struct nfs42_clone_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->src_fh, &hdr);
	encode_savefh(xdr, &hdr);
	encode_putfh(xdr, args->dst_fh, &hdr);
	encode_clone(xdr, args, &hdr);
	encode_getfattr(xdr, args->dst_bitmask, &hdr);
	encode_nops(&hdr);
}

/*
 * Encode LAYOUTERROR request
 */
static void nfs4_xdr_enc_layouterror(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs42_layouterror_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};
	int i;

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, NFS_FH(args->inode), &hdr);
	for (i = 0; i < args->num_errors; i++)
		encode_layouterror(xdr, &args->errors[i], &hdr);
	encode_nops(&hdr);
}

static int decode_allocate(struct xdr_stream *xdr, struct nfs42_falloc_res *res)
{
	return decode_op_hdr(xdr, OP_ALLOCATE);
}

static int decode_write_response(struct xdr_stream *xdr,
				 struct nfs42_write_res *res)
{
	__be32 *p;
	int status, count;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(!p))
		return -ERR(EIO);
	count = be32_to_cpup(p);
	if (count > 1)
		return -ERR(EREMOTEIO);
	else if (count == 1) {
		status = decode_opaque_fixed(xdr, &res->stateid,
				NFS4_STATEID_SIZE);
		if (unlikely(status))
			return -ERR(EIO);
	}
	p = xdr_inline_decode(xdr, 8 + 4);
	if (unlikely(!p))
		return -ERR(EIO);
	p = xdr_decode_hyper(p, &res->count);
	res->verifier.committed = be32_to_cpup(p);
	return decode_verifier(xdr, &res->verifier.verifier);
}

static int decode_nl4_server(struct xdr_stream *xdr, struct nl4_server *ns)
{
	struct nfs42_netaddr *naddr;
	uint32_t dummy;
	char *dummy_str;
	__be32 *p;
	int status;

	/* nl_type */
	p = xdr_inline_decode(xdr, 4);
	if (unlikely(!p))
		return -ERR(EIO);
	ns->nl4_type = be32_to_cpup(p);
	switch (ns->nl4_type) {
	case NL4_NAME:
	case NL4_URL:
		status = decode_opaque_inline(xdr, &dummy, &dummy_str);
		if (unlikely(status))
			return status;
		if (unlikely(dummy > NFS4_OPAQUE_LIMIT))
			return -ERR(EIO);
		memcpy(&ns->u.nl4_str, dummy_str, dummy);
		ns->u.nl4_str_sz = dummy;
		break;
	case NL4_NETADDR:
		naddr = &ns->u.nl4_addr;

		/* netid string */
		status = decode_opaque_inline(xdr, &dummy, &dummy_str);
		if (unlikely(status))
			return status;
		if (unlikely(dummy > RPCBIND_MAXNETIDLEN))
			return -ERR(EIO);
		naddr->netid_len = dummy;
		memcpy(naddr->netid, dummy_str, naddr->netid_len);

		/* uaddr string */
		status = decode_opaque_inline(xdr, &dummy, &dummy_str);
		if (unlikely(status))
			return status;
		if (unlikely(dummy > RPCBIND_MAXUADDRLEN))
			return -ERR(EIO);
		naddr->addr_len = dummy;
		memcpy(naddr->addr, dummy_str, naddr->addr_len);
		break;
	default:
		WARN_ON_ONCE(1);
		return -ERR(EIO);
	}
	return 0;
}

static int decode_copy_requirements(struct xdr_stream *xdr,
				    struct nfs42_copy_res *res) {
	__be32 *p;

	p = xdr_inline_decode(xdr, 4 + 4);
	if (unlikely(!p))
		return -ERR(EIO);

	res->consecutive = be32_to_cpup(p++);
	res->synchronous = be32_to_cpup(p++);
	return 0;
}

static int decode_copy(struct xdr_stream *xdr, struct nfs42_copy_res *res)
{
	int status;

	status = decode_op_hdr(xdr, OP_COPY);
	if (status == NFS4ERR_OFFLOAD_NO_REQS) {
		status = decode_copy_requirements(xdr, res);
		if (status)
			return status;
		return NFS4ERR_OFFLOAD_NO_REQS;
	} else if (status)
		return status;

	status = decode_write_response(xdr, &res->write_res);
	if (status)
		return status;

	return decode_copy_requirements(xdr, res);
}

static int decode_offload_cancel(struct xdr_stream *xdr,
				 struct nfs42_offload_status_res *res)
{
	return decode_op_hdr(xdr, OP_OFFLOAD_CANCEL);
}

static int decode_copy_notify(struct xdr_stream *xdr,
			      struct nfs42_copy_notify_res *res)
{
	__be32 *p;
	int status, count;

	status = decode_op_hdr(xdr, OP_COPY_NOTIFY);
	if (status)
		return status;
	/* cnr_lease_time */
	p = xdr_inline_decode(xdr, 12);
	if (unlikely(!p))
		return -ERR(EIO);
	p = xdr_decode_hyper(p, &res->cnr_lease_time.seconds);
	res->cnr_lease_time.nseconds = be32_to_cpup(p);

	status = decode_opaque_fixed(xdr, &res->cnr_stateid, NFS4_STATEID_SIZE);
	if (unlikely(status))
		return -ERR(EIO);

	/* number of source addresses */
	p = xdr_inline_decode(xdr, 4);
	if (unlikely(!p))
		return -ERR(EIO);

	count = be32_to_cpup(p);
	if (count > 1)
		pr_warn("NFS: %s: nsvr %d > Supported. Use first servers\n",
			 __func__, count);

	status = decode_nl4_server(xdr, &res->cnr_src);
	if (unlikely(status))
		return -ERR(EIO);
	return 0;
}

static int decode_deallocate(struct xdr_stream *xdr, struct nfs42_falloc_res *res)
{
	return decode_op_hdr(xdr, OP_DEALLOCATE);
}

static int decode_seek(struct xdr_stream *xdr, struct nfs42_seek_res *res)
{
	int status;
	__be32 *p;

	status = decode_op_hdr(xdr, OP_SEEK);
	if (status)
		return status;

	p = xdr_inline_decode(xdr, 4 + 8);
	if (unlikely(!p))
		return -ERR(EIO);

	res->sr_eof = be32_to_cpup(p++);
	p = xdr_decode_hyper(p, &res->sr_offset);
	return 0;
}

static int decode_layoutstats(struct xdr_stream *xdr)
{
	return decode_op_hdr(xdr, OP_LAYOUTSTATS);
}

static int decode_clone(struct xdr_stream *xdr)
{
	return decode_op_hdr(xdr, OP_CLONE);
}

static int decode_layouterror(struct xdr_stream *xdr)
{
	return decode_op_hdr(xdr, OP_LAYOUTERROR);
}

/*
 * Decode ALLOCATE request
 */
static int nfs4_xdr_dec_allocate(struct rpc_rqst *rqstp,
				 struct xdr_stream *xdr,
				 void *data)
{
	struct nfs42_falloc_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_allocate(xdr, res);
	if (status)
		goto out;
	decode_getfattr(xdr, res->falloc_fattr, res->falloc_server);
out:
	return status;
}

/*
 * Decode COPY response
 */
static int nfs4_xdr_dec_copy(struct rpc_rqst *rqstp,
			     struct xdr_stream *xdr,
			     void *data)
{
	struct nfs42_copy_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_savefh(xdr);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_copy(xdr, res);
	if (status)
		goto out;
	if (res->commit_res.verf)
		status = decode_commit(xdr, &res->commit_res);
out:
	return status;
}

/*
 * Decode OFFLOAD_CANCEL response
 */
static int nfs4_xdr_dec_offload_cancel(struct rpc_rqst *rqstp,
				       struct xdr_stream *xdr,
				       void *data)
{
	struct nfs42_offload_status_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->osr_seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_offload_cancel(xdr, res);

out:
	return status;
}

/*
 * Decode COPY_NOTIFY response
 */
static int nfs4_xdr_dec_copy_notify(struct rpc_rqst *rqstp,
				    struct xdr_stream *xdr,
				    void *data)
{
	struct nfs42_copy_notify_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->cnr_seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_copy_notify(xdr, res);

out:
	return status;
}

/*
 * Decode DEALLOCATE request
 */
static int nfs4_xdr_dec_deallocate(struct rpc_rqst *rqstp,
				   struct xdr_stream *xdr,
				   void *data)
{
	struct nfs42_falloc_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_deallocate(xdr, res);
	if (status)
		goto out;
	decode_getfattr(xdr, res->falloc_fattr, res->falloc_server);
out:
	return status;
}

/*
 * Decode SEEK request
 */
static int nfs4_xdr_dec_seek(struct rpc_rqst *rqstp,
			     struct xdr_stream *xdr,
			     void *data)
{
	struct nfs42_seek_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_seek(xdr, res);
out:
	return status;
}

/*
 * Decode LAYOUTSTATS request
 */
static int nfs4_xdr_dec_layoutstats(struct rpc_rqst *rqstp,
				    struct xdr_stream *xdr,
				    void *data)
{
	struct nfs42_layoutstat_res *res = data;
	struct compound_hdr hdr;
	int status, i;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	WARN_ON(res->num_dev > PNFS_LAYOUTSTATS_MAXDEV);
	for (i = 0; i < res->num_dev; i++) {
		status = decode_layoutstats(xdr);
		if (status)
			goto out;
	}
out:
	res->rpc_status = status;
	return status;
}

/*
 * Decode CLONE request
 */
static int nfs4_xdr_dec_clone(struct rpc_rqst *rqstp,
			      struct xdr_stream *xdr,
			      void *data)
{
	struct nfs42_clone_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_savefh(xdr);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_clone(xdr);
	if (status)
		goto out;
	status = decode_getfattr(xdr, res->dst_fattr, res->server);

out:
	res->rpc_status = status;
	return status;
}

/*
 * Decode LAYOUTERROR request
 */
static int nfs4_xdr_dec_layouterror(struct rpc_rqst *rqstp,
				    struct xdr_stream *xdr,
				    void *data)
{
	struct nfs42_layouterror_res *res = data;
	struct compound_hdr hdr;
	int status, i;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);

	for (i = 0; i < res->num_errors && status == 0; i++)
		status = decode_layouterror(xdr);
out:
	res->rpc_status = status;
	return status;
}

#endif /* __LINUX_FS_NFS_NFS4_2XDR_H */
