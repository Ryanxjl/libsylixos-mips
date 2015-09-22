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
** ��   ��   ��: SemaphorePostPend.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 01 �� 16 ��
**
** ��        ��: �� SMP ϵͳ�½��ͬ�����ͷ�һ���ź�����ͬʱ��ʼ��ȡ����һ���ź���.

** BUG:
2010.08.03  ʹ���µĻ�ȡϵͳʱ�ӷ���.
2011.02.23  ���� LW_OPTION_SIGNAL_INTER ѡ��, �¼�����ѡ���Լ��Ƿ�ɱ��жϴ��.
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
2013.05.05  �жϵ���������ֵ, �������������û����˳�.
2013.07.18  ʹ���µĻ�ȡ TCB �ķ���, ȷ�� SMP ϵͳ��ȫ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_SemaphorePostBPend
** ��������: �ͷ�һ���ź�����������ʼ�ȴ�����һ���ź���(�м��������л�����)
** �䡡��  : ulIdPost             ��Ҫ�ͷŵ��ź���
**           ulId                 ��Ҫ�ȴ����ź��� (��ֵ�ź���)
**           ulTimeout            �ȴ���ʱʱ��            
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if ((LW_CFG_SEMB_EN > 0) || (LW_CFG_SEMM_EN > 0)) && (LW_CFG_MAX_EVENTS > 0)

LW_API  
ULONG  API_SemaphorePostBPend (LW_OBJECT_HANDLE  ulIdPost, 
                               LW_OBJECT_HANDLE  ulId,
                               ULONG             ulTimeout)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
    REGISTER UINT16                usIndex;
    REGISTER ULONG                 ulObjectClass;
             ULONG                 ulError;
    
    REGISTER PLW_CLASS_EVENT       peventPost;
    REGISTER PLW_CLASS_EVENT       pevent;
    REGISTER UINT8                 ucPriorityIndex;
    REGISTER PLW_LIST_RING        *ppringList;
             ULONG                 ulTimeSave;                          /*  ϵͳ�¼���¼                */
             INT                   iSchedRet;
             
             ULONG                 ulEventOption;                       /*  �¼�����ѡ��                */
             
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */

    usIndex       = _ObjectGetIndex(ulIdPost);
    ulObjectClass = _ObjectGetClass(ulIdPost);
    
    if ((ulObjectClass != _OBJECT_SEM_B) &&
        (ulObjectClass != _OBJECT_SEM_C) &&
        (ulObjectClass != _OBJECT_SEM_M)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulIdPost invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);                         /*  ������ʹ���                */
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (_Event_Index_Invalid(usIndex)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulIdPost invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    peventPost = &_K_eventBuffer[usIndex];
    
    usIndex = _ObjectGetIndex(ulId);
    
    /*
     *  ���� post �ź���������, �ȴ��Ĺ�����, ������ CPU �����ܲ��� Post �ź���.
     */
    LW_SPIN_LOCK(&peventPost->EVENT_slLock);                            /*  ��ס spinlock               */
    
    /*
     *  SylixOS ������������!
     */
    API_SemaphorePost(ulIdPost);                                        /*  �ͷ��ź���                  */
    
__wait_again:
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_SEM_B)) {
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        ulError = ERROR_KERNEL_HANDLE_NULL;
        goto    __wait_over;
    }
    if (_Event_Index_Invalid(usIndex)) {
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        ulError = ERROR_KERNEL_HANDLE_NULL;
        goto    __wait_over;
    }
