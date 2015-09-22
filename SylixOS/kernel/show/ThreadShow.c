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
** ��   ��   ��: ThreadShow.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 09 �� 26 ��
**
** ��        ��: ��ʾ���е��̵߳���Ϣ.

** BUG
2007.10.28  ����ʹ�� tcb ��������. ʹ�� id ��.
2008.03.01  �� \r �ַ�ȥ��, ʹ�� tty �豸����ʱ, �ɲ��� OPT_CRMOD.
2008.03.05  ���뿴�Ź���ʱ����ʾ.
2008.04.14  �����߳����� CPU ��Ϣ����ʵ.
2008.04.22  ��ʾ errno ʱʹ��ʮ����.
2009.07.28  �ӳ�ʱ��ʹ���޷�������ʾ��ʽ.
            �߳�״̬ʹ���ַ�����ʾ.
2011.02.22  ����ȴ��ź�����.
2011.02.24  ���뽩��״̬�ȴ�.
2011.12.05  ����ÿ���߳�ȱҳ�ж�������ʾ.
2012.09.12  ������߳��Ƿ�ʹ�� FPU ����ʾ.
2012.12.11  ������ʾ��ջ�׵�ַ�����, ������������ ID ��ʾ.
            ������ʾ��չ�ӿ�, ������ʾָ�������ڵ��߳�.
