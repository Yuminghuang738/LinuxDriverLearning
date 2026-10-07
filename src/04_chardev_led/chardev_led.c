#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/io.h>

#define GPIO0_BASE (0xFF940000)
#define GPIO0_DR_H (GPIO0_BASE + 0x0004)
#define GPIO0_DDR_H (GPIO0_BASE + 0x000C)

#define GPIO0_IOC_BASE (0xFF950000)
#define GPIO0_IOC_GPIO0C_IOMUX_SEL_0 (GPIO0_IOC_BASE + 0x0010)

#define DEV_NAME "chardev_led"
#define DEV_COUNT 1

struct led_chardev 
{
    struct cdev dev;
    unsigned int __iomem *va_dr;
    unsigned int __iomem *va_ddr;
    unsigned int __iomem *va_iomux;
    unsigned int led_pin;
};

static int chardev_led_open(struct inode *inode, struct file *filp)
{
    struct led_chardev *led_cdev = container_of(inode->i_cdev, struct led_chardev, dev);
    filp->private_data = led_cdev;

    printk(KERN_INFO "chardev_led open\n");
    return 0;
}

static int chardev_led_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "chardev_led release\n");
    return 0;
}

static ssize_t chardev_led_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "chardev_led write\n");
    
    char ret = 0;
    get_user(ret, buf);
    
    struct led_chardev *led_cdev = filp->private_data;
    unsigned long val = ioread32(led_cdev->va_dr);
    if (ret == '0') 
    {
        val |= ((unsigned int)0x1 << (led_cdev->led_pin + 16));
        val &= ~((unsigned int)0x01 << (led_cdev->led_pin));
    } 
    else 
    {
        val |= ((unsigned int)0x1 << (led_cdev->led_pin + 16));
        val |= ((unsigned int)0x01 << (led_cdev->led_pin));
    }
    
    iowrite32(val, led_cdev->va_dr);
    return count;
}

static dev_t devno;

struct class *class;
struct device *device;
static struct file_operations chardev_led_fops = 
{
    .owner = THIS_MODULE,
    .open = chardev_led_open,
    .release = chardev_led_release,
    .write = chardev_led_write,
};
static struct led_chardev led_cdev[DEV_COUNT] = 
{
    {.led_pin = 0}
};

static int __init chardev_init(void)
{
    printk(KERN_INFO "chardev init\n");

    int ret = 0;
    led_cdev[0].va_dr = ioremap(GPIO0_DR_H, 4);
    led_cdev[0].va_ddr = ioremap(GPIO0_DDR_H, 4);
    led_cdev[0].va_iomux = ioremap(GPIO0_IOC_GPIO0C_IOMUX_SEL_0, 4);
    if (!led_cdev[0].va_dr || !led_cdev[0].va_ddr || !led_cdev[0].va_iomux)
    {
        printk(KERN_ERR "failed to ioremap GPIO registers\n");
        ret = -ENOMEM;
        goto ioremap_err;
    }

    iowrite32((0xF << 16) | (0x0 << 0), led_cdev[0].va_iomux);

    unsigned int val = 0;
    val = ioread32(led_cdev[0].va_ddr);
    val |= ((unsigned int)0x1 << (led_cdev[0].led_pin + 16));
    val |= ((unsigned int)0x1 << (led_cdev[0].led_pin));
    iowrite32(val, led_cdev[0].va_ddr);

    val = ioread32(led_cdev[0].va_dr);
    val |= ((unsigned int)0x1 << (led_cdev[0].led_pin + 16));
    val |= ((unsigned int)0x1 << (led_cdev[0].led_pin));
    iowrite32(val, led_cdev[0].va_dr);

    ret = alloc_chrdev_region(&devno, 0, DEV_COUNT, DEV_NAME);
    if (ret < 0) 
    {
        printk(KERN_ERR "fail to alloc devno\n");
        goto ioremap_err;
    }

    int major = MAJOR(devno);
    int minor = MINOR(devno);
    printk(KERN_INFO "major=%d, minor=%d\n", major, minor);

    cdev_init(&led_cdev[0].dev, &chardev_led_fops);
    led_cdev[0].dev.owner = THIS_MODULE;

    ret = cdev_add(&led_cdev[0].dev, devno, DEV_COUNT);
    if (ret < 0) 
    {
        printk(KERN_ERR "fail to add cdev\n");
        goto add_err;
    }

    class = class_create(THIS_MODULE, DEV_NAME);
    if (IS_ERR(class)) 
    {
        printk(KERN_ERR "fail to create class\n");
        ret = PTR_ERR(class);
        goto class_err;
    }

    device = device_create(class, NULL, devno, NULL, DEV_NAME);
    if (IS_ERR(device)) 
    {
        printk(KERN_ERR "fail to create device\n");
        ret = PTR_ERR(device);
        goto device_err;
    }

    return 0;

    device_err:
        class_destroy(class);

    class_err:
        cdev_del(&led_cdev[0].dev);

    add_err:
        unregister_chrdev_region(devno, DEV_COUNT);

    ioremap_err:
        return ret;
}

static void __exit chardev_exit(void)
{
    device_destroy(class, devno);
    class_destroy(class);
    cdev_del(&led_cdev[0].dev);
    unregister_chrdev_region(devno, DEV_COUNT);
    iounmap(led_cdev[0].va_iomux);
    iounmap(led_cdev[0].va_ddr);
    iounmap(led_cdev[0].va_dr);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");