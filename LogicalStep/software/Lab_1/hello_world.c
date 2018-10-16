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
	IOWR(LED_PIO_BASE, 0, 0x04);

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

	alt_u16 period, num_task_run, pulsewidth, missed, latency, multi , ready, first_ran, falling_edge;




	if (IORD(SWITCH_PIO_BASE, 0)) {

		while (IORD(BUTTON_PIO_BASE, 0) != 14) {}

		printf("This is the polling !!\n");
		int i, j;

		for (i = 2; i <= 5000; i+=2){

			num_task_run = 0;
			first_ran = 0;
			falling_edge = 0;
			ready = 0;

			period = i;
			pulsewidth = i/2;
			IOWR(LED_PIO_BASE, 0, 0x0002);
			// Set up EGM
			IOWR(EGM_BASE, 2, period);
			IOWR(EGM_BASE, 3, pulsewidth);
			IOWR(EGM_BASE, 0, 1);

			// Turn on led_1

			// Turn off led_1
			// IOWR(LED_PIO_BASE, 0, 0x0000);


		// When EGM is busy, do tight polling
			while(IORD(EGM_BASE, 1)){


				// If stimulus is high, at one, we do response
				if(IORD(STIMULUS_IN_BASE, 0))

				{

					IOWR(RESPONSE_OUT_BASE, 1, 0x01);
					IOWR(RESPONSE_OUT_BASE, 0, 0x01);
					IOWR(RESPONSE_OUT_BASE, 0, 0x00);
					IOWR(RESPONSE_OUT_BASE, 1, 0x00);
					ready = 1;

				}

				// If Stimulus is at the bottom, we encountered a falling edge already
				if(IORD(STIMULUS_IN_BASE, 0) == 0){
					falling_edge = 1;
				}

				if (!IORD(EGM_BASE, 1)){
					break;
				}

				//
				if(ready == 1){

					// Characterization if first run, otherwise, we do run background task (n-1) times
					if(first_ran == 0){

						// run background task when we don't encounter a falling edge OR the stimulus is zero
						while (falling_edge != 1 || IORD(STIMULUS_IN_BASE, 0) == 0) {
							if (!IORD(EGM_BASE, 1)){
								break;
							}

							// set falling_edge flag to one if encounter a falling edge
							if(IORD(STIMULUS_IN_BASE, 0) == 0){
								falling_edge = 1;
							}

							background();
							num_task_run++;
						}
						first_ran = 1;
					}
					else{
						for (j = 0; j < num_task_run - 1; j++){
							background();
						}
					}
				}

				ready = 0;

			}

			multi = IORD(EGM_BASE, 6);
			missed = IORD(EGM_BASE, 5);
			latency = IORD(EGM_BASE, 4);

			printf("period\tpulsewidth\t\tmissed\tlatency\tmulti\tbgtasks\n");
			printf("%d\t%d\t\t%d\t%d\t%d\t%d\n", period, pulsewidth, missed, latency, multi, num_task_run);
			IOWR(EGM_BASE, 0, 0);


		}

		printf("end of test! \n" );

	} else {
		printf("This is the interrupt!!\n");
		while (IORD(BUTTON_PIO_BASE, 0) != 14) {}
		IOWR(STIMULUS_IN_BASE, 2, 0x1);
		alt_irq_register( STIMULUS_IN_IRQ, (void *)0, ISRfunc);

		int i;
		for (i = 2; i <= 5000; i+=2){

			// Turn on led_1
			IOWR(LED_PIO_BASE, 0, 2);
			IOWR(LED_PIO_BASE, 0, 0);

			num_task_run = 0;
			period = i;
			pulsewidth = i/2;

			// Set up EGM
			IOWR(EGM_BASE, 2, period);
			IOWR(EGM_BASE, 3, pulsewidth);

			IOWR(EGM_BASE, 0, 1);


			while(IORD(EGM_BASE, 1)){
				IOWR(LED_PIO_BASE, 0, 0x01);
				background();
				num_task_run++;
				IOWR(LED_PIO_BASE, 0, 0x00);
			}

			multi = IORD(EGM_BASE, 6);
			missed = IORD(EGM_BASE, 5);
			latency = IORD(EGM_BASE, 4);

			printf("period\tpulsewidth\t\tmissed\tlatency\tmulti\tbgtasks\n");
			printf("%d\t%d\t\t%d\t%d\t%d\t%d\n", period, pulsewidth, missed, latency, multi, num_task_run);
			IOWR(EGM_BASE, 0, 0);

		}
		printf("end of test! \n" );
	}

	return 0;

}
