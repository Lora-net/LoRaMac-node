/**
 ******************************************************************************
 * @file      startup_coide.c
 * @author    Coocox
 * @version   V1.0
 * @date      20/07/2010
 * @brief     M0 M3 Devices Startup code.
 *            This module performs:
 *                - Set the initial SP
 *                - Set the vector table entries with the exceptions ISR address
 *                - Initialize data and bss
 *                - Setup the microcontroller system. 			
 *                - Call the application's entry point.
 *            After Reset the Cortex-M0 M3 processor is in Thread mode,
 *            priority is Privileged, and the Stack is set to Main.
 *******************************************************************************
 */
 
 
/*----------Stack Configuration-----------------------------------------------*/  
#define STACK_SIZE       0x00000100      /*!< Stack size (in Words)           */
__attribute__ ((section(".co_stack")))
unsigned long pulStack[STACK_SIZE];      


/*----------Macro definition--------------------------------------------------*/  
#define WEAK __attribute__ ((weak))           


/*----------Declaration of the default fault handlers-------------------------*/  
/* System exception vector handler */
void WEAK  Reset_Handler(void);
void WEAK  NMI_Handler(void);
void WEAK  HardFault_Handler(void);
void WEAK  MemManage_Handler(void);
void WEAK  BusFault_Handler(void);
void WEAK  UsageFault_Handler(void);
void WEAK  SVC_Handler(void);
void WEAK  DebugMon_Handler(void);
void WEAK  PendSV_Handler(void);
void WEAK  SysTick_Handler(void);

void WEAK WWDG_IRQHandler(void);             
void WEAK PVD_IRQHandler(void);              
void WEAK TAMPER_STAMP_IRQHandler(void);     
void WEAK RTC_WKUP_IRQHandler(void);         
void WEAK FLASH_IRQHandler(void);            
void WEAK RCC_IRQHandler(void);              
void WEAK EXTI0_IRQHandler(void);            
void WEAK EXTI1_IRQHandler(void);            
void WEAK EXTI2_IRQHandler(void);            
void WEAK EXTI3_IRQHandler(void);            
void WEAK EXTI4_IRQHandler(void);            
void WEAK DMA1_Channel1_IRQHandler(void);    
void WEAK DMA1_Channel2_IRQHandler(void);    
void WEAK DMA1_Channel3_IRQHandler(void);    
void WEAK DMA1_Channel4_IRQHandler(void);    
void WEAK DMA1_Channel5_IRQHandler(void);    
void WEAK DMA1_Channel6_IRQHandler(void);    
void WEAK DMA1_Channel7_IRQHandler(void);    
void WEAK ADC1_IRQHandler(void);             
void WEAK USB_HP_IRQHandler(void);           
void WEAK USB_LP_IRQHandler(void);           
void WEAK DAC_IRQHandler(void);              
void WEAK COMP_IRQHandler(void);             
void WEAK EXTI9_5_IRQHandler(void);          
void WEAK LCD_IRQHandler(void);              
void WEAK TIM9_IRQHandler(void);             
void WEAK TIM10_IRQHandler(void);            
void WEAK TIM11_IRQHandler(void);            
void WEAK TIM2_IRQHandler(void);             
void WEAK TIM3_IRQHandler(void);             
void WEAK TIM4_IRQHandler(void);             
void WEAK I2C1_EV_IRQHandler(void);          
void WEAK I2C1_ER_IRQHandler(void);          
void WEAK I2C2_EV_IRQHandler(void);          
void WEAK I2C2_ER_IRQHandler(void);          
void WEAK SPI1_IRQHandler(void);             
void WEAK SPI2_IRQHandler(void);             
void WEAK USART1_IRQHandler(void);           
void WEAK USART2_IRQHandler(void);           
void WEAK USART3_IRQHandler(void);           
void WEAK EXTI15_10_IRQHandler(void);        
void WEAK RTC_Alarm_IRQHandler(void);        
void WEAK USB_FS_WKUP_IRQHandler(void);      
void WEAK TIM6_IRQHandler(void);             
void WEAK TIM7_IRQHandler(void);      
void WEAK TIM5_IRQHandler(void);             
void WEAK SPI3_IRQHandler(void);      
void WEAK DMA2_Channel1_IRQHandler(void);    
void WEAK DMA2_Channel2_IRQHandler(void);    
void WEAK DMA2_Channel3_IRQHandler(void);    
void WEAK DMA2_Channel4_IRQHandler(void);    
void WEAK DMA2_Channel5_IRQHandler(void);    
void WEAK AES_IRQHandler(void);              
void WEAK COMP_ACQ_IRQHandler(void);         


/*----------Symbols defined in linker script----------------------------------*/  
extern unsigned long _sidata;    /*!< Start address for the initialization 
                                      values of the .data section.            */
extern unsigned long _sdata;     /*!< Start address for the .data section     */    
extern unsigned long _edata;     /*!< End address for the .data section       */    
extern unsigned long _sbss;      /*!< Start address for the .bss section      */
extern unsigned long _ebss;      /*!< End address for the .bss section        */      
extern void _eram;               /*!< End address for ram                     */


