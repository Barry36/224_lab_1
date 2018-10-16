/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"

int background()
{
	IOWR(LED_PIO_BASE, 0, 0x01);
	int j;
	int x = 0;
	int grainsize = 4;
	int g_taskProcessed = 0;
	for(j = 0; j < grainsize; j++)
	{
		g_taskProcessed++;
	}
	IOWR(LED_PIO_BASE, 0, 0x00);
	return x;
}

static void ISRfunc(void* context, alt_u32 id){

	IOWR(LED_PIO_BASE, 0, 4);
	IOWR(RESPONSE_OUT_BASE, 1, 0x01);
	IOWR(RESPONSE_OUT_BASE, 0, 0x01);
	IOWR(RESPONSE_OUT_BASE, 0, 0x00);
	IOWR(RESPONSE_OUT_BASE, 1, 0x00);

	//ISR code
	IOWR(STIMULUS_IN_BASE, 3, 0x0);
	IOWR(LED_PIO_BASE, 0, 0);
}
int main()
{
	printf("start of test! \n" );
	alt_u16 period, pulsewidth, missed, latency, x, j , ready, bgtask,multi, first_run, falling_edge, button_value, total_bgtask;
	

	// Identify the interrupt and tight polling using a switch
	if( !IORD(SWITCH_PIO_BASE, 0)){
		printf("This is the Tight polling\n");
		printf("PERIOD,	\tPULSE WIDTH,\tBACKGROUND TASKS,\tAVG LATENCY, MISSED PULSES,\tMULTI\n");
		
		// Wait push button
		while(IORD(BUTTON_PIO_BASE, 0) != 14){}
		for (x = 2; x <= 5000; x += 2){

			// Turn led 1
			IOWR(LED_PIO_BASE, 0, 2);
			total_bgtask = 0;
			bgtask = 0;
			first_run = 0;
			falling_edge = 0;
			ready = 0;
			period = x;
			pulsewidth = x/2;
			IOWR(EGM_BASE, 2, period);
			IOWR(EGM_BASE, 3, pulsewidth);
			IOWR(EGM_BASE, 0, 1);
			while(IORD(EGM_BASE, 1)){
				if(IORD(STIMULUS_IN_BASE, 0))
				{
					IOWR(RESPONSE_OUT_BASE, 1, 0x01);
					IOWR(RESPONSE_OUT_BASE, 0, 0x01);
					IOWR(RESPONSE_OUT_BASE, 0, 0x00);
					IOWR(RESPONSE_OUT_BASE, 1, 0x00);
					ready = 1;
				}
				if(IORD(STIMULUS_IN_BASE, 0) == 0){
					falling_edge = 1;
				}

				// If EGM is busy then break
				if(IORD(EGM_BASE, 1) == 0){
					break;
				}
				if(ready == 1){
					if(first_run == 0){
						if(IORD(EGM_BASE, 1) == 0){
							break;
						}
						while(falling_edge != 1 || IORD(STIMULUS_IN_BASE, 0) != 1){
							if(IORD(STIMULUS_IN_BASE, 0) == 0){
								falling_edge = 1;
							}
							background();
							total_bgtask++;
							bgtask++;

							// If EGM is busy then break
							if(IORD(EGM_BASE, 1) == 0){
								break;
							}
						}
						if(bgtask == 1){
							bgtask = 2;
						}
						first_run = 1;
					}
					else{
						if(IORD(EGM_BASE, 1) == 0){
							break;
						}
						for (j = 0; j < bgtask - 1; j++){
							background();
							total_bgtask++;
							if(IORD(EGM_BASE, 1) == 0){
								break;
							}
						}
					}
				}
				ready = 0;
			}
			missed = IORD(EGM_BASE, 5);
			latency = IORD(EGM_BASE, 4);
			multi = IORD(EGM_BASE, 6);
			printf("%d,	\t%d,\t\t%d,\t\t%d, \t\t%d, \t\t%d\n", period, pulsewidth, total_bgtask, latency, missed, multi);
			IOWR(EGM_BASE, 0, 0);
		}
		
	}
	else{
		printf("This is the: Interrupt\n");
		printf("PERIOD,	\tPULSE WIDTH,\tBACKGROUND TASKS,\tAVG LATENCY,	MISSED PULSES,\tMULTI\n");

		// Wait push button
		while(IORD(BUTTON_PIO_BASE, 0) != 14){}

		IOWR(STIMULUS_IN_BASE, 2, 0x1);
		alt_irq_register( STIMULUS_IN_IRQ, (void *)0, ISRfunc);
		for (x = 2; x <= 5000; x+=2){
			IOWR(LED_PIO_BASE, 0, 2);
			period = x;
			pulsewidth = x/2;
			total_bgtask = 0;
			IOWR(EGM_BASE, 2, period);
			IOWR(EGM_BASE, 3, pulsewidth);
			IOWR(EGM_BASE, 0, 1);
			while(IORD(EGM_BASE, 1)){
				background();
				total_bgtask++;
			}
			missed = IORD(EGM_BASE, 5);
			latency = IORD(EGM_BASE, 4);
			multi = IORD(EGM_BASE, 6);
			printf("%d, \t%d,\t\t%d,\t\t%d, \t\t%d, \t\t%d\n", period, pulsewidth, total_bgtask, latency, missed, multi);
			IOWR(EGM_BASE, 0, 0);
		}

	}
  printf("end of test! \n" );
  return 0;
}





