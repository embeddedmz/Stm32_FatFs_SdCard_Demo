/*
 ******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 8.0.0   2017-09-29

The MIT License (MIT)
Copyright (c) 2009-2017 Atollic AB

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 ******************************************************************************
 */

/* Includes */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_sdio_sd.h"
#include "ff.h"

/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */
SD_Error Status = SD_OK;

FATFS filesystem;    /* volume lable */

FRESULT ret;           /* Result code */

FIL file;                /* File object */

DIR dir;              /* Directory object */

FILINFO fno;           /* File information object */

UINT bw, br;

uint8_t buff[128];

/* Private function prototypes -----------------------------------------------*/
static void Delay(__IO uint32_t nCount);
static void fault_err (FRESULT rc);
void debugInit(uint32_t baudrate);

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void)
{
   int i = 0;

   /**
    *  IMPORTANT NOTE!
    *  The symbol VECT_TAB_SRAM needs to be defined when building the project
    *  if code has been located to RAM and interrupts are used.
    *  Otherwise the interrupt table located in flash will be used.
    *  See also the <system_*.c> file and how the SystemInit() function updates
    *  SCB->VTOR register.
    *  E.g.  SCB->VTOR = 0x20000000;
    */

   /* TODO - Add your application code here */
   /* Initialize LEDs */
   STM_EVAL_LEDInit(LED3);
   STM_EVAL_LEDInit(LED4);
   STM_EVAL_LEDInit(LED5);
   STM_EVAL_LEDInit(LED6);

   /* Turn on LEDs */
   //STM_EVAL_LEDOn(LED3);
   //STM_EVAL_LEDOn(LED4);
   //STM_EVAL_LEDOn(LED5);
   //STM_EVAL_LEDOn(LED6);

   //Initialize user button
   STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

   debugInit(9600);

   fprintf(stdout, "** SD Card - FATFS - Rs232 Demo by embeddedmz**\r\n");
   fprintf(stdout, "** Compiled on %s - %s\r\n\r\n", __DATE__, __TIME__);

   /* mount the filesystem */
   if (f_mount(0, &filesystem) != FR_OK) {
     printf("could not open filesystem \n\r");
   }
   Delay(10);

   printf("Open a test file (fontaine.txt) \n\r");
   ret = f_open(&file, "fontaine.txt", FA_READ);
   if (ret) {
     printf("the test file doesn't exist (fontaine.txt)\n\r");
   } else {
     printf("The file content : \n\r");
     /*for (;;) {
       ret = f_read(&file, buff, sizeof(buff), &br);   // Read a chunk of file
       if (ret || !br) {
         break;       //Error or end of file
       }
       buff[br] = 0;
       printf("%s",buff);
       printf("\n\r");
     }*/
     while (f_gets(buff , sizeof(buff) , &file) != NULL)
     {
        puts (buff);
     }
     if (ret) {
       printf("File reading error\n\r");
       fault_err(ret);
     }

     printf("Closing the file\n\r");
     ret = f_close(&file);
     if (ret) {
       printf("Error while closing the file\n\r");
     }
   }

   /*  hello.txt write test*/
#ifdef LAZY
   Delay(50);
   printf("Create a new file (hello.txt)\n\r");
   ret = f_open(&file, "HELLO.TXT", FA_WRITE | FA_CREATE_ALWAYS);
   if (ret) {
     printf("Create a new file error\n\r");
     fault_err(ret);
   } else {
     printf("Write a text data. (hello.txt)\n\r");
     ret = f_write(&file, "Hello world!", 14, &bw);
     if (ret) {
       printf("Write a text data to file error\n\r");
     } else {
       printf("%u bytes written\n\r", bw);
     }
     Delay(50);
     printf("Close the file\n\r");
     ret = f_close(&file);
     if (ret) {
       printf("Close the hello.txt file error\n\r");
     }
   }

   /*  hello.txt read test*/
   Delay(50);
   printf("read the file (hello.txt)\n\r");
   ret = f_open(&file, "HELLO.TXT", FA_READ);
   if (ret) {
     printf("open hello.txt file error\n\r");
   } else {
     printf("Type the file content(hello.txt)\n\r");
     for (;;) {
       ret = f_read(&file, buff, sizeof(buff), &br);   /* Read a chunk of file */
       if (ret || !br) {
         break;       /* Error or end of file */
       }
       buff[br] = 0;
       printf("%s",buff);
       printf("\n\r");
     }
     if (ret) {
       printf("Read file (hello.txt) error\n\r");
       fault_err(ret);
     }

     printf("Close the file (hello.txt)\n\r");
     ret = f_close(&file);
     if (ret) {
       printf("Close the file (hello.txt) error\n\r");
     }
   }

   /*  directory display test*/
   Delay(50);
   printf("Open root directory\n\r");
   ret = f_opendir(&dir, "");
   if (ret) {
     printf("Open root directory error\n\r");
   } else {
     printf("Directory listing...\n\r");
     for (;;) {
       ret = f_readdir(&dir, &fno);     /* Read a directory item */
       if (ret || !fno.fname[0]) {
         break; /* Error or end of dir */
       }
       if (fno.fattrib & AM_DIR) {
         printf("  <dir>  %s\n\r", fno.fname);
       } else {
         printf("%8lu  %s\n\r", fno.fsize, fno.fname);
       }
     }
     if (ret) {
       printf("Read a directory error\n\r");
       fault_err(ret);
     }
   }
   Delay(50);
   printf("Test completed\n\r");
#endif

   /* Infinite loop */
   while (1) {
     STM_EVAL_LEDToggle(LED3);
     Delay(100);
   }
}


/*
 * Callback used by stm32f4_discovery_audio_codec.c.
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
   /* TODO, implement your code here */
   return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
   /* TODO, implement your code here */
   return -1;
}

/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   USART_InitTypeDef USART_InitStructure;

   //Enable GPIOC clock
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
   //Enable USART6 clock
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

   //Configure USART6_TX (PC6) and USART6_RX (PC7)
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);

   //Remap USART6_TX and USART6_RX pins to PC6 and PC7
   GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
   GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

   //Configure USART6
   USART_InitStructure.USART_BaudRate = baudrate;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(USART6, &USART_InitStructure);

   //Enable USART6
   USART_Cmd(USART6, ENABLE);
}

/**
 * @brief   FatFs err dispose
 * @param  None
 * @retval None
*/
static void fault_err (FRESULT rc)
{
  const char *str =
                    "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
                    "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
                    "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
                    "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
  FRESULT i;

  for (i = (FRESULT)0; i != rc && *str; i++) {
    while (*str++) ;
  }
  printf("rc=%u FR_%s\n\r", (UINT)rc, str);
  STM_EVAL_LEDOn(LED6);
  while(1);
}

/**
  * @brief  Delay
  * @param  None
  * @retval None
  */
static void Delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0;
  for (index = (100000 * nCount); index != 0; index--);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif
