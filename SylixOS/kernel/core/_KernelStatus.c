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
** ��   ��   ��: _KernelStatus.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 05 �� 18 ��
**
** ��        ��: ����ϵͳ�ں�״̬����

** BUG
2008.05.31  ��ʹ�� _K_ucSysStatus ����״̬����. ������������ʹ�������ı��������������Ч��.
2009.04.14  ���� SMP ֧��, �����ں�ʱ�������ں� spinlock
2010.01.22  ������ IRQ ͬ������.
2011.02.22  ���� __kernelExitInt() ���ź������ж����˳��ں�, û�н���ǰ�ֳ�ѹջ.
2011.02.24  SIGNAL ȥ���� suspend �Զ���������, ���Բ�����Ҫ kernel exit int ֧��.
2011.12.08  ����鿴�ں�ӵ���߳�(���ж�)�Ĳ���. (�������)
2012.12.25  �����ں˿ռ�����ĺ���.
2013.07.17  ����Ҫ�ĵط������ڴ�����. ͬʱ���ں˼��������� CPU �ṹ��.
2013.08.28  �����ں��¼������.
2014.11.06  �����µ� SMP �ں˵��Ȼ���.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  kernel status
*********************************************************************************************************/
typedef struct {
    PLW_CLASS_CPU           KN_pcpuKernelCpu;
    LW_OBJECT_HANDLE        KN_ulKernelOwner;
    CPCHAR                  KN_ulKernelEnterFunc;
} LW_KERNEL_STATUS;
static LW_KERNEL_STATUS     _K_knstatus;
/*********************************************************************************************************
  kernel status macro
*********************************************************************************************************/
#define __LW_SAVE_KERNEL_OWNER(pcpuCur, pcFunc) \
        _K_knstatus.KN_pcpuKernelCpu     = pcpuCur; \
        _K_knstatus.KN_ulKernelEnterFunc = pcFunc;  \
        if (pcpuCur->CPU_ulInterNesting) {  \
            _K_knstatus.KN_ulKernelOwner = ~0;  \
        } else {    \
            _K_knstatus.KN_ulKernelOwner = pcpuCur->CPU_ptcbTCBCur->TCB_ulId;   \
        }
#define __LW_CLEAR_KERNEL_OWNER()   \
        _K_knstatus.KN_pcpuKernelCpu     = LW_NULL; \
        _K_knstatus.KN_ulKernelEnterFunc = LW_NULL; \
        _K_knstatus.KN_ulKernelOwner     = LW_OBJECT_HANDLE_INVALID;    \
/*********************************************************************************************************
** ��������: __kernelEnter
** ��������: �����ں�״̬
** �䡡��  : pcFunc        �����ں˺��� (������)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __kernelEnter (CPCHAR  pcFunc)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    
    LW_SPIN_LOCK_QUICK(&_K_slKernel, &iregInterLevel);                  /*  ���ں� spinlock ���ر��ж�  */
    
    pcpuCur = LW_CPU_GET_CUR();
    pcpuCur->CPU_iKernelCounter++;
    KN_SMP_WMB();                                                       /*  �ȴ����ϲ������            */
    
    __LW_SAVE_KERNEL_OWNER(pcpuCur, pcFunc);

    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */

    MONITOR_EVT_INT3(MONITOR_EVENT_ID_KERNEL, MONITOR_EVENT_KERNEL_ENTER, 
                     pcpuCur->CPU_ulCPUId,
                     pcpuCur->CPU_iKernelCounter, 0, LW_NULL);
}
/*********************************************************************************************************
** ��������: __kernelEnterIrq
** ��������: �����ں�״̬���ر��ж�
** �䡡��  : pcFunc        �����ں˺��� (������)
** �䡡��  : �жϼĴ���
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INTREG  __kernelEnterIrq (CPCHAR  pcFunc)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    
    LW_SPIN_LOCK_QUICK(&_K_slKernel, &iregInterLevel);                  /*  ���ں� spinlock ���ر��ж�  */
    
    pcpuCur = LW_CPU_GET_CUR();
    pcpuCur->CPU_iKernelCounter++;
    KN_SMP_WMB();                                                       /*  �ȴ����ϲ������            */
    
    __LW_SAVE_KERNEL_OWNER(pcpuCur, pcFunc);
    
