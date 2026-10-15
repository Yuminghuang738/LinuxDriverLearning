#include <linux/init.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/gpio.h>

struct device_node  *led_test_device_node;
struct device_node  *led_device_node;

struct property *led_property;

int size = 0;

unsigned int out_values[18];

static int get_dts_info_probe(struct platform_device *pdev)
{
    int error_status = -1;

    printk(KERN_INFO "%s\n",__func__);

    led_test_device_node = of_find_node_by_path("/get_dts_info_test");
    if(led_test_device_node == NULL)
    {
        printk(KERN_ERR "get led_device_node failed ! \n");
        return -1;
    }

    printk(KERN_INFO "name: %s",led_test_device_node->name);
    printk(KERN_INFO "child name: %s",led_test_device_node->child->name);

    led_device_node = of_get_next_child(led_test_device_node,NULL);
    if(led_device_node == NULL)
    {
        printk(KERN_ERR "\n get led_device_node failed ! \n");
        return -1;
    }

    printk(KERN_INFO "name: %s",led_device_node->name);
    printk(KERN_INFO "parent name: %s",led_device_node->parent->name);

    led_property = of_find_property(led_device_node,"compatible",&size);
    if(led_property == NULL)
    {
        printk(KERN_ERR "get led_property failed ! \n");
        return -1;
    }

    printk(KERN_INFO "size = : %d",size);
    printk(KERN_INFO "name: %s",led_property->name);
    printk(KERN_INFO "length: %d",led_property->length);
    printk(KERN_INFO "value : %s",(char*)led_property->value);

    error_status = of_property_read_u32_array(led_device_node,"reg",out_values, 2);
    if(error_status != 0)
    {
        printk(KERN_ERR "get out_values failed ! \n");
        return -1;
    }

    printk(KERN_INFO "0x%08X ", out_values[0]);
    printk(KERN_INFO "0x%08X ", out_values[1]);

    return 0;
}

static int get_dts_info_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "get_dts_info remove\n");
    return 0;
} 

static const struct of_device_id of_gpio_leds_match[] = 
{
    {.compatible = "get_dts_info_test"},
    {}
};

struct platform_driver get_dts_info_driver = 
{
    .probe = get_dts_info_probe,
    .remove = get_dts_info_remove,
    .driver = 
    {
        .name = "get_dts_info_test",
        .of_match_table = of_match_ptr(of_gpio_leds_match),
    }
};

module_platform_driver(get_dts_info_driver);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_LICENSE("GPL");