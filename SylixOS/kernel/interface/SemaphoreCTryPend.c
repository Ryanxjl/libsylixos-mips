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
** ��   ��   ��: SemaphoreCTryPend.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 07 �� 21 ��
**
** ��        ��: �������ĵȴ��������ź���

** BUG
2007.07.21  ���� _DebugHandle() ���ܡ�
2009.04.08  ����� SMP ��˵�֧��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
       �����ֲ� Pend API �������ж������е����⣬������ģʽʱ���൱�� Pend ���� LW_OPTION_NOT_WAIT
*********************************************************************************************************/
/*********************************************************************************************************
** ��������: API_SemaphoreCTryPend
** ��������: �������ĵȴ��������ź���
** �䡡��  : 
**           ulId            �¼����
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if (LW_CFG_SEMC_EN > 0) && (LW_CFG_MAX_EVENTS > 0)

LW_API  
ULONG  API_SemaphoreCTryPend (LW_OBJECT_HANDLE  ulId)
{
             INTREG                iregInterLevel;
    REGISTER UINT16                usIndex;
    REGISTER PLW_CLASS_EVENT       pevent;
    
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_SEM_C)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_Event_Index_Invalid(usIndex)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    pevent = &_K_eventBuffer[usIndex];
    
    LW_SPIN_LOCK_QUICK(&pevent->EVENT_slLock, &iregInterLevel);         /*  �ر��ж�ͬʱ��ס spinlock   */
    
    if (_Event_Type_Invalid(usIndex, LW_TYPE_EVENT_SEMC)) {
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        _ErrorHandle(ERROR_EVENT_TYPE);
        return  (ERROR_EVENT_TYPE);
    }
    
    if (pevent->EVENT_ulCounter) {                                      /*  �¼���Ч                    */
        pevent->EVENT_ulCounter--;
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
    
        return  (ERROR_NONE);
    
    } else {                                                            /*  �¼���Ч                    */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
    
        _ErrorHandle(ERROR_THREAD_WAIT_TIMEOUT);                        /*  û���¼�����                */
        return  (ERROR_THREAD_WAIT_TIMEOUT);
    }
}

#endif                                                                  /*  (LW_CFG_SEMC_EN > 0)        */
                                                                        /*  (LW_CFG_MAX_EVENTS > 0)     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
