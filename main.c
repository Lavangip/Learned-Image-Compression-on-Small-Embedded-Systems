#include "stm32f4xx.h"
#include "..\..\..\..\Downloads\model_weights_uint8.h"
#include "uart.h"

#define IMG_SIZE 128
#define KERNEL_SIZE 3
#define OUT_CHANNELS 8

uint8_t input_buffer[IMG_SIZE][IMG_SIZE];
uint8_t output_feature_map[OUT_CHANNELS][IMG_SIZE][IMG_SIZE];

void conv2d_layer(uint8_t input[IMG_SIZE][IMG_SIZE],
                  uint8_t output[OUT_CHANNELS][IMG_SIZE][IMG_SIZE],
                  const uint8_t *weights,
                  const uint8_t *bias);
void UART3_ReceiveImage(void);
void relu_activation(uint8_t feature_map[][IMG_SIZE][IMG_SIZE], int channels);
void UART3_SendOutputFeatureMap(void);

void UART3_ReceiveImage(void) {
    for (int i = 0; i < IMG_SIZE; ++i) {
        for (int j = 0; j < IMG_SIZE; ++j) {
            input_buffer[i][j] = UART3_ReceiveChar();
        }
    }
}

// ReLU: in-place activation
void relu_activation(uint8_t feature_map[][IMG_SIZE][IMG_SIZE], int channels) {
    for (int c = 0; c < channels; ++c) {
        for (int i = 0; i < IMG_SIZE; ++i) {
            for (int j = 0; j < IMG_SIZE; ++j) {
                if (feature_map[c][i][j] < 0)
                    feature_map[c][i][j] = 0;
            }
        }
    }
}

// Simple 1-layer Conv2D: 1 input channel ? N output channels, stride=1, no padding
void conv2d_layer(uint8_t input[IMG_SIZE][IMG_SIZE],
                  uint8_t output[OUT_CHANNELS][IMG_SIZE][IMG_SIZE],
                  const uint8_t *weights,
                  const uint8_t *bias) {
    for (int oc = 0; oc < OUT_CHANNELS; ++oc) {
        for (int i = 1; i < IMG_SIZE - 1; ++i) {
            for (int j = 1; j < IMG_SIZE - 1; ++j) {
                int32_t sum = 0;
                for (int ki = 0; ki < KERNEL_SIZE; ++ki) {
                    for (int kj = 0; kj < KERNEL_SIZE; ++kj) {
                        int ii = i + ki - 1;
                        int jj = j + kj - 1;
                        int w_idx = oc * KERNEL_SIZE * KERNEL_SIZE + ki * KERNEL_SIZE + kj;
                        sum += input[ii][jj] * weights[w_idx];
                    }
                }
                sum += bias[oc];
                sum = sum / 255;
                if (sum < 0) sum = 0;
                if (sum > 255) sum = 255;
                output[oc][i][j] = (uint8_t)sum;
            }
        }
    }
}

// Send output feature map data over UART
void UART3_SendOutputFeatureMap(void) {
    for (int c = 0; c < OUT_CHANNELS; ++c) {
        for (int i = 0; i < IMG_SIZE; ++i) {
            for (int j = 0; j < IMG_SIZE; ++j) {
                UART3_SendChar(output_feature_map[c][i][j]);
            }
        }
    }
}

int main(void) {
    UART3_Init();
    UART3_SendString("STM32 ready to receive image...\r\n");

    UART3_ReceiveImage();  // Receive image from PC over UART

    conv2d_layer(input_buffer, output_feature_map, conv2d_1_weights, conv2d_1_bias);

    UART3_SendString("Sending output...\r\n");
    UART3_SendOutputFeatureMap();  // Send result back

    #define LED_PIN 5
    RCC->AHB1ENR |= (1 << 0);           // GPIOA clock enable
    GPIOA->MODER |= (1 << (2 * LED_PIN)); // Set PA5 as output
    GPIOA->ODR ^= (1 << LED_PIN);       // Toggle PA5 (on-board LED on Nucleo board)
}
