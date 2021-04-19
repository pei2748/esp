// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "mriq_vivado.h"

#define DRV_NAME	"mriq_vivado"

/* <<--regs-->> */
#define MRIQ_BATCH_SIZE_X_REG 0x40


struct mriq_vivado_device {
	struct esp_device esp;
};

static struct esp_driver mriq_driver;

static struct of_device_id mriq_device_ids[] = {
	{
		.name = "SLD_MRIQ_VIVADO",
	},
	{
		.name = "eb_075",
	},
	{
		.compatible = "sld,mriq_vivado",
	},
	{ },
};

static int mriq_devs;

static inline struct mriq_vivado_device *to_mriq(struct esp_device *esp)
{
	return container_of(esp, struct mriq_vivado_device, esp);
}

static void mriq_prep_xfer(struct esp_device *esp, void *arg)
{
	struct mriq_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->num_batch_x, esp->iomem + MRIQ_BATCH_SIZE_X_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool mriq_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct mriq_vivado_device *mriq = to_mriq(esp); */
	/* struct mriq_vivado_access *a = arg; */

	return true;
}

static int mriq_probe(struct platform_device *pdev)
{
	struct mriq_vivado_device *mriq;
	struct esp_device *esp;
	int rc;

	mriq = kzalloc(sizeof(*mriq), GFP_KERNEL);
	if (mriq == NULL)
		return -ENOMEM;
	esp = &mriq->esp;
	esp->module = THIS_MODULE;
	esp->number = mriq_devs;
	esp->driver = &mriq_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	mriq_devs++;
	return 0;
 err:
	kfree(mriq);
	return rc;
}

static int __exit mriq_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct mriq_vivado_device *mriq = to_mriq(esp);

	esp_device_unregister(esp);
	kfree(mriq);
	return 0;
}

static struct esp_driver mriq_driver = {
	.plat = {
		.probe		= mriq_probe,
		.remove		= mriq_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = mriq_device_ids,
		},
	},
	.xfer_input_ok	= mriq_xfer_input_ok,
	.prep_xfer	= mriq_prep_xfer,
	.ioctl_cm	= MRIQ_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct mriq_vivado_access),
};

static int __init mriq_init(void)
{
	return esp_driver_register(&mriq_driver);
}

static void __exit mriq_exit(void)
{
	esp_driver_unregister(&mriq_driver);
}

module_init(mriq_init)
module_exit(mriq_exit)

MODULE_DEVICE_TABLE(of, mriq_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mriq_vivado driver");
