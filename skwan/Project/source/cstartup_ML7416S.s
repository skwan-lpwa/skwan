;/**************************************************
; *
; * Part one of the system initialization code, contains low-level
; * initialization, plain thumb variant.
; *
; * @note
; * Copyright (C) 2016 LAPIS Semiconductor Co., Ltd.  All rights reserved.
; *
; * @par
; * This software is provided "as is" and any expressed or implied warranties,
; * including, but not limited to, the implied warranties of merchantability
; * and fitness for a particular purpose are disclaimed.
; * LAPIS Semiconductor shall not be liable for any direct, indirect,
; * consequential or incidental damages arising from using or modifying
; * this software.
; * You can modify and use this software in whole or part on
; * your own responsibility, only for the purpose of developing the software
; * for use with microcontroller manufactured by LAPIS Semiconductor.
; *
; * @version  1.00
; * @date     25 Feb 2016
; *
; **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start

        PUBLIC  __vector_table
        DATA
__vector_table
        DCD     sfe(CSTACK)                 ; Top of Stack
        DCD     __iar_program_start         ; Reset Handler
        DCD     NMI_Handler                 ; NMI Handler
        DCD     HardFault_Handler           ; Hard Fault Handler
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     SVC_Handler                 ; SVCall Handler
        DCD     0                           ; Reserved
        DCD     0                           ; Reserved
        DCD     PendSV_Handler              ; PendSV Handler
        DCD     SysTick_Handler             ; SysTick Handler

        ; External Interrupts
                DCD     WDT_IRQHandler                 ;  0:WDT
                DCD     Default_Handler                ;  1: Reserved
                DCD     GPIO0_IRQHandler               ;  2:GPIO0
                DCD     TMRA_TIMER1_IRQHandler         ;  3:TMRA_TIMER1
                DCD     GPIO1_IRQHandler               ;  4:GPIO1
                DCD     RTC_IRQHandler                 ;  5:RTC
                DCD     TMRB_TIMER1_IRQHandler         ;  6:TMRB_TIMER1
                DCD     TMRC_TIMER1_IRQHandler         ;  7:TMRC_TIMER1
                DCD     FTMA_FTM0_IRQHandler           ;  8:FTMA_FTM0
                DCD     GPIO2_IRQHandler               ;  9:GPIO2
                DCD     UART0_IRQHandler               ;  10:UART0
                DCD     SSIS0_IRQHandler               ;  11:SSIS0
                DCD     ADC_CNT0_IRQHandler            ;  12:ADC_CNT0
                DCD     AES_IRQHandler                 ;  13:AES
                DCD     UART1_IRQHandler               ;  14:UART1
                DCD     UART2_IRQHandler               ;  15:UART2
                DCD     TMRD_TIMER1_IRQHandler         ;  16:TMRD_TIMER1
                DCD     FLASH_CNT0_IRQHandler          ;  17:FLASH_CNT0
                DCD     TMRE_TIMER1_IRQHandler         ;  18:TMRE_TIMER1
                DCD     EXTTMRA_TIMER_IRQHandler       ;  19:EXTTMRA_TIMER
                DCD     I2C0_IRQHandler                ;  20:I2C0
                DCD     DMAC0_IRQHandler               ;  21:DMAC0
                DCD     SPI0_IRQHandler                ;  22:SPI0
                DCD     SPI1_IRQHandler                ;  23:SPI1
                DCD     FDMAC0_IRQHandler              ;  24:FDMAC0
                DCD     FLASH_CNT1_IRQHandler          ;  25:FLASH_CNT1
                DCD     SPI2_IRQHandler                ;  26:SPI2
                DCD     DIO0_IRQHandler                ;  27:DIO0
                DCD     LVD_CNT0_IRQHandler            ;  28:LVD_CNT0
                DCD     RF_IRQHandler                  ;  29:RF
                DCD     CLK_TIMER0_IRQHandler          ;  30:CLK_TIMER0
                DCD     MODE0_IRQHandler               ;  31:MODE0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        SECTION .text:CODE:REORDER:NOROOT(1)

        PUBWEAK NMI_Handler
        PUBWEAK HardFault_Handler
        PUBWEAK SVC_Handler
        PUBWEAK PendSV_Handler
        PUBWEAK SysTick_Handler

        PUBWEAK WDT_IRQHandler
        PUBWEAK GPIO0_IRQHandler
        PUBWEAK TMRA_TIMER1_IRQHandler
        PUBWEAK GPIO1_IRQHandler
        PUBWEAK RTC_IRQHandler
        PUBWEAK TMRB_TIMER1_IRQHandler
        PUBWEAK TMRC_TIMER1_IRQHandler
        PUBWEAK FTMA_FTM0_IRQHandler
        PUBWEAK GPIO2_IRQHandler
        PUBWEAK UART0_IRQHandler
        PUBWEAK SSIS0_IRQHandler
        PUBWEAK ADC_CNT0_IRQHandler
        PUBWEAK AES_IRQHandler
        PUBWEAK UART1_IRQHandler
        PUBWEAK UART2_IRQHandler
        PUBWEAK TMRD_TIMER1_IRQHandler
        PUBWEAK FLASH_CNT0_IRQHandler
        PUBWEAK TMRE_TIMER1_IRQHandler
        PUBWEAK EXTTMRA_TIMER_IRQHandler
        PUBWEAK I2C0_IRQHandler
        PUBWEAK DMAC0_IRQHandler
        PUBWEAK SPI0_IRQHandler
        PUBWEAK SPI1_IRQHandler
        PUBWEAK FDMAC0_IRQHandler
        PUBWEAK FLASH_CNT1_IRQHandler
        PUBWEAK SPI2_IRQHandler
        PUBWEAK DIO0_IRQHandler
        PUBWEAK LVD_CNT0_IRQHandler
        PUBWEAK RF_IRQHandler
        PUBWEAK CLK_TIMER0_IRQHandler
        PUBWEAK MODE0_IRQHandler
        PUBWEAK Default_Handler

NMI_Handler:
HardFault_Handler:
SVC_Handler:
PendSV_Handler:
SysTick_Handler:
WDT_IRQHandler:
GPIO0_IRQHandler:
TMRA_TIMER1_IRQHandler:
GPIO1_IRQHandler:
RTC_IRQHandler:
TMRB_TIMER1_IRQHandler:
TMRC_TIMER1_IRQHandler:
FTMA_FTM0_IRQHandler:
GPIO2_IRQHandler:
UART0_IRQHandler:
SSIS0_IRQHandler:
ADC_CNT0_IRQHandler:
AES_IRQHandler:
UART1_IRQHandler:
UART2_IRQHandler:
TMRD_TIMER1_IRQHandler:
FLASH_CNT0_IRQHandler:
TMRE_TIMER1_IRQHandler:
EXTTMRA_TIMER_IRQHandler:
I2C0_IRQHandler:
DMAC0_IRQHandler:
SPI0_IRQHandler:
SPI1_IRQHandler:
FDMAC0_IRQHandler:
FLASH_CNT1_IRQHandler:
SPI2_IRQHandler:
DIO0_IRQHandler:
LVD_CNT0_IRQHandler:
RF_IRQHandler:
CLK_TIMER0_IRQHandler:
MODE0_IRQHandler:
Default_Handler:
        B Default_Handler


        END


