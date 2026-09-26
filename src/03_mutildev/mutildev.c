#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>

#define DEV_NAME "mutildev"
#define DEV_COUNT 2
#define BUFF_SIZE 128

struct char_dev 
{
    struct cdev dev;
    char vbuf[BUFF_SIZE];
} ;

static int char_dev_open(struct inode *inode, struct file *filp)
{
    filp->private_data = container_of(inode->i_cdev, struct char_dev, dev); 
    
    printk(KERN_INFO "chardev open\n");
    return 0;
}

static int char_dev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "chardev release\n");
    return 0;
}

static ssize_t char_dev_write(struct file *filp, const char __user *buf, size_t count,
                              loff_t *ppos)
{
    struct char_dev *dev = filp->private_data;
    char *vbuf = dev->vbuf;
    
    int ret = copy_from_user(vbuf, buf, count);
    if (ret == 0)
    {
        printk(KERN_INFO "write data: %s\n", vbuf);
    }
    else
    {
        printk(KERN_ERR "write failed!\n");
    }
    
    return 0;
}

static char data[] = {"chardev driver"};

static ssize_t char_dev_read(struct file *filp, char __user *buf, size_t count,
                             loff_t *ppos)
{
    struct char_dev *dev = filp->private_data;
    char *vbuf = dev->vbuf;

    memcpy(vbuf, data, sizeof(data));
    int ret = copy_to_user(buf, vbuf, count);
    if (ret != 0)
    {
        printk(KERN_ERR "read failed!\n");
    }

    return 0;
}

static dev_t devno;
static struct char_dev char_devs[DEV_COUNT];
static struct file_operations char_dev_fops = 
{
    .owner = THIS_MODULE,
    .open = char_dev_open,
    .release = char_dev_release,
    .write = char_dev_write,
    .read = char_dev_read
};
struct class *class;
struct device *devices[DEV_COUNT];

int major;
int minor;

static int __init chardev_init(void)
{
    printk(KERN_INFO "chardev init\n");

    int ret = alloc_chrdev_region(&devno, 0, DEV_COUNT, DEV_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "failed to alloc devno\n");
        goto alloc_err;
    }

    major = MAJOR(devno);
    minor = MINOR(devno);
    printk(KERN_INFO "major = %d, minor = %d\n", major, minor);

    for (int i = 0; i < DEV_COUNT; i++)
    {
        cdev_init(&char_devs[i].dev, &char_dev_fops);
        char_devs[i].dev.owner = THIS_MODULE;

        ret = cdev_add(&char_devs[i].dev, MKDEV(major, i), 1);
        if (ret < 0)
        {
            printk(KERN_ERR "failed to add cdev for device!\n");
            goto add_err;
        }
    }

    class = class_create(THIS_MODULE, DEV_NAME);
    if (IS_ERR(class))
    {
        printk(KERN_ERR "failed to create class!\n");
        goto class_err;
    }
    
    int i;
    for (i = 0; i < DEV_COUNT; i++)
    {
        devices[i] = device_create(class, NULL, MKDEV(major, i), NULL, "%s%d", DEV_NAME, i);
        if (IS_ERR(devices[i]))
        {
            printk(KERN_ERR "failed to create device%d \n", i);
            goto device_err;
        }

        printk(KERN_INFO "device%d created", i);
    } 
    
    return 0;

    device_err:
        while(i--)
        {
            device_destroy(class, MKDEV(major, i));
        }
        class_destroy(class);

    class_err:
        for(int i = 0; i < DEV_COUNT; i++)
        {
            cdev_del(&char_devs[i].dev);
        }

    add_err:
        unregister_chrdev_region(devno, DEV_COUNT);

    alloc_err:
        return ret;
}

static void __exit chardev_exit(void)
{
    for (int i = 0; i < DEV_COUNT; i++)
    {
        device_destroy(class, MKDEV(major, i));
    }
    class_destroy(class);
    for (int i = 0; i < DEV_COUNT; i++)
    {
        cdev_del(&char_devs[i].dev);
    }
    unregister_chrdev_region(devno, DEV_COUNT);

    printk(KERN_INFO "chardev exit\n");
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_DESCRIPTION("a chardev");
MODULE_LICENSE("GPL");