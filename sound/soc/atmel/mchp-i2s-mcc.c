// SPDX-License-Identifier: GPL-2.0
//
// Driver for Microchip I2S Multi-channel controller
//
// Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
//
// Author: Codrin Ciubotariu <codrin.ciubotariu@microchip.com>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/mfd/syscon.h>
#include <linux/lcm.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/dmaengine_pcm.h>

/*
 * ---- I2S Controller Register map ----
 */
#define MCHP_I2SMCC_CR		0x0000	/* Control Register */
#define MCHP_I2SMCC_MRA		0x0004	/* Mode Register A */
#define MCHP_I2SMCC_MRB		0x0008	/* Mode Register B */
#define MCHP_I2SMCC_SR		0x000C	/* Status Register */
#define MCHP_I2SMCC_IERA	0x0010	/* Interrupt Enable Register A */
#define MCHP_I2SMCC_IDRA	0x0014	/* Interrupt Disable Register A */
#define MCHP_I2SMCC_IMRA	0x0018	/* Interrupt Mask Register A */
#define MCHP_I2SMCC_ISRA	0X001C	/* Interrupt Status Register A */

#define MCHP_I2SMCC_IERB	0x0020	/* Interrupt Enable Register B */
#define MCHP_I2SMCC_IDRB	0x0024	/* Interrupt Disable Register B */
#define MCHP_I2SMCC_IMRB	0x0028	/* Interrupt Mask Register B */
#define MCHP_I2SMCC_ISRB	0X002C	/* Interrupt Status Register B */

#define MCHP_I2SMCC_RHR		0x0030	/* Receiver Holding Register */
#define MCHP_I2SMCC_THR		0x0034	/* Transmitter Holding Register */

#define MCHP_I2SMCC_RHL0R	0x0040	/* Receiver Holding Left 0 Register */
#define MCHP_I2SMCC_RHR0R	0x0044	/* Receiver Holding Right 0 Register */

#define MCHP_I2SMCC_RHL1R	0x0048	/* Receiver Holding Left 1 Register */
#define MCHP_I2SMCC_RHR1R	0x004C	/* Receiver Holding Right 1 Register */

#define MCHP_I2SMCC_RHL2R	0x0050	/* Receiver Holding Left 2 Register */
#define MCHP_I2SMCC_RHR2R	0x0054	/* Receiver Holding Right 2 Register */

#define MCHP_I2SMCC_RHL3R	0x0058	/* Receiver Holding Left 3 Register */
#define MCHP_I2SMCC_RHR3R	0x005C	/* Receiver Holding Right 3 Register */

#define MCHP_I2SMCC_THL0R	0x0060	/* Transmitter Holding Left 0 Register */
#define MCHP_I2SMCC_THR0R	0x0064	/* Transmitter Holding Right 0 Register */

#define MCHP_I2SMCC_THL1R	0x0068	/* Transmitter Holding Left 1 Register */
#define MCHP_I2SMCC_THR1R	0x006C	/* Transmitter Holding Right 1 Register */

#define MCHP_I2SMCC_THL2R	0x0070	/* Transmitter Holding Left 2 Register */
#define MCHP_I2SMCC_THR2R	0x0074	/* Transmitter Holding Right 2 Register */

#define MCHP_I2SMCC_THL3R	0x0078	/* Transmitter Holding Left 3 Register */
#define MCHP_I2SMCC_THR3R	0x007C	/* Transmitter Holding Right 3 Register */

#define MCHP_I2SMCC_VERSION	0x00FC	/* Version Register */

/*
 * ---- Control Register (Write-only) ----
 */
#define MCHP_I2SMCC_CR_RXEN		BIT(0)	/* Receiver Enable */
#define MCHP_I2SMCC_CR_RXDIS		BIT(1)	/* Receiver Disable */
#define MCHP_I2SMCC_CR_CKEN		BIT(2)	/* Clock Enable */
#define MCHP_I2SMCC_CR_CKDIS		BIT(3)	/* Clock Disable */
#define MCHP_I2SMCC_CR_TXEN		BIT(4)	/* Transmitter Enable */
#define MCHP_I2SMCC_CR_TXDIS		BIT(5)	/* Transmitter Disable */
#define MCHP_I2SMCC_CR_SWRST		BIT(7)	/* Software Reset */

/*
 * ---- Mode Register A (Read/Write) ----
 */
#define MCHP_I2SMCC_MRA_MODE_MASK		GENMASK(0, 0)
#define MCHP_I2SMCC_MRA_MODE_SLAVE		(0 << 0)
#define MCHP_I2SMCC_MRA_MODE_MASTER		(1 << 0)

