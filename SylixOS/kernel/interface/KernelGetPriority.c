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
** ��   ��   ��: KernelGetPriority.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 05 �� 11 ��
**
** ��        ��: �û����Ե������ API ����ں�������ȼ����������߳����ȼ�
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_KernelGetPriorityMax
** ��������: ����ں�������ȼ�
** �䡡��  : NONE
** �䡡��  : �����߳����ȼ�
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
UINT8  API_KernelGetPriorityMax (VOID)
{
    return  (LW_PRIO_LOWEST);
}
/*********************************************************************************************************
** ��������: API_KernelGetPriorityMin
** ��������: ����ں�������ȼ���
** �䡡��  : NONE
** �䡡��  : __HIGHEST_PRIO
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
UINT8  API_KernelGetPriorityMin (VOID)
{
    return  (LW_PRIO_HIGHEST);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
