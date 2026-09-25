#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

int itype = 0;
module_param(itype,int,0644);
MODULE_PARM_DESC(itype,"This is int variable");

static bool btype = 0;
module_param(btype,bool,0644);
MODULE_PARM_DESC(btype,"This is bool variable");

static char ctype = 0;
module_param(ctype,byte,0644);
MODULE_PARM_DESC(ctype,"This is byte variable");


static char *stype = 0;
module_param(stype,charp,0644);
MODULE_PARM_DESC(stype,"This is charp variable");

static int iarr[3] = {0, 1, 2};
module_param_array(iarr, int, NULL, 0644);
MODULE_PARM_DESC(iarr,"This is array of int");


static int __init parameter_init(void)
{
    pr_info(KERN_INFO "parameter init!\n");
    pr_info(KERN_INFO "itype=%d\n",itype);
    pr_info(KERN_INFO "btype=%d\n",btype);
    pr_info(KERN_INFO "ctype=%d\n",ctype);
    pr_info(KERN_INFO "stype=%s\n",stype);
    pr_info("*iarr* parameter: %d, %d, %d\n", iarr[0], iarr[1], iarr[2]);
    
    return 0;
}

static void __exit parameter_exit(void)
{
    printk(KERN_INFO "parameter exit!\n");
}

EXPORT_SYMBOL(itype);

int my_add(int a, int b)
{
   return a+b;
}

EXPORT_SYMBOL(my_add);

int my_sub(int a, int b)
{
   return a-b;
}

EXPORT_SYMBOL(my_sub);

module_init(parameter_init);
module_exit(parameter_exit);

MODULE_AUTHOR("Yuminghuang738 <Yuminghuang738@gmail.com>");
MODULE_DESCRIPTION("parameter module");
MODULE_LICENSE("GPL");