#define MCHP_I2SMCC_MRA_DATALENGTH_MASK			GENMASK(3, 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_32_BITS		(0 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_24_BITS		(1 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_20_BITS		(2 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_18_BITS		(3 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_16_BITS		(4 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_16_BITS_COMPACT	(5 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_8_BITS		(6 << 1)
#define MCHP_I2SMCC_MRA_DATALENGTH_8_BITS_COMPACT	(7 << 1)

#define MCHP_I2SMCC_MRA_WIRECFG_MASK		GENMASK(5, 4)
#define MCHP_I2SMCC_MRA_WIRECFG_I2S_1_TDM_0	(0 << 4)
#define MCHP_I2SMCC_MRA_WIRECFG_I2S_2_TDM_1	(1 << 4)
#define MCHP_I2SMCC_MRA_WIRECFG_I2S_4_TDM_2	(2 << 4)
#define MCHP_I2SMCC_MRA_WIRECFG_TDM_3		(3 << 4)

#define MCHP_I2SMCC_MRA_FORMAT_MASK		GENMASK(7, 6)
#define MCHP_I2SMCC_MRA_FORMAT_I2S		(0 << 6)
#define MCHP_I2SMCC_MRA_FORMAT_LJ		(1 << 6) /* Left Justified */
#define MCHP_I2SMCC_MRA_FORMAT_TDM		(2 << 6)
#define MCHP_I2SMCC_MRA_FORMAT_TDMLJ		(3 << 6)

/* Transmitter uses one DMA channel ... */
/* Left audio samples duplicated to right audio channel */
#define MCHP_I2SMCC_MRA_RXMONO			BIT(8)

/* I2SDO output of I2SC is internally connected to I2SDI input */
#define MCHP_I2SMCC_MRA_RXLOOP			BIT(9)

/* Receiver uses one DMA channel ... */
/* Left audio samples duplicated to right audio channel */
#define MCHP_I2SMCC_MRA_TXMONO			BIT(10)

/* x sample transmitted when underrun */
#define MCHP_I2SMCC_MRA_TXSAME_ZERO		(0 << 11) /* Zero sample */
#define MCHP_I2SMCC_MRA_TXSAME_PREVIOUS		(1 << 11) /* Previous sample */

/* select between peripheral clock and generated clock */
#define MCHP_I2SMCC_MRA_SRCCLK_PCLK		(0 << 12)
#define MCHP_I2SMCC_MRA_SRCCLK_GCLK		(1 << 12)

/* Number of TDM Channels - 1 */
#define MCHP_I2SMCC_MRA_NBCHAN_MASK		GENMASK(15, 13)
#define MCHP_I2SMCC_MRA_NBCHAN(ch) \
	((((ch) - 1) << 13) & MCHP_I2SMCC_MRA_NBCHAN_MASK)

/* Selected Clock to I2SMCC Master Clock ratio */
#define MCHP_I2SMCC_MRA_IMCKDIV_MASK		GENMASK(21, 16)
#define MCHP_I2SMCC_MRA_IMCKDIV(div) \
	(((div) << 16) & MCHP_I2SMCC_MRA_IMCKDIV_MASK)

/* TDM Frame Synchronization */
#define MCHP_I2SMCC_MRA_TDMFS_MASK		GENMASK(23, 22)
#define MCHP_I2SMCC_MRA_TDMFS_SLOT		(0 << 22)
#define MCHP_I2SMCC_MRA_TDMFS_HALF		(1 << 22)
#define MCHP_I2SMCC_MRA_TDMFS_BIT		(2 << 22)

/* Selected Clock to I2SMC Serial Clock ratio */
#define MCHP_I2SMCC_MRA_ISCKDIV_MASK		GENMASK(29, 24)
#define MCHP_I2SMCC_MRA_ISCKDIV(div) \
	(((div) << 24) & MCHP_I2SMCC_MRA_ISCKDIV_MASK)

/* Master Clock mode */
#define MCHP_I2SMCC_MRA_IMCKMODE_MASK		GENMASK(30, 30)
/* 0: No master clock generated*/
#define MCHP_I2SMCC_MRA_IMCKMODE_NONE		(0 << 30)
/* 1: master clock generated (internally generated clock drives I2SMCK pin) */
#define MCHP_I2SMCC_MRA_IMCKMODE_GEN		(1 << 30)

/* Slot Width */
/* 0: slot is 32 bits wide for DATALENGTH = 18/20/24 bits. */
/* 1: slot is 24 bits wide for DATALENGTH = 18/20/24 bits. */
#define MCHP_I2SMCC_MRA_IWS			BIT(31)

/*
 * ---- Mode Register B (Read/Write) ----
 */
/* all enabled I2S left channels are filled first, then I2S right channels */
#define MCHP_I2SMCC_MRB_CRAMODE_LEFT_FIRST	(0 << 0)
/*
 * an enabled I2S left channel is filled, then the corresponding right
 * channel, until all channels are filled
 */
