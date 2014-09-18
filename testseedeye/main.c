/* ###*B*###
 * ERIKA Enterprise - a tiny RTOS for small microcontrollers
 *
 * Copyright (C) 2011  Evidence Srl
 *
 * This file is part of ERIKA Enterprise.
 *
 * ERIKA Enterprise is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation, 
 * (with a special exception described below).
 *
 * Linking this code statically or dynamically with other modules is
 * making a combined work based on this code.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this code with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this code, you may extend
 * this exception to your version of the code, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * ERIKA Enterprise is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with ERIKA Enterprise; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 * ###*E*### */
 
 /** 
    @file      main.c
    @author    Dario Di Stefano
    @date      2011
*/

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


