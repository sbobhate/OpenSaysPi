/*
 * mylockcontrol.c - Module that is installed on the Pi
 */

#include <linux/module.h>			/* Needed by all modules 		    */
#include <linux/kernel.h>			/* Needed for KERN_INFO 		    */
#include <linux/init.h>				    /* Needed for init/exit Macros    */
#include <linux/timer.h>			    /* Needed for Timer API 		        */
#include <linux/proc_fs.h>			/* Needed for /proc File System */
#include <linux/gpio.h>				/* Needed for GPIO control         */
#include <asm/uaccess.h>			/* Needed for "current" 	            */
#include <linux/slab.h>				/* Needed for kmalloc */
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define MS_TO_NS(x) (x * 1E6L)
#define SEC 1000
#define SCL 5  // G5
#define SDA 6  // G6
#define BTN 20 // G20
#define SERVO 21 // G21
#define LOW 0
#define HIGH 1
#define DEV_FILE_NAME "mylockcontrol"
#define NUM_BITS 8
#define MAJOR_NUM 61
//#define DEBUG for_debugging

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Bobhate");
MODULE_DESCRIPTION("Module to control locking system from Pi");

/*
 * Variables
 */
//static struct hrtimer hr_timer;
//unsigned int LED_state;
//unsigned int count;
struct timer_list clock_timer;
static char input[256];
static unsigned int lock = 1;

/*
 * Function Declarations
 */
static int __init init_mylockcontrol(void);
static void __exit cleanup_mylockcontrol(void);
static ssize_t write_mylockcontrol(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int open_mylockcontrol(struct inode *inode, struct file *filp);
static int release_mylockcontrol(struct inode *inode, struct file *filp);
static void TimerCallbackFunction(unsigned long data);
//enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer );
//void actuate(void);

struct file_operations fops_mylockcontrol = {
	write: write_mylockcontrol,
	open: open_mylockcontrol,
	release: release_mylockcontrol
};

/*
 * Function that is called when module is initialized
 */
static int __init init_mylockcontrol(void)
{
	 register_chrdev(MAJOR_NUM, DEV_FILE_NAME, &fops_mylockcontrol);

	 // Request GPIO pins
	 gpio_request(SCL, DEV_FILE_NAME);
	 gpio_request(SDA, DEV_FILE_NAME);
	 gpio_request(BTN, DEV_FILE_NAME);
//	 gpio_request(SERVO, DEV_FILE_NAME); 

	 // Set GPIO pin direction
	 gpio_direction_output(SCL, LOW);
	 gpio_direction_output(SDA, LOW);
	 gpio_direction_input(BTN);
//	 gpio_direction_output(SERVO, LOW);

	 //input = (char *) kmalloc(16*sizeof(char), GFP_KERNEL);
//	 lock = 1;
//	 LED_state = LOW;

	 setup_timer( &clock_timer, TimerCallbackFunction, 0 );
	 mod_timer( &clock_timer, jiffies + SEC );

//	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
//	hr_timer.function = &my_hrtimer_callback;

    return 0;
}

/*
 *
 * Function that is called when the module is moved
 */
static void __exit cleanup_mylockcontrol(void)
{
    del_timer( &clock_timer );
  //  hrtimer_cancel( &hr_timer );

    gpio_set_value(SCL, LOW);
    gpio_set_value(SDA, LOW);
   // gpio_set_value(SERVO, LOW);
 
    gpio_free(SCL);
    gpio_free(SDA);
    gpio_free(BTN);
    //gpio_free(SERVO);
 
    unregister_chrdev(MAJOR_NUM, DEV_FILE_NAME);

    return;
}

/*
 * 1Hz Clock for communication
 */
static void TimerCallbackFunction(unsigned long data)
{
	unsigned int button_val;
	if (gpio_get_value(SCL) == LOW)
	{
		/* Rising Edge */
		//button_val = gpio_get_value(BTN);
		//printk(KERN_ALERT "Sending a %u\n", button_val);
		if (lock == 0) 
		{
			gpio_set_value(SDA, 1);
			printk(KERN_INFO "SENDING a 1\n");
			//lock = 1;
			//actuate();
		}
		gpio_set_value(SCL, HIGH);
	}
	else 
	{
		gpio_set_value(SCL, LOW);
		gpio_set_value(SDA, LOW);
		lock = 1;
	}
	mod_timer( &clock_timer, jiffies + msecs_to_jiffies(SEC) );
}

static ssize_t write_mylockcontrol(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{;
	if ((copy_from_user(input, buf, sizeof(256*sizeof(char))))) return -EFAULT;
	//printk(KERN_INFO "Message :- %s", input);
	printk(KERN_INFO "Unlocking\n");
	lock = 0;
	//actuate();
	return count;
	//sprintf(input, "%s(%zu letters)", buf, count);   // appending received string with its length
   	//size_of_message = strlen(input);                 // store the length of the stored message
   	//printk(KERN_INFO "EBBChar: Received %zu characters from the user saying %s\n", count, input);
   	//return count;
}

static int open_mylockcontrol(struct inode *inode, struct file *filp)
{
	return 0;
}

static int release_mylockcontrol(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
	unsigned long delay_in_ms;
	ktime_t ktime;
	printk(KERN_INFO "Timer was fired with count (%u).\n", count);
	if (LED_state == HIGH)
	{
		if (lock == 1) delay_in_ms = 19L;
		else delay_in_ms = 17L;
		ktime = ktime_set( 0, MS_TO_NS(delay_in_ms) );
		LED_state = LOW;
	}
	else
	{
		if (lock == 1) delay_in_ms = 1L;
		else delay_in_ms = 3L;
		ktime = ktime_set( 0, MS_TO_NS(delay_in_ms) );
		LED_state = HIGH;
		if (count < 50) count++;
	}
	gpio_set_value(SERVO, LED_state);

	if (count < 50) {
		hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
	}

	return HRTIMER_RESTART;
}

void actuate(void)
{
	ktime_t ktime;
	unsigned long delay_in_ms = 19L;
	count = 0;
	LED_state = LOW;
	gpio_set_value(SERVO, LED_state);
	ktime = ktime_set( 0, MS_TO_NS(delay_in_ms) );
	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
}
*/

/*
 * Declare the init/exit functions
 */
module_init(init_mylockcontrol);
module_exit(cleanup_mylockcontrol);

/*
 * SOME NOTES:
 *  - 
 *
 * Great References:
 *  - 
 */