#define MCHP_I2SMCC_MRB_CRAMODE_REGULAR		(1 << 0)

#define MCHP_I2SMCC_MRB_FIFOEN			BIT(1)

#define MCHP_I2SMCC_MRB_DMACHUNK_MASK		GENMASK(9, 8)
#define MCHP_I2SMCC_MRB_DMACHUNK(no_words) \
	(((fls(no_words) - 1) << 8) & MCHP_I2SMCC_MRB_DMACHUNK_MASK)

#define MCHP_I2SMCC_MRB_CLKSEL_MASK		GENMASK(16, 16)
#define MCHP_I2SMCC_MRB_CLKSEL_EXT		(0 << 16)
#define MCHP_I2SMCC_MRB_CLKSEL_INT		(1 << 16)

/*
 * ---- Status Registers (Read-only) ----
 */
#define MCHP_I2SMCC_SR_RXEN		BIT(0)	/* Receiver Enabled */
#define MCHP_I2SMCC_SR_TXEN		BIT(4)	/* Transmitter Enabled */

/*
 * ---- Interrupt Enable/Disable/Mask/Status Registers A ----
 */
#define MCHP_I2SMCC_INT_TXRDY_MASK(ch)		GENMASK((ch) - 1, 0)
#define MCHP_I2SMCC_INT_TXRDYCH(ch)		BIT(ch)
#define MCHP_I2SMCC_INT_TXUNF_MASK(ch)		GENMASK((ch) + 7, 8)
#define MCHP_I2SMCC_INT_TXUNFCH(ch)		BIT((ch) + 8)
#define MCHP_I2SMCC_INT_RXRDY_MASK(ch)		GENMASK((ch) + 15, 16)
#define MCHP_I2SMCC_INT_RXRDYCH(ch)		BIT((ch) + 16)
#define MCHP_I2SMCC_INT_RXOVF_MASK(ch)		GENMASK((ch) + 23, 24)
#define MCHP_I2SMCC_INT_RXOVFCH(ch)		BIT((ch) + 24)

/*
 * ---- Interrupt Enable/Disable/Mask/Status Registers B ----
 */
#define MCHP_I2SMCC_INT_WERR			BIT(0)
#define MCHP_I2SMCC_INT_TXFFRDY			BIT(8)
#define MCHP_I2SMCC_INT_TXFFEMP			BIT(9)
#define MCHP_I2SMCC_INT_RXFFRDY			BIT(12)
#define MCHP_I2SMCC_INT_RXFFFUL			BIT(13)

/*
 * ---- Version Register (Read-only) ----
 */
#define MCHP_I2SMCC_VERSION_MASK		GENMASK(11, 0)

#define MCHP_I2SMCC_MAX_CHANNELS		8
#define MCHP_I2MCC_TDM_SLOT_WIDTH		32

static const struct regmap_config mchp_i2s_mcc_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = MCHP_I2SMCC_VERSION,
};

struct mchp_i2s_mcc_dev {
	struct wait_queue_head			wq_txrdy;
	struct wait_queue_head			wq_rxrdy;
	struct device				*dev;
	struct regmap				*regmap;
	struct clk				*pclk;
	struct clk				*gclk;
	struct snd_dmaengine_dai_dma_data	playback;
	struct snd_dmaengine_dai_dma_data	capture;
	unsigned int				fmt;
	unsigned int				sysclk;
	unsigned int				frame_length;
	int					tdm_slots;
	int					channels;
	unsigned int				gclk_use:1;
	unsigned int				gclk_running:1;
	unsigned int				tx_rdy:1;
	unsigned int				rx_rdy:1;
};

static irqreturn_t mchp_i2s_mcc_interrupt(int irq, void *dev_id)
{
	struct mchp_i2s_mcc_dev *dev = dev_id;
	u32 sra, imra, srb, imrb, pendinga, pendingb, idra = 0;
	irqreturn_t ret = IRQ_NONE;

	regmap_read(dev->regmap, MCHP_I2SMCC_IMRA, &imra);
	regmap_read(dev->regmap, MCHP_I2SMCC_ISRA, &sra);
	pendinga = imra & sra;

	regmap_read(dev->regmap, MCHP_I2SMCC_IMRB, &imrb);
	regmap_read(dev->regmap, MCHP_I2SMCC_ISRB, &srb);
	pendingb = imrb & srb;

	if (!pendinga && !pendingb)
		return IRQ_NONE;

	/*
	 * Tx/Rx ready interrupts are enabled when stopping only, to assure
	 * availability and to disable clocks if necessary
	 */
	idra |= pendinga & (MCHP_I2SMCC_INT_TXRDY_MASK(dev->channels) |
			    MCHP_I2SMCC_INT_RXRDY_MASK(dev->channels));
	if (idra)
		ret = IRQ_HANDLED;

	if ((imra & MCHP_I2SMCC_INT_TXRDY_MASK(dev->channels)) &&
	    (imra & MCHP_I2SMCC_INT_TXRDY_MASK(dev->channels)) ==
	    (idra & MCHP_I2SMCC_INT_TXRDY_MASK(dev->channels))) {
		dev->tx_rdy = 1;
		wake_up_interruptible(&dev->wq_txrdy);
	}
	if ((imra & MCHP_I2SMCC_INT_RXRDY_MASK(dev->channels)) &&
	    (imra & MCHP_I2SMCC_INT_RXRDY_MASK(dev->channels)) ==
	    (idra & MCHP_I2SMCC_INT_RXRDY_MASK(dev->channels))) {
		dev->rx_rdy = 1;
		wake_up_interruptible(&dev->wq_rxrdy);
	}
	regmap_write(dev->regmap, MCHP_I2SMCC_IDRA, idra);

	return ret;
}

