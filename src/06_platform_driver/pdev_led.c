#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>

#define GPIO0_IOC_BASE (0xFF950000)
#define GPIO0_IOC_GPIO0C_IOMUX_SEL_0 (GPIO0_IOC_BASE + 0x0010)

#define GPIO0_BASE (0xFF940000)
#define GPIO0_DR_H (GPIO0_BASE + 0x0004)
#define GPIO0_DDR_H (GPIO0_BASE + 0x000C)

static struct resource pdev_led_resource[] = 
{
    [0] = DEFINE_RES_MEM(GPIO0_IOC_GPIO0C_IOMUX_SEL_0, 4),
    [1] = DEFINE_RES_MEM(GPIO0_DR_H, 4),
    [2] = DEFINE_RES_MEM(GPIO0_DDR_H, 4)
};

unsigned int pdev_led_hwinfo[1] = { 0 };

static void pdev_led_release(struct device *dev)
{
    printk(KERN_INFO "pdev release\n");
}

static struct platform_device pdev_led = 
{
    .name = "pdev_led",
    .id = 0,
    .num_resources = ARRAY_SIZE(pdev_led_resource),
    .resource = pdev_led_resource,
    .dev = 
    {
        .release = pdev_led_release,
        .platform_data = pdev_led_hwinfo,
    },
};

static __init int pdev_led_init(void)
{
    platform_device_register(&pdev_led);
    
    printk(KERN_INFO "pdev init\n");
    return 0;
}

static __exit void pdev_led_exit(void)
{
    platform_device_unregister(&pdev_led);

    printk(KERN_INFO "pdev exit\n");
}

module_init(pdev_led_init);
module_exit(pdev_led_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");