// SPDX-License-Identifier: GPL-2.0-only
/*
 * Intel SST loader on ACPI systems
 *
 * Copyright (C) 2013, Intel Corporation. All rights reserved.
 */

#include <linux/acpi.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "sst-dsp.h"
#include <sound/soc-acpi.h>
#include <sound/soc-acpi-intel-match.h>

#define SST_LPT_DSP_DMA_ADDR_OFFSET	0x0F0000
#define SST_WPT_DSP_DMA_ADDR_OFFSET	0x0FE000
#define SST_LPT_DSP_DMA_SIZE		(1024 - 1)

/* Descriptor for setting up SST platform data */
struct sst_acpi_desc {
	const char *drv_name;
	struct snd_soc_acpi_mach *machines;
	/* Platform resource indexes. Must set to -1 if not used */
	int resindex_lpe_base;
	int resindex_pcicfg_base;
	int resindex_fw_base;
	int irqindex_host_ipc;
	int resindex_dma_base;
	/* Unique number identifying the SST core on platform */
	int sst_id;
	/* DMA only valid when resindex_dma_base != -1*/
	int dma_engine;
	int dma_size;
};

struct sst_acpi_priv {
	struct platform_device *pdev_mach;
	struct platform_device *pdev_pcm;
	struct sst_pdata sst_pdata;
	struct sst_acpi_desc *desc;
	struct snd_soc_acpi_mach *mach;
};

static void sst_acpi_fw_cb(const struct firmware *fw, void *context)
{
	struct platform_device *pdev = context;
	struct device *dev = &pdev->dev;
	struct sst_acpi_priv *sst_acpi = platform_get_drvdata(pdev);
	struct sst_pdata *sst_pdata = &sst_acpi->sst_pdata;
	struct sst_acpi_desc *desc = sst_acpi->desc;
	struct snd_soc_acpi_mach *mach = sst_acpi->mach;

	sst_pdata->fw = fw;
	if (!fw) {
		dev_err(dev, "Cannot load firmware %s\n", mach->fw_filename);
		return;
	}

	/* register PCM and DAI driver */
	sst_acpi->pdev_pcm =
		platform_device_register_data(dev, desc->drv_name, -1,
					      sst_pdata, sizeof(*sst_pdata));
	if (IS_ERR(sst_acpi->pdev_pcm)) {
		dev_err(dev, "Cannot register device %s. Error %d\n",
			desc->drv_name, (int)PTR_ERR(sst_acpi->pdev_pcm));
	}

	return;
}

static int sst_acpi_probe(struct platform_device *pdev)
{
	const struct acpi_device_id *id;
	struct device *dev = &pdev->dev;
	struct sst_acpi_priv *sst_acpi;
	struct sst_pdata *sst_pdata;
	struct snd_soc_acpi_mach *mach;
	struct sst_acpi_desc *desc;
	struct resource *mmio;
	int ret = 0;

	sst_acpi = devm_kzalloc(dev, sizeof(*sst_acpi), GFP_KERNEL);
	if (sst_acpi == NULL)
		return -ENOMEM;

	id = acpi_match_device(dev->driver->acpi_match_table, dev);
	if (!id)
		return -ERR(ENODEV);

	desc = (struct sst_acpi_desc *)id->driver_data;
	mach = snd_soc_acpi_find_machine(desc->machines);
	if (mach == NULL) {
		dev_err(dev, "No matching ASoC machine driver found\n");
		return -ERR(ENODEV);
	}

	sst_pdata = &sst_acpi->sst_pdata;
	sst_pdata->id = desc->sst_id;
	sst_pdata->dma_dev = dev;
	sst_acpi->desc = desc;
	sst_acpi->mach = mach;

	sst_pdata->resindex_dma_base = desc->resindex_dma_base;
	if (desc->resindex_dma_base >= 0) {
		sst_pdata->dma_engine = desc->dma_engine;
		sst_pdata->dma_base = desc->resindex_dma_base;
		sst_pdata->dma_size = desc->dma_size;
	}

	if (desc->irqindex_host_ipc >= 0)
		sst_pdata->irq = platform_get_irq(pdev, desc->irqindex_host_ipc);

	if (desc->resindex_lpe_base >= 0) {
		mmio = platform_get_resource(pdev, IORESOURCE_MEM,
					     desc->resindex_lpe_base);
		if (mmio) {
			sst_pdata->lpe_base = mmio->start;
			sst_pdata->lpe_size = resource_size(mmio);
		}
	}

	if (desc->resindex_pcicfg_base >= 0) {
		mmio = platform_get_resource(pdev, IORESOURCE_MEM,
					     desc->resindex_pcicfg_base);
		if (mmio) {
			sst_pdata->pcicfg_base = mmio->start;
			sst_pdata->pcicfg_size = resource_size(mmio);
		}
	}

	if (desc->resindex_fw_base >= 0) {
		mmio = platform_get_resource(pdev, IORESOURCE_MEM,
					     desc->resindex_fw_base);
		if (mmio) {
			sst_pdata->fw_base = mmio->start;
			sst_pdata->fw_size = resource_size(mmio);
		}
	}

	platform_set_drvdata(pdev, sst_acpi);
	mach->pdata = sst_pdata;

	/* register machine driver */
	sst_acpi->pdev_mach =
		platform_device_register_data(dev, mach->drv_name, -1,
					      mach, sizeof(*mach));
	if (IS_ERR(sst_acpi->pdev_mach))
		return PTR_ERR(sst_acpi->pdev_mach);

	/* continue SST probing after firmware is loaded */
	ret = request_firmware_nowait(THIS_MODULE, true, mach->fw_filename,
				      dev, GFP_KERNEL, pdev, sst_acpi_fw_cb);
	if (ret)
		platform_device_unregister(sst_acpi->pdev_mach);

	return ret;
}