#endif
    pevent = &_K_eventBuffer[usIndex];

    LW_SPIN_LOCK_QUICK(&pevent->EVENT_slLock, &iregInterLevel);         /*  �ر��ж�ͬʱ��ס spinlock   */
    
    if (_Event_Type_Invalid(usIndex, LW_TYPE_EVENT_SEMB)) {
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        ulError = ERROR_EVENT_TYPE;
        goto    __wait_over;
    }
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    
    if (pevent->EVENT_ulCounter) {                                      /*  �¼���Ч                    */
        pevent->EVENT_ulCounter = LW_FALSE;
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        ulError = ERROR_NONE;
        goto    __wait_over;
    }
    
    if (ulTimeout == LW_OPTION_NOT_WAIT) {                              /*  ���ȴ�                      */
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        ulError = ERROR_THREAD_WAIT_TIMEOUT;                            /*  ��ʱ                        */
        goto    __wait_over;
    }
    
    ptcbCur->TCB_usStatus      |= LW_THREAD_STATUS_SEM;                 /*  д״̬λ����ʼ�ȴ�          */
    ptcbCur->TCB_ucWaitTimeout  = LW_WAIT_TIME_CLEAR;                   /*  ��յȴ�ʱ��                */
    
    if (ulTimeout == LW_OPTION_WAIT_INFINITE) {                         /*  �Ƿ�������ȴ�              */
	    ptcbCur->TCB_ulDelay = 0ul;
	} else {
	    ptcbCur->TCB_ulDelay = ulTimeout;                               /*  ���ó�ʱʱ��                */
	}
    __KERNEL_TIME_GET_NO_SPINLOCK(ulTimeSave, ULONG);                   /*  ��¼ϵͳʱ��                */
        
    if (pevent->EVENT_ulOption & LW_OPTION_WAIT_PRIORITY) {             /*  �����ȼ��ȴ�                */
        _EVENT_INDEX_Q_PRIORITY(ptcbCur->TCB_ucPriority, ucPriorityIndex);
        _EVENT_PRIORITY_Q_PTR(ppringList, ucPriorityIndex);
        ptcbCur->TCB_ppringPriorityQueue = ppringList;                  /*  ��¼�ȴ�����λ��            */
        _EventWaitPriority(pevent, ppringList);                         /*  �������ȼ��ȴ���            */
    
    } else {                                                            /*  �� FIFO �ȴ�                */
        _EVENT_FIFO_Q_PTR(ppringList);                                  /*  ȷ�� FIFO ���е�λ��        */
        _EventWaitFifo(pevent, ppringList);                             /*  ���� FIFO �ȴ���            */
    }
    
    LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                          /*  �ͷ� post �ź���������      */
    LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);        /*  ���ж�, ͬʱ�� spinlock */

    ulEventOption = pevent->EVENT_ulOption;
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_SEMB, MONITOR_EVENT_SEM_PEND, 
                      ulId, ulTimeout, LW_NULL);
    
    iSchedRet = __KERNEL_EXIT();                                        /*  ����������                  */
    if (iSchedRet == LW_SIGNAL_EINTR) {
        if (ulEventOption & LW_OPTION_SIGNAL_INTER) {
            _ErrorHandle(EINTR);
            return  (EINTR);
        }
        ulTimeout = _sigTimeoutRecalc(ulTimeSave, ulTimeout);           /*  ���¼��㳬ʱʱ��            */
        goto    __wait_again;
    
    } else if (iSchedRet == LW_SIGNAL_RESTART) {
        ulTimeout = _sigTimeoutRecalc(ulTimeSave, ulTimeout);           /*  ���¼��㳬ʱʱ��            */
        goto    __wait_again;
    }
    
    if (ptcbCur->TCB_ucWaitTimeout == LW_WAIT_TIME_OUT) {               /*  �ȴ���ʱ                    */
        LW_SPIN_LOCK_QUICK(&pevent->EVENT_slLock, &iregInterLevel);     /*  �ر��ж�, ��ס spinlock     */
        __KERNEL_ENTER();                                               /*  �����ں�                    */
        if (ptcbCur->TCB_ucWaitTimeout == LW_WAIT_TIME_CLEAR) {         /*  �Ƿ�������˲�䱻����        */
            LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, 
                                 iregInterLevel);                       /*  ���ж�, ͬʱ�� spinlock */
            __KERNEL_EXIT();                                            /*  �˳��ں�                    */
            ulError = ERROR_NONE;                                       /*  ����                        */
            goto    __wait_over;
        }
        
        if (pevent->EVENT_ulOption & LW_OPTION_WAIT_PRIORITY) {
            _EventTimeoutPriority(pevent, ppringList);                  /*  �ȴ���ʱ�ָ�                */
        } else {
            _EventTimeoutFifo(pevent, ppringList);                      /*  �ȴ���ʱ�ָ�                */
        }
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        ulError = ERROR_THREAD_WAIT_TIMEOUT;                            /*  ��ʱ                        */
        
    } else {
        if (ptcbCur->TCB_ucIsEventDelete == LW_EVENT_EXIST) {           /*  �¼��Ƿ����                */
            ulError = ERROR_NONE;                                       /*  ����                        */
        } else {
            ulError = ERROR_EVENT_WAS_DELETED;                          /*  �Ѿ���ɾ��                  */
        }
    }
    
__wait_over:
    _ErrorHandle(ulError);
    return  (ulError);
}

