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

	//Set the Frame Format: 8 Data| 1 Stop| 0 Parity
	UCSRC |= (3 << UCSZ0);
	UCSRC &= ~(1 << URSEL);
}

void USART_Flush(void)
{
	unsigned char flushData;
	//Flush Data from Recieve Register))
	while(!(UCSRA & (1 << UDRE))){
		flushData = UDR;
	} 
}

unsigned char USART_Read(void)
{
	//Wait for USART to finish recieving
	while(!(UCSRA & (1 << RXC)));
	//Return data when done
	return UDR;

}

void USART_Write(unsigned char data)
{
	//Wait for the Transmit Buffer to empty
	while(!(UCSRA & (1 << UDRE)));
	//Move the Data into the Transmit Buffer
	UDR = data;

}

unsigned char EEPROM_Read(unsigned int address)
{
	while(EECR & (1<<EEWE));	//Wait for completion of previous write
	EEAR = address; 		//Set up address register
	EECR |= (1<<EERE);		//Start EEPROM read
	return EEDR;			//Return data
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
			//Enable USART Reciever
			UCSRB |= (1 << RXEN);
			//Write to EEPROM Data from USART
			EEPROM_Write(USART_Read());
			//Disable USART Reciever
			UCSRB &= ~(1 << RXEN); 
		}
		//If Playing
		if(PINA & (1 << PINA1)){
			unsigned int data;
			//Enable USART Transmitter
			UCSRB |= (1 << TXEN);
			//Read Data from EEPROM
			data = EEPROM_Read();
			//If Modify is on
			if(PINA & (1 << PINA2)){
				
			}
			//Transmit Data
			USART_Write(data);
			//Disable USART Transmitter
			UCSRB &= ~(1 << TXEN);
		}
	}
    return 0;
}