2014.05.05  ���� pend show ����, ���Բ鿴�����������߳���Ϣ.
2014.06.02  pend ���͵���ʾ�����꾡.
2015.04.22  pend �¼�������ϸ.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if LW_CFG_FIO_LIB_EN > 0
/*********************************************************************************************************
  �������
*********************************************************************************************************/
#if LW_CFG_MODULELOADER_EN > 0
#include "../SylixOS/loader/include/loader_vppatch.h"
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static const CHAR   _G_cThreadInfoHdr[] = "\n\
      NAME         TID    PID  PRI STAT  ERRNO     DELAY   PAGEFAILS FPU CPU\n\
---------------- ------- ----- --- ---- ------- ---------- --------- --- ---\n";
static const CHAR   _G_cThreadPendHdr[] = "\n\
      NAME         TID    PID  STAT    DELAY          PEND EVENT        OWNER\n\
---------------- ------- ----- ---- ---------- ----------------------- -------\n";
/*********************************************************************************************************
** ��������: API_ThreadShowEx
** ��������: ��ʾ���е��̵߳���Ϣ
** �䡡��  : pid       ��Ҫ��ʾ��Ӧ�����ڵ��߳�, (-1 ��ʾ�����߳�)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID    API_ThreadShowEx (pid_t  pid)
{
    REGISTER INT                i;
    REGISTER INT                iThreadCounter = 0;
    REGISTER PLW_CLASS_TCB      ptcb;
             LW_CLASS_TCB_DESC  tcbdesc;
             
             PCHAR              pcPendType = LW_NULL;
             PCHAR              pcFpu = LW_NULL;
             pid_t              pidGet;
    
    if (LW_CPU_GET_CUR_NESTING()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return;
    }
    
    printf("thread show >>\n");
    printf(_G_cThreadInfoHdr);                                          /*  ��ӡ��ӭ��Ϣ                */
    
    for (i = 0; i < LW_CFG_MAX_THREADS; i++) {
        ptcb = _K_ptcbTCBIdTable[i];                                    /*  ��� TCB ���ƿ�             */
        if (ptcb == LW_NULL) {                                          /*  �̲߳�����                  */
            continue;
        }
        
        if (API_ThreadDesc(ptcb->TCB_ulId, &tcbdesc)) {
            continue;
        }
        
#if LW_CFG_MODULELOADER_EN > 0
        pidGet = vprocGetPidByTcbdesc(&tcbdesc);
#else
        pidGet = 0;
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
        
        if ((pidGet != pid) && (pid != -1)) {
            continue;
        }
        
        if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_SEM) {             /*  �ȴ��ź���                  */
            pcPendType = "SEM";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_MSGQUEUE) { /*  �ȴ���Ϣ����                */
            pcPendType = "MSGQ";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_JOIN) {     /*  �ȴ������߳�                */
            pcPendType = "JOIN";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_SUSPEND) {  /*  ����                        */
            pcPendType = "SUSP";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_EVENTSET) { /*  �ȴ��¼���                  */
            pcPendType = "ENTS";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_SIGNAL) {   /*  �ȴ��ź�                    */
            pcPendType = "WSIG";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_INIT) {     /*  ��ʼ����                    */
            pcPendType = "INIT";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_WDEATH) {   /*  ����״̬                    */
            pcPendType = "WDEA";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_DELAY) {    /*  ˯��                        */
            pcPendType = "SLP";
            
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_STOP) {     /*  ֹͣ                        */
            pcPendType = "STOP";
            
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_WSTAT) {    /*  �ȴ�״̬ת��                */
            pcPendType = "WSTA";
        
        } else {
            pcPendType = "RDY";                                         /*  ����̬                      */
        }
        
        if (tcbdesc.TCBD_ulOption & LW_OPTION_THREAD_USED_FP) {
            pcFpu = "USE";
        } else {
            pcFpu = "";
        }
        
        printf("%-16s %7lx %5d %3d %-4s %7ld %10lu %9llu %-3s %3ld\n",
               tcbdesc.TCBD_cThreadName,                                /*  �߳���                      */
               tcbdesc.TCBD_ulId,                                       /*  ID ��                       */
               pidGet,                                                  /*  �������� ID                 */
               tcbdesc.TCBD_ucPriority,                                 /*  ���ȼ�                      */
               pcPendType,                                              /*  ״̬                        */
               tcbdesc.TCBD_ulLastError,                                /*  �����                      */
               tcbdesc.TCBD_ulWakeupLeft,                               /*  �ȴ�������                  */
               tcbdesc.TCBD_i64PageFailCounter,                         /*  ȱҳ�ж�                    */
               pcFpu,
               tcbdesc.TCBD_ulCPUId);
        iThreadCounter++;
    }
    
    printf("\nthread : %d\n", iThreadCounter);                          /*  ��ʾ�߳�����                */
}
/*********************************************************************************************************
** ��������: API_ThreadPendShowEx
** ��������: ��ʾ�����������̵߳���Ϣ
** �䡡��  : pid       ��Ҫ��ʾ��Ӧ�����ڵ��߳�, (-1 ��ʾ�����߳�)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID    API_ThreadPendShowEx (pid_t  pid)
{
#define MAKE_ID(type, index)    \
        _MakeObjectId(type, LW_CFG_PROCESSOR_NUMBER, index)

    REGISTER INT                i;
    REGISTER INT                iThreadCounter = 0;
    REGISTER PLW_CLASS_TCB      ptcb;
             PLW_CLASS_TCB      ptcbOwner;
             LW_CLASS_TCB_DESC  tcbdesc;
             PLW_CLASS_EVENT    pevent;
#if (LW_CFG_EVENTSET_EN > 0) && (LW_CFG_MAX_EVENTSETS > 0)
             PLW_CLASS_EVENTSET pes;
#endif                                                                  /*  (LW_CFG_EVENTSET_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_EVENTSETS > 0)  */
             PCHAR              pcPendType = LW_NULL;
             pid_t              pidGet;
             CHAR               cEventName[LW_CFG_OBJECT_NAME_SIZE];
             LW_OBJECT_HANDLE   ulEvent;
             LW_OBJECT_HANDLE   ulOwner;
             
    
    if (LW_CPU_GET_CUR_NESTING()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return;
    }
    
    printf("thread pending show >>\n");
    printf(_G_cThreadPendHdr);                                          /*  ��ӡ��ӭ��Ϣ                */
    
    for (i = 0; i < LW_CFG_MAX_THREADS; i++) {
        ptcb = _K_ptcbTCBIdTable[i];                                    /*  ��� TCB ���ƿ�             */
        if (ptcb == LW_NULL) {                                          /*  �̲߳�����                  */
            continue;
        }
        
        __KERNEL_ENTER();                                               /*  �����ں�                    */
        if (ptcb->TCB_peventPtr) {                                      /*  �ȴ��¼�                    */
            pevent = ptcb->TCB_peventPtr;
            switch (pevent->EVENT_ucType) {
            
            case LW_TYPE_EVENT_MSGQUEUE:    
                ulEvent = MAKE_ID(_OBJECT_MSGQUEUE, pevent->EVENT_usIndex);
                break;
            
            case LW_TYPE_EVENT_SEMC:
                ulEvent = MAKE_ID(_OBJECT_SEM_C, pevent->EVENT_usIndex);
                break;
                
            case LW_TYPE_EVENT_SEMB:
                ulEvent = MAKE_ID(_OBJECT_SEM_B, pevent->EVENT_usIndex);
                break;
            
            case LW_TYPE_EVENT_MUTEX:
                ulEvent = MAKE_ID(_OBJECT_SEM_M, pevent->EVENT_usIndex);
                break;
                
            default:
                ulEvent = LW_OBJECT_HANDLE_INVALID;
                break;
            }
            
            lib_strlcpy(cEventName, pevent->EVENT_cEventName, LW_CFG_OBJECT_NAME_SIZE);
            if ((pevent->EVENT_ucType == LW_TYPE_EVENT_MUTEX) &&
                (pevent->EVENT_pvTcbOwn)) {
                ptcbOwner = (PLW_CLASS_TCB)pevent->EVENT_pvTcbOwn;
                ulOwner   = ptcbOwner->TCB_ulId;
            } else {
                ulOwner   = LW_OBJECT_HANDLE_INVALID;
            }
        
#if (LW_CFG_EVENTSET_EN > 0) && (LW_CFG_MAX_EVENTSETS > 0)
        } else if (ptcb->TCB_pesnPtr) {                                 /*  �ȴ��¼���־��              */
            pes     = (PLW_CLASS_EVENTSET)ptcb->TCB_pesnPtr->EVENTSETNODE_pesEventSet;
            ulEvent = MAKE_ID(_OBJECT_EVENT_SET, pes->EVENTSET_usIndex);
            lib_strlcpy(cEventName, pes->EVENTSET_cEventSetName, LW_CFG_OBJECT_NAME_SIZE);
            ulOwner = LW_OBJECT_HANDLE_INVALID;
#endif                                                                  /*  (LW_CFG_EVENTSET_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_EVENTSETS > 0)  */
        } else {
            cEventName[0] = PX_EOS;
            ulEvent       = LW_OBJECT_HANDLE_INVALID;
            ulOwner       = LW_OBJECT_HANDLE_INVALID;
        }
        __KERNEL_EXIT();                                                /*  �����ں�                    */
        
        if (API_ThreadDesc(ptcb->TCB_ulId, &tcbdesc)) {
            continue;
        }
        
#if LW_CFG_MODULELOADER_EN > 0
        pidGet = vprocGetPidByTcbdesc(&tcbdesc);
#else
        pidGet = 0;
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
        
        if ((pidGet != pid) && (pid != -1)) {
            continue;
        }
        
        if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_SEM) {             /*  �ȴ��ź���                  */
            pcPendType = "SEM";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_MSGQUEUE) { /*  �ȴ���Ϣ����                */
            pcPendType = "MSGQ";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_JOIN) {     /*  �ȴ������߳�                */
            pcPendType = "JOIN";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_SUSPEND) {  /*  ����                        */
            pcPendType = "SUSP";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_EVENTSET) { /*  �ȴ��¼���                  */
            pcPendType = "ENTS";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_SIGNAL) {   /*  �ȴ��ź�                    */
            pcPendType = "WSIG";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_INIT) {     /*  ��ʼ����                    */
            pcPendType = "INIT";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_WDEATH) {   /*  ����״̬                    */
            pcPendType = "WDEA";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_DELAY) {    /*  ˯��                        */
            pcPendType = "SLP";
        
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_STOP) {     /*  ֹͣ                        */
            pcPendType = "STOP";
            
        } else if (tcbdesc.TCBD_usStatus & LW_THREAD_STATUS_WSTAT) {    /*  �ȴ�״̬ת��                */
            pcPendType = "WSTA";
        
        } else {
            continue;                                                   /*  ����̬                      */
        }
        
        if (ulOwner) {
            printf("%-16s %7lx %5d %-4s %10lu %8lx:%-16s %7lx\n",
                   tcbdesc.TCBD_cThreadName,                            /*  �߳���                      */
                   tcbdesc.TCBD_ulId,                                   /*  ID ��                       */
                   pidGet,                                              /*  �������� ID                 */
                   pcPendType,                                          /*  ״̬                        */
                   tcbdesc.TCBD_ulWakeupLeft,                           /*  �ȴ�������                  */
                   ulEvent,
                   cEventName,
                   ulOwner);
        } else {
            printf("%-16s %7lx %5d %-4s %10lu %8lx:%-16s\n",
                   tcbdesc.TCBD_cThreadName,                            /*  �߳���                      */
                   tcbdesc.TCBD_ulId,                                   /*  ID ��                       */
                   pidGet,                                              /*  �������� ID                 */
                   pcPendType,                                          /*  ״̬                        */
                   tcbdesc.TCBD_ulWakeupLeft,                           /*  �ȴ�������                  */
                   ulEvent,
                   cEventName);
        }
        iThreadCounter++;
    }
    
    printf("\npending thread : %d\n", iThreadCounter);                  /*  ��ʾ�߳�����                */
}
/*********************************************************************************************************
** ��������: API_ThreadShow
** ��������: ��ʾ���е��̵߳���Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID    API_ThreadShow (VOID)
{
    API_ThreadShowEx(PX_ERROR);                                         /*  ��ʾ�����߳�                */
}
/*********************************************************************************************************
** ��������: API_ThreadPendShow
** ��������: ��ʾ�����������̵߳���Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID    API_ThreadPendShow (VOID)
{
    API_ThreadPendShowEx(PX_ERROR);                                     /*  ��ʾ�����߳�                */
}
#endif                                                                  /*  LW_CFG_FIO_LIB_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/