#if LW_CFG_MONITOR_EN > 0
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    MONITOR_EVT_INT3(MONITOR_EVENT_ID_KERNEL, MONITOR_EVENT_KERNEL_ENTER, 
                     pcpuCur->CPU_ulCPUId,
                     pcpuCur->CPU_iKernelCounter, 1, LW_NULL);
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
#endif                                                                  /*  LW_CFG_MONITOR_EN > 0       */
    
    return  (iregInterLevel);
}
/*********************************************************************************************************
** ��������: __kernelExit
** ��������: �˳��ں�״̬
** �䡡��  : NONE
** �䡡��  : ����������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  __kernelExit (VOID)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    INT             iRetVal;
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    
    pcpuCur = LW_CPU_GET_CUR();
    if (pcpuCur->CPU_iKernelCounter) {
    
#if LW_CFG_MONITOR_EN > 0
        KN_INT_ENABLE(iregInterLevel);                                  /*  ���ж�                    */
        MONITOR_EVT_INT3(MONITOR_EVENT_ID_KERNEL, MONITOR_EVENT_KERNEL_EXIT, 
                         pcpuCur->CPU_ulCPUId,
                         pcpuCur->CPU_iKernelCounter - 1, 0, LW_NULL);
        iregInterLevel = KN_INT_DISABLE();                              /*  �ر��ж�                    */
