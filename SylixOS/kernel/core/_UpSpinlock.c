/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: _UpSpinlock.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 07 �� 21 ��
**
** ��        ��: �� CPU ϵͳ������.
**
** ע        ��: �� CPU �����, ����Ҫ���� CPU_ulSpinNesting;
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_SMP_EN == 0
/*********************************************************************************************************
** ��������: _UpSpinInit
** ��������: ��������ʼ��
** �䡡��  : psl           ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinInit (spinlock_t *psl)
{
    (VOID)psl;
}
/*********************************************************************************************************
** ��������: _UpSpinLock
** ��������: ��������������
** �䡡��  : psl           ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinLock (spinlock_t *psl)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
}
/*********************************************************************************************************
** ��������: _UpSpinTryLock
** ��������: ���������Լ�������
** �䡡��  : psl           ������
** �䡡��  : LW_TRUE �������� LW_FALSE ����ʧ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
BOOL  _UpSpinTryLock (spinlock_t *psl)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
    
    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: _UpSpinUnlock
** ��������: ��������������
** �䡡��  : psl           ������
** �䡡��  : ����������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  _UpSpinUnlock (spinlock_t *psl)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur   = LW_CPU_GET_CUR();
    BOOL            bTrySched = LW_FALSE;
    
    iregInterLevel = KN_INT_DISABLE();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(pcpuCur->CPU_ptcbTCBCur);                     /*  �����������                */
        if (__COULD_SCHED(pcpuCur, 0)) {
            bTrySched = LW_TRUE;                                        /*  ��Ҫ���Ե���                */
        }
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    if (bTrySched) {
        return  (_ThreadSched(pcpuCur->CPU_ptcbTCBCur));
    
    } else {
        return  (ERROR_NONE);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: _UpSpinLockIgnIrq
** ��������: ��������������, �����ж����� (�������жϹرյ�״̬�±�����)
** �䡡��  : psl           ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinLockIgnIrq (spinlock_t *psl)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
}
/*********************************************************************************************************
** ��������: _UpSpinTryLockIgnIrq
** ��������: ���������Լ�������, �����ж����� (�������жϹرյ�״̬�±�����)
** �䡡��  : psl           ������
** �䡡��  : LW_TRUE �������� LW_FALSE ����ʧ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
BOOL  _UpSpinTryLockIgnIrq (spinlock_t *psl)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
    
    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: _UpSpinUnlockIgnIrq
** ��������: ��������������, �����ж����� (�������жϹرյ�״̬�±�����)
** �䡡��  : psl           ������
** �䡡��  : NONE (�����е��ȳ���)
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinUnlockIgnIrq (spinlock_t *psl)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
}
/*********************************************************************************************************
** ��������: _UpSpinLockIrq
** ��������: ��������������, ��ͬ�����ж�
** �䡡��  : psl               ������
**           piregInterLevel   �ж�������Ϣ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinLockIrq (spinlock_t *psl, INTREG  *piregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
    
    *piregInterLevel = KN_INT_DISABLE();
}
/*********************************************************************************************************
** ��������: _UpSpinTryLockIrq
** ��������: ���������Լ�������, ��ͬ�����ж�
** �䡡��  : psl               ������
**           piregInterLevel   �ж�������Ϣ
** �䡡��  : LW_TRUE �������� LW_FALSE ����ʧ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
BOOL  _UpSpinTryLockIrq (spinlock_t *psl, INTREG  *piregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
    
    *piregInterLevel = KN_INT_DISABLE();
    
    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: _UpSpinUnlockIrq
** ��������: ��������������, ��ͬ�����ж�
** �䡡��  : psl               ������
**           iregInterLevel    �ж�������Ϣ
** �䡡��  : ����������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  _UpSpinUnlockIrq (spinlock_t *psl, INTREG  iregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur   = LW_CPU_GET_CUR();
    BOOL            bTrySched = LW_FALSE;
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(pcpuCur->CPU_ptcbTCBCur);                     /*  �����������                */
        if (__COULD_SCHED(pcpuCur, 0)) {
            bTrySched = LW_TRUE;                                        /*  ��Ҫ���Ե���                */
        }
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    if (bTrySched) {
        return  (_ThreadSched(pcpuCur->CPU_ptcbTCBCur));
    
    } else {
        return  (ERROR_NONE);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: _UpSpinUnlockIrqQuick
** ��������: ��������������, ��ͬ�����ж�, �����г��Ե���
** �䡡��  : psl               ������
**           iregInterLevel    �ж�������Ϣ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinUnlockIrqQuick (spinlock_t *psl, INTREG  iregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(pcpuCur->CPU_ptcbTCBCur);                     /*  ���������ڵ�ǰ CPU          */
    }
    
    KN_INT_ENABLE(iregInterLevel);
}
/*********************************************************************************************************
** ��������: _UpSpinUnlockSched
** ��������: ���������������л���ɺ�ר���ͷź��� (���ж�״̬�±�����)
** �䡡��  : psl           ������
**           ptcbOwner     ���ĳ�����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _UpSpinUnlockSched (spinlock_t *psl, PLW_CLASS_TCB ptcbOwner)
{
    PLW_CLASS_CPU   pcpuCur = LW_CPU_GET_CUR();
    
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(ptcbOwner);                                   /*  ���������ڵ�ǰ CPU          */
    }
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