static int mchp_i2s_mcc_set_sysclk(struct snd_soc_dai *dai,
				   int clk_id, unsigned int freq, int dir)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);

	dev_dbg(dev->dev, "%s() clk_id=%d freq=%u dir=%d\n",
		__func__, clk_id, freq, dir);

	/* We do not need SYSCLK */
	if (dir == SND_SOC_CLOCK_IN)
		return 0;

	dev->sysclk = freq;

	return 0;
}

static int mchp_i2s_mcc_set_bclk_ratio(struct snd_soc_dai *dai,
				       unsigned int ratio)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);

	dev_dbg(dev->dev, "%s() ratio=%u\n", __func__, ratio);

	dev->frame_length = ratio;

	return 0;
}

static int mchp_i2s_mcc_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);

	dev_dbg(dev->dev, "%s() fmt=%#x\n", __func__, fmt);

	/* We don't support any kind of clock inversion */
	if ((fmt & SND_SOC_DAIFMT_INV_MASK) != SND_SOC_DAIFMT_NB_NF)
		return -ERR(EINVAL);

	/* We can't generate only FSYNC */
	if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) == SND_SOC_DAIFMT_CBM_CFS)
		return -ERR(EINVAL);

	/* We can only reconfigure the IP when it's stopped */
	if (fmt & SND_SOC_DAIFMT_CONT)
		return -ERR(EINVAL);

	dev->fmt = fmt;

	return 0;
}

static int mchp_i2s_mcc_set_dai_tdm_slot(struct snd_soc_dai *dai,
					 unsigned int tx_mask,
					 unsigned int rx_mask,
					 int slots, int slot_width)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);

	dev_dbg(dev->dev,
		"%s() tx_mask=0x%08x rx_mask=0x%08x slots=%d width=%d\n",
		__func__, tx_mask, rx_mask, slots, slot_width);

	if (slots < 0 || slots > MCHP_I2SMCC_MAX_CHANNELS ||
	    slot_width != MCHP_I2MCC_TDM_SLOT_WIDTH)
		return -ERR(EINVAL);

	if (slots) {
		/* We do not support daisy chain */
		if (rx_mask != GENMASK(slots - 1, 0) ||
		    rx_mask != tx_mask)
			return -ERR(EINVAL);
	}

	dev->tdm_slots = slots;
	dev->frame_length = slots * MCHP_I2MCC_TDM_SLOT_WIDTH;

	return 0;
}

static int mchp_i2s_mcc_clk_get_rate_diff(struct clk *clk,
					  unsigned long rate,
					  struct clk **best_clk,
					  unsigned long *best_rate,
					  unsigned long *best_diff_rate)
{
	long round_rate;
	unsigned int diff_rate;

	round_rate = clk_round_rate(clk, rate);
	if (round_rate < 0)
		return (int)round_rate;

	diff_rate = abs(rate - round_rate);
	if (diff_rate < *best_diff_rate) {
		*best_clk = clk;
		*best_diff_rate = diff_rate;
		*best_rate = rate;
	}

	return 0;
}

