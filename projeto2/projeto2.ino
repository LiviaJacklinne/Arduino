#define botao_on !(PIND & 0b10000000)
char liga;
// ============================ UART ============================ //

// Variáveis para entrada e saída (padrão)
char RX_buffer[32];
char RX_index = 0;

// Buffer para estado anterior do RX (PADRÃO)
char old_rx_hs[32];

// Variáveis para controle do estado do sistema (PADRÃO)
volatile char system_state = 0;

// A inicialização do UART consiste em definir a taxa de transmissão,
// definir o formato de quadro, e ativar o Transmissor ou o receptor.

// PADRÃO - Função de configuração do UART
void UART_init(int baud)
{
  // Calcula a taxa de transmissão
  int MYUBRR = 16000000 / 16 / baud - 1;

  // Definindo a taxa de transmissão
  UBRR0H = (unsigned char)(MYUBRR >> 8);
  UBRR0L = (unsigned char)(MYUBRR);

  // Definindo o formato de quadro, 8 bits e 1 stop bit
  UCSR0C = (0 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);

  // Ativa o Transmissor, receptor e define a interrupção
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

// PADRÃO - Função para envio de dados via UART
void UART_send(char *TX_buffer)
{
  // Enquanto tiver caracteres para enviar
  while (*TX_buffer != 0)
  {
    // Prepara o buffer para o envio
    UDR0 = *TX_buffer;

    // Espera o envio ser completado
    while (!(UCSR0A & (1 << UDRE0)));

    // Avança o ponteiro do buffer
    TX_buffer++;
  }
}

// PADRÃO - Limpa o buffer de entrada e saída
void limpa_RX_buffer(void)
{
  unsigned char dummy;

  // Enquanto houver dados no buffer
  while (UCSR0A & (1 << RXC0))
    dummy = UDR0;


  // Reseta o índice
  RX_index = 0;

  // Limpa todos os dados do buffer
  for (int i = 0; i < 32; i++)
  {
    old_rx_hs[i] = RX_buffer[i];
    RX_buffer[i] = 0;
  }
}

// EDITAR - Função ISR que salva um array de dados recebidos via UART
ISR(USART_RX_vect)
{
  // Salva o dado recebido
  RX_buffer[RX_index] = UDR0;
  RX_buffer[RX_index + 1] = 0;

  // Adiciona mais 1 na contagem
  RX_index++;

  // EDITAR

  // Se a mensagem for "L" o sistema ta ativo
  if (RX_buffer[0] == 'L' | RX_buffer[0] == 49)
  {
    UART_send("Sistema Ligado\n");
    system_state = 1;

    // Ativando o botão
    DDRD |= (1 << PD7);

    liga = '1';
    UART_send("liga: ");
    UART_send(liga);
  }

  limpa_RX_buffer();

}


// ============================ INTERRUPÇÃO ============================ //

// EDITAR
// Interrupt no botão S1 = INT0 (PD2)
ISR(INT0_vect)
{
  // Quando o botão for pressionado, siginifica que o sistema parou
  UART_send("Sistema Parado\n");
  system_state = 0;

  // Desativando o botão
  DDRD &= (0 << PD7);

  liga = "0";
  UART_send("liga: ");
  UART_send(liga);
  

}

// ============================ PWM ============================ //

// Função para configuração do ADC
void ADC_init ()
{
  // Configura Vref para VCC = 5V
  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

}

// Função para leitura do pino do ADC
int ADC_read (unsigned char pino)
{

  // Seleciona o pino de entrada
  ADMUX |= (pino & 0b00000111);

  // Pega a média das amostras
  int valor = 0;
  int valor_aux;

  // Realiza 8 leituras
  for (unsigned char i = 0; i < 8; i++) {

    // Inicia a conversão
    ADCSRA |= (1 << ADSC);

    // Aguarda o fim da conversão
    while (!ADCSRA & (1 << ADIF));

    // Pega o valor da conversão
    valor_aux = ADCL;
    valor_aux += ADCH << 8;

    // Soma o valor da amostra
    valor += valor_aux;
  }

  // Retorna a média de 8 leituras
  return valor >> 3;
}


// ============================ MAIN ============================ //

// EDITAR
int main()
{
  // Configura o INT0
  EICRA |= (1 << ISC01) | (0 << ISC00); // INTERRUPÇÃO NA TRANSIÇÃO DE DESCIDA
  EIMSK |= (1 << INT0); // INTERRUPÇÃO INT0

  // Configura TIMER0 para FAST PWM, 1/64
  TCCR0A |= (1 << WGM01) | (1 << WGM00);
  TIMSK0 |= (1 << OCIE0A);
  TCCR0A |= (1 << COM0A1);

  // Saída PD5 (LED)
  DDRD |= (1 << PD5);

  // PULLUP no PD2 (INT0)
  PORTD |= (1 << PD2);

  // PULLUP no PD7 (BOTAO)
  PORTD |= (1 << PD7);

  // Inicialização do UART
  UART_init(9600);

  // Inicialização ADC
  ADC_init ();

  // Habilita interrupção global
  sei();

  float peso;
  float velocidade;
  int valor1; // Variavel que recebe a leitura do ADC_read
  char vetor[10]; // vetor usado na conversão ITOA PESO
  char vect[10]; // vetor usado na conversão ITOA VELOCIDADE
  
  // Loop infinito
  for (;;)
  {
    UART_send("Liga: ");
   // UART_send(liga);
    UART_send("\n");
    _delay_ms(500);
    
    if (liga == "1")
    {
      // Botão está pressionado ?
      if (botao_on)
      {
        UART_send ("Sistema Desligado\n");
        system_state = 0;
        liga = "0";
      }

      valor1 = ADC_read(ADC0D); // Lendo PWM
      peso = (valor1 * 10) / 1023.0; // Regra de 3, Peso
      UART_send("Peso: ");

      // PESO = valor lido + VETOR = Recebe a msg + 10 = Conversão de base decimal
      itoa(peso, vetor, 10);
      UART_send(vetor);
      UART_send(" kg\n");
      _delay_ms(600);


      velocidade = ((peso * 255) / 10);
      OCR0A = velocidade;
      itoa(velocidade, vect, 10);
      UART_send("Velocidade: ");
      UART_send(vect);
      UART_send("\n");
      _delay_ms(600);
    }


  }
}