static int sst_acpi_remove(struct platform_device *pdev)
{
	struct sst_acpi_priv *sst_acpi = platform_get_drvdata(pdev);
	struct sst_pdata *sst_pdata = &sst_acpi->sst_pdata;

	platform_device_unregister(sst_acpi->pdev_mach);
	if (!IS_ERR_OR_NULL(sst_acpi->pdev_pcm))
		platform_device_unregister(sst_acpi->pdev_pcm);
	release_firmware(sst_pdata->fw);

	return 0;
}

static struct sst_acpi_desc sst_acpi_haswell_desc = {
	.drv_name = "haswell-pcm-audio",
	.machines = snd_soc_acpi_intel_haswell_machines,
	.resindex_lpe_base = 0,
	.resindex_pcicfg_base = 1,
	.resindex_fw_base = -1,
	.irqindex_host_ipc = 0,
	.sst_id = SST_DEV_ID_LYNX_POINT,
	.dma_engine = SST_DMA_TYPE_DW,
	.resindex_dma_base = SST_LPT_DSP_DMA_ADDR_OFFSET,
	.dma_size = SST_LPT_DSP_DMA_SIZE,
};

static struct sst_acpi_desc sst_acpi_broadwell_desc = {
	.drv_name = "haswell-pcm-audio",
	.machines = snd_soc_acpi_intel_broadwell_machines,
	.resindex_lpe_base = 0,
	.resindex_pcicfg_base = 1,
	.resindex_fw_base = -1,
	.irqindex_host_ipc = 0,
	.sst_id = SST_DEV_ID_WILDCAT_POINT,
	.dma_engine = SST_DMA_TYPE_DW,
	.resindex_dma_base = SST_WPT_DSP_DMA_ADDR_OFFSET,
	.dma_size = SST_LPT_DSP_DMA_SIZE,
};

#if !IS_ENABLED(CONFIG_SND_SST_IPC_ACPI)
static struct sst_acpi_desc sst_acpi_baytrail_desc = {
	.drv_name = "baytrail-pcm-audio",
	.machines = snd_soc_acpi_intel_baytrail_legacy_machines,
	.resindex_lpe_base = 0,
	.resindex_pcicfg_base = 1,
	.resindex_fw_base = 2,
	.irqindex_host_ipc = 5,
	.sst_id = SST_DEV_ID_BYT,
	.resindex_dma_base = -1,
};
#endif

static const struct acpi_device_id sst_acpi_match[] = {
	{ "INT33C8", (unsigned long)&sst_acpi_haswell_desc },
	{ "INT3438", (unsigned long)&sst_acpi_broadwell_desc },
#if !IS_ENABLED(CONFIG_SND_SST_IPC_ACPI)
	{ "80860F28", (unsigned long)&sst_acpi_baytrail_desc },
#endif
	{ }
};
MODULE_DEVICE_TABLE(acpi, sst_acpi_match);

static struct platform_driver sst_acpi_driver = {
	.probe = sst_acpi_probe,
	.remove = sst_acpi_remove,
	.driver = {
		.name = "sst-acpi",
		.acpi_match_table = ACPI_PTR(sst_acpi_match),
	},
};
module_platform_driver(sst_acpi_driver);

MODULE_AUTHOR("Jarkko Nikula <jarkko.nikula@linux.intel.com>");
MODULE_DESCRIPTION("Intel SST loader on ACPI systems");
MODULE_LICENSE("GPL v2");
