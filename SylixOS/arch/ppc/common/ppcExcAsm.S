;/*********************************************************************************************************
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
;** ��   ��   ��: ppcExcAsm.S
;**
;** ��   ��   ��: Jiao.JinXing (������)
;**
;** �ļ���������: 2015 �� 12 �� 15 ��
;**
;** ��        ��: PowerPC ��ϵ�����쳣����.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include "ppcContextAsm.h"

    FILE_BEGIN()
    
    IMPORT_LABEL(API_ThreadTcbInter)
    IMPORT_LABEL(API_InterStackBaseGet)
    IMPORT_LABEL(API_InterGetNesting)
    IMPORT_LABEL(API_InterEnter)
    IMPORT_LABEL(API_InterExit)
    
    IMPORT_LABEL(bspIntHandle)
    IMPORT_LABEL(archDataStorageExceptionHandle)
    IMPORT_LABEL(archInstructionStorageExceptionHandle)
    IMPORT_LABEL(archAlignmentExceptionHandle)
    IMPORT_LABEL(archProgramExceptionHandle)
    IMPORT_LABEL(archFpuUnavailableExceptionHandle)
    IMPORT_LABEL(archSystemCallHandle)
    IMPORT_LABEL(archDecrementerInterruptHandle)
    IMPORT_LABEL(archTraceHandle)
    
    EXPORT_LABEL(archDataStorageExceptionEntry)
    EXPORT_LABEL(archInstructionStorageExceptionEntry)
    EXPORT_LABEL(archExternalInterruptEntry)
    EXPORT_LABEL(archAlignmentExceptionEntry)
    EXPORT_LABEL(archProgramExceptionEntry)
    EXPORT_LABEL(archFpuUnavailableExceptionEntry)
    EXPORT_LABEL(archSystemCallEntry)
    EXPORT_LABEL(archDecrementerInterruptEntry)
    EXPORT_LABEL(archTraceEntry)
    
;/*********************************************************************************************************
;  �ж���ں�
;*********************************************************************************************************/

MACRO_DEF(IRQ_ENTRY, handle)
    EXC_SAVE_REGS                                                       ;/*  ���мĴ�����ջ              */

    ;/*
    ; *  if (API_InterEnter() == 1) {
    ; *      current stack = SP;
    ; *      SP = interrupt stack;
    ; *  }
    ; */
    BL      API_InterEnter                                              ;/*  ��ȡ�жϲ����ŵ�����ֵ V0   */

    CMPWI   CR0, R3 , 1
    BNE     CR0, 1f

    BL      API_ThreadTcbInter                                          ;/*  get current tcb             */
    STW     SP , 0(R3)                                                  ;/*  current stack = SP          */

    BL      API_InterStackBaseGet                                       ;/*  ��õ�ǰ CPU �ж϶�ջջ��   */
    OR      SP , R3 , R3

1:
    MFSPR   R3 , SRR0                                                   ;/*  ���ݷ��ص�ַ                */
    ISYNC
    BL      \handle                                                     ;/*  ���ô�������                */

    ;/*
    ; *  if (API_InterGetNesting() == 1) {
    ; *      SP = current stack;
    ; *  }
    ; */
    BL      API_InterGetNesting

    CMPWI   CR0, R3 , 1
    BNE     CR0, 2f

    BL      API_ThreadTcbInter                                          ;/*  get current tcb             */

    LWZ     SP , 0(R3)                                                  ;/*  SP = current stack          */

2:
    BL      API_InterExit

    RESTORE_REGS                                                        ;/*  ���мĴ�����ջ              */
    MACRO_END()

;/*********************************************************************************************************
;  ���ݴ洢�쳣���(Vector 0x300)
;*********************************************************************************************************/

FUNC_DEF(archDataStorageExceptionEntry)
    IRQ_ENTRY   archDataStorageExceptionHandle
    FUNC_END()

;/*********************************************************************************************************
;  ָ������쳣���(Vector 0x400)
;*********************************************************************************************************/

FUNC_DEF(archInstructionStorageExceptionEntry)
    IRQ_ENTRY   archInstructionStorageExceptionHandle
    FUNC_END()

;/*********************************************************************************************************
;  �ⲿ�ж����(Vector 0x500)
;*********************************************************************************************************/

FUNC_DEF(archExternalInterruptEntry)
    IRQ_ENTRY   bspIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  �Ƕ����쳣���(Vector 0x600)
;*********************************************************************************************************/

FUNC_DEF(archAlignmentExceptionEntry)
    IRQ_ENTRY   archAlignmentExceptionHandle
    FUNC_END()

;/*********************************************************************************************************
;  �����쳣���(Vector 0x700)
;*********************************************************************************************************/

FUNC_DEF(archProgramExceptionEntry)
    IRQ_ENTRY   archProgramExceptionHandle
    FUNC_END()

;/*********************************************************************************************************
;  FPU ���������(Vector 0x800)
;*********************************************************************************************************/

FUNC_DEF(archFpuUnavailableExceptionEntry)
    IRQ_ENTRY   archFpuUnavailableExceptionHandle
    FUNC_END()

;/*********************************************************************************************************
;  Decrementer �쳣���(Vector 0x900)
;*********************************************************************************************************/

FUNC_DEF(archDecrementerInterruptEntry)
    IRQ_ENTRY   archDecrementerInterruptHandle
    FUNC_END()

;/*********************************************************************************************************
;  ϵͳ�������(Vector 0xC00)
;*********************************************************************************************************/

FUNC_DEF(archSystemCallEntry)
    IRQ_ENTRY   archSystemCallHandle
    FUNC_END()

;/*********************************************************************************************************
;  Trace ���(Vector 0xD00)
;*********************************************************************************************************/

FUNC_DEF(archTraceEntry)
    IRQ_ENTRY   archTraceHandle
    FUNC_END()

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/