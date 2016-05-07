#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/kernel.h>
#include<asm/uaccess.h>   //Need it for copy_to_user function
#define  DEVICE_NAME "ebbchar"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "ebb"        ///< The device class -- this is a character device driver


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SUHAS");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("A SIMPLE CHARACTER DEVICE DRIVER TO UNDERSTAND THE WORKING OF A DEVICE DRIVER");


static int majorNumber;
static struct class*  ebbcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* ebbcharDevice = NULL; ///< The device-driver device struct pointer
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message; 


int fops_open (struct inode *pinode, struct file *pfile){
	printk(KERN_INFO "EBBChar: Device has been opened.\n");
	return 0;
}

ssize_t fops_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset){
	int error_count =0;
	error_count = copy_to_user(buffer,message,size_of_message);
	if(error_count==0){
		printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
      	return (size_of_message=0);  // clear the position to the start and return 0
   	}
   	else {
      	printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
      	return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   	}
	return 0;
}

ssize_t fops_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset){
	sprintf(message, "%s(%d letters)", buffer, length);   // appending received string with its length
   	size_of_message = strlen(message);                 // store the length of the stored message
   	printk(KERN_INFO "EBBChar: Received %d characters from the user\n", length);
	return length;
}

int fops_release (struct inode *pinode, struct file *pfile){
	printk(KERN_INFO "EBBChar: Device successfully closed\n");
  	return 0;
}

struct file_operations fops ={
	.owner	 = THIS_MODULE,						
	.open	 = fops_open,					
	.release = fops_release,				
	.read	 = fops_read,				
	.write	 = fops_write,				
};




//Initialize the Character Device Driver
int __init char_deviceDriver_init(void){
	printk(KERN_ALERT "Initializing the Character Device Driver.\n");
	//Register the device and obtain the major number 
	majorNumber = register_chrdev(0,DEVICE_NAME,&fops);
	if(majorNumber<0){
		printk(KERN_ALERT "Major Number cannot be registered.\n");
		return majorNumber;
	}
	printk(KERN_ALERT "ebbchar: Registered with number %d\n",majorNumber);	
	
	//Register the device class to avoid typing in the terminal
	ebbcharClass = class_create(THIS_MODULE,CLASS_NAME);
	if(IS_ERR(ebbcharClass)){
		//unregister_chrdev(majorNumber,DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class.\n");
		return PTR_ERR(ebbcharClass);
	}
	printk(KERN_ALERT "EBBChar: Device class registered successfully.\n");

	//Register the device driver
	// alternate to sudo mknod -m 666 /dev/devicename/ c major_no minor_no
	ebbcharDevice = device_create(ebbcharClass,NULL,MKDEV(majorNumber,0),NULL,DEVICE_NAME);
	//error checking
	if(IS_ERR(ebbcharDevice)){
		printk(KERN_ALERT "Failed to create the device.\n");
		return PTR_ERR(ebbcharDevice);
	}
	printk(KERN_ALERT "EBBChar:Successfully created the device.\n");
	
	return 0;
}

void __exit char_deviceDriver_exit(void){
	device_destroy(ebbcharClass,MKDEV(majorNumber,0));
	class_unregister(ebbcharClass);
	class_destroy(ebbcharClass); //clean up the class	
	unregister_chrdev(majorNumber,DEVICE_NAME);
	printk(KERN_ALERT "Exiting(Device Removed) the Character Device Driver.\n");
}

module_init(char_deviceDriver_init);
module_exit(char_deviceDriver_exit);