static int mchp_i2s_mcc_config_divs(struct mchp_i2s_mcc_dev *dev,
				    unsigned int bclk, unsigned int *mra,
				    unsigned long *best_rate)
{
	unsigned long clk_rate;
	unsigned long lcm_rate;
	unsigned long best_diff_rate = ~0;
	unsigned int sysclk;
	struct clk *best_clk = NULL;
	int ret;

	/* For code simplification */
	if (!dev->sysclk)
		sysclk = bclk;
	else
		sysclk = dev->sysclk;

	/*
	 * MCLK is Selected CLK / (2 * IMCKDIV),
	 * BCLK is Selected CLK / (2 * ISCKDIV);
	 * if IMCKDIV or ISCKDIV are 0, MCLK or BCLK = Selected CLK
	 */
	lcm_rate = lcm(sysclk, bclk);
	if ((lcm_rate / sysclk % 2 == 1 && lcm_rate / sysclk > 2) ||
	    (lcm_rate / bclk % 2 == 1 && lcm_rate / bclk > 2))
		lcm_rate *= 2;

	for (clk_rate = lcm_rate;
	     (clk_rate == sysclk || clk_rate / (sysclk * 2) <= GENMASK(5, 0)) &&
	     (clk_rate == bclk || clk_rate / (bclk * 2) <= GENMASK(5, 0));
	     clk_rate += lcm_rate) {
		ret = mchp_i2s_mcc_clk_get_rate_diff(dev->gclk, clk_rate,
						     &best_clk, best_rate,
						     &best_diff_rate);
		if (ret) {
			dev_err(dev->dev, "gclk error for rate %lu: %d",
				clk_rate, ret);
		} else {
			if (!best_diff_rate) {
				dev_dbg(dev->dev, "found perfect rate on gclk: %lu\n",
					clk_rate);
				break;
			}
		}

		ret = mchp_i2s_mcc_clk_get_rate_diff(dev->pclk, clk_rate,
						     &best_clk, best_rate,
						     &best_diff_rate);
		if (ret) {
			dev_err(dev->dev, "pclk error for rate %lu: %d",
				clk_rate, ret);
		} else {
			if (!best_diff_rate) {
				dev_dbg(dev->dev, "found perfect rate on pclk: %lu\n",
					clk_rate);
				break;
			}
		}
	}

	/* check if clocks returned only errors */
	if (!best_clk) {
		dev_err(dev->dev, "unable to change rate to clocks\n");
		return -ERR(EINVAL);
	}

	dev_dbg(dev->dev, "source CLK is %s with rate %lu, diff %lu\n",
		best_clk == dev->pclk ? "pclk" : "gclk",
		*best_rate, best_diff_rate);

	/* Configure divisors */
	if (dev->sysclk)
		*mra |= MCHP_I2SMCC_MRA_IMCKDIV(*best_rate / (2 * sysclk));
	*mra |= MCHP_I2SMCC_MRA_ISCKDIV(*best_rate / (2 * bclk));

	if (best_clk == dev->gclk)
		*mra |= MCHP_I2SMCC_MRA_SRCCLK_GCLK;
	else
		*mra |= MCHP_I2SMCC_MRA_SRCCLK_PCLK;

	return 0;
}

static int mchp_i2s_mcc_is_running(struct mchp_i2s_mcc_dev *dev)
{
	u32 sr;

	regmap_read(dev->regmap, MCHP_I2SMCC_SR, &sr);
	return !!(sr & (MCHP_I2SMCC_SR_TXEN | MCHP_I2SMCC_SR_RXEN));
}

