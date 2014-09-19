/* RT-Kernel */
#include <ee.h>
#include <cpu/pic32/inc/ee_utils.h>
#include "mcu/microchip_pic32/inc/ee_timer.h"
/* console */
#include <stdio.h>
#include <stdarg.h>
#include "console_serial.h"
#include "utils.h"

/* Other libraries */
#include "util.h"
#include "mainLaptop.h"

#pragma config POSCMOD = XT
#pragma config FNOSC = PRIPLL
#pragma config FPLLMUL = MUL_20
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLODIV = DIV_1
//#pragma config FPBDIV = DIV_1	/* SYSCLK = 80MHz, PBCLK = 80MHz */
#pragma config FPBDIV = DIV_2 	/* SYSCLK = 80MHz, PBCLK = 40MHz */
//#pragma config FPBDIV = DIV_8 /* SYSCLK = 80MHz, PBCLK = 10MHz */
#pragma config FWDTEN = OFF
#pragma config FMIIEN = OFF, FETHIO = ON	// external PHY in RMII/default configuration
#pragma config FSOSCEN = OFF

#define ENABLE_1V2_VOLTAGE() 	ODCCbits.ODCC14 = 0;\
								TRISCbits.TRISC14 = 0;\
								LATCbits.LATC14 = 1;

/* Variables for UDP communication */

/* Variables and macros for console */
console_descriptor_t *my_console_1;



int main(void)
{

	ENABLE_1V2_VOLTAGE();
    EE_system_init();
    EE_leds_init();

    /* Initialize Console */
    my_console_1 = console_serial_config(MY_FIRST_SERIAL, UART_BAUDRATE,
    					CONSOLE_SERIAL_FLAG_BIT8_NO |
    					CONSOLE_SERIAL_FLAG_BIT_STOP1);
    console_init(MY_FIRST_CONSOLE, my_console_1);
    if (console_open(MY_FIRST_CONSOLE) < 0) {
    	for (;;);
    }


    myprintf("dal device\n");
    char testtest[8];
    int asdasd = myread(testtest, 8);
    myprintf("ricevuto %d bytes ", asdasd);
    mywrite(testtest, 8);
    myprintf("\n");
    while(1){
    	mainLaptop(0, NULL);
    }
}

/******************************************************************************/
/* General Excepiton Handler: overrides the default handler                   */
/******************************************************************************/
static enum {
	EXCEP_IRQ = 0,	// interrupt
	EXCEP_AdEL = 4,	// address error exception (load or ifetch)
	EXCEP_AdES,	// address error exception (store)
	EXCEP_IBE,	// bus error (ifetch)
	EXCEP_DBE,	// bus error (load/store)
	EXCEP_Sys,	// syscall
	EXCEP_Bp,	// breakpoint
	EXCEP_RI,	// reserved instruction
	EXCEP_CpU,	// coprocessor unusable
	EXCEP_Overflow,	// arithmetic overflow
	EXCEP_Trap,	// trap (possible divide by zero)
	EXCEP_IS1 = 16,	// implementation specfic 1
	EXCEP_CEU,	// CorExtend Unuseable
	EXCEP_C2E	// coprocessor 2
} cp0_exception_code;

static unsigned int cp0_exception_cause; // CP0: CAUSE register
static unsigned int cp0_exception_epc;   // CP0: Exception Program Counter

void _general_exception_handler(void)
{
	asm volatile("mfc0 %0,$14" : "=r" (cp0_exception_epc));
	asm volatile("mfc0 %0,$13" : "=r" (cp0_exception_cause));
	cp0_exception_code = (cp0_exception_cause >> 2) & 0x0000001F;
	EE_led_toggle(0);
	EE_led_toggle(1);
	while (1) ;
}


