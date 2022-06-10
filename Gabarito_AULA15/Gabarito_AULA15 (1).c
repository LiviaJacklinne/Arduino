/**
 * @file    GABARITO_15.c
 * @author  Diego Anestor Coutinho
 * @date    Mai 2022
 *
 * O algoritmo é uma possível resolução para o(s) exercício(s) avaliativo(s).
 * EM CASO DE DÚVIDA PROCURE POR UM MONITOR, FAREMOS O MELHOR PARA LHE AJUDAR.
 */

/* EXERCÍCIO DO DIA 24/05/2022:

A olimpíada brasileira de minifoguetes está para acontecer. Com isso, os organizadores precisam de um
sistema de ignição para a base de lançamentos. Porém, o "isqueiro na ponta do bambu” não se mostrou
muito eficiente e você foi contratado para criar o sistema de ignição automática para a base de lançamento.
O sistema deverá conter as seguintes funcionalidades:

• ativar o sistema com um botão S1 e mostrar a mensagem “Sistema Ativado”;
• após a ativação do sistema, o computador deverá enviar o tempo de ativação de 1 em 1 segundo até
atingir os 10 segundos;
• ativar um LED vermelho após passar 10 segundos da ativação do sistema;
• aumentar o brilho de um LED verde à medida que a contagem avança, para indicar que o sistema
está prestes a ser acionado, evitando mais um caso “João do isqueiro”.
• interromper o lançamento com o botão S2, que desabilita o contador;
• além do botão S2, o contador também será desabilitado sempre que for enviado o comando
“interromper”

*/

// ======================================== UART ======================================== //

// Variáveis para entrada e saída
char RX_buffer[32];
char RX_index = 0;

// Buffer para estado anterior do RX
char old_rx_hs[32];

// Variáveis para controle do estado do sistema
volatile char system_state = 0;

// A inicialização do UART consiste em definir a taxa de transmissão,
// definir o formato de quadro, e ativar o Transmissor ou o receptor.

// Função de configuração do UART
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

// Função para envio de dados via UART
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

// Limpa o buffer de entrada e saída
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

// Função ISR que salva um array de dados recebidos via UART
ISR(USART_RX_vect)
{
    // Salva o dado recebido
    RX_buffer[RX_index] = UDR0;
    RX_buffer[RX_index + 1] = 0;

    // Adiciona mais 1 na contagem
    RX_index++;

    // Caminha pelo buffer e verifica se a mensagem é "interromper"
    for (int i = 0; i < RX_index; i++)
    {
        // Se a mensagem for "interromper"
        if (RX_buffer[i] == 'i' &&
            RX_buffer[i + 1] == 'n' &&
            RX_buffer[i + 2] == 't' &&
            RX_buffer[i + 3] == 'e' &&
            RX_buffer[i + 4] == 'r' &&
            RX_buffer[i + 5] == 'r' &&
            RX_buffer[i + 6] == 'o' &&
            RX_buffer[i + 7] == 'm' &&
            RX_buffer[i + 8] == 'p' &&
            RX_buffer[i + 9] == 'e' &&
            RX_buffer[i + 10] == 'r')
        {
            UART_send("Contagem parada\n");
            limpa_RX_buffer();
            system_state = 0;
        }
    }
}

// ======================================== INT_0/1 ======================================== //

// Interrupt no botão S1
ISR(INT0_vect)
{
    UART_send("Sistema Ativado\n");
    system_state = 1;
}

// Interrupt no botão S2
ISR(INT1_vect)
{
    UART_send("Contagem parada\n");
    system_state = 0;
}

// ======================================== TIMER0 ======================================== //

// Variáveis de contagem
int cont_v = 0;
int cont_s = 0;

// Interrupt do TIMER0
ISR(TIMER0_COMPA_vect)
{
    cont_v++;

    // A cada 1 segundo
    if (cont_v > 980)
    {
        // Atualiza contadores
        cont_v = 0;
        cont_s += 1;

        // Convertendo para string
        char TX_buffer[32];
        itoa(cont_s, TX_buffer, 10);

        // Envia a string do tempo
        UART_send("Contagem: ");
        UART_send(TX_buffer);
        UART_send("\n");

        // Aumenta D.C. do PWM
        OCR0A += 24;
    }

    // Ao atingir 10s
    if (cont_s == 10)
    {
        // Liga o LED vermelho
        PORTD |= (1 << PD7);

        // Trava a execução
        for (;;);
    }
}

// ======================================== MAIN ======================================== //

int main()
{
    // Saídas PD6 e PD7
    DDRD |= (1 << PD6) | (1 << PD7);

    // Inicialização do UART
    UART_init(9600);

    // Configura o INT0
    EICRA |= (1 << ISC01) | (1 << ISC11);
    EIMSK |= (1 << INT0) | (1 << INT1);

    // PULLUP no PD2 e PD3
    PORTD |= (1 << PD2) | (1 << PD3);

    // Configura TIMER0 para FAST PWM, 1/64
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    TIMSK0 |= (1 << OCIE0A);
    TCCR0A |= (1 << COM0A1);

    // Habilita interrupção global
    sei();

    // Loop infinito
    for (;;)
    {
        // Parado até ativação
        while (!system_state)
            TCCR0B = 0b00000000;

        // Ativa o TIMER0, 1/64
        TCCR0B |= (1 << CS00) | (1 << CS01);
    }
}