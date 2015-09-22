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
** ��   ��   ��: _EventInit.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 13 ��
**
** ��        ��: ����ϵͳ�¼���ʼ�������⡣

** BUG:
2009.07.28  ������������ĳ�ʼ��.
2013.11.14  ʹ�ö�����Դ�������ṹ���������Դ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _EventInit
** ��������: ��ʼ���¼������
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _EventInit (VOID)
{
#if  (LW_CFG_EVENT_EN > 0) && (LW_CFG_MAX_EVENTS > 0)
    REGISTER ULONG                 ulI;
    REGISTER PLW_CLASS_EVENT       peventTemp1;
    REGISTER PLW_LIST_MONO         pmonoTemp1;
    REGISTER PLW_CLASS_WAITQUEUE   pwqTemp1;

#if  LW_CFG_MAX_EVENTS == 1

    _K_resrcEvent.RESRC_pmonoFreeHeader = &_K_eventBuffer[0].EVENT_monoResrcList;
                                                                            /*  ������Դ��ͷ            */
    peventTemp1 = &_K_eventBuffer[0];                                       /*  ָ�򻺳���׵�ַ        */
    pmonoTemp1  = &peventTemp1->EVENT_monoResrcList;                        /*  �����Դ��              */

    peventTemp1->EVENT_ucType       = LW_TYPE_EVENT_UNUSED;                 /*  �¼�����                */
    peventTemp1->EVENT_ulCounter    = 0ul;                                  /*  ������ֵ                */
    peventTemp1->EVENT_ulMaxCounter = 0ul;                                  /*  �����ֵ              */
    peventTemp1->EVENT_ulOption     = LW_OPTION_NONE;                       /*  �¼�ѡ��                */
    peventTemp1->EVENT_pvPtr        = LW_NULL;                              /*  ����;ָ��              */
    peventTemp1->EVENT_pvTcbOwn     = LW_NULL;                              /*  �����߳�                */
    peventTemp1->EVENT_usIndex      = 0;                                    /*  �¼��������±�          */
    
    pwqTemp1 = &peventTemp1->EVENT_wqWaitList;                              /*  ��ʼ��ʼ���ȴ�����      */
    pwqTemp1->WAIT_Q_usWaitNum      = 0;                                    /*  û������ȴ��¼�        */
    
    for (ulI = 0; ulI < __THREAD_PRIORITY_Q_NUM; ulI++) {
        pwqTemp1->WAIT_Q_wlWaitList.WAIT_Q_pringPRIOList[ulI] = LW_NULL;    /*  ��ʼ���ȴ�����          */
    }
    
    LW_SPIN_INIT(&peventTemp1->EVENT_slLock);
    
    _INIT_LIST_MONO_HEAD(pmonoTemp1);                                       /*  ��ʼ�����ڵ�          */
    
    _K_resrcEvent.RESRC_pmonoFreeTail = pmonoTemp1;
    
#else

    REGISTER UINT8              ucJ;
    REGISTER PLW_CLASS_EVENT    peventTemp2;
    REGISTER PLW_LIST_MONO      pmonoTemp2;
    
    _K_resrcEvent.RESRC_pmonoFreeHeader = &_K_eventBuffer[0].EVENT_monoResrcList;
                                                                            /*  ������Դ��ͷ            */
    peventTemp1 = &_K_eventBuffer[0];                                       /*  ָ�򻺳���׵�ַ        */
    peventTemp2 = &_K_eventBuffer[1];                                       /*  ָ�򻺳���׵�ַ        */
    
    for (ulI = 0; ulI < ((LW_CFG_MAX_EVENTS) - 1); ulI++) {
        
        pmonoTemp1 = &peventTemp1->EVENT_monoResrcList;                     /*  �����Դ��              */
        pmonoTemp2 = &peventTemp2->EVENT_monoResrcList;                     /*  �����Դ��              */
        
        peventTemp1->EVENT_ucType       = LW_TYPE_EVENT_UNUSED;             /*  �¼�����                */
        peventTemp1->EVENT_ulCounter    = 0ul;                              /*  ������ֵ                */
        peventTemp1->EVENT_ulMaxCounter = 0ul;                              /*  �����ֵ              */
        peventTemp1->EVENT_ulOption     = LW_OPTION_NONE;                   /*  �¼�ѡ��                */
        peventTemp1->EVENT_pvPtr        = LW_NULL;                          /*  ����;ָ��              */
        peventTemp1->EVENT_pvTcbOwn     = LW_NULL;                          /*  �����߳�                */
        peventTemp1->EVENT_usIndex      = (UINT16)ulI;                      /*  �¼��������±�          */
        
        pwqTemp1 = &peventTemp1->EVENT_wqWaitList;                          /*  ��ʼ��ʼ���ȴ�����      */
        pwqTemp1->WAITQUEUE_usWaitNum      = 0;                             /*  û������ȴ��¼�        */
        
        for (ucJ = 0; ucJ < __THREAD_PRIORITY_Q_NUM; ucJ++) {               /*  ��ʼ���ȴ�����          */
            pwqTemp1->WAITQUEUE_wlWaitList.WAITLIST_pringPRIOList[ucJ] = LW_NULL;
        }
        
        LW_SPIN_INIT(&peventTemp1->EVENT_slLock);
   
        _LIST_MONO_LINK(pmonoTemp1, pmonoTemp2);                            /*  ������Դ����            */
        
        peventTemp1++;
        peventTemp2++;
        
    }
                                                                            /*  ��ʼ�����һ���ڵ�      */
    pmonoTemp1 = &peventTemp1->EVENT_monoResrcList;                         /*  �����Դ��              */
    
    peventTemp1->EVENT_ucType       = LW_TYPE_EVENT_UNUSED;                 /*  �¼�����                */
    peventTemp1->EVENT_ulCounter    = 0ul;                                  /*  ������ֵ                */
    peventTemp1->EVENT_ulMaxCounter = 0ul;                                  /*  �����ֵ              */
    peventTemp1->EVENT_ulOption     = LW_OPTION_NONE;                       /*  �¼�ѡ��                */
    peventTemp1->EVENT_pvPtr        = LW_NULL;                              /*  ����;ָ��              */
    peventTemp1->EVENT_pvTcbOwn     = LW_NULL;                              /*  �����߳�                */
    peventTemp1->EVENT_usIndex      = (UINT16)ulI;                          /*  �¼��������±�          */

    pwqTemp1 = &peventTemp1->EVENT_wqWaitList;                              /*  ��ʼ��ʼ���ȴ�����      */
    pwqTemp1->WAITQUEUE_usWaitNum      = 0;                                 /*  û������ȴ��¼�        */
    
    for (ucJ = 0; ucJ < __THREAD_PRIORITY_Q_NUM; ucJ++) {                   /*  ��ʼ���ȴ�����          */
        pwqTemp1->WAITQUEUE_wlWaitList.WAITLIST_pringPRIOList[ucJ] = LW_NULL;
    }
    
    LW_SPIN_INIT(&peventTemp1->EVENT_slLock);
    
    _INIT_LIST_MONO_HEAD(pmonoTemp1);                                       /*  ��ʼ�����ڵ�          */
    
    _K_resrcEvent.RESRC_pmonoFreeTail = pmonoTemp1;
#endif                                                                      /*  LW_CFG_MAX_EVENTS == 1  */

    _K_resrcEvent.RESRC_uiUsed    = 0;
    _K_resrcEvent.RESRC_uiMaxUsed = 0;

#endif                                                                      /*  (LW_CFG_EVENT_EN > 0)   */
                                                                            /*  (LW_CFG_MAX_EVENTS > 0) */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
