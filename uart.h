#include "stm32f4xx.h"

void UART3_Init(void);
void UART3_SendChar(char c);
void UART3_SendString(char *str);
char UART3_ReceiveChar(void);

void UART3_Init(void) {
    // Enable clocks for GPIOD and USART3
    RCC->APB1ENR |= (1 << 18); // USART3 clock enable
    RCC->AHB1ENR |= (1 << 3);  // GPIOD clock enable
    
    // Configure PD8 (TX) and PD9 (RX) as Alternate Function (AF7)
    GPIOD->MODER |= (1 << 17) | (1 << 19); // Set mode to AF
    GPIOD->AFR[1] |= 0x00000077; // Set alternate function to AF7 (USART3)
    
    // Configure USART3
    USART3->BRR = 0x0683; // Baud rate for 9600 bps (assuming 16MHz clock)
    USART3->CR1 |= (1 << 13); // Enable USART3
    USART3->CR1 |= (1 << 2) | (1 << 3); // Enable RX and TX
}

void UART3_SendChar(char c) {
    while (!(USART3->SR & (1 << 7))); // Wait until TXE is set (empty)
    USART3->DR = c;
}

void UART3_SendString(char *str) {
    while (*str) {
        UART3_SendChar(*str++);
    }
}

char UART3_ReceiveChar(void) {
    while (!(USART3->SR & (1 << 5))); // Wait until RXNE (Read Data Register Not Empty) is set
    return USART3->DR;
}
