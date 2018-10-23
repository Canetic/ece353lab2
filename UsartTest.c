#include <io.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <util/delay.h>

#define REC 	PINA0		//PINA0 = Record Switch
#define PLAY	PINA1		//PINA1 = Playback Switch
#define F_CLK 	4000000
#define BAUD	31250
#define UBRR	(F_CLK/16/BAUD)-1


ISR(TIMER1_COMPA_vect)	//Interrupt for TCNT1=OCR1A=0.8ms
{
	PORTB = 0;		//Turn LEDS off
}

USART_Init()
{
	UCSRC = 0;
	UCSRB = (1 << RXEN)|(1 << TXEN);
	UCSRC |= (1 << URSEL)|(3 << UCSZ0);
	UBRRH &= ~(1 <<URSEL);

	UBRRH |= (unsigned char)(UBRR >> 8);
	UBRRL |= (unsigned char)(UBRR);
	//UCSRB |= (1 << RXCIE);
}

unsigned char USART_Read(void)
{
	UCSRB |= (1 << RXEN);
	while(!(UCSRA & (1 << RXC)));

	return UDR;
}

void USART_Write(unsigned char data)
{
	UCSRB |= (1 << TXEN);
	//Wait for the Transmit Buffer to empty
	while(!(UCSRA & (1 << UDRE)));
	//Move the Data into the Transmit Buffer
	UDR = data;

}

int main(void)
{
	DDRB = 0xFF;
	DDRA = 0;
	DDRD |= (1 << PORTD1);
	USART_Init();
	TCCR1B |= (1 << CS12);
	TIMSK |= (1 << OCIE1A);	//Enable TIMER1_COMPA interrupt
	
	OCR1A = 0x30D4;			//Comparison A (800ms)
	
	sei();
	
	while(1){
		while(PINA & (1 << REC)){
			USART_Read();
			PORTB = USART_Read();
			USART_Read();
		}
		while(PINA & (1 << PLAY)){
			USART_Write(0x64);
			USART_Write(0x90);
			USART_Write(0x45);
			PORTB = 0x45;
			_delay_ms(1000);
			USART_Write(0x40);
			USART_Write(0x80);
			USART_Write(0x45);
			PORTB = 0x45;
		}
	}

	return 0;
}
