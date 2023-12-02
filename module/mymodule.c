#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

int init_module(void)
{
    pr_info("init my module\n");
    return 0;
}
void cleanup_module(void)
{
    pr_info("cleanup my module\n");
}
MODULE_LICENSE("GPL");