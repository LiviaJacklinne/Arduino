/*
  QUESTÃO 1
  DDRD |= 0b00001110; // PD1, PD2, PD3 SAIDA E PD4 ENTRADA
  PORTB |= 0B00001110; // PB1, PB2, PB3 NIVEL LÓGICO ALTO
*/

#define LED0 PIND & 0b0000001 // DEFININDO PD0 COMO LED0
#define LED1 PIND & 0b0000010 // DEFININDO PD1 COMO LED1

#define LED0_ON PORTD |= 0b0000001;
#define LED0_OFF PORTD |= 0b0000000;
#define LED1_ON PORTD |= 0b0000010;
#define LED1_OFF PORTD |= 0b0000000;

void setup(){}

void loop() 
{
  DDRD |= 0b0000011; // DEFININDO PD0 E PD1 COMO SAIDA
  PORTD |= 0b0000000; // INICIALIZANDO PD0 E PD1 EM NIVEL LÓGICO BAIXO
  
  for(;;)
  {
    //LEDS 00
    LED0_OFF; 
    LED1_OFF;
    _delay_ms(500);
    
    //LEDS 01
    LED0_ON;
    LED1_OFF;
    _delay_ms(500);
    
    //LEDS 10
    LED0_OFF;
    LED1_ON 
    _delay_ms(500);
    
    //LEDS 11
    LED0_ON;
    LED1_ON;
    _delay_ms(500);
    
  }

}
