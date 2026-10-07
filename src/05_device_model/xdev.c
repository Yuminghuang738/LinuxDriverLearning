#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/device.h>
#include <linux/types.h>

extern struct bus_type xbus;

void xdev_release(struct device *dev)
{
    printk(KERN_INFO "%s-%s\n", __FILE__, __func__);
}

static struct device xdev = 
{
    .init_name = "xdev",
    .bus = &xbus,
    .release = xdev_release,
};

unsigned long id = 0;

ssize_t xdev_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%ld\n", id);
}

ssize_t xdev_id_store(struct device *dev, struct device_attribute *attr, const char *buf,
                      size_t count)
{
    int ret = kstrtoul(buf, 10, &id);
    if (ret)
    {
        printk(KERN_ERR "error while calling kstrtoul!\n");
        return ret;
    }

    return count;
}

DEVICE_ATTR(xdev_id, S_IRUSR|S_IWUSR, xdev_id_show, xdev_id_store);

static __init int xdev_init(void)
{
    int ret = device_register(&xdev);
    if (ret)
    {
        printk(KERN_ERR "failed to register device!\n");
        return ret;
    }
    device_create_file(&xdev, &dev_attr_xdev_id);
    
    printk(KERN_INFO "xdev init\n");
    return 0;
}

static __exit void xdev_exit(void)
{
    device_remove_file(&xdev, &dev_attr_xdev_id);
    device_unregister(&xdev);
    
    printk(KERN_INFO "xdev exit\n");
}

module_init(xdev_init);
module_exit(xdev_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");