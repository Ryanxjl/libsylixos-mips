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
** ��   ��   ��: ThreadLock.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 18 ��
**
** ��        ��: ��������������.

** BUG
2007.11.04  �� 0xFFFFFFFF ��Ϊ __ARCH_ULONG_MAX
2008.01.20  ��ȫ������Ϊ�ֲ���.
2013.07.18  ���� SMP ϵͳ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_ThreadLock
** ��������: ����������ǰ CPU ������
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �� API ���Ե�ǰ CPU ��Ч, ������Ӱ������ CPU ����. ����������״̬����ʹ����������.

                                           API ����
*********************************************************************************************************/
LW_API
VOID  API_ThreadLock (VOID)
{
    PLW_CLASS_TCB   ptcbCur;
    
    if (!LW_SYS_STATUS_IS_RUNNING()) {                                  /*  ϵͳ�����Ѿ�����            */
        _ErrorHandle(ERROR_KERNEL_NOT_RUNNING);
        return;
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    if (__THREAD_LOCK_GET(ptcbCur) != __ARCH_ULONG_MAX) {
        __THREAD_LOCK_INC(ptcbCur);
    }
    
    KN_SMP_MB();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/