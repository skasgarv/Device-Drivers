#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/usb.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SUHAS");
MODULE_VERSION("1.0");

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x01
#define BULK_EP_IN 0x82
#define MAX_PKT_SIZE 512
 
static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];
 
//Function to open the USB Device
static int pen_open(struct inode *i, struct file *f)
{
    return 0;
}

//Function to close the USB Device
static int pen_close(struct inode *i, struct file *f)
{
    return 0;
}

//Function for reading from USB Device
static ssize_t pen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    	int retval;
    	int read_cnt;
 
    	/* Read the data from the bulk endpoint */
    	retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN),
            	bulk_buf, MAX_PKT_SIZE, &read_cnt, 5000);
	/* usb_bulk_msg returns 1 on success*/
    	
	if (retval)
    	{
        		printk(KERN_ERR "Bulk message returned %d\n", retval);
        		return retval;
    	}
    

	/*copy_to_user has similar working of Memcpy 
	Why can't you just call, say, memcpy? Two reasons. One, the kernel is capable of writing to any memory. 
	User process's can't. copy_to_user needs to check dst to ensure it is accessible and writable by the current process. 
	Two, depending on the 	architecture, you can't simply copy data from kernel to user-space. You might need to do 
	some special setup first, invalidate certain caches, or use special operations.
	
	Source: https://www.quora.com/Linux-Kernel-How-does-copy_to_user-work
	
	Data from bulk_buf address is read and stored in buf. The amount of data read is defined by MIN(cnt, read_cnt)
	*/
	
	if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))
    	{
        		return -EFAULT;
    	}
 
    	return MIN(cnt, read_cnt); //returns the amount of data read
}


static ssize_t pen_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)
{
    	int retval;
    	int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);
 
    	if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
    	{
      	  	return -EFAULT;
    	}
 
   	 /* Write the data into the bulk endpoint */
    	retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT),
            	bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
    
	if (retval)
    	{
        		printk(KERN_ERR "Bulk message returned %d\n", retval);
        		return retval;
    	}
 
    	return wrote_cnt;
}
 


static struct file_operations fops =
{
    .open = pen_open,
    .release = pen_close,
    .read = pen_read,
    .write = pen_write,
};

 
static int pen_probe(struct usb_interface *interface, const struct usb_device_id *id)
{

	/*
	Probe function will not always be called. Probe function is called only when the device is plugged 
	for the first time or the default driver module is not loaded to the device.
	*/
    	
	int retval;
    	device = interface_to_usbdev(interface);
 
    	class.name = "usb/pen%d";
   	
	/*
   	fops is a structure which has the basic functionalities for a read and write operation to 	
   	a USB Device. fops has "read", "write", "open" and "release" options which helps us 	
   	perform the required operation 
    	*/
	class.fops = &fops;
    

	if ((retval = usb_register_dev(interface, &class)) < 0)
    	{
        		/* Something prevented us from registering this driver */
        		printk(KERN_INFO "Not able to get a minor for this device.");
    	}
    	else
    	{
        		printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
    	}
 
    	return retval;
}
 


static void pen_disconnect(struct usb_interface *interface)
{
	/*
	This function is called when the Pendrive is disconnected from the system and this 	
	requires a USB Interface and the USB object which is defined in the class.
	*/
    
	usb_deregister_dev(interface, &class);
}
 
/* Table of devices that work with this driver */
static struct usb_device_id pen_table[] =
{
    
	{ USB_DEVICE(0x058F, 0x6387) }, //Syntax USB_DEVICE(Major Number,Minor Number)
    	/*
	Major and Minor number are obtained either by reverse engineering or using LSUSB command
	*/	
    
	{}, /* Terminating entry */

};
MODULE_DEVICE_TABLE (usb, pen_table);


/*
usb_driver structure
This structure defines the methodlogies for using a USB Device Driver
*/

static struct usb_driver pen_driver=
{
	.name = "SKVM_USB Stick Driver",
	.id_table = pen_table, //usb_device_id
	.probe = pen_probe, 
	.disconnect = pen_disconnect,
};

static int __init pen_init(void){
	
	int ret = -1;
	//registering the usb device
	printk(KERN_INFO "[*] SKVM_USB constructor of Driver.\n");
	printk(KERN_INFO "Registering Driver with Kernel.\n");

	//Register the device which returns 0 on success.
	ret = usb_register(&pen_driver);
	/*
	 here pen_driver is the name given for our pen drive file. we shall discuss this in the structure
	*/

	printk(KERN_INFO "Registration complete.\n");
	return ret;

}

static void pen_exit(void){

	//deregister the usb device
	printk(KERN_INFO "[*] SKVM_USB destructor of Driver.\n");
	usb_deregister(&pen_driver);
	printk(KERN_INFO "De-Registration Complete.\n");

}	

module_init(pen_init); //Module for the initilization of driver
module_exit(pen_exit); //Module for the exit of driver