/*----------Function prototypes-----------------------------------------------*/  
extern int main(void);           /*!< The entry point for the application.    */
extern void SystemInit(void);    /*!< Setup the microcontroller system(CMSIS) */
void Default_Reset_Handler(void);   /*!< Default reset handler                */
static void Default_Handler(void);  /*!< Default exception handler            */


/**
  *@brief The minimal vector table for a Cortex M3.  Note that the proper constructs
  *       must be placed on this to ensure that it ends up at physical address
  *       0x00000000.  
  */
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
  /*----------Core Exceptions------------------------------------------------ */
  (void *)&pulStack[STACK_SIZE],       /*!< The initial stack pointer         */
  Reset_Handler,                       /*!< The reset handler                 */
  NMI_Handler,                         /*!< The NMI handler                   */ 
  HardFault_Handler,                   /*!< The hard fault handler            */
  MemManage_Handler,                   /*!< The MPU fault handler             */
  BusFault_Handler,                    /*!< The bus fault handler             */
  UsageFault_Handler,                  /*!< The usage fault handler           */ 
  0,0,0,0,                             /*!< Reserved                          */
  SVC_Handler,                         /*!< SVCall handler                    */
  DebugMon_Handler,                    /*!< Debug monitor handler             */
  0,                                   /*!< Reserved                          */
  PendSV_Handler,                      /*!< The PendSV handler                */
  SysTick_Handler,                     /*!< The SysTick handler               */ 
  
  /*----------External Exceptions---------------------------------------------*/
	WWDG_IRQHandler,             
  PVD_IRQHandler,              
  TAMPER_STAMP_IRQHandler,     
  RTC_WKUP_IRQHandler,         
  FLASH_IRQHandler,            
  RCC_IRQHandler,              
  EXTI0_IRQHandler,            
  EXTI1_IRQHandler,            
  EXTI2_IRQHandler,            
  EXTI3_IRQHandler,            
  EXTI4_IRQHandler,            
  DMA1_Channel1_IRQHandler,    
  DMA1_Channel2_IRQHandler,    
  DMA1_Channel3_IRQHandler,    
  DMA1_Channel4_IRQHandler,    
  DMA1_Channel5_IRQHandler,    
  DMA1_Channel6_IRQHandler,    
  DMA1_Channel7_IRQHandler,    
  ADC1_IRQHandler,             
  USB_HP_IRQHandler,           
  USB_LP_IRQHandler,           
  DAC_IRQHandler,              
  COMP_IRQHandler,             
  EXTI9_5_IRQHandler,          
  LCD_IRQHandler,              
  TIM9_IRQHandler,             
  TIM10_IRQHandler,            
  TIM11_IRQHandler,            
  TIM2_IRQHandler,             
  TIM3_IRQHandler,             
  TIM4_IRQHandler,             
  I2C1_EV_IRQHandler,          
  I2C1_ER_IRQHandler,          
  I2C2_EV_IRQHandler,          
  I2C2_ER_IRQHandler,          
  SPI1_IRQHandler,             
  SPI2_IRQHandler,             
  USART1_IRQHandler,           
  USART2_IRQHandler,           
  USART3_IRQHandler,           
  EXTI15_10_IRQHandler,        
  RTC_Alarm_IRQHandler,        
  USB_FS_WKUP_IRQHandler,      
  TIM6_IRQHandler,             
  TIM7_IRQHandler,      
  TIM5_IRQHandler,             
  SPI3_IRQHandler,      
  DMA2_Channel1_IRQHandler,    
  DMA2_Channel2_IRQHandler,    
  DMA2_Channel3_IRQHandler,    
  DMA2_Channel4_IRQHandler,    
  DMA2_Channel5_IRQHandler,    
  AES_IRQHandler,              
  COMP_ACQ_IRQHandler,         
};


/**
  * @brief  This is the code that gets called when the processor first
  *         starts execution following a reset event. Only the absolutely
  *         necessary set is performed, after which the application
  *         supplied main() routine is called. 
  * @param  None
  * @retval None
  */
void Default_Reset_Handler(void)
{
  /* Initialize data and bss */
  unsigned long *pulSrc, *pulDest;

  /* Copy the data segment initializers from flash to SRAM */
  pulSrc = &_sidata;

  for(pulDest = &_sdata; pulDest < &_edata; )
  {
    *(pulDest++) = *(pulSrc++);
  }
  
  /* Zero fill the bss segment. */
  for(pulDest = &_sbss; pulDest < &_ebss; )
  {
    *(pulDest++) = 0;
  }

  /* Setup the microcontroller system. */
  SystemInit();
	
  /* Call the application's entry point.*/
  main();
}


/**
  *@brief Provide weak aliases for each Exception handler to the Default_Handler. 
  *       As they are weak aliases, any function with the same name will override 
  *       this definition.
  */
  
