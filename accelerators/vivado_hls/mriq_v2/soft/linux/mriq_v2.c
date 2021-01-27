#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "mriq_v2.h"

#define DRV_NAME	"mriq_v2"

/* <<--regs-->> */
#define MRIQ_V2_NUMX_REG 0x44
#define MRIQ_V2_NUMK_REG 0x40

struct mriq_v2_device {
	struct esp_device esp;
};

static struct esp_driver mriq_v2_driver;

static struct of_device_id mriq_v2_device_ids[] = {
	{
		.name = "SLD_MRIQ_V2",
	},
	{
		.name = "eb_04B",
	},
	{
		.compatible = "sld,mriq_v2",
	},
	{ },
};

static int mriq_v2_devs;

static inline struct mriq_v2_device *to_mriq_v2(struct esp_device *esp)
{
	return container_of(esp, struct mriq_v2_device, esp);
}

static void mriq_v2_prep_xfer(struct esp_device *esp, void *arg)
{
	struct mriq_v2_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->numX, esp->iomem + MRIQ_V2_NUMX_REG);
	iowrite32be(a->numK, esp->iomem + MRIQ_V2_NUMK_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool mriq_v2_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct mriq_v2_device *mriq_v2 = to_mriq_v2(esp); */
	/* struct mriq_v2_access *a = arg; */

	return true;
}

static int mriq_v2_probe(struct platform_device *pdev)
{
	struct mriq_v2_device *mriq_v2;
	struct esp_device *esp;
	int rc;

	mriq_v2 = kzalloc(sizeof(*mriq_v2), GFP_KERNEL);
	if (mriq_v2 == NULL)
		return -ENOMEM;
	esp = &mriq_v2->esp;
	esp->module = THIS_MODULE;
	esp->number = mriq_v2_devs;
	esp->driver = &mriq_v2_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	mriq_v2_devs++;
	return 0;
 err:
	kfree(mriq_v2);
	return rc;
}

static int __exit mriq_v2_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct mriq_v2_device *mriq_v2 = to_mriq_v2(esp);

	esp_device_unregister(esp);
	kfree(mriq_v2);
	return 0;
}

static struct esp_driver mriq_v2_driver = {
	.plat = {
		.probe		= mriq_v2_probe,
		.remove		= mriq_v2_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = mriq_v2_device_ids,
		},
	},
	.xfer_input_ok	= mriq_v2_xfer_input_ok,
	.prep_xfer	= mriq_v2_prep_xfer,
	.ioctl_cm	= MRIQ_V2_IOC_ACCESS,
	.arg_size	= sizeof(struct mriq_v2_access),
};

static int __init mriq_v2_init(void)
{
	return esp_driver_register(&mriq_v2_driver);
}

static void __exit mriq_v2_exit(void)
{
	esp_driver_unregister(&mriq_v2_driver);
}

module_init(mriq_v2_init)
module_exit(mriq_v2_exit)

MODULE_DEVICE_TABLE(of, mriq_v2_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mriq_v2 driver");
