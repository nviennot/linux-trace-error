// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2008-2009 Patrick McHardy <kaber@trash.net>
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables_core.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables_offload.h>

struct nft_bitwise {
	enum nft_registers	sreg:8;
	enum nft_registers	dreg:8;
	enum nft_bitwise_ops	op:8;
	u8			len;
	struct nft_data		mask;
	struct nft_data		xor;
	struct nft_data		data;
};

static void nft_bitwise_eval_bool(u32 *dst, const u32 *src,
				  const struct nft_bitwise *priv)
{
	unsigned int i;

	for (i = 0; i < DIV_ROUND_UP(priv->len, 4); i++)
		dst[i] = (src[i] & priv->mask.data[i]) ^ priv->xor.data[i];
}

static void nft_bitwise_eval_lshift(u32 *dst, const u32 *src,
				    const struct nft_bitwise *priv)
{
	u32 shift = priv->data.data[0];
	unsigned int i;
	u32 carry = 0;

	for (i = DIV_ROUND_UP(priv->len, sizeof(u32)); i > 0; i--) {
		dst[i - 1] = (src[i - 1] << shift) | carry;
		carry = src[i - 1] >> (BITS_PER_TYPE(u32) - shift);
	}
}

static void nft_bitwise_eval_rshift(u32 *dst, const u32 *src,
				    const struct nft_bitwise *priv)
{
	u32 shift = priv->data.data[0];
	unsigned int i;
	u32 carry = 0;

	for (i = 0; i < DIV_ROUND_UP(priv->len, sizeof(u32)); i++) {
		dst[i] = carry | (src[i] >> shift);
		carry = src[i] << (BITS_PER_TYPE(u32) - shift);
	}
}

void nft_bitwise_eval(const struct nft_expr *expr,
		      struct nft_regs *regs, const struct nft_pktinfo *pkt)
{
	const struct nft_bitwise *priv = nft_expr_priv(expr);
	const u32 *src = &regs->data[priv->sreg];
	u32 *dst = &regs->data[priv->dreg];

	switch (priv->op) {
	case NFT_BITWISE_BOOL:
		nft_bitwise_eval_bool(dst, src, priv);
		break;
	case NFT_BITWISE_LSHIFT:
		nft_bitwise_eval_lshift(dst, src, priv);
		break;
	case NFT_BITWISE_RSHIFT:
		nft_bitwise_eval_rshift(dst, src, priv);
		break;
	}
}

static const struct nla_policy nft_bitwise_policy[NFTA_BITWISE_MAX + 1] = {
	[NFTA_BITWISE_SREG]	= { .type = NLA_U32 },
	[NFTA_BITWISE_DREG]	= { .type = NLA_U32 },
	[NFTA_BITWISE_LEN]	= { .type = NLA_U32 },
	[NFTA_BITWISE_MASK]	= { .type = NLA_NESTED },
	[NFTA_BITWISE_XOR]	= { .type = NLA_NESTED },
	[NFTA_BITWISE_OP]	= { .type = NLA_U32 },
	[NFTA_BITWISE_DATA]	= { .type = NLA_NESTED },
};

static int nft_bitwise_init_bool(struct nft_bitwise *priv,
				 const struct nlattr *const tb[])
{
	struct nft_data_desc mask, xor;
	int err;

	if (tb[NFTA_BITWISE_DATA])
		return -ERR(EINVAL);

	if (!tb[NFTA_BITWISE_MASK] ||
	    !tb[NFTA_BITWISE_XOR])
		return -ERR(EINVAL);

	err = nft_data_init(NULL, &priv->mask, sizeof(priv->mask), &mask,
			    tb[NFTA_BITWISE_MASK]);
	if (err < 0)
		return err;
	if (mask.type != NFT_DATA_VALUE || mask.len != priv->len) {
		err = -ERR(EINVAL);
		goto err1;
	}

	err = nft_data_init(NULL, &priv->xor, sizeof(priv->xor), &xor,
			    tb[NFTA_BITWISE_XOR]);
	if (err < 0)
		goto err1;
	if (xor.type != NFT_DATA_VALUE || xor.len != priv->len) {
		err = -ERR(EINVAL);
		goto err2;
	}

	return 0;
err2:
	nft_data_release(&priv->xor, xor.type);
err1:
	nft_data_release(&priv->mask, mask.type);
	return err;
}

static int nft_bitwise_init_shift(struct nft_bitwise *priv,
				  const struct nlattr *const tb[])
{
	struct nft_data_desc d;
	int err;

	if (tb[NFTA_BITWISE_MASK] ||
	    tb[NFTA_BITWISE_XOR])
		return -ERR(EINVAL);

	if (!tb[NFTA_BITWISE_DATA])
		return -ERR(EINVAL);

	err = nft_data_init(NULL, &priv->data, sizeof(priv->data), &d,
			    tb[NFTA_BITWISE_DATA]);
	if (err < 0)
		return err;
	if (d.type != NFT_DATA_VALUE || d.len != sizeof(u32) ||
	    priv->data.data[0] >= BITS_PER_TYPE(u32)) {
		nft_data_release(&priv->data, d.type);
		return -ERR(EINVAL);
	}

	return 0;
}

