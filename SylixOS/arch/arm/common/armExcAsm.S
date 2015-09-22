;/**********************************************************************************************************
;**
;**                                    �й�������Դ��֯
;**
;**                                   Ƕ��ʽʵʱ����ϵͳ
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------�ļ���Ϣ--------------------------------------------------------------------------------
;**
;** ��   ��   ��: armExcAsm.S
;**
;** ��   ��   ��: Han.Hui (����)
;**
;** �ļ���������: 2013 �� 12 �� 07 ��
;**
;** ��        ��: ARM ��ϵ�����쳣����.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>

    FILE_BEGIN()
    
    IMPORT_LABEL(API_ThreadTcbInter)
    IMPORT_LABEL(API_InterStackBaseGet)
    IMPORT_LABEL(API_InterGetNesting)
    IMPORT_LABEL(API_InterEnter)
    IMPORT_LABEL(API_InterExit)
    
    IMPORT_LABEL(archAbtHandle)
    IMPORT_LABEL(archUndHandle)
    IMPORT_LABEL(archSwiHandle)
    IMPORT_LABEL(bspIntHandle)
    
    EXPORT_LABEL(archIntEntry)
    EXPORT_LABEL(archAbtEntry)
    EXPORT_LABEL(archPreEntry)
    EXPORT_LABEL(archUndEntry)
    EXPORT_LABEL(archSwiEntry)
    
;/*********************************************************************************************************
;  �ж����
;*********************************************************************************************************/

MACRO_DEF(IRQ_ENTRY)
    STMFD   SP!, {R1-R3}                                                ;/*  �Ĵ���ѹ�� IRQ ģʽ��ջ     */
    MOV     R1 , SP                                                     ;/*  ���� IRQ ��ջָ��           */
    ADD     SP , SP, #12                                                ;/*  ���� IRQ ��ջָ��           */
    SUB     R2 , LR, #4                                                 ;/*  ���������жϷ��ص� PC ֵ    */
    MRS     R3 , SPSR                                                   ;/*  �� SPSR_irq ������ R3       */
    MSR     CPSR_c, #(DIS_INT | SVC32_MODE)                             ;/*  �ص� SVC ģʽ               */
    
    STMFD   SP!, {R2}                                                   ;/*  ���񷵻ص�ַѹջ            */
    STMFD   SP!, {LR}                                                   ;/*  LR ѹջ                     */
    STMFD   SP!, {R4-R12}                                               ;/*  R12 - R4 ѹջ               */
    
    LDMFD   R1!, {R4-R6}                                                ;/*  ������Ĵ��� R1 - R3 ��     */
                                                                        ;/*  IRQ ��ջ������ SVC ��ջ     */
    STMFD   SP!, {R4-R6}
    STMFD   SP!, {R0}                                                   ;/*  R0 ѹջ                     */
    STMFD   SP!, {R3}                                                   ;/*  CPSR ѹջ                   */
    
    ;/*
    ; *  if (API_InterEnter() == 1) {
    ; *      current stack = SP;
    ; *      SP = interrupt stack;
    ; *  }
    ; */
    LDR     R0 , =API_InterEnter
    MOV     LR , PC
    BX      R0
    CMP     R0 , #1
    BNE     1f
    
    LDR     R0 , =API_ThreadTcbInter                                    ;/*  get current tcb             */
    MOV     LR , PC
    BX      R0
    STR     SP , [R0]                                                   ;/*  current stack = SP          */
        
    LDR     R0 , =API_InterStackBaseGet                                 ;/*  ��õ�ǰ CPU �ж϶�ջջ��   */
    MOV     LR , PC
    BX      R0
    MOV     SP , R0
    
1:
    LDR     R5 , =bspIntHandle
    MOV     LR , PC
    BX      R5
    
    ;/*
    ; *  if (API_InterGetNesting() == 1) {
    ; *      SP = current stack;
    ; *  }
    ; */
    LDR     R0 , =API_InterGetNesting
    MOV     LR , PC
    BX      R0
    CMP     R0 , #1
    BNE     2f
    
    LDR     R0 , =API_ThreadTcbInter                                    ;/*  get current tcb             */
    MOV     LR , PC
    BX      R0
    LDR     SP , [R0]                                                   ;/*  SP = current stack          */
    
