#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>

#define DEV_NAME "mutildev"
#define DEV_COUNT 2
#define BUFF_SIZE 128

struct char_dev 
{
    struct cdev dev;
    char vbuf[BUFF_SIZE];
} ;

static char data[] = {"chardev driver"};
static dev_t devno;
static struct char_dev char_devs[DEV_COUNT];
struct class *class;
struct device *devices[DEV_COUNT];

int major;
int minor;

static int char_dev_open(struct inode *inode, struct file *filp)
{
    filp->private_data = container_of(inode->i_cdev, struct char_dev, dev); 
    
    printk(KERN_INFO "chardev open\n");
}
