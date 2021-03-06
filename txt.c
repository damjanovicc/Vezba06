#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/vmalloc.h>


static int buff_size = 1000;
module_param(buff_size, int, 0);

static char *buff;
static dev_t dev_hello;
static int hello_count = 1;
static struct cdev cdev_hello;

struct proc_dir_entry *struktura;


static ssize_t
hello_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int remaining_size, transfer_size;
    remaining_size = buff_size - (int) (*ppos); // bytes left to transfer
    if (remaining_size == 0) { /* All read, returning 0 (End Of File) */
        return 0;
    }
    /* Size of this transfer */
    transfer_size = min(remaining_size, (int) count);
    if (copy_to_user(buf /* to */, buff + *ppos /* from */, transfer_size)){
        return -EFAULT;
    } else { /* Increase the position in the open file */
        *ppos += transfer_size;
        return transfer_size;
    }
}

static ssize_t
hello_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int remaining_bytes;
    /* Number of bytes not written yet in the device */
    remaining_bytes = buff_size - (*ppos);
    if (count > remaining_bytes) {
        /* Can't write beyond the end of the device */
        return -EIO;
    }
    if (copy_from_user(buff + *ppos /* to */, buf /* from */, count)) {
        return -EFAULT;
    } else {
        /* Increase the position in the open file */
        *ppos += count;
        return count;
    }

}

/*static inline struct proc_dir_entry *proc_create(const char *name, umode_t mode, struct proc_dir_entry *parent, const struct file_operations *proc_fops);

void proc_remove(struct proc_dir_entry *struktura);


static long ioctl_foo(struct file *file, unsigned int cmd, unsigned long arg){

	unsigned int i = 0;
	switch(cmd){
		case 0:
			while(buff[i]){
				buff[i] = toupper(buff[i]);
				i++;
			}
			break;
		
	}


}*/

static struct file_operations fops_hello = {
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write,
};

static int __init hello_init(void) {
    int err;
    pr_info("Hello world!\n");

	struktura = proc_create("name",0,NULL,&fops_hello);
	if(struktura == NULL){
		return 0;
	}

	
    buff = kmalloc(buff_size, GFP_KERNEL);
    if (buff == NULL) 
    {
        pr_info("Error allocating buffer\n");
        err = -ENOMEM;
	goto err_exit;
    }
    memset(buff, 0x00, buff_size);

    if (alloc_chrdev_region(&dev_hello, 0, hello_count, "hello"))
    {
	pr_info("Error allocating chardev region\n");
	err = -ENODEV;
	goto err_free_buff;
    }

    cdev_init(&cdev_hello, &fops_hello);
    if(cdev_add(&cdev_hello, dev_hello, hello_count))
    {
	pr_info("Error adding device\n");
	err = -ENODEV;
	goto err_dev_unregister;
    }

    return 0;
err_dev_unregister:
    unregister_chrdev_region(dev_hello, hello_count);
err_free_buff:
    kfree(buff);
err_exit:
    return err;
}

static void __exit hello_exit(void) {
    pr_info("Goodbye!\n");
    cdev_del(&cdev_hello);
    unregister_chrdev_region(dev_hello, hello_count);
    kfree(buff);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Greeting module");
MODULE_AUTHOR("William Shakespeare");
;
