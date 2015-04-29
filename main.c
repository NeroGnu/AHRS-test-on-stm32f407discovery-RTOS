//******************************************************************************
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "discoveryf4utils.h"
#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "AHRS.h"
//******************************************************************************

//******************************************************************************
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"
//******************************************************************************

xSemaphoreHandle AHRS_Syn;

void vLedBlinkBlue(void *pvParameters);
void vLedBlinkRed(void *pvParameters);
void vLedBlinkGreen(void *pvParameters);
void vLedBlinkOrange(void *pvParameters);
void vGetYaw(void *pvParameters);

#define STACK_SIZE_MIN	128	/* usStackDepth	- the stack size DEFINED IN WORDS.*/

//******************************************************************************
int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f4xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f4xx.c file
	 */
	
	/*!< Most systems default to the wanted configuration, with the noticeable 
		exception of the STM32 driver library. If you are using an STM32 with 
		the STM32 driver library then ensure all the priority bits are assigned 
		to be preempt priority bits by calling 
		NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
	*/
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	Com2Init();
	AHRS_Com5Init();
	STM_EVAL_LEDInit(LED_BLUE);
	STM_EVAL_LEDInit(LED_GREEN);
	STM_EVAL_LEDInit(LED_ORANGE);
	STM_EVAL_LEDInit(LED_RED);
	
	AHRS_Syn = xSemaphoreCreateCounting(1, 0);
	
	xTaskCreate( vLedBlinkBlue, (const signed char*)"Led Blink Task Blue", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vLedBlinkRed, (const signed char*)"Led Blink Task Red", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vLedBlinkGreen, (const signed char*)"Led Blink Task Green", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vLedBlinkOrange, (const signed char*)"Led Blink Task Orange", 
		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( vGetYaw, (const signed char*)"vGetYaw", 
		STACK_SIZE_MIN, NULL, 1, NULL );	
		printf("start!");
	
	vTaskStartScheduler();
}
//******************************************************************************

//******************************************************************************
void vLedBlinkBlue(void *pvParameters)
{
	for(;;)
	{
		STM_EVAL_LEDToggle(LED_BLUE);
		vTaskDelay( 500 / portTICK_RATE_MS );
	}
}

void vLedBlinkRed(void *pvParameters)
{
	for(;;)
	{
		STM_EVAL_LEDToggle(LED_RED);
		vTaskDelay( 750 / portTICK_RATE_MS );
	}
}

void vLedBlinkGreen(void *pvParameters)
{
	for(;;)
	{
		STM_EVAL_LEDToggle(LED_GREEN);
		vTaskDelay( 250 / portTICK_RATE_MS );
	}
}

void vLedBlinkOrange(void *pvParameters)
{
	for(;;)
	{
		STM_EVAL_LEDToggle(LED_ORANGE);
		vTaskDelay( 900 / portTICK_RATE_MS );
	}
}

void vGetYaw(void *pvParameters)
{
	u8 StartCmd[]={0xa5, 0x5a, 0x04, 0x15, 0x19, 0xaa};
	u8* pS = NULL;
	DMA_Cmd(DMA1_Stream0,ENABLE);
	USART_DMACmd(UART5, USART_DMAReq_Rx, ENABLE);
	for (pS = StartCmd; 0xaa != *pS; pS++)
	{
		UART5->SR;
		USART_SendData(UART5, *pS);
		while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET){}
	}
	UART5->SR;
	USART_SendData(UART5, *pS);
	
	for(;;)
	{
		if (pdTRUE == xSemaphoreTake(AHRS_Syn, portMAX_DELAY))
		{
			if (AHRS_Check())
			{
				printf("Yaw=%f\n", AHRS_GetYaw());
			}
		}
	}
	
}
//******************************************************************************