#pragma weak Reset_Handler = Default_Reset_Handler
#pragma weak NMI_Handler = Default_Handler
#pragma weak HardFault_Handler = Default_Handler
#pragma weak MemManage_Handler = Default_Handler
#pragma weak BusFault_Handler = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak SVC_Handler = Default_Handler
#pragma weak DebugMon_Handler = Default_Handler
#pragma weak PendSV_Handler = Default_Handler
#pragma weak SysTick_Handler = Default_Handler

#pragma weak WWDG_IRQHandler = Default_Reset_Handler             
#pragma weak PVD_IRQHandler = Default_Reset_Handler              
#pragma weak TAMPER_STAMP_IRQHandler = Default_Reset_Handler     
#pragma weak RTC_WKUP_IRQHandler = Default_Reset_Handler         
#pragma weak FLASH_IRQHandler = Default_Reset_Handler            
#pragma weak RCC_IRQHandler = Default_Reset_Handler              
#pragma weak EXTI0_IRQHandler = Default_Reset_Handler            
#pragma weak EXTI1_IRQHandler = Default_Reset_Handler            
#pragma weak EXTI2_IRQHandler = Default_Reset_Handler            
#pragma weak EXTI3_IRQHandler = Default_Reset_Handler            
#pragma weak EXTI4_IRQHandler = Default_Reset_Handler            
#pragma weak DMA1_Channel1_IRQHandler = Default_Reset_Handler    
#pragma weak DMA1_Channel2_IRQHandler = Default_Reset_Handler    
#pragma weak DMA1_Channel3_IRQHandler = Default_Reset_Handler    
#pragma weak DMA1_Channel4_IRQHandler = Default_Reset_Handler    
#pragma weak DMA1_Channel5_IRQHandler = Default_Reset_Handler    
#pragma weak DMA1_Channel6_IRQHandler = Default_Reset_Handler    
#pragma weak DMA1_Channel7_IRQHandler = Default_Reset_Handler    
#pragma weak ADC1_IRQHandler = Default_Reset_Handler             
#pragma weak USB_HP_IRQHandler = Default_Reset_Handler           
#pragma weak USB_LP_IRQHandler = Default_Reset_Handler           
#pragma weak DAC_IRQHandler = Default_Reset_Handler              
#pragma weak COMP_IRQHandler = Default_Reset_Handler             
#pragma weak EXTI9_5_IRQHandler = Default_Reset_Handler          
#pragma weak LCD_IRQHandler = Default_Reset_Handler              
#pragma weak TIM9_IRQHandler = Default_Reset_Handler             
#pragma weak TIM10_IRQHandler = Default_Reset_Handler            
#pragma weak TIM11_IRQHandler = Default_Reset_Handler            
#pragma weak TIM2_IRQHandler = Default_Reset_Handler             
#pragma weak TIM3_IRQHandler = Default_Reset_Handler             
#pragma weak TIM4_IRQHandler = Default_Reset_Handler             
#pragma weak I2C1_EV_IRQHandler = Default_Reset_Handler          
#pragma weak I2C1_ER_IRQHandler = Default_Reset_Handler          
#pragma weak I2C2_EV_IRQHandler = Default_Reset_Handler          
#pragma weak I2C2_ER_IRQHandler = Default_Reset_Handler          
#pragma weak SPI1_IRQHandler = Default_Reset_Handler             
#pragma weak SPI2_IRQHandler = Default_Reset_Handler             
#pragma weak USART1_IRQHandler = Default_Reset_Handler           
#pragma weak USART2_IRQHandler = Default_Reset_Handler           
#pragma weak USART3_IRQHandler = Default_Reset_Handler           
#pragma weak EXTI15_10_IRQHandler = Default_Reset_Handler        
#pragma weak RTC_Alarm_IRQHandler = Default_Reset_Handler        
#pragma weak USB_FS_WKUP_IRQHandler = Default_Reset_Handler      
#pragma weak TIM6_IRQHandler = Default_Reset_Handler             
#pragma weak TIM7_IRQHandler = Default_Reset_Handler      
#pragma weak TIM5_IRQHandler = Default_Reset_Handler             
#pragma weak SPI3_IRQHandler = Default_Reset_Handler      
#pragma weak DMA2_Channel1_IRQHandler = Default_Reset_Handler    
#pragma weak DMA2_Channel2_IRQHandler = Default_Reset_Handler    
#pragma weak DMA2_Channel3_IRQHandler = Default_Reset_Handler    
#pragma weak DMA2_Channel4_IRQHandler = Default_Reset_Handler    
#pragma weak DMA2_Channel5_IRQHandler = Default_Reset_Handler    
#pragma weak AES_IRQHandler = Default_Reset_Handler              
#pragma weak COMP_ACQ_IRQHandler = Default_Reset_Handler         

/**
  * @brief  This is the code that gets called when the processor receives an 
  *         unexpected interrupt.  This simply enters an infinite loop, 
  *         preserving the system state for examination by a debugger.
  * @param  None
  * @retval None  
  */
static void Default_Handler(void) 
{
  /* Go into an infinite loop. */
  while (1) 
  {
  }
}

/*********************** (C) COPYRIGHT 2009 Coocox ************END OF FILE*****/
