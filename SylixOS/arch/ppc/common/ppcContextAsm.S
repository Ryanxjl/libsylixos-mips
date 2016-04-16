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
;** ��   ��   ��: ppcContextAsm.S
;**
;** ��   ��   ��: Jiao.JinXing (������)
;**
;** �ļ���������: 2015 �� 12 �� 15 ��
;**
;** ��        ��: PowerPC ��ϵ�����������л�.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include "ppcContextAsm.h"

    FILE_BEGIN()

    IMPORT_LABEL(_SchedSwp)
#if LW_CFG_COROUTINE_EN > 0
    IMPORT_LABEL(_SchedCrSwp)
#endif
    
    EXPORT_LABEL(archTaskCtxStart)
    EXPORT_LABEL(archTaskCtxSwitch)
    EXPORT_LABEL(archCrtCtxSwitch)
    EXPORT_LABEL(archIntCtxLoad)
    EXPORT_LABEL(archSigCtxLoad)

;/*********************************************************************************************************
;  ������ջ֡(����ʱ��)
;*********************************************************************************************************/

#undef __CTX_FRAME

#ifdef __CTX_FRAME
MACRO_DEF(PUSH_FRAME)
    SUBI    SP , SP , 8
    STW     FP , 0(SP)
    MFLR    R4
    ISYNC
    STW     R4 , 4(SP)
    SYNC
    MACRO_END()

MACRO_DEF(POP_FRAME)
    LWZ     FP , 0(SP)
    LWZ     R4 , 4(SP)
    SYNC
    MTLR    R4
    ISYNC
    ADDI    SP , SP , 8
    MACRO_END()
#else
MACRO_DEF(PUSH_FRAME)
    MACRO_END()

MACRO_DEF(POP_FRAME)
    MACRO_END()
#endif

;/*********************************************************************************************************
;  ��ǰ CPU ��ʼ���ж�����ִ��
;  ����Ϊ��ǰ CPU ���ƿ�, �� R3 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    LWZ     R4 , 0(R3)                                                  ;/*  ��ȡ�������ջջ����ַ      */
    LWZ     SP , 0(R4)                                                  ;/*  ��ջָ�븳ֵ                */

    RESTORE_REGS                                                        ;/*  ���мĴ�����ջ              */
    FUNC_END()

;/*********************************************************************************************************
;  �߳��л� 
;  ����Ϊ��ǰ CPU ���ƿ�, �� R3 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    SAVE_REGS                                                           ;/*  ���мĴ�����ջ              */

    LWZ     R4 , 0(R3)                                                  ;/*  stack current = SP          */
    STW     SP , 0(R4)

    SUBI    SP , SP , 8
    STW     R3 , 0(SP)                                                  ;/*  ��ǰ CPU ��Ϣ��ջ           */

    PUSH_FRAME
    BL      _SchedSwp                                                   ;/*  _SchedSwp();                */
    POP_FRAME

    LWZ     R3 , 0(SP)                                                  ;/*  ��ǰ CPU ��Ϣ��ջ           */
    ADDI    SP , SP , 8

    LWZ     R4 , 0(R3)                                                  ;/*  SP = stack current          */
    LWZ     SP , 0(R4)

    RESTORE_REGS                                                        ;/*  ���мĴ�����ջ              */
    FUNC_END()

;/*********************************************************************************************************
;  Э���л� 
;  ����Ϊ��ǰ CPU ���ƿ�, �� R3 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    SAVE_REGS                                                           ;/*  ���мĴ�����ջ              */

    LWZ     R4 , 0(R3)                                                  ;/*  stack current = SP          */
    STW     SP , 0(R4)

    SUBI    SP , SP , 8
    STW     R3 , 0(SP)                                                  ;/*  ��ǰ CPU ��Ϣ��ջ           */

    PUSH_FRAME
    BL      _SchedCrSwp                                                 ;/*  _SchedCrSwp();              */
    POP_FRAME

    LWZ     R3 , 0(SP)                                                  ;/*  ��ǰ CPU ��Ϣ��ջ           */
    ADDI    SP , SP , 8

    LWZ     R4 , 0(R3)                                                  ;/*  SP = stack current          */
    LWZ     SP , 0(R4)

    RESTORE_REGS                                                        ;/*  ���мĴ�����ջ              */
    FUNC_END()
#endif

;/*********************************************************************************************************
;  �жϷ���ʱ, �߳�װ�� 
;  ����Ϊ��ǰ CPU ���ƿ�, �� R3 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    SUBI    SP , SP , 8
    STW     R3 , 0(SP)                                                  ;/*  ��ǰ CPU ��Ϣ��ջ           */

    PUSH_FRAME
    BL      _SchedSwp                                                   ;/*  _SchedSwp();                */
    POP_FRAME

    LWZ     R3 , 0(SP)                                                  ;/*  ��ǰ CPU ��Ϣ��ջ           */
    ADDI    SP , SP , 8

    LWZ     R4 , 0(R3)                                                  ;/*  SP = stack current          */
    LWZ     SP , 0(R4)

    RESTORE_REGS                                                        ;/*  ���мĴ�����ջ              */
    FUNC_END()

;/*********************************************************************************************************
;  �ź������ķ���
;  ����Ϊ��Ҫ���صĶ�ջջ��λ��
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    OR      SP , R3 , R3                                                ;/*  ���صĶ�ջָ��              */

    RESTORE_REGS                                                        ;/*  ���мĴ�����ջ              */
    FUNC_END()

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/