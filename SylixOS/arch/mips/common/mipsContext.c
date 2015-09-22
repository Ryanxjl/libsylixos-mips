/*********************************************************************************************************
**
**                                    �й�������Դ��֯
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
    UINT uiCP0_STATUS;
    UINT uiGP;

    asm volatile("mfc0   %0,$12"   : "=r"(uiCP0_STATUS));
    uiCP0_STATUS &= 0xfffffffe;
    uiCP0_STATUS |= 0x001;
    asm volatile("addi   %0,$28,0" : "=r"(uiGP));

    if ((addr_t)pstkTop & 0x7) {                                        /*  ��֤��ջ�� CPU SP 8 �ֽڶ���*/
        pstkTop = (PLW_STACK)((addr_t)pstkTop - 4);                     /*  ��͵�ַ�ƽ� 4 �ֽ�         */
    }

    pfpctx  = (ARCH_FP_CTX  *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));
    pregctx = (ARCH_REG_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX) - sizeof(ARCH_REG_CTX));

    pfpctx->FP_uiFP = (ARCH_REG_T)LW_NULL;

    pregctx->REG_uiCP0_STATUS = uiCP0_STATUS;

    pregctx->REG_uiEPC   = (ARCH_REG_T)pfuncTask;

    pregctx->REG_uiAT = (ARCH_REG_T)0x01010101;
    pregctx->REG_uiV0 = (ARCH_REG_T)0x02020202;
    pregctx->REG_uiV1 = (ARCH_REG_T)0x03030303;
    pregctx->REG_uiA0 = (ARCH_REG_T)pvArg;
    pregctx->REG_uiA1 = (ARCH_REG_T)0x05050505;
    pregctx->REG_uiA2 = (ARCH_REG_T)0x06060606;
    pregctx->REG_uiA3 = (ARCH_REG_T)0x07070707;
    pregctx->REG_uiT0 = (ARCH_REG_T)0x08080808;
    pregctx->REG_uiT1 = (ARCH_REG_T)0x09090909;
    pregctx->REG_uiT2 = (ARCH_REG_T)0x10101010;
    pregctx->REG_uiT3 = (ARCH_REG_T)0x11111111;
    pregctx->REG_uiT4 = (ARCH_REG_T)0x12121212;
    pregctx->REG_uiT5 = (ARCH_REG_T)0x13131313;
    pregctx->REG_uiT6 = (ARCH_REG_T)0x14141414;
    pregctx->REG_uiT7 = (ARCH_REG_T)0x15151515;
    pregctx->REG_uiS0 = (ARCH_REG_T)0x16161616;
    pregctx->REG_uiS1 = (ARCH_REG_T)0x17171717;
    pregctx->REG_uiS2 = (ARCH_REG_T)0x18181818;
    pregctx->REG_uiS3 = (ARCH_REG_T)0x19191919;
    pregctx->REG_uiS4 = (ARCH_REG_T)0x20202020;
    pregctx->REG_uiS5 = (ARCH_REG_T)0x21212121;
    pregctx->REG_uiS6 = (ARCH_REG_T)0x22222222;
    pregctx->REG_uiS7 = (ARCH_REG_T)0x23232323;
    pregctx->REG_uiT8 = (ARCH_REG_T)0x24242424;
    pregctx->REG_uiT9 = (ARCH_REG_T)0x25252525;
    pregctx->REG_uiGP = (ARCH_REG_T)uiGP;
    pregctx->REG_uiFP = pfpctx->FP_uiFP;
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

}

#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