static int mchp_i2s_mcc_hw_params(struct snd_pcm_substream *substream,
				  struct snd_pcm_hw_params *params,
				  struct snd_soc_dai *dai)
{
	unsigned long rate = 0;
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);
	u32 mra = 0;
	u32 mrb = 0;
	unsigned int channels = params_channels(params);
	unsigned int frame_length = dev->frame_length;
	unsigned int bclk_rate;
	int set_divs = 0;
	int ret;
	bool is_playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);

	dev_dbg(dev->dev, "%s() rate=%u format=%#x width=%u channels=%u\n",
		__func__, params_rate(params), params_format(params),
		params_width(params), params_channels(params));

	switch (dev->fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		if (dev->tdm_slots) {
			dev_err(dev->dev, "I2S with TDM is not supported\n");
			return -ERR(EINVAL);
		}
		mra |= MCHP_I2SMCC_MRA_FORMAT_I2S;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		if (dev->tdm_slots) {
			dev_err(dev->dev, "Left-Justified with TDM is not supported\n");
			return -ERR(EINVAL);
		}
		mra |= MCHP_I2SMCC_MRA_FORMAT_LJ;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		mra |= MCHP_I2SMCC_MRA_FORMAT_TDM;
		break;
	default:
		dev_err(dev->dev, "unsupported bus format\n");
		return -ERR(EINVAL);
	}

	switch (dev->fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		/* cpu is BCLK and LRC master */
		mra |= MCHP_I2SMCC_MRA_MODE_MASTER;
		if (dev->sysclk)
			mra |= MCHP_I2SMCC_MRA_IMCKMODE_GEN;
		set_divs = 1;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
		/* cpu is BCLK master */
		mrb |= MCHP_I2SMCC_MRB_CLKSEL_INT;
		set_divs = 1;
		/* fall through */
	case SND_SOC_DAIFMT_CBM_CFM:
		/* cpu is slave */
		mra |= MCHP_I2SMCC_MRA_MODE_SLAVE;
		if (dev->sysclk)
			dev_warn(dev->dev, "Unable to generate MCLK in Slave mode\n");
		break;
	default:
		dev_err(dev->dev, "unsupported master/slave mode\n");
		return -ERR(EINVAL);
	}

	if (dev->fmt & (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_LEFT_J)) {
		switch (channels) {
		case 1:
			if (is_playback)
				mra |= MCHP_I2SMCC_MRA_TXMONO;
			else
				mra |= MCHP_I2SMCC_MRA_RXMONO;
			break;
		case 2:
			break;
		default:
			dev_err(dev->dev, "unsupported number of audio channels\n");
			return -ERR(EINVAL);
		}

		if (!frame_length)
			frame_length = 2 * params_physical_width(params);
	} else if (dev->fmt & SND_SOC_DAIFMT_DSP_A) {
		if (dev->tdm_slots) {
			if (channels % 2 && channels * 2 <= dev->tdm_slots) {
				/*
				 * Duplicate data for even-numbered channels
				 * to odd-numbered channels
				 */
				if (is_playback)
					mra |= MCHP_I2SMCC_MRA_TXMONO;
				else
					mra |= MCHP_I2SMCC_MRA_RXMONO;
			}
			channels = dev->tdm_slots;
		}

		mra |= MCHP_I2SMCC_MRA_NBCHAN(channels);
		if (!frame_length)
			frame_length = channels * MCHP_I2MCC_TDM_SLOT_WIDTH;
	}

	/*
	 * We must have the same burst size configured
	 * in the DMA transfer and in out IP
	 */
	mrb |= MCHP_I2SMCC_MRB_DMACHUNK(channels);
	if (is_playback)
		dev->playback.maxburst = 1 << (fls(channels) - 1);
	else
		dev->capture.maxburst = 1 << (fls(channels) - 1);

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_8_BITS;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_16_BITS;
		break;
	case SNDRV_PCM_FORMAT_S18_3LE:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_18_BITS |
		       MCHP_I2SMCC_MRA_IWS;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_20_BITS |
		       MCHP_I2SMCC_MRA_IWS;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_24_BITS |
		       MCHP_I2SMCC_MRA_IWS;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_24_BITS;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		mra |= MCHP_I2SMCC_MRA_DATALENGTH_32_BITS;
		break;
	default:
		dev_err(dev->dev, "unsupported size/endianness for audio samples\n");
		return -ERR(EINVAL);
	}

	if (set_divs) {
		bclk_rate = frame_length * params_rate(params);
		ret = mchp_i2s_mcc_config_divs(dev, bclk_rate, &mra,
					       &rate);
		if (ret) {
			dev_err(dev->dev,
				"unable to configure the divisors: %d\n", ret);
			return ret;
		}
	}

	/*
	 * If we are already running, the wanted setup must be
	 * the same with the one that's currently ongoing
	 */
	if (mchp_i2s_mcc_is_running(dev)) {
		u32 mra_cur;
		u32 mrb_cur;

		regmap_read(dev->regmap, MCHP_I2SMCC_MRA, &mra_cur);
		regmap_read(dev->regmap, MCHP_I2SMCC_MRB, &mrb_cur);
		if (mra != mra_cur || mrb != mrb_cur)
			return -ERR(EINVAL);

		return 0;
	}

	if (mra & MCHP_I2SMCC_MRA_SRCCLK_GCLK && !dev->gclk_use) {
		/* set the rate */
		ret = clk_set_rate(dev->gclk, rate);
		if (ret) {
			dev_err(dev->dev,
				"unable to set rate %lu to GCLK: %d\n",
				rate, ret);
			return ret;
		}

		ret = clk_prepare(dev->gclk);
		if (ret < 0) {
			dev_err(dev->dev, "unable to prepare GCLK: %d\n", ret);
			return ret;
		}
		dev->gclk_use = 1;
	}

	/* Save the number of channels to know what interrupts to enable */
	dev->channels = channels;

	ret = regmap_write(dev->regmap, MCHP_I2SMCC_MRA, mra);
	if (ret < 0) {
		if (dev->gclk_use) {
			clk_unprepare(dev->gclk);
			dev->gclk_use = 0;
		}
		return ret;
	}
	return regmap_write(dev->regmap, MCHP_I2SMCC_MRB, mrb);
}

