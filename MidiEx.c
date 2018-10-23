#include <io.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <util/delay.h>

#define REC 	PINA0	//PINA0 = Record Switch
#define PLAY	PINA1	//PINA1 = Playback Switch
#define MOD	PINA2	//PINA2 = Modify Switch
#define PHRES1	PINA7	//PINA7 = Photoresistor 1
#define PHRES2	PINA6	//PINA6 = Photoresistor 2
#define F_CLK 	4000000			//Define Clock Speed
#define BAUD	31250			//Define Baud Rate
#define UBRR	(F_CLK/16/BAUD)-1	//Cacluate UBRR Value

void USART_Init()
{
	UCSRC = 0;
	//Enable the Transmitter and Reciever
	UCSRB = (1 << RXEN)|(1 << TXEN);
	
	//Select the UCSRC register
	//Set the Frame to 8 bits| 0 parity| 1 stop bit
	UCSRC |= (1 << URSEL)|(3 << UCSZ0);

	//Select the UBRRH register
	UBRRH &= ~(1 <<URSEL);
	
	//Set the Baud Rate registers
	UBRRH |= (unsigned char)((UBRR) >> 8);
	UBRRL |= (unsigned char)(UBRR);
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
	//Wait for the recieve to complete
	while(!(UCSRA & (1 << RXC))){
		if(!(PINA & (1 << REC))){
			UCSRB &= ~(1 << RXEN);
			return USART_Flush();
		} 
	}
	//Return what was recieved
	return UDR;
}

void USART_Write(unsigned char data)
{
	//Wait for the Transmit Buffer to empty
	while(!(UCSRA & (1 << UDRE)))
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

int ReadADC(unsigned int ch)
{
	// Reference voltage = AVCC
	ADMUX=(1<<REFS0);

	// Selects prescaler division factor to 32
	ADCSRA=(1<<ADEN)|(7<<ADPS0);

	// Selects ADC channel
	ADMUX|=ch;

	// single conversion
	ADCSRA|=(1<<ADSC);

	// wait for conversion to complete
	while(!(ADCSRA & (1 << ADIF)));

	// Clear ADIF by writing 1 into it
	ADCSRA|=(1<<ADIF);

	return (ADC);
}

int main(void)
{
    DDRA = 0;                    //Set PortA as input    
    DDRB = 0xFF;                //Set PORTB as output
    DDRD |= (1 << PORTD1);
    TCCR1B |= (1 << CS12);        //Timer1A prescale by 256
    USART_Init();            //Initialize the USART with Baud Rate 31,250bps
    sei();
	unsigned char data;
	unsigned int writeAddr, readAddr;
	
    while(1){
		//Record Mode
		if(PINA & (1 << REC)){
			writeAddr = 0;
			//Prevent Playback from overriding Record
			while(PINA & (1 << REC)){
				
			}
			
		}
	    	readAddr = 0;
		//Prevent Record from overriding Playback
		while(PINA & (1 << PLAY)){
			
			//Modify Mode
			if(PINA & (1 << MOD)){
				
			}
			readAddr = (readAddr > writeAddr) ? 0 : readAddr+1;
		}
		
	}
    return 0;
}
