/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * LPS node firmware.
 *
 * Copyright 2016, Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dwOps.h"

#include <string.h>

#include <stm32f4xx_hal.h>
#include "spi.h"
#include "led.h"

extern SPI_HandleTypeDef hspi1;
extern volatile uint8_t flag_pressKey;

//#define DEBUG_SPI

#define DWM_IRQn EXTI0_IRQn     // dw1000���ж����IRQ/GPIO8������STM32��PB0�������ж�����0
#define DWM_IRQ_PIN GPIO_PIN_0  // PB0


static dwDevice_t *dev;

/*****************************************************************************************
// Initialize interrupts
******************************************************************************************/
void dwOpsInit(dwDevice_t *device)
{
  dev = device;

	HAL_NVIC_SetPriority(DWM_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DWM_IRQn);    //ʹ��PB0�ⲿ�ж�
}

/*****************************************************************************************
//����Ƿ����жϵ���
******************************************************************************************/
static int checkIrq()
{
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
}

/*****************************************************************************************
//PB0���жϻص�����
******************************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin) {
    case DWM_IRQ_PIN:
      do{
          dwHandleInterrupt(dev);
      } while(checkIrq() != 0); //while IRS line active (ARM can only do edge sensitive interrupts)
      HAL_NVIC_ClearPendingIRQ(DWM_IRQn);
      break;
    case GPIO_PIN_9:
      ledOn(ledUser1);
		  flag_pressKey = 1;
	  	HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
      break;
    default:
      break;
  }
}

// Aligned buffer of 128bytes
// This is used as a "scratch" buffer to the SPI transfers
// The problem is that the Cortex-m0 only supports 2Bytes-aligned memory access

uint8_t alignedBuffer[1024];

/*****************************************************************************************
//
******************************************************************************************/
static void spiWrite(dwDevice_t* dev, const void *header, size_t headerLength,
                                      const void* data, size_t dataLength)
{
#ifdef DEBUG_SPI
  int i;
  printf("Write to SPI: [ ");
  for (i=0; i<headerLength; i++)
    printf("%02x ", (unsigned int)((uint8_t*)header)[i]);
  printf("] [ ");

  for (i=0; i<dataLength; i++)
    printf("%02x ", (unsigned int)((uint8_t*)data)[i]);
  printf("]\r\n");
#endif

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); //����SPI��CSSƬѡ��
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  memcpy(alignedBuffer, header, headerLength);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)alignedBuffer, headerLength, HAL_MAX_DELAY);
  memcpy(alignedBuffer, data, dataLength);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)alignedBuffer, dataLength, HAL_MAX_DELAY);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); //��д��ɺ�����SPI��CSSƬѡ��
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
}

/*****************************************************************************************
//
******************************************************************************************/
static void spiRead(dwDevice_t* dev, const void *header, size_t headerLength,
                                     void* data, size_t dataLength)
{
  // volatile int dummy;

#ifdef DEBUG_SPI
  int i;
  printf("Read from SPI: [ ");
  for (i=0; i<headerLength; i++)
    printf("%02x ", (unsigned int)((uint8_t*)header)[i]);
  printf("] ");
#endif

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  memcpy(alignedBuffer, header, headerLength);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)alignedBuffer, headerLength, HAL_MAX_DELAY);
  // printf("%lu", hspi1.Instance->SR & SPI_FLAG_RXNE);
  // while (hspi1.Instance->SR & SPI_FLAG_RXNE) {
  //   printf("R");
  //   dummy = hspi1.Instance->DR;
  // }
  // dummy;
  // printf("%lu ", hspi1.Instance->SR & SPI_FLAG_RXNE);
  HAL_SPI_Receive(&hspi1, (uint8_t *)alignedBuffer, dataLength, HAL_MAX_DELAY);
  memcpy(data, alignedBuffer, dataLength);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);

#ifdef DEBUG_SPI
  printf("{ ");
  for (i=0; i<dataLength; i++)
    printf("%02x ", (unsigned int)((uint8_t*)data)[i]);
  printf("}\r\n");
#endif
}

/*****************************************************************************************
//
******************************************************************************************/
static void spiSetSpeed(dwDevice_t* dev, dwSpiSpeed_t speed)
{
  if(speed == dwSpiSpeedLow) {
    MX_SPI1_Init();
  } else {
    MX_SPI1_Init_Fast();
  }
}

/*****************************************************************************************
//
******************************************************************************************/
static void delayms(dwDevice_t* dev, unsigned int delay)
{
  HAL_Delay(delay);
}

/************************************************************************************/
dwOps_t dwOps = {
  .spiRead = spiRead,
  .spiWrite = spiWrite,
  .spiSetSpeed = spiSetSpeed,
  .delayms = delayms,
};