static int mchp_i2s_mcc_hw_free(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);
	bool is_playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	long err;

	if (is_playback) {
		err = wait_event_interruptible_timeout(dev->wq_txrdy,
						       dev->tx_rdy,
						       msecs_to_jiffies(500));
		if (err == 0) {
			dev_warn_once(dev->dev,
				      "Timeout waiting for Tx ready\n");
			regmap_write(dev->regmap, MCHP_I2SMCC_IDRA,
				     MCHP_I2SMCC_INT_TXRDY_MASK(dev->channels));
			dev->tx_rdy = 1;
		}
	} else {
		err = wait_event_interruptible_timeout(dev->wq_rxrdy,
						       dev->rx_rdy,
						       msecs_to_jiffies(500));
		if (err == 0) {
			dev_warn_once(dev->dev,
				      "Timeout waiting for Rx ready\n");
			regmap_write(dev->regmap, MCHP_I2SMCC_IDRA,
				     MCHP_I2SMCC_INT_RXRDY_MASK(dev->channels));
			dev->rx_rdy = 1;
		}
	}

	if (!mchp_i2s_mcc_is_running(dev)) {
		regmap_write(dev->regmap, MCHP_I2SMCC_CR, MCHP_I2SMCC_CR_CKDIS);

		if (dev->gclk_running) {
			clk_disable(dev->gclk);
			dev->gclk_running = 0;
		}
		if (dev->gclk_use) {
			clk_unprepare(dev->gclk);
			dev->gclk_use = 0;
		}
	}

	return 0;
}

static int mchp_i2s_mcc_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);
	bool is_playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	u32 cr = 0;
	u32 iera = 0;
	u32 sr;
	int err;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (is_playback)
			cr = MCHP_I2SMCC_CR_TXEN | MCHP_I2SMCC_CR_CKEN;
		else
			cr = MCHP_I2SMCC_CR_RXEN | MCHP_I2SMCC_CR_CKEN;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		regmap_read(dev->regmap, MCHP_I2SMCC_SR, &sr);
		if (is_playback && (sr & MCHP_I2SMCC_SR_TXEN)) {
			cr = MCHP_I2SMCC_CR_TXDIS;
			dev->tx_rdy = 0;
			/*
			 * Enable Tx Ready interrupts on all channels
			 * to assure all data is sent
			 */
			iera = MCHP_I2SMCC_INT_TXRDY_MASK(dev->channels);
		} else if (!is_playback && (sr & MCHP_I2SMCC_SR_RXEN)) {
			cr = MCHP_I2SMCC_CR_RXDIS;
			dev->rx_rdy = 0;
			/*
			 * Enable Rx Ready interrupts on all channels
			 * to assure all data is received
			 */
			iera = MCHP_I2SMCC_INT_RXRDY_MASK(dev->channels);
		}
		break;
	default:
		return -ERR(EINVAL);
	}

	if ((cr & MCHP_I2SMCC_CR_CKEN) && dev->gclk_use &&
	    !dev->gclk_running) {
		err = clk_enable(dev->gclk);
		if (err) {
			dev_err_once(dev->dev, "failed to enable GCLK: %d\n",
				     err);
		} else {
			dev->gclk_running = 1;
		}
	}

	regmap_write(dev->regmap, MCHP_I2SMCC_IERA, iera);
	regmap_write(dev->regmap, MCHP_I2SMCC_CR, cr);

	return 0;
}

static int mchp_i2s_mcc_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);

	/* Software reset the IP if it's not running */
	if (!mchp_i2s_mcc_is_running(dev)) {
		return regmap_write(dev->regmap, MCHP_I2SMCC_CR,
				    MCHP_I2SMCC_CR_SWRST);
	}

	return 0;
}

static const struct snd_soc_dai_ops mchp_i2s_mcc_dai_ops = {
	.set_sysclk	= mchp_i2s_mcc_set_sysclk,
	.set_bclk_ratio = mchp_i2s_mcc_set_bclk_ratio,
	.startup	= mchp_i2s_mcc_startup,
	.trigger	= mchp_i2s_mcc_trigger,
	.hw_params	= mchp_i2s_mcc_hw_params,
	.hw_free	= mchp_i2s_mcc_hw_free,
	.set_fmt	= mchp_i2s_mcc_set_dai_fmt,
	.set_tdm_slot	= mchp_i2s_mcc_set_dai_tdm_slot,
};

static int mchp_i2s_mcc_dai_probe(struct snd_soc_dai *dai)
{
	struct mchp_i2s_mcc_dev *dev = snd_soc_dai_get_drvdata(dai);

	init_waitqueue_head(&dev->wq_txrdy);
	init_waitqueue_head(&dev->wq_rxrdy);
	dev->tx_rdy = 1;
	dev->rx_rdy = 1;

	snd_soc_dai_init_dma_data(dai, &dev->playback, &dev->capture);

	return 0;
}

#define MCHP_I2SMCC_RATES              SNDRV_PCM_RATE_8000_192000

