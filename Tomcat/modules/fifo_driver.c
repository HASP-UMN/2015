#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/signal.h>
#include <asm/io.h>
#include <linux/time.h>

MODULE_AUTHOR("Aron Lindell");
MODULE_DESCRIPTION("HASP FIFO Driver");
MODULE_LICENSE("GPL");

#define IRQ_NUM 5
#define CLASS_NAME "Photon_Fifo"
#define DEVICE_NAME "FIFO_DEV"
#define BUFMAX 504

static unsigned char* fifo_buffer;
unsigned short port_start = 0x800;
static int major;
static struct class* fifo_class;
static struct device* fifo_device;
static int open_count = 0;
wait_queue_head_t wq;
static int interrupt_flag;

//function declarations

static int fifo_open(struct inode* inode, struct file* filp);
static ssize_t fifo_read(struct file*, char*, size_t, loff_t *);
static void fifo_write(struct file*, char*, size_t, loff_t* );
irq_handler_t handler(int irq, void* dev_id, struct pt_regs *regs);
static int fifo_close(struct inode* inode, struct file* filp);
static void __exit fifo_exit(void);


static struct file_operations fifo_fops = {
    .open = fifo_open,
    .read = fifo_read,
    .release = fifo_close,
    .write = fifo_write,
};

/* Module Initialization */
static int __init fifo_init(void)
{
    int retval = 0;
    /* register the module */
	fifo_fops.owner = THIS_MODULE;

	if(!request_region(port_start, BUFMAX, "fifo_dev") )
	{
		printk("<1> cant request region for fifo_dev\n");
		return -ENODEV;
	}
	else
	{
		printk("<1> Succesfully requested region %d\n", port_start);
	}

    major = register_chrdev(0, DEVICE_NAME, &fifo_fops);
    if (major < 0) {
		printk("<1> failed to register simple_driver.c device\n");
        retval = major;
        goto failed_chrdevreg;
    }
	else
	{
		printk("<1> MADE IT PAST REGISTER_CHRDEV WITH SUCCESS\n");
	}

    fifo_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(fifo_class)) {
		printk("<1> failed to register simple_device.c: FIFO_CLASS = %s\n", fifo_class->name);
		retval = (int) fifo_class;
        goto failed_classreg;
    }
	else
	{
		printk("<1> MADE IT PAST class_create WITH SUCCESS: class = %s \n", fifo_class->name);
	}

    fifo_device = device_create(fifo_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(fifo_device)) {
		printk("<1> failed to create device \n");
		retval = (int)fifo_device;
        goto failed_devreg;
    }
	else
	{
		printk("<1> MADE IT PAST device_create WITH SUCCESS: Return value: %d\n", retval);
	}

	init_waitqueue_head(&wq);
	enable_irq(IRQ_NUM);

	return retval;

failed_devreg:
    class_unregister(fifo_class);
    class_destroy(fifo_class);
failed_classreg:
    unregister_chrdev(major, DEVICE_NAME);
failed_chrdevreg:
    return retval;
}





static int fifo_open(struct inode* inode, struct file* filp)
{
    if (open_count >= 1)//already been opened once
    {
        printk("<1> Fifo_device driver already open. Only supports one open at a time\n");
        return -1;
    }

    if ( (request_irq(IRQ_NUM, (irq_handler_t) handler, 0, DEVICE_NAME, NULL) ) != 0 ){
        printk("<1> Could not request IRQ\n");
        return -1;
    }
	else
	{
		printk("<1> REQUEST_IRQ SUCCESS: returned 0\n");
	}

	fifo_buffer = kmalloc(BUFMAX, GFP_KERNEL);
	interrupt_flag = 0;
    open_count++;
    return 0;
}


irq_handler_t handler(int irq, void* dev_id, struct pt_regs *regs)
{
	int i;
//	struct timeval start, end;
//	do_gettimeofday(&start);


	for(i = 0; i < BUFMAX; i++)
	{
		fifo_buffer[i] = inb(port_start);
	}

    // or can you insb as below
//    insb(port_start, fifo_buffer, BUFMAX);
	interrupt_flag = 1;
    wake_up_interruptible(&wq);
//	do_gettimeofday(&end);
//	printk("<1> Time elapsed during interrupt handler = %lu\n", (end.tv_usec - start.tv_usec) );
//	printk("<1> handler start = %lu \n ", start.tv_usec);
//	printk("<1> handler end = %lu \n", end.tv_usec);
	return 0;
}

static ssize_t fifo_read(struct file* filp, char __user* buf, size_t count, loff_t* offp)
{
	/* count isn't really going to be considered because read is going to automatically
	dump the entire fifo_buffer which could have more than 1 interrupts worth of data
	Also, __user* buf needs to be at least as big as fifo_buf, so probably 4*BUFMAX */

	ssize_t not_copied;
	wait_event_interruptible(wq, interrupt_flag != 0);
	interrupt_flag = 0;
	not_copied = copy_to_user(buf, fifo_buffer, BUFMAX);

	if (not_copied != 0){
	    printk("<1> copy_to_user returned %d\n", not_copied);
	}
    
    return (ssize_t)count - not_copied;
}

static ssize_t fifo_close(struct file* fp, const char __user* buf, size_t len, loff_t* off )
{
    if (buf[1] == DISABLE){
        disable_irq(IRQ_NUM);
    }
    if (buf[1] == ENABLE){
        enable_irq(IRQ_NUM);
    }
    
    return 0;
}


static int fifo_close(struct inode* inode, struct file* filp)
{
    open_count --;

    if (open_count == 0)
    {
        free_irq(IRQ_NUM, NULL);
	kfree(fifo_buffer);
    	return open_count;
    }

    printk(KERN_ALERT "Closed Dev but didn't free irq or kernel buffer. open count = %d\n", open_count);
    return open_count;
}


/* Module deconstructor */
static void __exit fifo_exit(void)
{
	device_destroy(fifo_class, MKDEV(major, 0) );
	class_unregister(fifo_class);
	class_destroy(fifo_class);
	unregister_chrdev(major, DEVICE_NAME);
	disable_irq(IRQ_NUM);
	release_region(port_start, BUFMAX);
}




/* Specify init and exit functions */
module_init(fifo_init);
module_exit(fifo_exit);
