/* clkinit.c */

/* avr specific */

#include <xinu.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

typedef struct {
	uint8_t tccr2a;
	uint8_t tccr2b;
	uint8_t tcnt2;
	uint8_t ocr2a;
	uint8_t ocr2b;
} volatile timer2_t;

volatile timer2_t *timer2 = (timer2_t *)(0xb0);
volatile unsigned char *timer2_timsk2 = (unsigned char *)(0x70);


uint32	clktime;		/* Seconds since boot			*/
unsigned long  count1000;	/* ms since last clock tick             */
qid16	sleepq;			/* Queue of sleeping processes		*/
unsigned long preempt;		/* Preemption counter			*/

/*------------------------------------------------------------------------
 * clkinit  -  Initialize the clock and sleep queue at startup
 *------------------------------------------------------------------------
 */
void clkinit(void)
{
	
	struct clock_csreg * clockptr;

	sleepq = newqueue();	/* Allocate a queue to hold the delta	*/
				/*   list of sleeping processes		*/
	preempt = QUANTUM;	/* Set the preemption time		*/
	clktime = 0;		/* Start counting seconds		*/
        count1000 = 0;


	/* 
         * AVR atmega328p timer/clock init: interrupt every 1ms 
	 * The AVR TIMER interrupt rutine is in clkhandler.c
         */

	timer2->tccr2a = 0;
	timer2->tccr2b = 0;
	timer2->tcnt2 = 0;
	*timer2_timsk2 = 0;
	
	timer2->tccr2a = 0b00000010;  // "toggle OC2A on match, CTC mode"
        #if ATMEGA     // prescaler 128, ocr2a 125, 1 milisegundo
	timer2->tccr2b = 0b00000101; // TCCR2B |= (1<<CS22) | (0<<CS21) | (1<<CS20);
	timer2->ocr2a = 125;
        #else    // prescaler 32, ocr2a 125, 1 milisegundo
	timer2->tccr2b = 0b00000011; // TCCR2B |= (1<<CS21) | (1<<CS20);
	timer2->ocr2a = 125;
        #endif

	*timer2_timsk2 = 0b00000010;  // TIMSK2 |= (1<<OCIE2A);

	return;
}