static int nft_bitwise_init(const struct nft_ctx *ctx,
			    const struct nft_expr *expr,
			    const struct nlattr * const tb[])
{
	struct nft_bitwise *priv = nft_expr_priv(expr);
	u32 len;
	int err;

	if (!tb[NFTA_BITWISE_SREG] ||
	    !tb[NFTA_BITWISE_DREG] ||
	    !tb[NFTA_BITWISE_LEN])
		return -ERR(EINVAL);

	err = nft_parse_u32_check(tb[NFTA_BITWISE_LEN], U8_MAX, &len);
	if (err < 0)
		return err;

	priv->len = len;

	priv->sreg = nft_parse_register(tb[NFTA_BITWISE_SREG]);
	err = nft_validate_register_load(priv->sreg, priv->len);
	if (err < 0)
		return err;

	priv->dreg = nft_parse_register(tb[NFTA_BITWISE_DREG]);
	err = nft_validate_register_store(ctx, priv->dreg, NULL,
					  NFT_DATA_VALUE, priv->len);
	if (err < 0)
		return err;

	if (tb[NFTA_BITWISE_OP]) {
		priv->op = ntohl(nla_get_be32(tb[NFTA_BITWISE_OP]));
		switch (priv->op) {
		case NFT_BITWISE_BOOL:
		case NFT_BITWISE_LSHIFT:
		case NFT_BITWISE_RSHIFT:
			break;
		default:
			return -ERR(EOPNOTSUPP);
		}
	} else {
		priv->op = NFT_BITWISE_BOOL;
	}

	switch(priv->op) {
	case NFT_BITWISE_BOOL:
		err = nft_bitwise_init_bool(priv, tb);
		break;
	case NFT_BITWISE_LSHIFT:
	case NFT_BITWISE_RSHIFT:
		err = nft_bitwise_init_shift(priv, tb);
		break;
	}

	return err;
}

static int nft_bitwise_dump_bool(struct sk_buff *skb,
				 const struct nft_bitwise *priv)
{
	if (nft_data_dump(skb, NFTA_BITWISE_MASK, &priv->mask,
			  NFT_DATA_VALUE, priv->len) < 0)
		return -1;

	if (nft_data_dump(skb, NFTA_BITWISE_XOR, &priv->xor,
			  NFT_DATA_VALUE, priv->len) < 0)
		return -1;

	return 0;
}

static int nft_bitwise_dump_shift(struct sk_buff *skb,
				  const struct nft_bitwise *priv)
{
	if (nft_data_dump(skb, NFTA_BITWISE_DATA, &priv->data,
			  NFT_DATA_VALUE, sizeof(u32)) < 0)
		return -1;
	return 0;
}

static int nft_bitwise_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_bitwise *priv = nft_expr_priv(expr);
	int err = 0;

	if (nft_dump_register(skb, NFTA_BITWISE_SREG, priv->sreg))
		return -1;
	if (nft_dump_register(skb, NFTA_BITWISE_DREG, priv->dreg))
		return -1;
	if (nla_put_be32(skb, NFTA_BITWISE_LEN, htonl(priv->len)))
		return -1;
	if (nla_put_be32(skb, NFTA_BITWISE_OP, htonl(priv->op)))
		return -1;

	switch (priv->op) {
	case NFT_BITWISE_BOOL:
		err = nft_bitwise_dump_bool(skb, priv);
		break;
	case NFT_BITWISE_LSHIFT:
	case NFT_BITWISE_RSHIFT:
		err = nft_bitwise_dump_shift(skb, priv);
		break;
	}

	return err;
}

static struct nft_data zero;

static int nft_bitwise_offload(struct nft_offload_ctx *ctx,
			       struct nft_flow_rule *flow,
			       const struct nft_expr *expr)
{
	const struct nft_bitwise *priv = nft_expr_priv(expr);
	struct nft_offload_reg *reg = &ctx->regs[priv->dreg];

	if (priv->op != NFT_BITWISE_BOOL)
		return -ERR(EOPNOTSUPP);

	if (memcmp(&priv->xor, &zero, sizeof(priv->xor)) ||
	    priv->sreg != priv->dreg || priv->len != reg->len)
		return -ERR(EOPNOTSUPP);

	memcpy(&reg->mask, &priv->mask, sizeof(priv->mask));

	return 0;
}

static const struct nft_expr_ops nft_bitwise_ops = {
	.type		= &nft_bitwise_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_bitwise)),
	.eval		= nft_bitwise_eval,
	.init		= nft_bitwise_init,
	.dump		= nft_bitwise_dump,
	.offload	= nft_bitwise_offload,
};

struct nft_expr_type nft_bitwise_type __read_mostly = {
	.name		= "bitwise",
	.ops		= &nft_bitwise_ops,
	.policy		= nft_bitwise_policy,
	.maxattr	= NFTA_BITWISE_MAX,
	.owner		= THIS_MODULE,
};
