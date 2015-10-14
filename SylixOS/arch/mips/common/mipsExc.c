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
** ��   ��   ��: mipsExc.c
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 09 �� 01 ��
**
** ��        ��: MIPS ��ϵ�����쳣����.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "dtrace.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
  ����ʹ���������
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
#define VECTOR_OP_LOCK()    LW_SPIN_LOCK_IGNIRQ(&_K_slVectorTable)
#define VECTOR_OP_UNLOCK()  LW_SPIN_UNLOCK_IGNIRQ(&_K_slVectorTable)
#else
#define VECTOR_OP_LOCK()
#define VECTOR_OP_UNLOCK()
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
** ��������: archIntHandle
** ��������: bspIntHandle ��Ҫ���ô˺��������ж� (�ر��ж����������)
** �䡡��  : ulVector         �ж�����
**           bPreemptive      �ж��Ƿ����ռ
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : �˺����˳�ʱ����Ϊ�жϹر�״̬.
*********************************************************************************************************/
VOID  archIntHandle (ULONG  ulVector, BOOL  bPreemptive)
{
    REGISTER irqreturn_t irqret;

    if (_Inter_Vector_Invalid(ulVector)) {
        return;                                                         /*  �����Ų���ȷ                */
    }

    if (LW_IVEC_GET_FLAG(ulVector) & LW_IRQ_FLAG_PREEMPTIVE) {
        bPreemptive = LW_TRUE;
    }

    if (bPreemptive) {
        VECTOR_OP_LOCK();
        __ARCH_INT_VECTOR_DISABLE(ulVector);                            /*  ���� vector �ж�            */
        VECTOR_OP_UNLOCK();
        KN_INT_ENABLE_FORCE();                                          /*  �����ж�                    */
    }

    irqret = API_InterVectorIsr(ulVector);                              /*  �����жϷ������            */

    KN_INT_DISABLE();                                                   /*  �����ж�                    */

    if (bPreemptive) {
        if (irqret != LW_IRQ_HANDLED_DISV) {
            VECTOR_OP_LOCK();
            __ARCH_INT_VECTOR_ENABLE(ulVector);                         /*  ���� vector �ж�            */
            VECTOR_OP_UNLOCK();
        }

    } else if (irqret == LW_IRQ_HANDLED_DISV) {
        VECTOR_OP_LOCK();
        __ARCH_INT_VECTOR_DISABLE(ulVector);                            /*  ���� vector �ж�            */
        VECTOR_OP_UNLOCK();
    }
}
/*********************************************************************************************************
** ��������: archCacheErrorHandle
** ��������: Cache ������
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  archCacheErrorHandle (addr_t  ulRetAddr)
{
    /*
     * TODO
     */
    while (1) {

    }
}
/*********************************************************************************************************
** ��������: archExceptionHandle
** ��������: ͨ���쳣����
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  archExceptionHandle (addr_t  ulRetAddr)
{
    REGISTER UINT32  uiCause   = mipsCp0CauseRead();
    REGISTER UINT32  uiExcCode = ((uiCause & M_CauseExcCode) >> S_CauseExcCode);
    PLW_CLASS_TCB    ptcbCur;

    LW_TCB_GET_CUR(ptcbCur);

    switch (uiExcCode) {
    case EX_MOD:                                                        /* TLB modified                 */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_WRITE, ptcbCur);
        break;

    case EX_TLBL:                                                       /* TLB exception(load or ifetch)*/
    case EX_TLBS:                                                       /* TLB exception (store)        */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_MAP, ptcbCur);
        break;

    case EX_ADEL:                                                       /* Address error(load or ifetch)*/
    case EX_ADES:                                                       /* Address error (store)        */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_TERMINAL, ptcbCur);
        break;

    case EX_IBE:                                                        /* Instruction Bus Error        */
    case EX_DBE:                                                        /* Data Bus Error               */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_BUS, ptcbCur);
        break;

    case EX_SYS:                                                        /* Syscall                      */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_SYS, ptcbCur);
        break;

    case EX_BP:                                                         /* Breakpoint                   */
    case EX_TR:                                                         /* Trap instruction             */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_BREAK, ptcbCur);
        break;

    case EX_RI:                                                         /* Reserved instruction         */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_UNDEF, ptcbCur);
        break;

    case EX_FPE:                                                        /* floating point exception     */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_FPE, ptcbCur);
        break;

    case EX_CPU:                                                        /* CoProcessor Unusable         */
    case EX_OV:                                                         /* OVerflow                     */
    case EX_C2E:                                                        /* COP2 exception               */
    case EX_MDMX:                                                       /* MDMX exception               */
    case EX_WATCH:                                                      /* Watch exception              */
    case EX_MCHECK:                                                     /* Machine check exception      */
    case EX_CacheErr:                                                   /* Cache error caused re-entry  */
                                                                        /* to Debug Mode                */
        API_VmmAbortIsr(ulRetAddr, mipsCp0BadVAddrRead(), LW_VMM_ABORT_TYPE_TERMINAL, ptcbCur);
        break;

    default:
        _DebugFormat(__ERRORMESSAGE_LEVEL, "Unknow exception: %d\r\n", uiExcCode);
        while (1) {
        }
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
