#include <linux/stddef.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/types.h>
#include <linux/device.h>

int xbus_match(struct device *dev, struct device_driver *drv)
{
    printk("%s-%s\n",__FILE__, __func__);
    if(!strncmp(dev_name(dev), drv->name, strlen(drv->name)))
    {
        printk(KERN_INFO "dev & drv match\n");
        return 1;
    }
    
    return 0;
}

struct bus_type xbus = 
{
    .name = "xbus",
    .match = xbus_match,
};

EXPORT_SYMBOL(xbus);

static char *bus_name = "xbus";

ssize_t xbus_test_show(struct bus_type *bus, char *buf)
{
    return sprintf(buf, "%s\n", bus_name);
}

BUS_ATTR_RO(xbus_test);

static __init int xbus_init(void)
{
    int ret = bus_register(&xbus);
    if (ret)
    {
        printk(KERN_ERR "failed to register bus!\n");
        goto register_err;
    }

    ret = bus_create_file(&xbus, &bus_attr_xbus_test);
    if (ret)
    {
        printk(KERN_ERR "failed to create bus file!\n");   
        goto create_err;
    }
    
    printk(KERN_INFO "xbus init\n");
    return 0;

    create_err:
        bus_unregister(&xbus);

    register_err:
        return ret;
}

static __exit void xbus_exit(void)
{
    bus_remove_file(&xbus, &bus_attr_xbus_test);
    bus_unregister(&xbus);
    
    printk("xbus exit\n");
}

module_init(xbus_init);
module_exit(xbus_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");