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
#define UBRR	(F_CLK/16/BAUD)-1	//Calcuate UBRR Value

unsigned int writeAddr, readAddr, recording, interval;
uint8_t status, note, vel;
float modLight, ambLight;

ISR(TIMER1_COMPA_vect)	//Interrupt for TCNT1=OCR1A=0.8ms
{
	PORTB = 0;		//Turn LEDS off
}

ISR(TIMER1_COMPB_vect)	//Interrupt for TCNT1=OCR1B=4s
{
	recording = 0;		//Exit record mode
	TCNT1 = 0;			//Reset TIMER1
}

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

void USART_Flush(void)
{
	unsigned char flushData;
	//Flush Data from Recieve Register))
	while(UCSRA & (1 << RXC)){
		flushData = UDR;
	}
	
}

unsigned char USART_Read(void)
{
	//Wait for the recieve to complete
	while(!(UCSRA & (1 << RXC))&&(PINA &(1<<REC)));
	//Return what was recieved
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

int ReadADC(unsigned int ch)
{
	// Reference voltage = AVCC
	ADMUX=(1<<REFS0);

	// Selects prescaler division factor to 32
	ADCSRA=(1<<ADEN)|(5<<ADPS0);

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

void record(void)
{
	USART_Flush();
	writeAddr = 0;
	status = USART_Read();		//Read initial note to begin the recording
	note = USART_Read();
	vel = USART_Read();
	EEPROM_Write(writeAddr, status);	//writing into EEPROM status, note, vel 
	EEPROM_Write(writeAddr+1, note);
	EEPROM_Write(writeAddr+2, vel);
	TCNT1 = 0;				//Reset Clock
	PORTB = note;				//Display note on LED
	writeAddr = 3;				//Incrementing Index

	recording = 1;
	while((writeAddr < 0x3FD)&&(PINA & (1 << REC))&&recording)						//1kB of memory
	{
		status = USART_Read();	//read status, note, and velocity frames
		note = USART_Read();
		vel = USART_Read();		
		interval = TCNT1;
		EEPROM_Write(writeAddr, (interval>>8));		//write upper half of interval
		EEPROM_Write(writeAddr+1, interval);			//write lower half
		EEPROM_Write(writeAddr+2, status);
		EEPROM_Write(writeAddr+3, note);
		EEPROM_Write(writeAddr+4, vel);
		PORTB = note;								//display note on LED
		writeAddr+=5;								//increment index
		TCNT1 = 0;									//reset clock
	}
	EEPROM_Write(writeAddr-3, 0xFF);		//To have 3 0XFF to determine when to loop recording
	EEPROM_Write(writeAddr-2, 0xFF);
	EEPROM_Write(writeAddr-1, 0xFF);
}

void playback()
{
	readAddr = 0;
	//when play is turned on 
	while(PINA & (1 << PLAY))
	{
		//determine if mod will be on or not
		if(PINA & (1 << MOD))
		{
			modLight = ReadADC(PHRES1);	//if mod is on then constantly get new modlight values for the delay_ms
			modLight *= modLight;		//Modlight is squared to increase the speed difference when shing and lowering light levels
		} else {
			// if mod is off
			modLight = ambLight;
		}

		status = EEPROM_Read(readAddr);		//read from eeprom STATUS
		note = EEPROM_Read(readAddr+1);		//read from eeprom DATA1
		vel = EEPROM_Read(readAddr+2);		//read from eeprom DATA2
		interval = (EEPROM_Read(readAddr+3)<<8)+EEPROM_Read(readAddr+4); //calculating the interval between notes
		if(status & note & vel){break;}			//stop EEPROM read at the end of recording
		USART_Write(status);					//transmit to midiox
		USART_Write(note);				//transmit to midiox
		USART_Write(vel);				//transmit to midiox
		PORTB = note;					//LED reading note from EEPROM DATA1
		_delay_ms(((float) interval)/15.0*ambLight/modLight);			//0x0 to 0xF4 maps from 0 to 4s
		readAddr += 5;					//Index for the playback
	}
}

int main(void)
{
    DDRA = 0;                    //Set PortA as input    
    DDRB = 0xFF;                //Set PORTB as output
    DDRD |= (1 << PORTD1);	// Setting PortD1 as output
    TCCR1B |= (1 << CS12);        //Timer1A prescale by 256
    USART_Init();            //Initialize the USART with Baud Rate 31,250bps
    ////Timer Interrupt Setup////
	TIMSK |= (1 << OCIE1A);	//Enable TIMER1_COMPA interrupt
	TIMSK |= (1 << OCIE1B);	//Enable TIMER1_COMPB interrupt
	OCR1A = 0x30D4;			//Comparison A (800ms)
	OCR1B = 0xF424;			//Comparison B (4s)
	sei();				//Initialize interrupt function
	ambLight = ReadADC(PHRES1);	//Initialize the current light value for the delay_ms algorithm
	ambLight *= ambLight;		//Square ambLight to get a bigger difference when shining and lowering light levels
	
    while(1){
		//Record Mode
		if(PINA & (1 << REC)){
			record();
		}
		//Playback Mode
		if(PINA & (1 << PLAY)){
			playback();
		}
		
	}
    return 0;
}