#endif                                                                  /*  ((LW_CFG_SEMB_EN > 0) ||    */
                                                                        /*   (LW_CFG_SEMM_EN > 0)) &&   */
                                                                        /*  (LW_CFG_MAX_EVENTS > 0)     */
/*********************************************************************************************************
** ��������: API_SemaphorePostCPend
** ��������: �ͷ�һ���ź�����������ʼ�ȴ�����һ���ź���(�м��������л�����)
** �䡡��  : ulIdPost             ��Ҫ�ͷŵ��ź���
**           ulId                 ��Ҫ�ȴ����ź��� (�����ź���)
**           ulTimeout            �ȴ���ʱʱ��            
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if ((LW_CFG_SEMC_EN > 0) || (LW_CFG_SEMM_EN > 0)) && (LW_CFG_MAX_EVENTS > 0)

LW_API  
ULONG  API_SemaphorePostCPend (LW_OBJECT_HANDLE  ulIdPost, 
                               LW_OBJECT_HANDLE  ulId,
                               ULONG             ulTimeout)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
    REGISTER UINT16                usIndex;
    REGISTER ULONG                 ulObjectClass;
             ULONG                 ulError;
    
    REGISTER PLW_CLASS_EVENT       peventPost;
    REGISTER PLW_CLASS_EVENT       pevent;
    REGISTER UINT8                 ucPriorityIndex;
    REGISTER PLW_LIST_RING        *ppringList;
             ULONG                 ulTimeSave;                          /*  ϵͳ�¼���¼                */
             INT                   iSchedRet;
             
             ULONG                 ulEventOption;                       /*  �¼�����ѡ��                */
             
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */

    usIndex       = _ObjectGetIndex(ulIdPost);
    ulObjectClass = _ObjectGetClass(ulIdPost);
    
    if ((ulObjectClass != _OBJECT_SEM_B) &&
        (ulObjectClass != _OBJECT_SEM_C) &&
        (ulObjectClass != _OBJECT_SEM_M)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulIdPost invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);                         /*  ������ʹ���                */
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (_Event_Index_Invalid(usIndex)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulIdPost invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    peventPost = &_K_eventBuffer[usIndex];
    
    usIndex = _ObjectGetIndex(ulId);
    
    /*
     *  ���� post �ź���������, �ȴ��Ĺ�����, ������ CPU �����ܲ��� Post �ź���.
     */
    LW_SPIN_LOCK(&peventPost->EVENT_slLock);                            /*  ��ס spinlock               */
    
    /*
     *  SylixOS ������������!
     */
    API_SemaphorePost(ulIdPost);                                        /*  �ͷ��ź���                  */
    
__wait_again:
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_SEM_C)) {
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        ulError = ERROR_KERNEL_HANDLE_NULL;
        goto    __wait_over;
    }
    if (_Event_Index_Invalid(usIndex)) {
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        ulError = ERROR_KERNEL_HANDLE_NULL;
        goto    __wait_over;
    }
