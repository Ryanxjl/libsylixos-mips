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
** ��   ��   ��: InterLock.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 18 ��
**
** ��        ��: ϵͳ�ر��ж�
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_InterLock
** ��������: ϵͳ�ر��ж�
** �䡡��  : piregInterLevel   �����жϿ�����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API
ULONG  API_InterLock (INTREG  *piregInterLevel)
{
#if LW_CFG_ARG_CHK_EN > 0
    if (!piregInterLevel) {
        _ErrorHandle(ERROR_INTER_LEVEL_NULL);
        return  (ERROR_INTER_LEVEL_NULL);
    }
#endif
        
    *piregInterLevel = KN_INT_DISABLE();
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_InterUnlock
** ��������: ϵͳ���ж�
** �䡡��  : iregInterLevel    ��Ҫ�ָ����жϿ�����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API
ULONG  API_InterUnlock (INTREG  iregInterLevel)
{
    KN_INT_ENABLE(iregInterLevel);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
