#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

extern int itype;

int my_add(int a, int b);
int my_sub(int a, int b);

static int __init calculation_init(void)
{
   pr_info(KERN_INFO "calculation init!\n");
   pr_info(KERN_INFO "itype+1 = %d, itype-1 = %d\n", my_add(itype,1), my_sub(itype,1));
   return 0;
}

static void __exit calculation_exit(void)
{
   pr_info(KERN_INFO "calculation exit!\n");
}

module_init(calculation_init);
module_exit(calculation_exit);

MODULE_AUTHOR("Yuminghuang738 <Yuminghuang738@gmail.com>");
MODULE_DESCRIPTION("parameter module");
MODULE_LICENSE("GPL");