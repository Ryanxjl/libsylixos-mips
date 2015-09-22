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
** ��   ��   ��: inlSearchEventQueue.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 20 ��
**
** ��        ��: ϵͳ�¼��ȴ����л���������

** BUG
2007.09.12  ����ɲü������ơ�
2007.11.13  ʹ���������������������ȫ��װ.
2007.11.18  ����ע��.
*********************************************************************************************************/

#ifndef  __INLSEARCHEVENTQUEUE_H
#define  __INLSEARCHEVENTQUEUE_H

#if (LW_CFG_EVENT_EN > 0) && (LW_CFG_MAX_EVENTS > 0)

/*********************************************************************************************************
  _EVENT_FIFO_Q_PTR
*********************************************************************************************************/

#define _EVENT_FIFO_Q_PTR(ppringListPtr)                                                        \
do {                                                                                            \
    ppringListPtr = &pevent->EVENT_wqWaitList.WAITQUEUE_wlWaitList.WAITLIST_pringFIFOList;      \
} while (0)

/*********************************************************************************************************
  _EVENT_PRIORITY_Q_PTR
*********************************************************************************************************/

#define _EVENT_PRIORITY_Q_PTR(ppringListPtr, ucPrioIndex)                                               \
do {                                                                                                    \
    ppringListPtr = &pevent->EVENT_wqWaitList.WAITQUEUE_wlWaitList.WAITLIST_pringPRIOList[ucPrioIndex]; \
} while (0)

/*********************************************************************************************************
  _EVENT_DEL_Q_FIFO
        
  find the pointer which point a FIFO queue, a thread removed form it.
*********************************************************************************************************/

#define _EVENT_DEL_Q_FIFO(ppringListPtr)                                                        \
do {                                                                                            \
    ppringListPtr = &pevent->EVENT_wqWaitList.WAITQUEUE_wlWaitList.WAITLIST_pringFIFOList;      \
} while (0)

/*********************************************************************************************************
  _EVENT_DEL_Q_PRIORITY
        
  find the pointer which point a priority queue, a thread removed form it.
*********************************************************************************************************/

#define _EVENT_DEL_Q_PRIORITY(ppringListPtr)                                                  \
do {                                                                                          \
    REGISTER INT    i;                                                                        \
    ppringListPtr = &pevent->EVENT_wqWaitList.WAITQUEUE_wlWaitList.WAITLIST_pringPRIOList[0]; \
    for (i = 0; i < __THREAD_PRIORITY_Q_NUM; i++) {                                           \
        if (*ppringListPtr) {                                                                 \
            break;                                                                            \
        } else {                                                                              \
            ppringListPtr++;                                                                  \
        }                                                                                     \
    }                                                                                         \
} while (0)

/*********************************************************************************************************
  _EVENT_INDEX_Q_PRIORITY
*********************************************************************************************************/

#define _EVENT_INDEX_Q_PRIORITY(ucPriority, ucIndex)                    \
do {                                                                    \
    ucIndex = (UINT8)(ucPriority / (__EACH_WAIT_QUEUE_PRIORITY + 1));   \
} while (0)

/*********************************************************************************************************
  _EventWaitNum
*********************************************************************************************************/

static LW_INLINE UINT16  _EventWaitNum (PLW_CLASS_EVENT    pevent)
{
    return  (pevent->EVENT_wqWaitList.WAITQUEUE_usWaitNum);
}

/*********************************************************************************************************
  _EventQGetTcbFifo (Make Oldest thread ready, So this is The Last Node In Ring)
*********************************************************************************************************/

static LW_INLINE PLW_CLASS_TCB  _EventQGetTcbFifo (PLW_CLASS_EVENT    pevent, 
                                                   PLW_LIST_RING     *ppringList)
{
    REGISTER PLW_CLASS_TCB    ptcb;
    REGISTER PLW_LIST_RING    pringOldest;
    
    pringOldest = _list_ring_get_prev(*ppringList);
    
    ptcb = _LIST_ENTRY(pringOldest, LW_CLASS_TCB, TCB_ringEvent);
    
    return  (ptcb);
}

/*********************************************************************************************************
  _EventQGetTcbPriority (Make highest priority thread ready, So this is The Frist Node In Ring)
*********************************************************************************************************/

static LW_INLINE PLW_CLASS_TCB  _EventQGetTcbPriority (PLW_CLASS_EVENT    pevent, 
                                                       PLW_LIST_RING     *ppringList)
{
    REGISTER PLW_CLASS_TCB    ptcb;

    ptcb = _LIST_ENTRY(*ppringList, LW_CLASS_TCB, TCB_ringEvent);
    
    return  (ptcb);
}

#endif                                                                  /*  (LW_CFG_EVENT_EN > 0)       */
                                                                        /*  (LW_CFG_MAX_EVENTS > 0)     */
#endif                                                                  /*  __INLSEARCHEVENTQUEUE_H     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
