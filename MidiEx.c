#include <io.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <avr/delay.h>

#define REC 	PINA0	//PINA0 = Record Switch
#define PLAY	PINA1	//PINA1 = Playback Switch
#define MOD	PINA2	//PINA2 = Modify Switch
#define PHRES1	PINA7	//PINA7 = Photoresistor 1
#define PHRES2	PINA6	//PINA6 = Photoresistor 2

void USART_Init(unsigned int baud)
{
	UCSRC = 0;
	//Set the baud rate
	UBRRH = (unsigned char)(baud >> 8);
	UBRRL = (unsigned char)(baud);
	UCSRC |= (1 << URSEL);
	//Set the Frame Format: 8 Data| 1 Stop| 0 Parity
	UCSRC |= (3 << UCSZ0);
	UCSRB |= (1 << RXEN) | (1 << TXEN);
	UCSRC &= ~(1 << URSEL);
}

unsigned char USART_Flush(void)
{
	unsigned char flushData;
	//Flush Data from Recieve Register))
	while(UCSRA & (1 << RXC)){
		flushData = UDR;
	}
	PORTB = UDR;
	return flushData;
}

unsigned char USART_Read(void)
{	
	PORTB = 0;
	//Wait for USART to finish recieving
	while(!(UCSRA & (1 << RXC))){
		if(!(PINA & (1 << REC))){
			UCSRB &= ~(1 << RXEN);
			return USART_Flush();
		} 
	}
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
		if(PINA & (1 << REC)){
			//Enable USART Reciever
			UCSRB |= (1 << RXEN);
			//Write to EEPROM Data from USART
			EEPROM_Write(0x0,USART_Read());
			//Disable USART Reciever
			UCSRB &= ~(1 << RXEN); 
		}
		//If Playing
		if(PINA & (1 << PLAY)){
			unsigned int data;
			//Enable USART Transmitter
			UCSRB |= (1 << TXEN);
			//Read Data from EEPROM
			data = EEPROM_Read(0x0);
			//If Modify is on
			if(PINA & (1 << MOD)){
				
			}
			//Transmit Data
			USART_Write(data);
			//Disable USART Transmitter
			UCSRB &= ~(1 << TXEN);
		}
	}
    return 0;
}

int ReadADC(unsigned int ch)
{
	// Reference voltage = AVCC
	ADMUX=(1<<REFS0);

	// Selects prescaler division factor to 32
	ADCSRA=(1<<ADEN)|(5<<ADPS0);

	// Sets port A as input
	DDRA=0;	

	// Selects ADC channel to be pin 7
	ch=PINA7;
	ADMUX|=ch;

	// single conversion
	ADCSRA|=(1<<ADSC);

	// wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	// Clear ADIF by writing 1 into it
	ADCSRA|=(1<<ADIF);

	return (ADC);
}
