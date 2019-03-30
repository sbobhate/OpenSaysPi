
/*
 * mytimer.c - Timer Module for Linux
 */

#include <linux/module.h>			/* Needed by all modules  */
#include <linux/kernel.h>			/* Needed for KERN_INFO */
#include <linux/init.h>				/* Needed for init/exit Macros */
#include<linux/slab.h>			/* Needed for kmalloc         */
#include <linux/timer.h>			/* Needed for Timer API      */
#include <linux/errno.h> 			/* Needed for Error Codes   */
#include <linux/gpio.h>			/* Needed for GPIO control */
#include <asm/uaccess.h>		/* Needed to copy_from/to user */
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define MS_TO_NS(x) (x * 1E6L)

#define LED 21
#define HIGH 1
#define LOW 0
#define PERIOD 20 // milliseconds
#define DEV_FILE_NAME "mytimer"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Bobhate");
MODULE_DESCRIPTION("Module to create Kernel Timers and Blink LED");

static struct hrtimer hr_timer;
unsigned int LED_state;
unsigned int count;

enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
	unsigned long delay_in_ms;
	printk( "my_hrtimer_callback called (%ld).\n", jiffies);
	//LED_state = !LED_state;
	ktime_t ktime;
	if (LED_state == HIGH)
	{
		if (count > 100)
		{
			count = 0;
		}
		
		if (count > 50)
		{
			delay_in_ms = 17L;
			count++;
		}
		else 
		{
			delay_in_ms = 19L;
			count++;
		}
		ktime = ktime_set( 0, MS_TO_NS(delay_in_ms) );
		LED_state = LOW;
	}
	else
	{
		if (count > 50) delay_in_ms = 3L;
		else delay_in_ms = 1L;
		ktime = ktime_set( 0 , MS_TO_NS(delay_in_ms) );
		LED_state = HIGH;
	}
	gpio_set_value(LED, LED_state);
	ktime_t now = hrtimer_cb_get_time( &hr_timer );
	hrtimer_forward( &hr_timer, now, ktime );
	
	//hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );

	return HRTIMER_RESTART;
}

static int __init init_mytimer(void);
static void __exit cleanup_mytimer(void);

/*
 * Function that is called when module is initialized
 */
static int __init init_mytimer(void)
{   
	LED_state = LOW;
	gpio_request(LED, DEV_FILE_NAME);
	gpio_direction_output(LED, LED_state);

	count = 0;

	ktime_t ktime;
	unsigned long delay_in_ms = 19L;

	ktime = ktime_set( 0, MS_TO_NS(delay_in_ms) );

	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );

	hr_timer.function = &my_hrtimer_callback;

	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );

	return 0;
}

/*
 * Function that is called when the module is moved
 */
static void __exit cleanup_mytimer(void)
{
	hrtimer_cancel( &hr_timer );

	gpio_set_value(LED, LOW);
    	gpio_free(LED);
    
    	return;
}

/*
 * Declare the init/exit functions
 */
module_init(init_mytimer);
module_exit(cleanup_mytimer);

/*
 * SOME NOTES:
 *  - The Timer API offers less accuracy that high-resolution
 *    timers.
 *  - The API does a good job of managing a reasonable number of timers.
 *  - Jiffies is used as the granularity of time.
 *
 * Great References:
 *  - The Linux Kernel Module Programming Guide:
 *  	www.tldp.org/LDP/lkmpg.pdf
 *  - Timers and lists in the 2.6 kernel:
 *  	www.ibm.com/developerworks/library/l-timers-list/
 *  - How to Implement Multiple Periodic Timers in Linux Kernel:
 *      qnaplus.com/how-to-implement-multiple-periodic-timers-in-linux-kernel
 *  - FAQ/LinkedLists:
 *      kernelnewbies.org/FAQ/LinkedLists
 *  - User space memory access from the Linux kernel:
 *      www.ibm.com/developerworks/library/l-kernel-memory-access/
 *  - Writing a Linux Kernel Module - Part 2: A Character Device:
 *      derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
 */