#endif
    pevent = &_K_eventBuffer[usIndex];

    LW_SPIN_LOCK_QUICK(&pevent->EVENT_slLock, &iregInterLevel);         /*  �ر��ж�ͬʱ��ס spinlock   */
    
    if (_Event_Type_Invalid(usIndex, LW_TYPE_EVENT_SEMC)) {
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "semaphore handle invalidate.\r\n");
        ulError = ERROR_EVENT_TYPE;
        goto    __wait_over;
    }
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    
    if (pevent->EVENT_ulCounter) {                                      /*  �¼���Ч                    */
        pevent->EVENT_ulCounter--;
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        ulError = ERROR_NONE;
        goto    __wait_over;
    }
    
    if (ulTimeout == LW_OPTION_NOT_WAIT) {                              /*  ���ȴ�                      */
        LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                      /*  �ͷ� post �ź���������      */
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        ulError = ERROR_THREAD_WAIT_TIMEOUT;                            /*  ��ʱ                        */
        goto    __wait_over;
    }
    
    ptcbCur->TCB_usStatus      |= LW_THREAD_STATUS_SEM;                 /*  д״̬λ����ʼ�ȴ�          */
    ptcbCur->TCB_ucWaitTimeout  = LW_WAIT_TIME_CLEAR;                   /*  ��յȴ�ʱ��                */
    
    if (ulTimeout == LW_OPTION_WAIT_INFINITE) {                         /*  �Ƿ�������ȴ�              */
	    ptcbCur->TCB_ulDelay = 0ul;
	} else {
	    ptcbCur->TCB_ulDelay = ulTimeout;                               /*  ���ó�ʱʱ��                */
	}
    __KERNEL_TIME_GET_NO_SPINLOCK(ulTimeSave, ULONG);                   /*  ��¼ϵͳʱ��                */
        
    if (pevent->EVENT_ulOption & LW_OPTION_WAIT_PRIORITY) {             /*  �����ȼ��ȴ�                */
        _EVENT_INDEX_Q_PRIORITY(ptcbCur->TCB_ucPriority, ucPriorityIndex);
        _EVENT_PRIORITY_Q_PTR(ppringList, ucPriorityIndex);
        ptcbCur->TCB_ppringPriorityQueue = ppringList;                  /*  ��¼�ȴ�����λ��            */
        _EventWaitPriority(pevent, ppringList);                         /*  �������ȼ��ȴ���            */
    
    } else {                                                            /*  �� FIFO �ȴ�                */
        _EVENT_FIFO_Q_PTR(ppringList);                                  /*  ȷ�� FIFO ���е�λ��        */
        _EventWaitFifo(pevent, ppringList);                             /*  ���� FIFO �ȴ���            */
    }
    
    LW_SPIN_UNLOCK(&peventPost->EVENT_slLock);                          /*  �ͷ� post �ź���������      */
    LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);        /*  ���ж�, ͬʱ�� spinlock */

    ulEventOption = pevent->EVENT_ulOption;
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_SEMC, MONITOR_EVENT_SEM_PEND, 
                      ulId, ulTimeout, LW_NULL);
    
    iSchedRet = __KERNEL_EXIT();                                        /*  ����������                  */
    if (iSchedRet == LW_SIGNAL_EINTR) {
        if (ulEventOption & LW_OPTION_SIGNAL_INTER) {
            _ErrorHandle(EINTR);
            return  (EINTR);
        }
        ulTimeout = _sigTimeoutRecalc(ulTimeSave, ulTimeout);           /*  ���¼��㳬ʱʱ��            */
        goto    __wait_again;
    
    } else if (iSchedRet == LW_SIGNAL_RESTART) {
        ulTimeout = _sigTimeoutRecalc(ulTimeSave, ulTimeout);           /*  ���¼��㳬ʱʱ��            */
        goto    __wait_again;
    }
    
    if (ptcbCur->TCB_ucWaitTimeout == LW_WAIT_TIME_OUT) {               /*  �ȴ���ʱ                    */
        LW_SPIN_LOCK_QUICK(&pevent->EVENT_slLock, &iregInterLevel);     /*  �ر��ж�, ��ס spinlock     */
        __KERNEL_ENTER();                                               /*  �����ں�                    */
        if (ptcbCur->TCB_ucWaitTimeout == LW_WAIT_TIME_CLEAR) {         /*  �Ƿ�������˲�䱻����        */
            LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, 
                                 iregInterLevel);                       /*  ���ж�, ͬʱ�� spinlock */
            __KERNEL_EXIT();                                            /*  �˳��ں�                    */
            ulError = ERROR_NONE;                                       /*  ����                        */
            goto    __wait_over;
        }
        
        if (pevent->EVENT_ulOption & LW_OPTION_WAIT_PRIORITY) {
            _EventTimeoutPriority(pevent, ppringList);                  /*  �ȴ���ʱ�ָ�                */
        } else {
            _EventTimeoutFifo(pevent, ppringList);                      /*  �ȴ���ʱ�ָ�                */
        }
        LW_SPIN_UNLOCK_QUICK(&pevent->EVENT_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        ulError = ERROR_THREAD_WAIT_TIMEOUT;                            /*  ��ʱ                        */
        
    } else {
        if (ptcbCur->TCB_ucIsEventDelete == LW_EVENT_EXIST) {           /*  �¼��Ƿ����                */
            ulError = ERROR_NONE;                                       /*  ����                        */
        } else {
            ulError = ERROR_EVENT_WAS_DELETED;                          /*  �Ѿ���ɾ��                  */
        }
    }
    
__wait_over:
    _ErrorHandle(ulError);
    return  (ulError);
}

#endif                                                                  /*  ((LW_CFG_SEMC_EN > 0) ||    */
                                                                        /*   (LW_CFG_SEMM_EN > 0)) &&   */
                                                                        /*  (LW_CFG_MAX_EVENTS > 0)     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
