#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/string.h>

#define MAJOR_NUMBER 61
#define MAX_LENGTH 4000000
#define SCULL_IOC_MAGIC 'k'

#define SCULL_IOCRESET _IO(SCULL_IOC_MAGIC, 0)
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SCULL_SETMSG _IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_GETMSG _IOR(SCULL_IOC_MAGIC, 3, int)
#define SCULL_SET_GETMSG _IOWR(SCULL_IOC_MAGIC, 4, int)
#define SCULL_IOC_MAXNR 14

/* forward declaration */
int onebyte_open(struct inode *inode, struct file *filep);
int onebyte_release(struct inode *inode, struct file *filep);
ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos);
ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos);
loff_t onebyte_llseek(struct file *filep, loff_t loffset, int whence);
long onebyte_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

static int onebyte_init(void);
static void onebyte_exit(void);

/* definition of file_operation structure */
struct file_operations onebyte_fops = {
    read: onebyte_read,
    write: onebyte_write,
    open: onebyte_open,
    llseek: onebyte_llseek,
    release: onebyte_release,
    unlocked_ioctl: onebyte_ioctl
};

const int MSG_LEN = 100;
char *onebyte_data = NULL;
static int cur_pos = 0;
char *dev_msg=NULL; 

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
    int bytes_to_read;

    //if (*f_pos + count > MAX_LENGTH){
    //    bytes_to_read = MAX_LENGTH - *f_pos;
    //} else {
    //	bytes_to_read = count;
    //}
    //else bytes_to_read = maxbytes;
    bytes_to_read = cur_pos;
    copy_to_user(buf, onebyte_data, bytes_to_read);
    
    if (*f_pos == 0){
        *f_pos+=bytes_to_read;
        return bytes_to_read;
    } 
    return 0;
}
ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{
    /*please complete the function on your own*/
    int bytes_to_write;
    if (cur_pos >= MAX_LENGTH) cur_pos=0;

    if (cur_pos + count>MAX_LENGTH){
        bytes_to_write = MAX_LENGTH-cur_pos;
    	printk(KERN_INFO "File too big: write %d byte of data from user", bytes_to_write);
	return -ENOSPC;
    }
    else{
        bytes_to_write = count;
    	printk(KERN_INFO "File is ok: Write %d byte of data from user", bytes_to_write);
    }
    copy_from_user(onebyte_data + cur_pos, buf, bytes_to_write);
    //*f_pos += bytes_to_write;
    cur_pos += bytes_to_write;
    return bytes_to_write;
}

loff_t onebyte_llseek(struct file *filep, loff_t offset, int whence)
{
    loff_t new_pos=0;
    switch (whence)
    {
        case 0: /*SEEK_SET:*/
	    new_pos = offset;
	    break;
	case 1: /*SEEK_CUR*/
	    new_pos = cur_pos + offset;
	    break;
	case 2: /*SEEK_END*/
	    new_pos = MAX_LENGTH - offset;
	    break;
	default :
	    new_pos = 0;
	    printk("Invalid lseek argument!!");
    }
    if(new_pos > MAX_LENGTH) new_pos = MAX_LENGTH;
    if(new_pos < 0) new_pos = 0;
    return new_pos;
}

long onebyte_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int err=0;
    int retval=0;

    if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;

    if(_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd));
    else if(_IOC_DIR(cmd)&_IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
    if(err) return -EFAULT;
    
    switch(cmd){
        case SCULL_HELLO:
            printk(KERN_WARNING "Hello World!!\n");
	    break;
	case SCULL_SETMSG:
	    printk(KERN_WARNING "Set the dev_msg!!\n");
	    printk("Received pointer: %d \n", arg);
	    copy_from_user(dev_msg, (unsigned char *)arg, MSG_LEN);
	    printk(KERN_WARNING "dev_msg: %s \n", dev_msg);
	    break;
	case SCULL_GETMSG:
	    printk(KERN_WARNING "Copy from dev_msg to usr_msg!!\n");
	    copy_to_user((unsigned char *)arg, dev_msg, MSG_LEN);
	    break;
	case SCULL_SET_GETMSG:
	    copy_from_user(dev_msg, (unsigned char *)arg, MSG_LEN);
	    printk(KERN_WARNING "dev_msg: %s \n", dev_msg);
	    copy_to_user((unsigned char *)arg, dev_msg, MSG_LEN);
	    break;
        default: /*=.=*/
            return -ENOTTY;
    }
    return retval;
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
    onebyte_data = kmalloc(sizeof(char)*MAX_LENGTH, GFP_KERNEL);
    dev_msg = kmalloc(sizeof(char)*MSG_LEN, GFP_KERNEL);
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
    printk(KERN_ALERT "4Mbyte device module is unloaded\n");
}
MODULE_LICENSE("GPL");
module_init(onebyte_init);
module_exit(onebyte_exit);
