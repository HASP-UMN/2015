// first draft at the device driver / kernel module

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/uaccess.h>

/* Check  if the Device File System (experimental) is installed */
#ifndef CONFIG_DEVFS_FS
# error "This module requires the Device File System (devfs)"
#endif

/* MODULE CONFIGURATION */

MODULE_AUTHOR("Aron Lindell");
MODULE_DESCRIPTION("HASP_FIFO Driver");
MODULE_LICENSE("GPL");

static devfs_handle_t fifo_handle;    /* handle for the device     */
static char* dev_name = "FIFO_DEV";  /* create /dev/FIFO_DEV entry */
int irq_num = 6;
static unsigned char fifo_buffer[BUFMAX];

int fifo_open(const char*);
static ssize_t fifo_read(struct file*, char*, size_t, loff_t *);
static ssize_t fifo_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fifo_fops = {
    open : fifo_open,
    read : fifo_read,
    write: fifo_write,
    /* NULL (default) */
};


/* Module Initialization */
static int __init fifo_init(void)
{
    /* register the module */
    SET_MODULE_OWNER(&fifo_fops);
    fifo_handle = devfs_register
    (
     NULL,                      /* parent dir in /dev (none)     */
     fifo_name,                 /* /dev entry name (skeldev2)    */
     DEVFS_FL_AUTO_DEVNUM |     /* automatic major/minor numbers */
     DEVFS_FL_AUTO_OWNER,
     0, 0,                      /* major/minor (not used)        */
     S_IFCHR,                   /* character device              */
     &fifo_fops,                /* file ops handlers             */
     NULL                       /* other                         */
     );
    
    if (fifo_handle <= 0){
        return(-EBUSY);
        printk(KERN_ALERT "Error registering FIFO_DEV module\n");
    }
    
    return(0);
} /* fifo_init() */




/* Module deconstructor */
static void __exit fifo_exit(void)
{
    devfs_unregister(fifo_handle);
    free_irq(irq_num, NULL);
    
} /* fifo_exit() */





/* Read from device */
static ssize_t fifo_read(struct file* filp, char* buf, size_t count, loff_t* offp)
{
    if (count > BUFMAX)
    {
        count = BUFMAX;  /* trim data */
    }
    copy_to_user(buf, fifo_buffer, count);
    return(count);
} /* fifo_read() */




/* Write to device */
static ssize_t fifo_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
    if (count > BUFMAX)
    {
        count = BUFMAX;
    }
    copy_from_user(fifo_buffer, buf, count);
    return(count);
}




static int fifo_open(const char*)
{
    if ( (request_irq(irq_num, handler, SA_INTERRUPT, dev_name, NULL) ) != 0 ){
        printk(KERN_ALERT "Could not request IRQ\n");
    }
}



/* Specify init and exit functions */
module_init(fifo_init);
module_exit(fifo_exit);










