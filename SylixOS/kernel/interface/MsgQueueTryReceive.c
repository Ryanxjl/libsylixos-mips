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
** ��   ��   ��: MsgQueueTryReceive.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 01 �� 07 ��
**
** ��        ��: �޵ȴ������Ϣ������Ϣ

** BUG
2007.09.19  ���� _DebugHandle() ���ܡ�
2009.04.08  ����� SMP ��˵�֧��.
2009.06.25  pulMsgLen ����Ϊ NULL,
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_MsgQueueTryReceive
** ��������: �޵ȴ������Ϣ������Ϣ
** �䡡��  : 
**           ulId            ��Ϣ���о��
**           pvMsgBuffer     ��Ϣ������
**           stMaxByteSize   ��Ϣ��������С
**           pstMsgLen       ��Ϣ����
** �䡡��  : ERROR_CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if (LW_CFG_MSGQUEUE_EN > 0) && (LW_CFG_MAX_MSGQUEUES > 0)

LW_API  
ULONG  API_MsgQueueTryReceive (LW_OBJECT_HANDLE    ulId,
                               PVOID               pvMsgBuffer,
                               size_t              stMaxByteSize,
                               size_t             *pstMsgLen)
{
             INTREG                iregInterLevel;
    REGISTER UINT16                usIndex;
    REGISTER PLW_CLASS_EVENT       pevent;
    REGISTER PLW_CLASS_MSGQUEUE    pmsgqueue;
            
             size_t                stMsgLenTemp;
             
    usIndex = _ObjectGetIndex(ulId);
    
    if (pstMsgLen == LW_NULL) {
        pstMsgLen =  &stMsgLenTemp;                                     /*  ��ʱ������¼��Ϣ����        */
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!pvMsgBuffer) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "pvMsgBuffer invalidate.\r\n");
        _ErrorHandle(ERROR_MSGQUEUE_MSG_NULL);
        return  (ERROR_MSGQUEUE_MSG_NULL);
    }
    if (!_ObjectClassOK(ulId, _OBJECT_MSGQUEUE)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "msgqueue handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_Event_Index_Invalid(usIndex)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "msgqueue handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    pevent = &_K_eventBuffer[usIndex];
    
    LW_SPIN_LOCK_QUICK(&pevent->EVENT_slLock, &iregInterLevel);         /*  �ر��ж�ͬʱ��ס spinlock   */
    
    if (_Event_Type_Invalid(usIndex, LW_TYPE_EVENT_MSGQUEUE)) {
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "msgqueue handle invalidate.\r\n");
        _ErrorHandle(ERROR_MSGQUEUE_TYPE);
        return  (ERROR_MSGQUEUE_TYPE);
    }
    
    pmsgqueue = (PLW_CLASS_MSGQUEUE)pevent->EVENT_pvPtr;
    
    if (pevent->EVENT_ulCounter) {                                      /*  �¼���Ч                    */
        pevent->EVENT_ulCounter--;
        
        _MsgQueueGetMsg(pmsgqueue, 
                        pvMsgBuffer, 
                        stMaxByteSize,
                        pstMsgLen);                                     /*  �����Ϣ                    */
        
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        
        return  (ERROR_NONE);
    
    } else {                                                            /*  �¼���Ч                    */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
    
        _ErrorHandle(ERROR_THREAD_WAIT_TIMEOUT);                        /*  û���¼�����                */
        return  (ERROR_THREAD_WAIT_TIMEOUT);
    }
}
#endif                                                                  /*  (LW_CFG_MSGQUEUE_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_MSGQUEUES > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
