/*
        ENUNCIADO
    Uma memória FILO é um componente que Bufferiza dados para que sejam transmitidos bit a bit no futuro. 
    Para um valor qualquer armazenado nessa memória, o sistema envia o bit na posição menos significativa da 
    memória, e empurra todo o registro para a direita usando o operador bit shift ">>", e então repete o 
    processo até que toda a informação tenha sido enviada.

    Você tem a tarefa de integrar uma memória FILO em um sistema de transmissão RF, seu processador deverá
    armazenar em uma variavel de 8bits num buffer interno, que deverá ser transmitido bit a bit para o
    circuito Tx do sistema. Para verificar o funcionamento, utilize um LED que indica por 100ms o valor
    do bit a ser transmitido sempre que um botão S1 for pressionado.
    E usando outro botão S2 faça com que o bit seja transmitido, ou seja, ande a fila da memória FILO em 
    uma posição.
*/

// Buffer para armazenar os 8 bits da memória FILO
unsigned char _Buffer = 0b00110100; // 52

// Ao pressionar o botão S2
ISR(INT0_vect)
{
    // Buffer tem seus bits empurrados 1 posição para a direita
    _Buffer = _Buffer >> 1;
    _delay_ms(100);
}

// S1, PD3 (SERVIÇO DE INTERRUPÇÃO)
// Ao pressionar o botão S1
ISR(INT1_vect)
{
    // Filtra o bit 0 do buffer, e verifica se é 1
    if((_Buffer & 0b00000001))
    {
        // Só se for 1, então ativa o LED no PD7 por 100ms
        PORTD |= (1 << PD7);
        _delay_ms(100);
    }

    // Em qualquer caso, desliga o LED no PD7
    PORTD &= ~(1 << PD7);
}

int main()
{
    // Configura o PD7 como saída e coloca PULL-UP no PD2 e PD3
    DDRD |= (1 << PD7); // PINO PD7 COMO SAIDA
    PORTD |= (1 << PD2) | (1 << PD3); // PINOS DE ENTRADA COM PULL UP

    /* Configura o interruptor para executrar ISR (int1, int0)
       sempre que os botões S1 e S2 forem pressionados */
    // CONFIGURANDO INTERRUPÇÃO DE ALTA PRIORIDADE
    EICRA = 0b00001111; // TRANSIÇÃO DE SUBIDA
    EIMSK = 0B00000011; // HABILITANDO OS DOIS BIT DE INTERRUÇÃO

    sei(); // Serviçõ de interrupção

    // SUPER LOOP NÃO FAZ NADA, PORÉM É NECESSÁRIO DECLARAR
    for (;;)
    {
        /* Tudo com interrupção, não precisa executar nada no super-loop  */
    }
}