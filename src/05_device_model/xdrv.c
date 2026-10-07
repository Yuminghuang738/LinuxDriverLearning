#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/device.h>
#include <linux/types.h>

extern struct bus_type xbus;

int xdrv_probe(struct device *dev)
{
    printk(KERN_INFO "%s-%s\n", __FILE__, __func__);
    return 0;
}

int xdrv_remove(struct device *dev)
{
    printk(KERN_INFO "%s-%s\n", __FILE__, __func__);
    return 0;
}

static struct device_driver xdrv = 
{
    .name = "xdev",
    .bus = &xbus,
    .probe = xdrv_probe,
    .remove = xdrv_remove,
};

char *name = "xdrv";

ssize_t drvname_show(struct device_driver *drv, char *buf)
{
    return sprintf(buf, "%s\n", name);
}

DRIVER_ATTR_RO(drvname);

static __init int xdrv_init(void)
{
    int ret = driver_register(&xdrv);
    if (ret)
    {
        printk(KERN_ERR "failed to register driver!\n");
        goto register_err;
    }
    
    ret = driver_create_file(&xdrv, &driver_attr_drvname);
    if (ret)
    {
        printk(KERN_ERR "failed to create file!\n");
        goto create_err;
    }
    
    printk(KERN_INFO "xdrv init\n");
    return 0;

    create_err:
        driver_unregister(&xdrv);

    register_err:
        return ret;
}

static __exit void xdrv_exit(void)
{
    driver_remove_file(&xdrv, &driver_attr_drvname);
    driver_unregister(&xdrv);
    
    printk(KERN_INFO "xdrv exit\n");
}

module_init(xdrv_init);
module_exit(xdrv_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");