#define MCHP_I2SMCC_FORMATS	(SNDRV_PCM_FMTBIT_S8 |          \
				 SNDRV_PCM_FMTBIT_S16_LE |      \
				 SNDRV_PCM_FMTBIT_S18_3LE |     \
				 SNDRV_PCM_FMTBIT_S20_3LE |     \
				 SNDRV_PCM_FMTBIT_S24_3LE |     \
				 SNDRV_PCM_FMTBIT_S24_LE |      \
				 SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver mchp_i2s_mcc_dai = {
	.probe	= mchp_i2s_mcc_dai_probe,
	.playback = {
		.stream_name = "I2SMCC-Playback",
		.channels_min = 1,
		.channels_max = 8,
		.rates = MCHP_I2SMCC_RATES,
		.formats = MCHP_I2SMCC_FORMATS,
	},
	.capture = {
		.stream_name = "I2SMCC-Capture",
		.channels_min = 1,
		.channels_max = 8,
		.rates = MCHP_I2SMCC_RATES,
		.formats = MCHP_I2SMCC_FORMATS,
	},
	.ops = &mchp_i2s_mcc_dai_ops,
	.symmetric_rates = 1,
	.symmetric_samplebits = 1,
	.symmetric_channels = 1,
};

static const struct snd_soc_component_driver mchp_i2s_mcc_component = {
	.name	= "mchp-i2s-mcc",
};

#ifdef CONFIG_OF
static const struct of_device_id mchp_i2s_mcc_dt_ids[] = {
	{
		.compatible = "microchip,sam9x60-i2smcc",
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mchp_i2s_mcc_dt_ids);
#endif

static int mchp_i2s_mcc_probe(struct platform_device *pdev)
{
	struct mchp_i2s_mcc_dev *dev;
	struct resource *mem;
	struct regmap *regmap;
	void __iomem *base;
	u32 version;
	int irq;
	int err;

	dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(base))
		return PTR_ERR(base);

	regmap = devm_regmap_init_mmio(&pdev->dev, base,
				       &mchp_i2s_mcc_regmap_config);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	err = devm_request_irq(&pdev->dev, irq, mchp_i2s_mcc_interrupt, 0,
			       dev_name(&pdev->dev), dev);
	if (err)
		return err;

	dev->pclk = devm_clk_get(&pdev->dev, "pclk");
	if (IS_ERR(dev->pclk)) {
		err = PTR_ERR(dev->pclk);
		dev_err(&pdev->dev,
			"failed to get the peripheral clock: %d\n", err);
		return err;
	}

	/* Get the optional generated clock */
	dev->gclk = devm_clk_get(&pdev->dev, "gclk");
	if (IS_ERR(dev->gclk)) {
		if (PTR_ERR(dev->gclk) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		dev_warn(&pdev->dev,
			 "generated clock not found: %d\n", err);
		dev->gclk = NULL;
	}

	dev->dev = &pdev->dev;
	dev->regmap = regmap;
	platform_set_drvdata(pdev, dev);

	err = clk_prepare_enable(dev->pclk);
	if (err) {
		dev_err(&pdev->dev,
			"failed to enable the peripheral clock: %d\n", err);
		return err;
	}

	err = devm_snd_soc_register_component(&pdev->dev,
					      &mchp_i2s_mcc_component,
					      &mchp_i2s_mcc_dai, 1);
	if (err) {
		dev_err(&pdev->dev, "failed to register DAI: %d\n", err);
		clk_disable_unprepare(dev->pclk);
		return err;
	}

	dev->playback.addr	= (dma_addr_t)mem->start + MCHP_I2SMCC_THR;
	dev->capture.addr	= (dma_addr_t)mem->start + MCHP_I2SMCC_RHR;

	err = devm_snd_dmaengine_pcm_register(&pdev->dev, NULL, 0);
	if (err) {
		dev_err(&pdev->dev, "failed to register PCM: %d\n", err);
		clk_disable_unprepare(dev->pclk);
		return err;
	}

	/* Get IP version. */
	regmap_read(dev->regmap, MCHP_I2SMCC_VERSION, &version);
	dev_info(&pdev->dev, "hw version: %#lx\n",
		 version & MCHP_I2SMCC_VERSION_MASK);

	return 0;
}

static int mchp_i2s_mcc_remove(struct platform_device *pdev)
{
	struct mchp_i2s_mcc_dev *dev = platform_get_drvdata(pdev);

	clk_disable_unprepare(dev->pclk);

	return 0;
}

static struct platform_driver mchp_i2s_mcc_driver = {
	.driver		= {
		.name	= "mchp_i2s_mcc",
		.of_match_table	= of_match_ptr(mchp_i2s_mcc_dt_ids),
	},
	.probe		= mchp_i2s_mcc_probe,
	.remove		= mchp_i2s_mcc_remove,
};
module_platform_driver(mchp_i2s_mcc_driver);

MODULE_DESCRIPTION("Microchip I2S Multi-Channel Controller driver");
MODULE_AUTHOR("Codrin Ciubotariu <codrin.ciubotariu@microchip.com>");
MODULE_LICENSE("GPL v2");
