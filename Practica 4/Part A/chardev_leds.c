/*
 *  chardev_leds.c: 
 * 
 * Device node created in kernel space are defaultly set to 0600 permission. 
 * In other words, only root user can access to the driver. We want to change 
 * the permission of /dev/chardev_leds to 666 so that all users are able to 
 * access to the driver. To do this, create a simple rule in /etc/udev/rules.d/. 
 * 
 *  cd /etc/udev/rules.d/
 *  sudo touch 10-local.rules
 *  sudo gedit 10-local.rules
 *  Write:
 *  # device node create in kernel space using device_create has default
 *  # permission of 600
 *  # change default permission 600 to 666 in user space
 *  # chardev_leds: DEVICE_NAME
 *  # chardev: CLASS_NAME
 *  KERNEL=="chardev_leds", SUBSYSTEM=="chardev", MODE="666"
 *
 *  After this, load the driver.
 * 
 * Instructions to load and unload the driver:
 * 
 * Compile the driver: 
 *  $ make
 * 
 * Load driver:
 *  $ sudo insmod chardev_leds.ko
 * 
 * Turn on/off the leds, example:
 *  $ sudo echo 1 > /dev/chardev_leds
 * 
 * Unload driver:
 *  $ sudo rm /dev/chardev_leds
 *  $ sudo rmmod chardev_leds
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for copy_to_user */
#include <linux/cdev.h>
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/version.h> /* For LINUX_VERSION_CODE */
#include <linux/device.h>         // Header to support the kernel Driver Model

MODULE_LICENSE("GPL");

/*
 *  Prototypes
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev_leds"	/* Dev name as it appears in /proc/devices   */
#define  CLASS_NAME  "chardev"        ///< The device class -- this is a character device driver
#define BUF_LEN 80		/* Max length of the message from the device */
#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0

/*
 * Global variables are declared as static, so are global within the file.
 */

dev_t start;
struct cdev* chardev=NULL;
static int Device_Open = 0;	/* Is device open?
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;		/* This will be initialized every time the
				   device is opened successfully */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

struct tty_driver* kbd_driver= NULL;
static struct class*  chardevClass  = NULL; // The device-driver class struct pointer
static struct device* chardevDevice = NULL; // The device-driver device struct pointer


/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void)
{
    printk(KERN_INFO "modleds: loading\n");
    printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return vc_cons[fg_console].d->port.tty->driver;
#else
    return vc_cons[fg_console].d->vc_tty->driver;
#endif
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask)
{
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
#else
    return (handler->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, mask);
#endif
}

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    int major;		/* Major number assigned to our device driver */
    int minor;		/* Minor number assigned to the associated character device */

    /* Register driver */
    if (alloc_chrdev_region (&start, 0, 1, DEVICE_NAME)) {
        printk(KERN_INFO "Can't register chrdev_region()");
        return -ENOMEM;
    }
    printk(KERN_INFO "Device registered correctly with major number %d\n", MAJOR(start));

    /* Create associated cdev */
    if ((chardev=cdev_alloc())==NULL) {
        printk(KERN_INFO "cdev_alloc() failed ");
        unregister_chrdev_region(start, 1);
        return -ENOMEM;
    }

    chardevClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(chardevClass))    //$ls /sys/class
    {
        printk(KERN_INFO "class_create() failed ");
        unregister_chrdev_region(start, 1);
        return PTR_ERR(chardevClass);
    }

    cdev_init(chardev,&fops);

    if (cdev_add(chardev,start,1)) {
        printk(KERN_INFO "cdev_add() failed ");
        class_destroy(chardevClass);
        unregister_chrdev_region(start, 1);
        return -ENOMEM;
    }

    chardevDevice = device_create(chardevClass, NULL, start, NULL, DEVICE_NAME);
    if (IS_ERR(chardevDevice)) //$ls /dev/
    {
        device_destroy(chardevClass, start);
        class_destroy(chardevClass);
        unregister_chrdev_region(start, 1);
        return PTR_ERR(chardevDevice);
    }

    // Leds start

    kbd_driver= get_kbd_driver_handler();
    set_leds(kbd_driver,ALL_LEDS_ON);

    major=MAJOR(start);
    minor=MINOR(start);

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver. Try to cat and echo to the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    set_leds(kbd_driver,ALL_LEDS_OFF);

    /* Destroy chardev */
    if (chardev)
        cdev_del(chardev);

    device_destroy(chardevClass, start);
    class_unregister(chardevClass);
    class_destroy(chardevClass);
    /*
     * Unregister the device
     */
    unregister_chrdev_region(start, 1);
    printk(KERN_INFO "Driver %s released\n", DEVICE_NAME);
}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
        return -EBUSY;

    Device_Open++;

    /* Increase the module's reference counter */
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;		/* We're now ready for our next caller */

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    module_put(THIS_MODULE);

    return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
                           char *buffer,	/* buffer to fill with data */
                           size_t length,	/* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_to_read = length;

    /*
     * If we're at the end of the message,
     * return 0 -> end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /* Make sure we don't read more chars than
     * those remaining to read
     */
    if (bytes_to_read > strlen(msg_Ptr))
        bytes_to_read=strlen(msg_Ptr);

    /*
     * Actually transfer the data onto the userspace buffer.
     * For this task we use copy_to_user() due to security issues
     */
    if (copy_to_user(buffer,msg_Ptr,bytes_to_read))
        return -EFAULT;

    /* Update the pointer for the next read operation */
    msg_Ptr+=bytes_to_read;

    /*
     * The read operation returns the actual number of bytes
     * we copied  in the user's buffer
     */
    return bytes_to_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev
 */
static ssize_t device_write(struct file *filp,	/* see include/linux/fs.h   */
							const char *buffer,	/* buffer to fill with data */
							size_t length,	/* length of the buffer     */
							loff_t * offset)
{
	unsigned int mask = 0;
    int i;
    	
	/*
     * Actually transfer the data from the userspace buffer.
     * For this task we use copy_from_user() due to security issues
     */
    if (copy_from_user(msg, buffer, BUF_LEN)){
        printk(KERN_INFO "Error reading from user\n");
        return -EFAULT;
    }

    for (i = 0; i < length; i++) {
        if (msg[i] == '1') {
            mask |= 2;
        }
        else if (msg[i] == '2') {
            mask |= 4;
        }
        else if (msg[i] == '3') {
            mask |= 1;
        }
    }

	set_leds(kbd_driver, mask);
	
    /*
     * The write operation returns the actual number of bytes
     * we copied from the user's buffer
     */
    return length;
}