#endif                                                                  /*  LW_CFG_MONITOR_EN > 0       */
    
        pcpuCur->CPU_iKernelCounter--;
        KN_SMP_WMB();                                                   /*  �ȴ����ϲ������            */
        
        if (!pcpuCur->CPU_iKernelCounter) {
            __LW_CLEAR_KERNEL_OWNER();
            
            iRetVal = _Schedule();                                      /*  ���Ե���                    */
            LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);         /*  �����ں� spinlock �����ж�*/
            
            return  (iRetVal);
        }
    }
    
    LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);                 /*  �����ں� spinlock �����ж�*/
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kernelExitIrq
** ��������: �˳��ں�״̬ͬʱ���ж�
** �䡡��  : iregInterLevel     �жϼĴ���
** �䡡��  : ����������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  __kernelExitIrq (INTREG     iregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur;
    INT             iRetVal;
    
    pcpuCur = LW_CPU_GET_CUR();
    if (pcpuCur->CPU_iKernelCounter) {
    
#if LW_CFG_MONITOR_EN > 0
        KN_INT_ENABLE(iregInterLevel);                                  /*  ���ж�                    */
        MONITOR_EVT_INT3(MONITOR_EVENT_ID_KERNEL, MONITOR_EVENT_KERNEL_EXIT, 
                         pcpuCur->CPU_ulCPUId,
                         pcpuCur->CPU_iKernelCounter - 1, 1, LW_NULL);
        iregInterLevel = KN_INT_DISABLE();                              /*  �ر��ж�                    */
#endif                                                                  /*  LW_CFG_MONITOR_EN > 0       */

        pcpuCur->CPU_iKernelCounter--;
        KN_SMP_WMB();                                                   /*  �ȴ����ϲ������            */

        if (!pcpuCur->CPU_iKernelCounter) {
            __LW_CLEAR_KERNEL_OWNER();
            
            iRetVal = _Schedule();                                      /*  ���Ե���                    */
            LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);         /*  �����ں� spinlock �����ж�*/
            
            return  (iRetVal);
        }
    }
    
    LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);                 /*  �����ں� spinlock �����ж�*/
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kernelIsEnter
** ��������: �Ƿ�����˽����ں�״̬
** �䡡��  : NONE
** �䡡��  : LW_TRUE or LW_FALSE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
BOOL  __kernelIsEnter (VOID)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    INT             iKernelCounter;
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    
    pcpuCur = LW_CPU_GET_CUR();
    iKernelCounter = pcpuCur->CPU_iKernelCounter;
    
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    if (iKernelCounter) {
        return  (LW_TRUE);
    } else {
        return  (LW_FALSE);
    }
}
/*********************************************************************************************************
** ��������: __kernelSched
** ��������: �ں˵���
** �䡡��  : NONE
** �䡡��  : ����������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  __kernelSched (VOID)
{
    INTREG          iregInterLevel;
    INT             iRetVal;
    
    LW_SPIN_LOCK_QUICK(&_K_slKernel, &iregInterLevel);                  /*  ���ں� spinlock ���ر��ж�  */
    iRetVal = _Schedule();                                              /*  ���Ե���                    */
    LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);                 /*  �����ں� spinlock �����ж�*/
    
    return  (iRetVal);
}
/*********************************************************************************************************
** ��������: __kernelSchedInt
** ��������: �˳��ж�ʱ�ں˵���
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __kernelSchedInt (VOID)
{
    INTREG          iregInterLevel;
    
    LW_SPIN_LOCK_QUICK(&_K_slKernel, &iregInterLevel);                  /*  ���ں� spinlock ���ر��ж�  */
    _ScheduleInt();                                                     /*  ���Ե���                    */
    LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);                 /*  �����ں� spinlock �����ж�*/
}
/*********************************************************************************************************
** ��������: __kernelOwner
** ��������: ��ȡ�ں�ӵ�����߳�
** �䡡��  : NONE
** �䡡��  : �߳̾��, ~0 ��ʾ�ж��н���, 0 û�н����ں�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
LW_OBJECT_HANDLE __kernelOwner (VOID)
{
    INTREG  iregInterLevel;
    ULONG   ulKernelOwner;
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    
    ulKernelOwner = _K_knstatus.KN_ulKernelOwner;
    
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */

    return  (ulKernelOwner);
}
/*********************************************************************************************************
** ��������: __kernelEnterFunc
** ��������: ��ȡ�ں�ӵ���߽��뺯��
** �䡡��  : NONE
** �䡡��  : �ں˽��뺯��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
CPCHAR __kernelEnterFunc (VOID)
{
    INTREG  iregInterLevel;
    CPCHAR  pcKernelEnterFunc;
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    
    pcKernelEnterFunc = _K_knstatus.KN_ulKernelEnterFunc;
    
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    return  (pcKernelEnterFunc);
}
/*********************************************************************************************************
  �ں˻����ж� IO �ķ���Ϊ�ں��ļ���������
*********************************************************************************************************/
/*********************************************************************************************************
** ��������: __kernelSpaceEnter
** ��������: �����ں˻��� 
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __kernelSpaceEnter (VOID)
{
    PLW_CLASS_TCB   ptcbCur;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);

    __LW_ATOMIC_INC(&ptcbCur->TCB_atomicKernelSpace);
}
/*********************************************************************************************************
** ��������: __kernelSpaceExit
** ��������: �˳��ں˻���
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __kernelSpaceExit (VOID)
{
    PLW_CLASS_TCB   ptcbCur;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);

    if (__LW_ATOMIC_DEC(&ptcbCur->TCB_atomicKernelSpace) < 0) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "kernel space counter error.\r\n");
        __LW_ATOMIC_SET(0, &ptcbCur->TCB_atomicKernelSpace);
    }
}
/*********************************************************************************************************
** ��������: __kernelSpaceGet
** ��������: ����ں˻���
** �䡡��  : NONE
** �䡡��  : Ƕ�ײ���
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  __kernelSpaceGet (VOID)
{
    PLW_CLASS_TCB   ptcbCur;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);

    return  (__LW_ATOMIC_GET(&ptcbCur->TCB_atomicKernelSpace));
}
/*********************************************************************************************************
** ��������: __kernelSpaceGet2
** ��������: ����ں˻���
** �䡡��  : ptcb          ������ƿ�
** �䡡��  : Ƕ�ײ���
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  __kernelSpaceGet2 (PLW_CLASS_TCB  ptcb)
{
    return  (__LW_ATOMIC_GET(&ptcb->TCB_atomicKernelSpace));
}
/*********************************************************************************************************
** ��������: __kernelSpaceSet
** ��������: �����ں˻��� 
** �䡡��  : iNesting      Ƕ�ײ���
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __kernelSpaceSet (INT  iNesting)
{
    PLW_CLASS_TCB   ptcbCur;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);

    __LW_ATOMIC_SET(iNesting, &ptcbCur->TCB_atomicKernelSpace);
}
/*********************************************************************************************************
** ��������: __kernelSpaceSet2
** ��������: �����ں˻��� 
** �䡡��  : ptcb          ������ƿ�
**           iNesting      Ƕ�ײ���
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __kernelSpaceSet2 (PLW_CLASS_TCB  ptcb, INT  iNesting)
{
    __LW_ATOMIC_SET(iNesting, &ptcb->TCB_atomicKernelSpace);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
