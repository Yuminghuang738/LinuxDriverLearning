#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEV_NAME "chardev"
#define DEV_COUNT 1
#define BUFF_SIZE 128

static char vbuf[BUFF_SIZE];

static int char_dev_open(struct inode *inode, struct file *filp)
{
    filp->private_data = vbuf;
    
    printk(KERN_INFO "chardev open\n");
    return 0;
}

static int char_dev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "chardev release\n");
    return 0;
}

static ssize_t char_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    char *vbuf = filp->private_data;

    int ret = 0;
    ret = copy_from_user(vbuf, buf, count);
    if (ret == 0)
    {
        printk(KERN_INFO "write data: %s\n", vbuf);
    }
    else 
    {
        printk(KERN_ERR "write failed\n");
    }

    return 0;
}

static char data[] = {"chardev driver"};

static ssize_t char_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    char *vbuf = filp->private_data;
    memcpy(vbuf, data, sizeof(data));

    int ret = 0;
    ret = copy_to_user(buf, vbuf, count);
    if (ret != 0)
    {
        printk(KERN_ERR "read failed\n");
    }

    return 0;
}

static dev_t devno;
static struct cdev char_dev;
static struct file_operations char_dev_fops = 
{
    .owner = THIS_MODULE,
    .open = char_dev_open,
    .release = char_dev_release,
    .write = char_dev_write,
    .read = char_dev_read
};
struct class *class;
struct device *device;

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

    char_dev.owner = THIS_MODULE;
    cdev_init(&char_dev, &char_dev_fops);

    ret = cdev_add(&char_dev, devno, DEV_COUNT);
    if (ret < 0)
    {
        printk(KERN_ERR "failed to add cdev\n");
        goto add_err;
    }

    class = class_create(THIS_MODULE, DEV_NAME);
    if (IS_ERR(class))
    {
        printk(KERN_ERR "failed to add class\n");
        goto class_err;
    }

    device = device_create(class, NULL, MKDEV(major, 0), NULL, DEV_NAME);
    if (IS_ERR(device))
    {
        printk(KERN_ERR "failed to create device\n");
        goto device_err;
    }

    printk(KERN_INFO "device created\n");
    return 0;

    alloc_err   :
        return ret;

    add_err:
        unregister_chrdev_region(devno, DEV_COUNT);
        return 0;

    class_err:
        cdev_del(&char_dev);
        return 0;

    device_err:
        class_destroy(class);
        device_destroy(class, MKDEV(major, 0));
        return 0;
}

static void __exit chardev_exit(void)
{
    device_destroy(class, MKDEV(major, 0));
    class_destroy(class);
    cdev_del(&char_dev);
    unregister_chrdev_region(devno, DEV_COUNT);
    
    printk(KERN_INFO "chardev exit\n");
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_AUTHOR("Yuminghaung728 <Yuminghuang738@gmail.com>");
MODULE_DESCRIPTION("a chardev");
MODULE_LICENSE("GPL");