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
** ��   ��   ��: InterEnterExit.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: ����ϵͳ�ں��жϳ��ں�����

** BUG
2008.01.04  �޸Ĵ����ʽ��ע��.
2009.04.29  ���� SMP ֧��.
2011.02.22  ֱ�ӵ��� _SchedInt() �����жϵ���.
2012.09.05  API_InterEnter() ��һ�ν����ж�ʱ, ��Ҫ���浱ǰ����� FPU ������.
            API_InterExit() ���û�в�������, ��ָ����ж������ FPU ������.
2012.09.23  ���� INT ENTER �� INT EXIT �ص�.
2013.07.17  �����������ͨ���˼��ж����.
2013.07.18  ʹ���µĻ�ȡ TCB �ķ���, ȷ�� SMP ϵͳ��ȫ.
2013.07.19  �ϲ���ͨ CPU �жϺͺ˼��жϳ���, �������ֺ˼��ж�����ͨ�жϳ���.
2013.12.12  ���ﲻ�ٴ����ж� hook.
2015.05.14  �Ż��жϽ������˳�����.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  macro
*********************************************************************************************************/
#define __INTER_FPU_CTX(nest)      &pcpu->CPU_fpuctxContext[nest]
/*********************************************************************************************************
** ��������: __fpuInterEnter
** ��������: �����ж�״̬ FPU ���� (�ڹ��жϵ�����±�����)
** �䡡��  : pcpu  ��ǰ CPU ���ƿ�
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_CPU_FPU_EN > 0

static VOID  __fpuInterEnter (PLW_CLASS_CPU  pcpu)
{
    PLW_CLASS_TCB  pctbCur;
    ULONG          ulInterNesting = pcpu->CPU_ulInterNesting;
    
    if (pcpu->CPU_ulInterNesting == 1) {                                /*  ������̬�����ж�            */
        pctbCur = pcpu->CPU_ptcbTCBCur;
        if (pctbCur->TCB_ulOption & LW_OPTION_THREAD_USED_FP) {
            __ARCH_FPU_SAVE(pctbCur->TCB_pvStackFP);                    /*  ���浱ǰ���ж��߳� FPU CTX  */
        
        } else {
            __ARCH_FPU_ENABLE();                                        /*  ʹ�ܵ�ǰ�ж��� FPU          */
        }
    } else {
        REGISTER ULONG  ulOldNest = ulInterNesting - 1;
        __ARCH_FPU_SAVE(__INTER_FPU_CTX(ulOldNest));
    }
}
/*********************************************************************************************************
** ��������: __fpuInterExit
** ��������: �˳��ж�״̬ FPU ���� (�ڹ��жϵ�����±�����)
** �䡡��  : pcpu  ��ǰ CPU ���ƿ�
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __fpuInterExit (PLW_CLASS_CPU  pcpu)
{
    PLW_CLASS_TCB  pctbCur;
    ULONG          ulInterNesting = pcpu->CPU_ulInterNesting;
    
    if (ulInterNesting) {                                               /*  �˳������ж���            */
        __ARCH_FPU_RESTORE(__INTER_FPU_CTX(ulInterNesting));
    
    } else {                                                            /*  �˳�������״̬              */
        pctbCur = pcpu->CPU_ptcbTCBCur;
        if (pctbCur->TCB_ulOption & LW_OPTION_THREAD_USED_FP) {
            __ARCH_FPU_RESTORE(pctbCur->TCB_pvStackFP);                 /*  û�в�������, ��ָ� FPU CTX*/
        
        } else {
            __ARCH_FPU_DISABLE();                                       /*  ����ִ�е�������Ҫ FPU    */
        }
    }
}
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
/*********************************************************************************************************
** ��������: API_InterEnter
** ��������: �ں��ж���ں��� (�ڹ��жϵ�����±�����)
** �䡡��  : 
** �䡡��  : �жϲ���
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API
ULONG    API_InterEnter (VOID)
{
    PLW_CLASS_CPU  pcpu;
    
    pcpu = LW_CPU_GET_CUR();
    if (pcpu->CPU_ulInterNesting != LW_CFG_MAX_INTER_SRC) {
        pcpu->CPU_ulInterNesting++;
    }
    
    KN_SMP_WMB();                                                       /*  �ȴ����ϲ������            */

#if LW_CFG_CPU_FPU_EN > 0
    if (_K_bInterFpuEn) {                                               /*  �ж�״̬����ʹ�ø�������    */
        __fpuInterEnter(pcpu);
    }
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

    return  (pcpu->CPU_ulInterNesting);
}
/*********************************************************************************************************
** ��������: API_InterExit
** ��������: �ں��жϳ��ں��� (�ڹ��жϵ�����±�����)
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API
VOID    API_InterExit (VOID)
{
    PLW_CLASS_CPU  pcpu;
    
    pcpu = LW_CPU_GET_CUR();
    
#if LW_CFG_INTER_INFO > 0
    if (pcpu->CPU_ulInterNestingMax < pcpu->CPU_ulInterNesting) {
        pcpu->CPU_ulInterNestingMax = pcpu->CPU_ulInterNesting;
    }
#endif                                                                  /*  LW_CFG_INTER_INFO > 0       */

    if (pcpu->CPU_ulInterNesting) {                                     /*  ϵͳ�ж�Ƕ�ײ���--          */
        pcpu->CPU_ulInterNesting--;
    }
    
    KN_SMP_WMB();                                                       /*  �ȴ����ϲ������            */
    
    if (pcpu->CPU_ulInterNesting) {                                     /*  �鿴ϵͳ�Ƿ����ж�Ƕ����    */
#if LW_CFG_CPU_FPU_EN > 0                                               /*  �ָ���һ�ȼ��ж� FPU CTX    */
        if (_K_bInterFpuEn) {
            __fpuInterExit(pcpu);
        }
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
        return;
    }
    
    __KERNEL_SCHED_INT();                                               /*  �ж��еĵ���                */
    
#if LW_CFG_CPU_FPU_EN > 0
    if (_K_bInterFpuEn) {
        __fpuInterExit(pcpu);
    }
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/