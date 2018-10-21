#include <io.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <avr/delay.h>

#define REC 	PINA0		//PINA0 = Record Switch
#define F_CPU 	4000000
#define BAUD	31250
#define UBRR	(F_CPU/16/BAUD)-1

ISR(USART_RXC_vect)
{
	PORTB = UDR;
	_delay_ms(500);
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
	PORTB = 0xFF;
	while(!(UCSRA & (1 << RXC)));

	return UDR;
}

int main(void)
{
	DDRB = 0xFF;
	DDRA = 0;
	DDRD |= (1 << PORTD1);
	USART_Init();
	sei();
	
	while(1){
		PORTB = USART_Read();
		_delay_ms(500);
		PORTB = 0;
	}

	return 0;
}
