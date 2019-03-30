/*
 * mytimer.c - Smart Lock Module for Linux Gumstix
 */

#include <linux/module.h>			/* Needed by all modules 		  */
#include <linux/kernel.h>			/* Needed for KERN_INFO 		  */
#include <linux/init.h>				/* Needed for init/exit Macros  */
#include <linux/timer.h>			/* Needed for Timer API 		  */
#include <linux/proc_fs.h>			/* Needed for /proc File System */
#include <asm/uaccess.h>			/* Needed for "current" 	     */
#include "gpio.h"					/* Needed for GPIO control 	  */
#include "interrupt.h"				/* Needed for Interrupts        */

#define DEBUG_LOCK (0)

/* Pin Setup */
#define SEC 1000
#define SCL 117		// I2C-4
#define SDA 118		// I2C-2
#define LED 16		// I2C-5  :- For Lock Status
#define LED2 17		// I2C-6  :- For Incorrect Passcode
#define GREEN 101	// I2C-7  :- Pin connected to Green Led
#define BTN1 28		// AC97-2 :- Buttons for Manual Unlock + Lock
#define BTN2 29		// AC97-3
#define BTN3 30		// AC97-5
#define BTN4 31		// AC97-8
#define LOW 0
#define HIGH 1
#define PER 100
#define DEV_FILE_NAME "smartlock"
#define NUM_BITS 8

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Bobhate");
MODULE_DESCRIPTION("Module to control electric lock");

static unsigned int locked = 1;
static unsigned int btn1PrevVal = 0;
static const unsigned int passcode[4] = { 3, 2, 1, 4 };
static unsigned int inputBuffer[4];
static unsigned int index = 0;

static struct timer_list blinkTimer;
static void BlinkCallback(unsigned long);
static unsigned int blinkCount = 0;
static struct timer_list unlockTimer;
static void UnlockCallback(unsigned long);

/*
 * Function Declarations
 */
static int __init init_smartlock(void);
static void __exit cleanup_smartlock(void);
static irqreturn_t ClkInterruptHandler(int irq, void *dev_id, struct pt_regs *regs);
static irqreturn_t Btn1InterruptHandler(int irq, void *dev_id, struct pt_regs *regs);
static irqreturn_t Btn2InterruptHandler(int irq, void *dev_id, struct pt_regs *regs);
static irqreturn_t Btn3InterruptHandler(int irq, void *dev_id, struct pt_regs *regs);
static irqreturn_t Btn4InterruptHandler(int irq, void *dev_id, struct pt_regs *regs);
void Authenticate(void);
void Blink(void);

/*
 * Function that is called when module is initialized
 */
static int __init init_smartlock(void)
{
	 // Request GPIO pins
	 gpio_request(SCL, DEV_FILE_NAME);
	 gpio_request(SDA, DEV_FILE_NAME);
	 gpio_request(LED, DEV_FILE_NAME);
	 gpio_request(LED2, DEV_FILE_NAME);
	 gpio_request(GREEN, DEV_FILE_NAME);
	 gpio_request(BTN1, DEV_FILE_NAME);
	 gpio_request(BTN2, DEV_FILE_NAME);
	 gpio_request(BTN3, DEV_FILE_NAME);
	 gpio_request(BTN4, DEV_FILE_NAME);
	 
	 // Set GPIO pin direction
	 gpio_direction_input(SCL);
	 gpio_direction_input(SDA);
	 gpio_direction_output(LED, LOW);
	 gpio_direction_output(LED2, LOW);
	 gpio_direction_output(GREEN, HIGH);
	 gpio_set_value(GREEN, HIGH);
	 gpio_direction_input(BTN1);
	 gpio_direction_input(BTN2);
	 gpio_direction_input(BTN3);
	 gpio_direction_input(BTN4);
	 
	 // Setup Handlers
	 request_irq(gpio_to_irq(SCL), (irq_handler_t) ClkInterruptHandler, IRQF_TRIGGER_RISING, DEV_FILE_NAME, NULL);
	 request_irq(gpio_to_irq(BTN1), (irq_handler_t) Btn1InterruptHandler, IRQF_TRIGGER_RISING, DEV_FILE_NAME, NULL);
	 request_irq(gpio_to_irq(BTN2), (irq_handler_t) Btn2InterruptHandler, IRQF_TRIGGER_RISING, DEV_FILE_NAME, NULL);
	 request_irq(gpio_to_irq(BTN3), (irq_handler_t) Btn3InterruptHandler, IRQF_TRIGGER_RISING, DEV_FILE_NAME, NULL);
	 request_irq(gpio_to_irq(BTN4), (irq_handler_t) Btn4InterruptHandler, IRQF_TRIGGER_RISING, DEV_FILE_NAME, NULL);

	 // Setup timers
	 setup_timer( &unlockTimer, UnlockCallback, 0 );
	 mod_timer( &unlockTimer, jiffies + msecs_to_jiffies(50) );

    return 0;
}

/*
 * Function that is called when the module is removed
 */
