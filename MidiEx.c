#include <io.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <avr/delay.h>

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

int EEPROM_Write(void)
{
    return 0;
}

int main(void)
{
    DDRA = 0;                    //Set PortA as input    
    DDRB = 0xFF;                //Set PORTB as output
    DDRD |= (1 << PORTD1);
    TCCR1A |= (1 << CS12);        //Timer1A prescale by 256
    TIMSK |= (1 << TOIE1);
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
