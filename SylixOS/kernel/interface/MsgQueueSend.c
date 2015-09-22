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
** ��   ��   ��: MsgQueueSend.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 01 �� 07 ��
**
** ��        ��: ����Ϣ���з�����Ϣ(���׽ӿں���)

** BUG
2007.09.19  ���� _DebugHandle() ���ܡ�
2008.05.18  ʹ�� __KERNEL_ENTER() ���� ThreadLock();
2009.04.08  ����� SMP ��˵�֧��.
2010.01.22  ���������ں˵�ʱ��.
2013.03.17  ������Զ��ض���Ϣʱ���ж�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_MsgQueueSend
** ��������: ����Ϣ���з�����Ϣ
** �䡡��  : 
**           ulId                   ��Ϣ���о��
**           pvMsgBuffer            ��Ϣ������
**           stMsgLen               ��Ϣ����
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if (LW_CFG_MSGQUEUE_EN > 0) && (LW_CFG_MAX_MSGQUEUES > 0)

LW_API  
ULONG  API_MsgQueueSend (LW_OBJECT_HANDLE  ulId,
                         const PVOID       pvMsgBuffer,
                         size_t            stMsgLen)
{
             INTREG                iregInterLevel;
    REGISTER UINT16                usIndex;
    REGISTER PLW_CLASS_EVENT       pevent;
    REGISTER PLW_CLASS_MSGQUEUE    pmsgqueue;
    REGISTER PLW_CLASS_TCB         ptcb;
    REGISTER PLW_LIST_RING        *ppringList;                          /*  �ȴ����е�ַ                */
    
    REGISTER size_t                stRealLen;
    
    usIndex = _ObjectGetIndex(ulId);
    
__re_send:
#if LW_CFG_ARG_CHK_EN > 0
    if (!pvMsgBuffer) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "pvMsgBuffer invalidate.\r\n");
        _ErrorHandle(ERROR_MSGQUEUE_MSG_NULL);
        return  (ERROR_MSGQUEUE_MSG_NULL);
    }
    if (!stMsgLen) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulMsgLen invalidate.\r\n");
        _ErrorHandle(ERROR_MSGQUEUE_MSG_LEN);
        return  (ERROR_MSGQUEUE_MSG_LEN);
    }
    if (!_ObjectClassOK(ulId, _OBJECT_MSGQUEUE)) {                     /*  �����Ƿ���ȷ                 */
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
    pmsgqueue = (PLW_CLASS_MSGQUEUE)pevent->EVENT_pvPtr;
    
    if (stMsgLen > pmsgqueue->MSGQUEUE_stEachMsgByteSize) {             /*  ����̫��                    */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulMsgLen invalidate.\r\n");
        _ErrorHandle(ERROR_MSGQUEUE_MSG_LEN);
        return  (ERROR_MSGQUEUE_MSG_LEN);
    }
    
    if (_EventWaitNum(pevent)) {
        BOOL    bSendOk = LW_TRUE;
        
        __KERNEL_ENTER();                                               /*  �����ں�                    */
        
        if (pevent->EVENT_ulOption & LW_OPTION_WAIT_PRIORITY) {         /*  ���ȼ��ȴ�����              */
            _EVENT_DEL_Q_PRIORITY(ppringList);                          /*  �����Ҫ����Ķ���          */
                                                                        /*  �������ȼ��ȴ��߳�          */
            ptcb = _EventReadyPriorityLowLevel(pevent, LW_NULL, ppringList);
        
        } else {
            _EVENT_DEL_Q_FIFO(ppringList);                              /*  �����Ҫ�����FIFO����      */
                                                                        /*  ����FIFO�ȴ��߳�            */
            ptcb = _EventReadyFifoLowLevel(pevent, LW_NULL, ppringList);
    
        }
        
        if ((stMsgLen > ptcb->TCB_stMaxByteSize) && 
            !(ptcb->TCB_ulRecvOption & LW_OPTION_NOERROR)) {            /*  �Ƿ������Զ��ض�            */
            *ptcb->TCB_pstMsgByteSize = 0;
            ptcb->TCB_stMaxByteSize = 0;
            bSendOk = LW_FALSE;
            
        } else {
            stRealLen = (stMsgLen < ptcb->TCB_stMaxByteSize) ?
                        (stMsgLen) : (ptcb->TCB_stMaxByteSize);         /*  ȷ����Ϣ��������            */
            
            *ptcb->TCB_pstMsgByteSize = stRealLen;                      /*  ���泤��                    */
            lib_memcpy(ptcb->TCB_pvMsgQueueMessage,                     /*  ������Ϣ                    */
                       pvMsgBuffer, 
                       stRealLen);
        }
                   
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        
        _EventReadyHighLevel(ptcb, LW_THREAD_STATUS_MSGQUEUE);          /*  ���� TCB                    */
        
        MONITOR_EVT_LONG2(MONITOR_EVENT_ID_MSGQ, MONITOR_EVENT_MSGQ_POST, 
                          ulId, ptcb->TCB_ulId, LW_NULL);
        
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        
        if (bSendOk == LW_FALSE) {
            goto    __re_send;                                          /*  ���·���                    */
        }
        return  (ERROR_NONE);
        
    } else {                                                            /*  û���̵߳ȴ�                */
        if (pevent->EVENT_ulCounter < pevent->EVENT_ulMaxCounter) {     /*  ����Ƿ��пռ��          */
            pevent->EVENT_ulCounter++;
            _MsgQueueSendMsg(pmsgqueue, pvMsgBuffer, stMsgLen);         /*  ������Ϣ                    */
            LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);
                                                                        /*  ���ж�, ͬʱ�� spinlock */
            return  (ERROR_NONE);
        
        } else {                                                        /*  �Ѿ�����                    */
            LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);
                                                                        /*  ���ж�, ͬʱ�� spinlock */
            
            _ErrorHandle(ERROR_MSGQUEUE_FULL);
            return  (ERROR_MSGQUEUE_FULL);
        }
    }
}

#endif                                                                  /*  (LW_CFG_MSGQUEUE_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_MSGQUEUES > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
