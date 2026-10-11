#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/io.h>

static struct platform_device_id pdev_led_ids[] = 
{
    {.name = "pdev_led"},
    {}
};

MODULE_DEVICE_TABLE(platform, pdev_led_ids);

#define DEV_NAME "pdrv_led"
#define DEV_CNT 1

static dev_t devno;
static struct class *class;
static struct device *device;

struct led_chrdev 
{
    struct cdev dev;
    unsigned int __iomem *va_iomux;
    unsigned int __iomem *va_dr;
    unsigned int __iomem *va_ddr;
    unsigned int led_pin;
};

static struct led_chrdev *led_cdev;

static int pdrv_led_open(struct inode *inode, struct file *filp)
{
    struct led_chrdev *led_cdev = container_of(inode->i_cdev, struct led_chrdev, dev);
    filp->private_data = led_cdev;

    printk(KERN_INFO "pdrv_led open\n");
    return 0;
}

static int pdrv_led_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "pdrv_led release\n");
    return 0;
}

static ssize_t pdrv_led_write(struct file *filp, const char __user * buf, size_t count, 
                              loff_t * ppos)
{
    printk(KERN_INFO "pdrv_led write\n");
    
    unsigned long ret = 0;
    get_user(ret, buf);
    
    struct led_chrdev *led_cdev = filp->private_data;
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

static ssize_t pdrv_led_read(struct file *filp, char __user *buf, size_t count,
                             loff_t *ppos)
{
    return 0;
}

static struct file_operations pdrv_led_fops = 
{
    .owner = THIS_MODULE,
    .open = pdrv_led_open,
    .release = pdrv_led_release,
    .write = pdrv_led_write,
    .read = pdrv_led_read,
};

static int pdrv_led_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "led platform driver probe\n");

    led_cdev = devm_kzalloc(&pdev->dev, sizeof(struct led_chrdev), GFP_KERNEL);
    if (!led_cdev)
    {
        return -ENOMEM;
    }

    unsigned int *pdev_led_hwinfo = dev_get_platdata(&pdev->dev);
    if (!pdev_led_hwinfo) 
    {
        return -ENOMEM;
    }
    
    led_cdev->led_pin = pdev_led_hwinfo[0];

    struct resource *mem_iomux = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    struct resource *mem_dr = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    struct resource *mem_ddr = platform_get_resource(pdev, IORESOURCE_MEM, 2);

    led_cdev->va_iomux = devm_ioremap(&pdev->dev, mem_iomux->start, resource_size(mem_iomux));
    led_cdev->va_dr = devm_ioremap(&pdev->dev, mem_dr->start, resource_size(mem_dr));
    led_cdev->va_ddr = devm_ioremap(&pdev->dev, mem_ddr->start, resource_size(mem_ddr));
    int ret = 0;
    if (!led_cdev->va_iomux || !led_cdev->va_dr || !led_cdev->va_ddr) 
    {
        ret = -ENOMEM;
        
        printk(KERN_ERR "fail to ioremap GPIO registers\n");
        goto ioremap_err;
    }

    unsigned int val = ioread32(led_cdev->va_iomux);
    val |= ((unsigned int)0xF << (led_cdev->led_pin * 4 + 16));
    val &= ~((unsigned int)0xF << (led_cdev->led_pin * 4));
    iowrite32(val, led_cdev->va_iomux);

    val = ioread32(led_cdev->va_ddr);
    val |= ((unsigned int)0x1 << (led_cdev->led_pin + 16));
    val |= ((unsigned int)0x1 << (led_cdev->led_pin));
    iowrite32(val, led_cdev->va_ddr);

    val = ioread32(led_cdev->va_dr);
    val |= ((unsigned int)0x1 << (led_cdev->led_pin + 16));
    val |= ((unsigned int)0x1 << (led_cdev->led_pin));
    iowrite32(val, led_cdev->va_dr);

    ret = alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    if (ret < 0) 
    {
        printk(KERN_ERR "fail to alloc devno\n");
        goto ioremap_err;
    }

    int major = MAJOR(devno);
    int minor = MINOR(devno);
    printk(KERN_INFO "major=%d, minor=%d\n", major, minor);

    cdev_init(&led_cdev->dev, &pdrv_led_fops);
    led_cdev->dev.owner = THIS_MODULE;

    ret = cdev_add(&led_cdev->dev, devno, DEV_CNT);
    if (ret < 0) 
    {
        printk(KERN_ERR "fail to add cdev\n");
        goto add_err;
    }

    class = class_create(THIS_MODULE, DEV_NAME);
    if (IS_ERR(class)) 
    {
        ret = PTR_ERR(class);
        
        printk(KERN_ERR"fail to create class\n");
        goto class_err;
    }

    device = device_create(class, NULL, devno, NULL, DEV_NAME);
    if (IS_ERR(device)) 
    {
        ret = PTR_ERR(device);

        printk("fail to create device\n");
        goto device_err;
    }

    platform_set_drvdata(pdev, led_cdev);

    return 0;

    device_err:
        class_destroy(class);

    class_err:
        cdev_del(&led_cdev->dev);

    add_err:
        unregister_chrdev_region(devno, DEV_CNT);

    ioremap_err:
        return ret;
}

static int pdrv_led_remove(struct platform_device *pdev)
{
    device_destroy(class, devno);
    
    class_destroy(class);
    
    struct led_chrdev *led_cdev = platform_get_drvdata(pdev);
    cdev_del(&led_cdev->dev);
    
    unregister_chrdev_region(devno, DEV_CNT);
    
    printk(KERN_INFO "pdrv_led remove\n");
    return 0;
}

static struct platform_driver pdrv_led = 
{
    .probe = pdrv_led_probe,
    .remove = pdrv_led_remove,
    .driver.name = "pdev_led",
    .id_table = pdev_led_ids,
};

static __init int pdrv_led_init(void)
{
    platform_driver_register(&pdrv_led);
 
    printk(KERN_INFO "led platform driver init\n");
    return 0;
}

static __exit void pdrv_led_exit(void)
{
    platform_driver_unregister(&pdrv_led);
    
    printk(KERN_INFO "led platform driver exit\n");
}

module_init(pdrv_led_init);
module_exit(pdrv_led_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");