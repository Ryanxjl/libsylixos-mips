/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: armContext.c
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 09 �� 01 ��
**
** ��        ��: MIPS ��ϵ���������Ĵ���.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
** ��������: archTaskCtxCreate
** ��������: ��������������
** �䡡��  : pfuncTask      �������
**           pvArg          ��ڲ���
**           pstkTop        ��ʼ����ջ���
**           ulOpt          ���񴴽�ѡ��
** �䡡��  : ��ʼ����ջ������
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : ��ջ�Ӹߵ�ַ��͵�ַ����.
*********************************************************************************************************/
PLW_STACK  archTaskCtxCreate (PTHREAD_START_ROUTINE  pfuncTask,
                              PVOID                  pvArg,
                              PLW_STACK              pstkTop,
                              ULONG                  ulOpt)
{
    ARCH_REG_CTX      *pregctx;
    ARCH_FP_CTX       *pfpctx;
    UINT32             uiCP0Status;
    UINT32             uiGP;

    uiCP0Status  = mipsCp0StatusRead();

    uiCP0Status |= (1 << 0) |
                   (1 << (8 + 7)) |
                   (1 << (8 + 4));

    MIPS_EXEC_INS("addi   %0, " MIPS_GP ", 0" : "=r"(uiGP));

    if ((addr_t)pstkTop & 0x7) {                                        /*  ��֤��ջ�� CPU SP 8 �ֽڶ���*/
        pstkTop = (PLW_STACK)((addr_t)pstkTop - 4);                     /*  ��͵�ַ�ƽ� 4 �ֽ�         */
    }

    pfpctx  = (ARCH_FP_CTX  *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));
    pregctx = (ARCH_REG_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX) - sizeof(ARCH_REG_CTX));

    pfpctx->FP_uiFP = (ARCH_REG_T)LW_NULL;
    pfpctx->FP_uiRA = (ARCH_REG_T)LW_NULL;

    pregctx->REG_uiCP0_STATUS = uiCP0Status;

    pregctx->REG_uiEPC = (ARCH_REG_T)pfuncTask;

    pregctx->REG_uiAT = (ARCH_REG_T)0xAF;
    pregctx->REG_uiV0 = (ARCH_REG_T)0x0;
    pregctx->REG_uiV1 = (ARCH_REG_T)0x1;
    pregctx->REG_uiA0 = (ARCH_REG_T)pvArg;
    pregctx->REG_uiA1 = (ARCH_REG_T)0xA1;
    pregctx->REG_uiA2 = (ARCH_REG_T)0xA2;
    pregctx->REG_uiA3 = (ARCH_REG_T)0xA3;
    pregctx->REG_uiT0 = (ARCH_REG_T)0xF0;
    pregctx->REG_uiT1 = (ARCH_REG_T)0xF1;
    pregctx->REG_uiT2 = (ARCH_REG_T)0xF2;
    pregctx->REG_uiT3 = (ARCH_REG_T)0xF3;
    pregctx->REG_uiT4 = (ARCH_REG_T)0xF4;
    pregctx->REG_uiT5 = (ARCH_REG_T)0xF5;
    pregctx->REG_uiT6 = (ARCH_REG_T)0xF6;
    pregctx->REG_uiT7 = (ARCH_REG_T)0xF7;
    pregctx->REG_uiS0 = (ARCH_REG_T)0x80;
    pregctx->REG_uiS1 = (ARCH_REG_T)0x81;
    pregctx->REG_uiS2 = (ARCH_REG_T)0x82;
    pregctx->REG_uiS3 = (ARCH_REG_T)0x83;
    pregctx->REG_uiS4 = (ARCH_REG_T)0x84;
    pregctx->REG_uiS5 = (ARCH_REG_T)0x85;
    pregctx->REG_uiS6 = (ARCH_REG_T)0x86;
    pregctx->REG_uiS7 = (ARCH_REG_T)0x87;
    pregctx->REG_uiT8 = (ARCH_REG_T)0xF8;
    pregctx->REG_uiT9 = (ARCH_REG_T)0xF9;
    pregctx->REG_uiGP = (ARCH_REG_T)uiGP;
    pregctx->REG_uiFP = (ARCH_REG_T)pfpctx->FP_uiFP;
    pregctx->REG_uiRA = (ARCH_REG_T)pfuncTask;

    return  ((PLW_STACK)pregctx);
}
/*********************************************************************************************************
** ��������: archTaskCtxSetFp
** ��������: ��������������ջ֡ (���� backtrace ����, ������� backtrace ����ļ�)
** �䡡��  : pstkDest  Ŀ�� stack frame
**           pstkSrc   Դ�� stack frame
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  archTaskCtxSetFp (PLW_STACK  pstkDest, PLW_STACK  pstkSrc)
{
    ARCH_REG_CTX      *pregctxDest = (ARCH_REG_CTX *)pstkDest;
    ARCH_REG_CTX      *pregctxSrc  = (ARCH_REG_CTX *)pstkSrc;
    ARCH_FP_CTX       *pfpctx      = (ARCH_FP_CTX *)((PCHAR)pregctxDest + sizeof(ARCH_REG_CTX));

    /*
     *  �� ARCH_FP_CTX ������, ģ����һ��
     *  push {fp, lr}
     *  add  fp, sp, #4
     */
    pfpctx->FP_uiFP = pregctxSrc->REG_uiFP;
    pfpctx->FP_uiRA = pregctxSrc->REG_uiRA;

    pregctxDest->REG_uiFP = (ARCH_REG_T)&pfpctx->FP_uiRA;
}
/*********************************************************************************************************
** ��������: archTaskCtxShow
** ��������: ��ӡ����������
** �䡡��  : iFd        �ļ�������
             pstkTop    ��ջջ��
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
#if LW_CFG_DEVICE_EN > 0

VOID  archTaskCtxShow (INT  iFd, PLW_STACK  pstkTop)
{
    fdprintf(iFd, "\n");

    fdprintf(iFd, "RA  = 0x%08x  ", pstkTop[STK_OFFSET_RA / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "FP  = 0x%08x  ", pstkTop[STK_OFFSET_FP / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "GP  = 0x%08x\n", pstkTop[STK_OFFSET_GP / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "S7  = 0x%08x  ", pstkTop[STK_OFFSET_S7 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "S6  = 0x%08x\n", pstkTop[STK_OFFSET_S6 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "S5  = 0x%08x  ", pstkTop[STK_OFFSET_S4 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "S4  = 0x%08x\n", pstkTop[STK_OFFSET_S4 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "S3  = 0x%08x  ", pstkTop[STK_OFFSET_S3 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "S2  = 0x%08x\n", pstkTop[STK_OFFSET_S2 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "S1  = 0x%08x  ", pstkTop[STK_OFFSET_S1 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "S0  = 0x%08x\n", pstkTop[STK_OFFSET_S0 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "T9  = 0x%08x  ", pstkTop[STK_OFFSET_T9 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "T8  = 0x%08x\n", pstkTop[STK_OFFSET_T8 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "T7  = 0x%08x  ", pstkTop[STK_OFFSET_T7 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "T6  = 0x%08x\n", pstkTop[STK_OFFSET_T6 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "T5  = 0x%08x  ", pstkTop[STK_OFFSET_T5 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "T4  = 0x%08x\n", pstkTop[STK_OFFSET_T4 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "T3  = 0x%08x  ", pstkTop[STK_OFFSET_T3 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "T2  = 0x%08x\n", pstkTop[STK_OFFSET_T2 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "T1  = 0x%08x  ", pstkTop[STK_OFFSET_T1 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "T0  = 0x%08x\n", pstkTop[STK_OFFSET_T0 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "A3  = 0x%08x  ", pstkTop[STK_OFFSET_A3 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "A2  = 0x%08x\n", pstkTop[STK_OFFSET_A2 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "A1  = 0x%08x  ", pstkTop[STK_OFFSET_A1 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "A0  = 0x%08x\n", pstkTop[STK_OFFSET_A0 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "V1  = 0x%08x  ", pstkTop[STK_OFFSET_V1 / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "V0  = 0x%08x\n", pstkTop[STK_OFFSET_V0 / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "AT  = 0x%08x  ", pstkTop[STK_OFFSET_AT / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "SR  = 0x%08x\n", pstkTop[STK_OFFSET_SR / sizeof(ARCH_REG_T)]);

    fdprintf(iFd, "EPC = 0x%08x  ", pstkTop[STK_OFFSET_EPC / sizeof(ARCH_REG_T)]);
    fdprintf(iFd, "SP  = 0x%08x\n", (ARCH_REG_T)pstkTop);
}

#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/