static void __exit cleanup_smartlock(void)
{
	del_timer( &unlockTimer );

	free_irq(gpio_to_irq(SCL), NULL);
	free_irq(gpio_to_irq(BTN1), NULL);
	free_irq(gpio_to_irq(BTN2), NULL);
	free_irq(gpio_to_irq(BTN3), NULL);
	free_irq(gpio_to_irq(BTN4), NULL);

    gpio_set_value(LED, LOW);
    gpio_set_value(LED2, LOW);
    gpio_set_value(GREEN, LOW);
    	
	gpio_free(SCL);
    gpio_free(SDA);
    gpio_free(LED);
    gpio_free(LED2);
    gpio_free(GREEN);
    gpio_free(BTN1);
    gpio_free(BTN2);
    gpio_free(BTN3);
    gpio_free(BTN4);
 
    return;
}

static void UnlockCallback(unsigned long data)
{
		unsigned int btn1Val = gpio_get_value(BTN1);
		if (btn1Val != LOW && btn1PrevVal != LOW)
		{
			gpio_set_value(LED, HIGH);
			gpio_set_value(GREEN, LOW);
			locked = 1;
			btn1PrevVal = HIGH;
		}
		btn1PrevVal = btn1Val;
		
		mod_timer( &unlockTimer, jiffies + msecs_to_jiffies(50) );
}

/*
 * Read data on rising edge of clk
 */
static irqreturn_t ClkInterruptHandler(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int data = gpio_get_value(SDA);
	
#if DEBUG_LOCK
	printk(KERN_INFO "Clk Handler\n");
#endif

	if (data != LOW) 
	{
#if DEBUG_LOCK
		printk(KERN_INFO "Unlock Signal Recieved\n");
#endif
		gpio_set_value(LED, LOW);
		gpio_set_value(GREEN, HIGH);
		locked = 0;
	}
	
	return IRQ_HANDLED;
}

static irqreturn_t Btn1InterruptHandler(int irq, void *dev_id, struct pt_regs *regs)
{	
#if DEBUG_LOCK
	printk(KERN_INFO "Btn1 Handler\n");
#endif

	if (index != 4) 
	{
		inputBuffer[index] = 1;
		index++;
		if (index == 4) 
		{
			Authenticate();
		}
	}
	
	return IRQ_HANDLED;
}

static irqreturn_t Btn2InterruptHandler(int irq, void *dev_id, struct pt_regs *regs)
{
#if DEBUG_LOCK
	printk(KERN_INFO "Btn2 Handler\n");
#endif

	if (index != 4) 
	{
		inputBuffer[index] = 2;
		index++;
		if (index == 4) 
		{
			Authenticate();
		}
	}
	
	return IRQ_HANDLED;
}

static irqreturn_t Btn3InterruptHandler(int irq, void *dev_id, struct pt_regs *regs)
{
#if DEBUG_LOCK
	printk(KERN_INFO "Btn3 Handler\n");
#endif	

	if (index != 4) 
	{
		inputBuffer[index] = 3;
		index++;
		if (index == 4) 
		{
			Authenticate();
		}
	}
		
	return IRQ_HANDLED;
}

static irqreturn_t Btn4InterruptHandler(int irq, void *dev_id, struct pt_regs *regs)
{
#if DEBUG_LOCK
	printk(KERN_INFO "Btn4 Handler\n");
#endif

	if (index != 4) 
	{
		inputBuffer[index] = 4;
		index++;
		if (index == 4) 
		{
			Authenticate();
		}
	}
	
	return IRQ_HANDLED;
}

void Authenticate(void)
{
	int ii;
	index = 0;
	for (ii = 0; ii < 4; ii++)
	{
		if (passcode[ii] != inputBuffer[ii]) 
		{
#if DEBUG_LOCK
			printk(KERN_INFO "Wrong Passcode\n");
#endif
			Blink();
			return;
		}
		inputBuffer[ii] = 0;
	}
	
	// Authenticated
#if DEBUG_LOCK
	printk(KERN_INFO "Authenticated\n");
#endif
	gpio_set_value(LED, LOW);
	gpio_set_value(GREEN, HIGH);
	locked = 0;
}

void Blink(void)
{
	blinkCount = 0;
	setup_timer( &blinkTimer, BlinkCallback, 0 );
	mod_timer( &blinkTimer, jiffies + msecs_to_jiffies(PER) );
}

static void BlinkCallback(unsigned long data)
{
	if (gpio_get_value(LED2) == LOW)
	{
		gpio_set_value(LED2, HIGH);
	}
	else
	{
		gpio_set_value(LED2, LOW);
		blinkCount++;
	}
	
	if (blinkCount >= 3)
	{
		blinkCount = 0;
		del_timer( &blinkTimer );
	}
	else
	{
		mod_timer( &blinkTimer, jiffies + msecs_to_jiffies(PER) );
	}
}

/*
 * Declare the init/exit functions
 */
module_init(init_smartlock);
module_exit(cleanup_smartlock);

/*
 * SOME NOTES:
 *  - 
 *
 * Great References:
 *  - 
 */
