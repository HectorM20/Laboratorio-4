//***********************************************************************************************
// Universidad del Valle de Guatemala
// IE2023: Programación de Microcontroladores
// Autor: Héctor Martínez - 22369
// Proyecto: Laboratorio 4
// Hardware: ATmega328p
//***********************************************************************************************

#define F_CPU 16000000
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t counter = 0;
volatile uint8_t adcL = 0;
volatile uint8_t adcH = 0;

const uint8_t tabla[] = {0x3F, 0x28, 0x5D, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, 0x77, 0x2C, 0x39, 0x5E, 0x57, 0x17};

void setup(void);
void initADC(void);
void alarmaLED(void);
int main(void)
{
	// Configuración
	setup();

void setup(void)
{
	//Puerto D como salida para el contador y displays
	DDRD = 0xFF;
	UCSR0B = 0;//Pines RX y TX como pines digitales
	PORTD =0;
	
	//Pines PC0 y PC1 como entrada (resistencias pull-up)
	DDRC &= ~((1 << DDC0) | (1 << DDC1));
	PORTC |= (1 << PORTC0) | (1 << PORTC1);
	
	//Interrupciones para pulsadores, pines PC0 y PC1
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9);
	//Pines PC2 y PC3 como salida para la multiplexación de los displays
	DDRC |= (1 << DDC2) | (1 << DDC3);

	//Pin PB2 como salida para la multiplexación de las LEDs del contador
	DDRB |= (1 << DDB2)|(1 << DDB1);
}

void initADC(void)
{
	ADMUX = 0;
	//Se establece ADC6 como entrada analógica
	ADMUX |= (1 << MUX2) | (1 << MUX1);

	//Referencia de voltaje AVCC
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1); 
		
	//Justificación izquierda
	ADMUX |= (1 << ADLAR);
	ADCSRA = 0;
	
	//Configura la prescaler a 16M/128
	ADCSRA |= (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	//Habilitar ADC
	ADCSRA |= (1 << ADEN);
}

ISR(ADC_vect){
	//Almacenar valor de ADC
	uint16_t ADC_Val = ADC;
	uint8_t adcVal = (adc_val16 >> 8);
	
	//Separar el valor en 4 bits del ADC, apagar bandera de interrupción
	adcH = ADC_Val >> 4;
	adc_valueL = adcVal & 0x0F;
	ADCSRA |= (1<<ADSC);	
}

	while (1)
	{
										//Multiplexación del contador binario
		PORTB |= (1 << PORTB2);			//Enciende las LEDs del contador
		PORTD = counter;
		_delay_ms(15);					//Breve retardo para evitar parpadeos
		PORTB &= ~(1 << PORTB2);		//Apaga las LEDs del contador
		_delay_ms(25);
		
		ADCSRA |= (1<<ADSC);
										// Multiplexación de displays
		PORTC |= (1 << PORTC3);			//Enciende el segundo display
		PORTD = tabla[adc_valueL];		//Muestra valores
		_delay_ms(10);

		PORTC &= ~(1 << PORTC3);		//Apaga el segundo display
		PORTC |= (1 << PORTC2);			//Enciende el primer display
		PORTD = tabla[adcH];
		PORTC &= ~(1 << PORTC2);		//Apaga el primer display
		_delay_ms(10);

		
		alarmaLED();
	}
	return 0;
}




void alarmaLED(){
	if (ADCH>counter)
	{
		PORTB |= (1<<PORTB1);
	}
	else
	{
		PORTB &= ~(1<<PORTB1);
	}
	
}
ISR(PCINT1_vect){
	{
		if(((PINC) & (1<<0)) == 0){
			counter ++;
			if(counter >= 255){
				counter = 255;
			}	
		}
		
		if(((PINC) & (1<<1)) == 0){
			counter --;
			if(counter <= 0){
				counter = 0;
			}
			
		}
		
	}
}

