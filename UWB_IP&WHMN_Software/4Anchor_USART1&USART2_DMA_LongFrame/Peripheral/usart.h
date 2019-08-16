#ifndef __USART_H__
#define __USART_H__

#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

#define USART1_REC_LEN      1
#define USART2_REC_LEN  	  1000  		// �����������ֽ��� 200
#define USART1_REC_LEN_DMA  1       // ����USART1_DMAÿ�ν��ջ��͵��ֽ���
#define USART2_REC_LEN_DMA  810      // ����USART2_DMAÿ�ν��ջ��͵��ֽ�����������Ĳ�һ������Ҫ����HAL_UART_Receive_DMA()������

extern void _Error_Handler(char*, int);
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

extern volatile uint8_t USART1_RX_STA;         			   // USART1����״̬���	
extern volatile uint8_t USART2_RX_STA;         			   // USART2����״̬���	
extern uint8_t USART1_RX_BUF[USART1_REC_LEN];  //����1�Ľ��ܻ��壬ʹ��DMA���գ�ÿ��ֻ����һ���ֽ�
extern uint8_t USART2_RX_BUF[USART2_REC_LEN];  //���ջ���,���USART2_REC_LEN���ֽ�.

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

#endif
