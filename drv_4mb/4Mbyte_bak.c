#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define MAJOR_NUMBER 61

/* forward declaration */
int onebyte_open(struct inode *inode, struct file *filep);
int onebyte_release(struct inode *inode, struct file *filep);
ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos);
ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos);
static void onebyte_exit(void);

/* definition of file_operation structure */
struct file_operations onebyte_fops = {
    read: onebyte_read,
    write: onebyte_write,
    open: onebyte_open,
    release: onebyte_release
};

char *onebyte_data = NULL;

int onebyte_open(struct inode *inode, struct file *filep)
{
    return 0; // always successful
}
int onebyte_release(struct inode *inode, struct file *filep)
{
    return 0; // always successful
}
ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
    /*please complete the function on your own*/
    //copy_to_user has format (* to, * from, size)
    int maxbytes = 4000000; /* number */
    int bytes_to_read;
   // int bytes_read;

    if (maxbytes > count) bytes_to_read = count;
    else bytes_to_read = maxbytes;

    copy_to_user(buf, onebyte_data, bytes_to_read);
    if (*f_pos == 0){
        *f_pos+=bytes_to_read;
        return bytes_to_read;
    } 
    else{
        return 0;
    }
}
ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{
    /*please complete the function on your own*/
    int maxbytes = 4000000; /*set the max bytes can be written*/
    int bytes_to_write;

    if (count>maxbytes){
        bytes_to_write = maxbytes;
        copy_from_user(onebyte_data, buf, bytes_to_write);
    	printk(KERN_INFO "Write %d byte of data from user", bytes_to_write);
	return -ENOSPC;
    }
    else{
        bytes_to_write = count;
        copy_from_user(onebyte_data, buf, bytes_to_write);
    	printk(KERN_INFO "Write %d byte of data from user", bytes_to_write);
	return bytes_to_write;
    }
}
static int onebyte_init(void)
{
    int result;
    // register the device
    result = register_chrdev(MAJOR_NUMBER, "onebyte", &onebyte_fops);
    if (result < 0) {
        return result;
    }
    // allocate one byte of memory for storage
    // kmalloc is just like malloc, the second parameter is

    // the type of memory to be allocated.
    // To release the memory allocated by kmalloc, use kfree.
    onebyte_data = kmalloc((sizeof(char)*4000000), GFP_KERNEL);
    if (!onebyte_data) {
        onebyte_exit();
        // cannot allocate memory
        // return no memory error, negative signify a failure
        return -ENOMEM;
    }
    // initialize the value to be X
    *onebyte_data = 'X';
    printk(KERN_ALERT "This is a 4Mbyte device module\n");
    return 0;
}
static void onebyte_exit(void)
{
    // if the pointer is pointing to something
    if (onebyte_data) {
        // free the memory and assign the pointer to NULL
        kfree(onebyte_data);
        onebyte_data = NULL;
    }
    // unregister the device
    unregister_chrdev(MAJOR_NUMBER, "onebyte");
    printk(KERN_ALERT "Onebyte device module is unloaded\n");
}
MODULE_LICENSE("GPL");
module_init(onebyte_init);
module_exit(onebyte_exit);