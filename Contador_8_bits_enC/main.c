/*
 * main.c
 *
 * Created: 9/4/2024 7:56:33 PM
 *  Author: Ovall
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUTTON1_PIN   PC0
#define BUTTON2_PIN   PC1

volatile uint8_t CONTADOR_ADC = 0; //CONTADOR ADC
volatile uint8_t DISPLAY1 = 0; //VALOR DISPLAY 1
volatile uint8_t DISPLAY2 = 0; //VALOR DISPLAY 2
const uint8_t LISTA[] = {0x3F, 0xC, 0x5B, 0x73, 0x66, 0x75, 0x7D, 0x23, 0x7F, 0x67, 0x6F, 0x7C, 0x1D, 0x7A, 0x5D, 0x4D};


// Variable global para almacenar el valor del contador
volatile uint8_t counter = 0;


// Función para inicializar los pines
void init_pins() {
	
	// Configurar los pines PC0 y PC1 como entradas y habilitar resistencias de pull-up
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1) & ~(1 << DDC6);
	PORTC |= (1 << PORTC0) | (1 << PORTC1);
	

	// Pines de salida
	
	DDRD |= (1 << DDD0) | (1 << DDD1) | (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // PD0-PD7 como salidas
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3) | (1 << DDB4) | (1 << DDB5); // 
	DDRC |= (1 << DDC2) | (1 << DDC3) | (1 << DDC4) | (1 << DDC5); // 
	
	
	// Habilitar interrupciones por cambio de estado en PC0 y PC1
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9);

	// Habilitar interrupciones globales
	sei();
	
	// Deshabilitar la funcionalidad de los pines RX y TX (PD0 y PD1)
	UCSR0B = 0x00;
	PORTD = 0;
}

// Función para aumentar el contador
void increment_counter() {
	counter++;
	if (counter == 256) {
		// Pasar a cero
		counter = 0;
	}
}

// Función para decrementar el contador
void decrement_counter() {
	if (counter > 0) {
		counter--;
		} else {
		// Pasar al máximo
		counter = 255;
	}
}

// Función para mostrar el valor del contador en los LEDs
void display_counter() {
	// Apagar todos los LEDs
	
	PORTD &= ~((1 << PIND0) | (1 << PIND1) | (1 << PIND2) | (1 << PIND3) | (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7));
	
	// Mostrar el valor del contador en los LEDs
	PORTD = counter; 
	
}

// Rutina de interrupción
ISR(PCINT1_vect) {	
	// Comprobar si el botón de incremento (PC0) está presionado
	if (!(PINC & (1 << BUTTON1_PIN))) {
		increment_counter();
	}

	// Comprobar si el botón de decremento (PC1) está presionado
	if (!(PINC & (1 << BUTTON2_PIN))) {
		decrement_counter();
	}
}


// Función para inicializar ADC
void init_ADC() {
	// Habilitar ADC y configurar preescalador a 128
	ADMUX = 0;
	ADCSRA = 0;
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// Configurar referencia de voltaje AVCC con capacitor externo en AREF
	ADMUX |= (1 << REFS0)|(1 <<  ADLAR);
	
	// Configurar pin A6 como entrada analógica
	ADMUX |= (1 << MUX2) | (1 << MUX1); // A6 en ADMUX[5:0]

	// Habilitar interrupción de conversión completa del ADC
	ADCSRA |= (1 << ADIE);

}

void Display_ADC() {
	
	PORTC |= (LISTA[DISPLAY1]& 0b1000000)>>4|(PORTC & 0b000011);
	PORTC |= (LISTA[DISPLAY1]& 0b0100000)>>4;
	PORTB |= (LISTA[DISPLAY1]& 0b0010000)>>3;
	PORTB |= (LISTA[DISPLAY1]& 0b0001000)>>1;
	PORTB |= (LISTA[DISPLAY1]& 0b0000100)<<1;
	PORTB |= (LISTA[DISPLAY1]& 0b0000010)<<3;
	PORTB |= (LISTA[DISPLAY1]& 0b0000001);
	
	
}

void Display2_ADC() {
	
	PORTC |= (LISTA[DISPLAY2]& 0b1000000)>>4|(PORTC & 0b000011);
	PORTC |= (LISTA[DISPLAY2]& 0b0100000)>>4;
	PORTB |= (LISTA[DISPLAY2]& 0b0010000)>>3;
	PORTB |= (LISTA[DISPLAY2]& 0b0001000)>>1;
	PORTB |= (LISTA[DISPLAY2]& 0b0000100)<<1;
	PORTB |= (LISTA[DISPLAY2]& 0b0000010)<<3;
	PORTB |= (LISTA[DISPLAY2]& 0b0000001);
	
	
}


ISR(ADC_vect){

	CONTADOR_ADC = ADCH;
	DISPLAY1 = CONTADOR_ADC & 0b00001111;
	DISPLAY2 = CONTADOR_ADC >> 4;

	ADCSRA |= (1<<ADIF);


}

void comparador(){
	if (CONTADOR_ADC>counter)
	{
		PORTB |= (1 << PB0);
		} else{
		PORTB &= ~(1 << PB0);
	}
}


// Función principal
int main() {
	
	// Inicializar los pines
	init_pins();
	init_ADC();

	while (1) {
		display_counter();
		PORTC |= (1<<PC5);
		PORTC &= ~(1<<PC4);
		
		PORTB &= 0b11000001;
		PORTC &= 0b11110011;
		
		Display_ADC();
		_delay_ms(10);
		PORTC |= (1<<PC4);
		PORTC &= ~(1<<PC5);
		
		PORTB &= 0b11000001;
		PORTC &= 0b11110011;
		
		Display2_ADC();
		_delay_ms(10);
		ADCSRA |= (1<<ADSC);
		comparador();
	}

	return 0;
	
}