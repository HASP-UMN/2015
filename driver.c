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
#include <linux/wait.h>

#include "globaldefs.h"

/* Check  if the Device File System (experimental) is installed */
#ifndef CONFIG_DEVFS_FS
# error "This module requires the Device File System (devfs)"
#endif

/* MODULE CONFIGURATION */

MODULE_AUTHOR("Aron Lindell");
MODULE_DESCRIPTION("HASP_FIFO Driver");
MODULE_LICENSE("GPL");








#define IRQ_NUM 6

static devfs_handle_t fifo_handle;    /* handle for the device     */
static char* dev_name = "FIFO_DEV";  /* create /dev/FIFO_DEV entry */
static unsigned char fifo_buffer[BUFMAX]; //BUFMAX is in globaldefs.h.. should be 500
unsigned short port_start = 0x800;
static int open_count = 0;



//function declarations
int fifo_open(const char*);
static ssize_t fifo_read(struct file*, char*, size_t, loff_t *);
int fifo_close();

static struct file_operations fifo_fops = {
    open : fifo_open,
    read : fifo_read,
    close: fifo_close,
    /* NULL (default) */
};




wait_queue_head_t wq;











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
    
    init_waitqueue_head(&wq);
    
    return(0);
} /* fifo_init() */







static int fifo_open(const char*)
{
    if ( (request_irq(IRQ_NUM, handler, SA_INTERRUPT, dev_name, NULL) ) != 0 ){
        printk(KERN_ALERT "Could not request IRQ\n");
        reuturn -1;
    }
    
    open_count++;
    
    return 0;
}







/* Read from device */
static ssize_t fifo_read(struct file* filp, char* buf, size_t count, loff_t* offp)
{
    if (count > BUFMAX)
    {
        count = BUFMAX;  /* trim data */
    }
    
    interruptible_sleep_on(&wq);
    
    copy_to_user(buf, fifo_buffer, count);
    return(count);
} /* fifo_read() */










int handler(int irq, void* dev_id, struct pt_regs *regs)
{
    int i;
    for (i = 0; i < BUFMAX; i++) {
        fifo_buffer[i] = inb(port_start + i);
    }
    
    // or can you insb as below
    //insb(port_start, fifo_buffer, BUFMAX);
    
    wake_up_interruptible(&wq);
}







static int fifo_close()
{
    open_count --;
    
    if (open_count == 0)
    {
        fifo_exit();
    }
    
    return 0;
}

/* Module deconstructor */
static void __exit fifo_exit(void)
{
    devfs_unregister(fifo_handle);
    free_irq(IRQ_NUM, NULL);
    
} /* fifo_exit() */




/* Specify init and exit functions */
module_init(fifo_init);
module_exit(fifo_exit);










