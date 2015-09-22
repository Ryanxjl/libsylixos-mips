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
** ��   ��   ��: EventSetSet.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 20 ��
**
** ��        ��: �ͷ��¼�������¼�.

** BUG
2007.11.13  ʹ���������������������ȫ��װ.
2007.11.18  ����ע��.
2008.01.13  ���� _DebugHandle() ���ܡ�
2009.04.08  ����� SMP ��˵�֧��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_EventSetSet
** ��������: �ͷ��¼�������¼�
** �䡡��  : 
**           ulId            �¼������
**           ulEvent         �ͷ��¼�
**           ulOption        �ͷŷ���ѡ��
** �䡡��  : �¼����
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if (LW_CFG_EVENTSET_EN > 0) && (LW_CFG_MAX_EVENTSETS > 0)

LW_API  
ULONG  API_EventSetSet (LW_OBJECT_HANDLE  ulId, 
                        ULONG             ulEvent,
                        ULONG             ulOption)
{
             INTREG                    iregInterLevel;
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_EVENTSET        pes;
    REGISTER PLW_CLASS_EVENTSETNODE    pesn;
             PLW_LIST_LINE             plineList;
    REGISTER ULONG                     ulEventRdy;
    
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_EVENT_SET)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "eventset handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_EventSet_Index_Invalid(usIndex)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "eventset handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    pes = &_K_esBuffer[usIndex];
    
    LW_SPIN_LOCK_QUICK(&pes->EVENTSET_slLock, &iregInterLevel);         /*  �ر��ж�ͬʱ��ס spinlock   */
    
    if (_EventSet_Type_Invalid(usIndex, LW_TYPE_EVENT_EVENTSET)) {
        LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "eventset handle invalidate.\r\n");
        _ErrorHandle(ERROR_EVENTSET_TYPE);
        return  (ERROR_EVENTSET_TYPE);
    }

    switch (ulOption) {
    
    case LW_OPTION_EVENTSET_CLR:                                        /*  �������                    */
        pes->EVENTSET_ulEventSets &= (~ulEvent);
        break;
    
    case LW_OPTION_EVENTSET_SET:                                        /*  ��λ����                    */
        pes->EVENTSET_ulEventSets |= ulEvent;
        break;
    
    default:
        LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulOption invalidate.\r\n");
        _ErrorHandle(ERROR_EVENTSET_OPTION);
        return  (ERROR_EVENTSET_OPTION);
    }
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    
    for (plineList  = pes->EVENTSET_plineWaitList;                      /*  ��ͷָ��                    */
         plineList != LW_NULL;
         plineList  = _list_line_get_next(plineList)) {                 /*  �ͷ�������Ч�߳�            */
         
        pesn = _LIST_ENTRY(plineList, LW_CLASS_EVENTSETNODE, EVENTSETNODE_lineManage);
        
        switch (pesn->EVENTSETNODE_ucWaitType) {
        
        /*
         *  ����λ������������
         */
        case LW_OPTION_EVENTSET_WAIT_SET_ALL:
            ulEventRdy = (ULONG)(pesn->EVENTSETNODE_ulEventSets & pes->EVENTSET_ulEventSets);
            if (ulEventRdy == pesn->EVENTSETNODE_ulEventSets) {
                MONITOR_EVT_LONG4(MONITOR_EVENT_ID_ESET, MONITOR_EVENT_ESET_POST, 
                                  ulId, ((PLW_CLASS_TCB)pesn->EVENTSETNODE_ptcbMe)->TCB_ulId, 
                                  ulEvent, ulOption, LW_NULL);
                _EventSetThreadReady(pesn, ulEventRdy);
                
            }
            break;
            
        case LW_OPTION_EVENTSET_WAIT_CLR_ALL:
            ulEventRdy = (ULONG)(pesn->EVENTSETNODE_ulEventSets & ~pes->EVENTSET_ulEventSets);
            if (ulEventRdy == pesn->EVENTSETNODE_ulEventSets) {
                MONITOR_EVT_LONG4(MONITOR_EVENT_ID_ESET, MONITOR_EVENT_ESET_POST, 
                                  ulId, ((PLW_CLASS_TCB)pesn->EVENTSETNODE_ptcbMe)->TCB_ulId, 
                                  ulEvent, ulOption, LW_NULL);
                _EventSetThreadReady(pesn, ulEventRdy);
            }
            break;

        /*
         *  ����λ��������
         */
        case LW_OPTION_EVENTSET_WAIT_SET_ANY:
            ulEventRdy = (ULONG)(pesn->EVENTSETNODE_ulEventSets & pes->EVENTSET_ulEventSets);
            if (ulEventRdy) {
                MONITOR_EVT_LONG4(MONITOR_EVENT_ID_ESET, MONITOR_EVENT_ESET_POST, 
                                  ulId, ((PLW_CLASS_TCB)pesn->EVENTSETNODE_ptcbMe)->TCB_ulId, 
                                  ulEvent, ulOption, LW_NULL);
                _EventSetThreadReady(pesn, ulEventRdy);
            }
            break;
            
        case LW_OPTION_EVENTSET_WAIT_CLR_ANY:
            ulEventRdy = (ULONG)(pesn->EVENTSETNODE_ulEventSets & ~pes->EVENTSET_ulEventSets);
            if (ulEventRdy) {
                MONITOR_EVT_LONG4(MONITOR_EVENT_ID_ESET, MONITOR_EVENT_ESET_POST, 
                                  ulId, ((PLW_CLASS_TCB)pesn->EVENTSETNODE_ptcbMe)->TCB_ulId, 
                                  ulEvent, ulOption, LW_NULL);
                _EventSetThreadReady(pesn, ulEventRdy);
            }
            break;
        }
    }
    
    LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, iregInterLevel);        /*  ���ж�, ͬʱ�� spinlock */
    
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  (LW_CFG_EVENTSET_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_EVENTSETS > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
