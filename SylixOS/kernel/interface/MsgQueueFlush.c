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
** ��   ��   ��: MsgQueueFlush.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 01 �� 09 ��
**
** ��        ��: �����еȴ���Ϣ���е��߳̾���

** BUG
2007.09.19  ���� _DebugHandle() ���ܡ�
2008.05.18  ʹ�� __KERNEL_ENTER() ���� ThreadLock();
2009.04.08  ����� SMP ��˵�֧��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_MsgQueueFlush
** ��������: �����еȴ���Ϣ���е��߳̾���
** �䡡��  : 
**           ulId                   ��Ϣ���о��
**           pulThreadUnblockNum    ���������߳�����   ����ΪNULL
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if (LW_CFG_MSGQUEUE_EN > 0) && (LW_CFG_MAX_MSGQUEUES > 0)

LW_API  
ULONG  API_MsgQueueFlush (LW_OBJECT_HANDLE  ulId, ULONG  *pulThreadUnblockNum)
{
             INTREG                iregInterLevel;
    REGISTER UINT16                usIndex;
    REGISTER PLW_CLASS_EVENT       pevent;
    REGISTER PLW_CLASS_TCB         ptcb;
    REGISTER PLW_LIST_RING        *ppringList;                          /*  �ȴ����е�ַ                */
    
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_MSGQUEUE)) {                      /*  �����Ƿ���ȷ                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "msgqueue handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_Event_Index_Invalid(usIndex)) {                                /*  �±��Ƿ�����ȷ              */
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
    
    if (pulThreadUnblockNum) {                                          /*  ���潫Ҫ�������߳�����      */
        *pulThreadUnblockNum = _EventWaitNum(pevent);
    }
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    
    while (_EventWaitNum(pevent)) {                                     /*  �Ƿ�������ڵȴ�������      */
    
        if (pevent->EVENT_ulOption & LW_OPTION_WAIT_PRIORITY) {         /*  ���ȼ��ȴ�����              */
            _EVENT_DEL_Q_PRIORITY(ppringList);                          /*  �����Ҫ����Ķ���          */
                                                                        /*  �������ȼ��ȴ��߳�          */
            ptcb = _EventReadyPriorityLowLevel(pevent, LW_NULL, ppringList);
        
        } else {
            _EVENT_DEL_Q_FIFO(ppringList);                              /*  ���� FIFO ����ָ���ַ      */
                                                                        /*  ����FIFO�ȴ��߳�            */
            ptcb = _EventReadyFifoLowLevel(pevent, LW_NULL, ppringList);
    
        }
        
        *ptcb->TCB_pstMsgByteSize = 0;                                  /*  ��Ϣ����Ϊ 0                */

        /*
         *  ע��: ���²���û���ͷ� spinlock.
         */
        KN_INT_ENABLE(iregInterLevel);                                  /*  ���ж�                    */
        _EventReadyHighLevel(ptcb, LW_THREAD_STATUS_MSGQUEUE);          /*  ���� TCB                    */
        iregInterLevel = KN_INT_DISABLE();                              /*  �ر��ж�                    */
    }
    
    LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);        /*  ���ж�, ͬʱ�� spinlock */
    
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_MSGQ, MONITOR_EVENT_MSGQ_FLUSH, ulId, LW_NULL);
    
    return  (ERROR_NONE);
}
#endif                                                                  /*  (LW_CFG_MSGQUEUE_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_MSGQUEUES > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