2:
    LDR     R0, =API_InterExit
    MOV     LR, PC
    BX      R0
    
    LDMFD   SP!, {R4}
    MSR     SPSR_cxsf, R4
    LDMFD   SP!, {R0-R12, LR, PC}^
    MACRO_END()
    
FUNC_DEF(archIntEntry)
    IRQ_ENTRY
    FUNC_END()
    
;/*********************************************************************************************************
;  abort ��� 
;  data_abort     $retoff == 8
;  prefetch_abort $retoff == 4
;*********************************************************************************************************/

MACRO_DEF(ABT_ENTRY retoff)
    STMFD   SP!, {R1-R3}                                                ;/*  �Ĵ���ѹ�� abt ģʽ��ջ     */
    MOV     R1 , SP                                                     ;/*  ���� abt ��ջָ��           */
    ADD     SP , SP, #12                                                ;/*  ���� abt ��ջָ��           */
    SUB     R2 , LR, #\retoff                                           ;/*  ���������жϷ��ص� PC ֵ    */
    MRS     R3 , SPSR                                                   ;/*  �� SPSR ������ R3           */
    MSR     CPSR_c, #(DIS_INT | SVC32_MODE)                             ;/*  �ص� SVC ģʽ               */
    
    STMFD   SP!, {R2}                                                   ;/*  ���񷵻ص�ַѹջ            */
    STMFD   SP!, {LR}                                                   ;/*  LR ѹջ                     */
    STMFD   SP!, {R4-R12}                                               ;/*  R12 - R4 ѹջ               */
    
    LDMFD   R1!, {R4-R6}                                                ;/*  ������Ĵ��� R1 - R3 ��     */
                                                                        ;/*  IRQ ��ջ������ SVC ��ջ     */
    STMFD   SP!, {R4-R6}
    STMFD   SP!, {R0}                                                   ;/*  R0 ѹջ                     */
    STMFD   SP!, {R3}                                                   ;/*  CPSR ѹջ                   */
    
    ;/*
    ; *  if (API_InterEnter() == 1) {
    ; *      current stack = SP;
    ; *      SP = interrupt stack;
    ; *  }
    ; */
    LDR     R0 , =API_InterEnter
    MOV     LR , PC
    BX      R0
    LDR     R8 , [SP , #(15 * 4)]                                       ;/*  R8 -> LR ��ַ               */
    CMP     R0 , #1
    BNE     1f
    
    LDR     R0 , =API_ThreadTcbInter                                    ;/*  get current tcb             */
    MOV     LR , PC
    BX      R0
    STR     SP , [R0]                                                   ;/*  current stack = SP          */
    
    LDR     R0 , =API_InterStackBaseGet                                 ;/*  ��õ�ǰ CPU �ж϶�ջջ��   */
    MOV     LR , PC
    BX      R0
    MOV     SP , R0
    
1:
    MOV     R0 , R8                                                     ;/*  R8 -> LR ��ַ               */
    MOV     R1 , #\retoff                                               ;/*  $retoff                     */
    LDR     R5 , =archAbtHandle
    MOV     LR , PC
    BX      R5
    
    ;/*
    ; *  if (API_InterGetNesting() == 1) {
    ; *      SP = current stack;
    ; *  }
    ; */
    LDR     R0 , =API_InterGetNesting
    MOV     LR , PC
    BX      R0
    CMP     R0 , #1
    BNE     2f
    
    LDR     R0 , =API_ThreadTcbInter                                    ;/*  get current tcb             */
    MOV     LR , PC
    BX      R0
    LDR     SP , [R0]                                                   ;/*  SP = current stack          */
    
2:
    LDR     R0, =API_InterExit
    MOV     LR, PC
    BX      R0
    
    LDMFD   SP!, {R4}
    MSR     SPSR_cxsf, R4
    LDMFD   SP!, {R0-R12, LR, PC}^
    MACRO_END()
    
FUNC_DEF(archAbtEntry)
    ABT_ENTRY 8
    FUNC_END()

FUNC_DEF(archPreEntry)
    ABT_ENTRY 4
    FUNC_END()

;/*********************************************************************************************************
;  undefine ���
;*********************************************************************************************************/

MACRO_DEF(UND_ENTRY)
    STMFD   SP!, {R1-R3}                                                ;/*  �Ĵ���ѹ�� und ģʽ��ջ     */
    MOV     R1 , SP                                                     ;/*  ���� und ��ջָ��           */
    ADD     SP , SP, #12                                                ;/*  ���� und ��ջָ��           */
    MRS     R3 , SPSR                                                   ;/*  �� SPSR ������ R3           */
    TST     R3 , #0x20                                                  ;/*  �Ƿ�Ϊ Thumb ģʽ           */
    SUBNE   R2 , LR, #2                                                 ;/*  ���� Thumb ģʽ����λ��     */
    SUBEQ   R2 , LR, #4                                                 ;/*  ���� ARM ģʽ����λ��       */
    MSR     CPSR_c, #(DIS_INT | SVC32_MODE)                             ;/*  �ص� SVC ģʽ               */
    
    STMFD   SP!, {R2}                                                   ;/*  ���񷵻ص�ַѹջ            */
    STMFD   SP!, {LR}                                                   ;/*  LR ѹջ                     */
    STMFD   SP!, {R4-R12}                                               ;/*  R12 - R4 ѹջ               */
    
    LDMFD   R1!, {R4-R6}                                                ;/*  ������Ĵ��� R1 - R3 ��     */
                                                                        ;/*  IRQ ��ջ������ SVC ��ջ     */
    STMFD   SP!, {R4-R6}
    STMFD   SP!, {R0}                                                   ;/*  R0 ѹջ                     */
    STMFD   SP!, {R3}                                                   ;/*  CPSR ѹջ                   */
    
    ;/*
    ; *  if (API_InterEnter() == 1) {
    ; *      current stack = SP;
    ; *      SP = interrupt stack;
    ; *  }
    ; */
    LDR     R0 , =API_InterEnter
    MOV     LR , PC
    BX      R0
    LDR     R8 , [SP, #(15 * 4)]                                        ;/*  ����Ϊ����δ����ָ���жϵ�ַ*/
    LDR     R9 , [SP]                                                   ;/*  �����쳣�� CPSR             */
    CMP     R0 , #1
    BNE     1f
    
    LDR     R0 , =API_ThreadTcbInter                                    ;/*  get current tcb             */
    MOV     LR , PC
    BX      R0
    STR     SP , [R0]                                                   ;/*  current stack = SP          */
    
    LDR     R0 , =API_InterStackBaseGet                                 ;/*  ��õ�ǰ CPU �ж϶�ջջ��   */
    MOV     LR , PC
    BX      R0
    MOV     SP , R0
    
1:
    MOV     R0 , R8
    MOV     R1 , R9
    LDR     R5 , =archUndHandle
    MOV     LR , PC
    BX      R5
    
    ;/*
    ; *  if (API_InterGetNesting() == 1) {
    ; *      SP = current stack;
    ; *  }
    ; */
    LDR     R0 , =API_InterGetNesting
    MOV     LR , PC
    BX      R0
    CMP     R0 , #1
    BNE     2f
    
    LDR     R0 , =API_ThreadTcbInter                                    ;/*  get current tcb             */
    MOV     LR , PC
    BX      R0
    LDR     SP , [R0]                                                   ;/*  SP = current stack          */
    
2:
    LDR     R0, =API_InterExit
    MOV     LR, PC
    BX      R0
    
    LDMFD   SP!, {R4}
    MSR     SPSR_cxsf, R4
    LDMFD   SP!, {R0-R12, LR, PC}^
    MACRO_END()
    
FUNC_DEF(archUndEntry)
    UND_ENTRY
    FUNC_END()
    
;/*********************************************************************************************************
;  swi ���
;*********************************************************************************************************/

MACRO_DEF(SWI_ENTRY)
    STMFD   SP!, {R0-R12, LR}

    LDR     R0 , [LR, #-4]                                              ;/*  ��ȡ SWI ָ��               */
    BIC     R0 , R0, #0xff000000                                        ;/*  ��� SWI ָ���             */
    MOV     R1 , SP                                                     ;/*  ��ò�����λ��              */

    LDR     R5 , =archSwiHandle
    MOV     LR , PC
    BX      R5

    LDMFD   SP!, {R0-R12, PC}^                                          ;/*  SWI ����                    */
    MACRO_END()

FUNC_DEF(archSwiEntry)
    SWI_ENTRY
    FUNC_END()

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/