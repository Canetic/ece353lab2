#include <io.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <avr/delay.h>

void USART_Init(unsigned int baud)
{
	UCSRC = 0;
	//Set the baud rate
	UBRRH = (unsigned char)(baud >> 8);
	UBRRL = (unsigned char)(baud);
	
	//Enable the Reciever and Transmitter
	UCSRB |= (1 << RXEN) | (1 << TXEN);

	//Set the Frame Format: 8 Data| 1 Stop| 0 Parity
	UCSRC |= (3 << UCSZ0);
	UCSRC &= ~(1 << URSEL);
}

int USART_Read(void)
{
    return 0;
}

int USART_Write(void)
{
    return 0;
}

int EEPROM_Read(void)
{
    return 0;
}

void EEPROM_Write(unsigned int address, unsigned char data)
{
	while (EECR & (1<<EEWE));	//Wait for completion of previous write

	EEAR = address;			//Set up address and data registers
	EEDR = data;

	EECR |= (1<<EEMWE);		//Enable Master Write Enable

	EECR |= (1<<EEWE);		//Enable Write Enable
}

int main(void)
{
    DDRA = 0;                    //Set PortA as input    
    DDRB = 0xFF;                //Set PORTB as output
    DDRD |= (1 << PORTD1);
    TCCR1A |= (1 << CS12);        //Timer1A prescale by 256
    USART_Init(0x7);            //Initialize the USART with Baud Rate 31,250bps
    sei();

    while(1){
        //If Recording
        if(PINA & (1 << PINA0)){
            USART_Read();
            EEPROM_Write();
        }
        //If Playing
        if(PINA & (1 << PINA1)){
            EEPROM_Read();
            //If Modify is on
            if(PINA & (1 << PINA2)){
             
            }
            
            USART_Write();
        }
    }
    return 0;
}
