#include <linux/kernel.h>
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/mm.h>

#define procfs_name "tsu"
static struct proc_dir_entry *our_proc_file = NULL;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    struct sysinfo memInfo;
    si_meminfo(&memInfo);

    long long totalMem = memInfo.totalram;

    long long freeMem = memInfo.freeram;

    int percentFree = (freeMem * 100) / totalMem;

    ssize_t ret;
    char s[128];

    ret = sprintf(s, "Free memory:%d%%\n", percentFree);
    if (*offset >= ret || copy_to_user(buffer, s, ret))
    {
        ret = 0;
    }
    else
    {
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
        *offset += ret;
    }

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
};
#endif

static int __init procfs1_init(void)
{
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    pr_info("/proc/%s created\n", procfs_name);
    return 0;
}

static void __exit procfs1_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", procfs_name);
}

module_init(procfs1_init);
module_exit(procfs1_exit);
MODULE_LICENSE("GPL");