/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: _SmpIpi.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 07 �� 19 ��
**
** ��        ��: CPU �˼��ж�, (���� SMP ���ϵͳ)

** BUG:
2014.04.09  ������û�� ACTIVE �� CPU ���ͺ˼��ж�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
** ��������: _SmpSendIpi
** ��������: ����һ�����Զ�������ĺ˼��жϸ�ָ���� CPU 
             ���ж�����±�����, �����Ҫ�ȴ�, ����뱣֤���� CPU �Ѿ�����.
** �䡡��  : ulCPUId       CPU ID
**           ulIPIVec      �˼��ж����� (���Զ��������ж�����)
**           iWait         �Ƿ�ȴ�������� (LW_IPI_SCHED ��������ȴ�, ���������)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpSendIpi (ULONG  ulCPUId, ULONG  ulIPIVec, INT  iWait)
{
    PLW_CLASS_CPU   pcpuDst = LW_CPU_GET(ulCPUId);
    ULONG           ulMask  = (ULONG)(1 << ulIPIVec);
    
    if (!LW_CPU_IS_ACTIVE(pcpuDst)) {                                   /*  CPU ���뱻����              */
        return;
    }
    
    LW_SPIN_LOCK_IGNIRQ(&pcpuDst->CPU_slIpi);                           /*  ����Ŀ�� CPU                */
    LW_CPU_ADD_IPI_PEND(ulCPUId, ulMask);                               /*  ��� PEND λ                */
    LW_SPIN_UNLOCK_IGNIRQ(&pcpuDst->CPU_slIpi);                         /*  ����Ŀ�� CPU                */
    
    archMpInt(ulCPUId);
    
    if (iWait && (ulIPIVec != LW_IPI_SCHED)) {
        while (LW_CPU_GET_IPI_PEND(ulCPUId) & ulMask) {                 /*  �ȴ�����                    */
            LW_SPINLOCK_DELAY();
        }
    }
}
/*********************************************************************************************************
** ��������: _SmpSendIpiAllOther
** ��������: ����һ�����Զ�������ĺ˼��жϸ��������� CPU 
             ���ж�����±�����, �����Ҫ�ȴ�, ����뱣֤���� CPU �Ѿ�����.
** �䡡��  : ulIPIVec      �˼��ж����� (���Զ��������ж�����)
**           iWait         �Ƿ�ȴ�������� (LW_IPI_SCHED ��������ȴ�, ���������)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpSendIpiAllOther (ULONG  ulIPIVec, INT  iWait)
{
    ULONG   i;
    ULONG   ulCPUId;
    
    ulCPUId = LW_CPU_GET_CUR_ID();
    
    KN_SMP_WMB();
    for (i = 0; i < LW_NCPUS; i++) {
        if (ulCPUId != i) {
            _SmpSendIpi(i, ulIPIVec, iWait);
        }
    }
}
/*********************************************************************************************************
** ��������: _SmpCallIpi
** ��������: ����һ���Զ���˼��жϸ�ָ���� CPU
             ���ж�����±�����, �����Ҫ�ȴ�, ����뱣֤���� CPU �Ѿ�����.
** �䡡��  : ulCPUId       CPU ID
**           pipim         �˼��жϲ���
** �䡡��  : ���÷���ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  _SmpCallIpi (ULONG  ulCPUId, PLW_IPI_MSG  pipim)
{
    PLW_CLASS_CPU   pcpuDst = LW_CPU_GET(ulCPUId);
    
    if (!LW_CPU_IS_ACTIVE(pcpuDst)) {                                   /*  CPU ���뱻����              */
        return  (ERROR_NONE);
    }
    
    LW_SPIN_LOCK_IGNIRQ(&pcpuDst->CPU_slIpi);                           /*  ����Ŀ�� CPU                */
    _List_Ring_Add_Last(&pipim->IPIM_ringManage, &pcpuDst->CPU_pringMsg);
    pcpuDst->CPU_uiMsgCnt++;
    LW_CPU_ADD_IPI_PEND(ulCPUId, LW_IPI_CALL_MSK);
    LW_SPIN_UNLOCK_IGNIRQ(&pcpuDst->CPU_slIpi);                         /*  ����Ŀ�� CPU                */
    
    archMpInt(ulCPUId);
    
    while (pipim->IPIM_iWait) {                                         /*  �ȴ�����                    */
        LW_SPINLOCK_DELAY();
    }
    
    return  (pipim->IPIM_iRet);
}
/*********************************************************************************************************
** ��������: _SmpCallIpiAllOther
** ��������: ����һ���Զ���˼��жϸ��������� CPU 
             ���ж�����±�����, �����Ҫ�ȴ�, ����뱣֤���� CPU �Ѿ�����.
** �䡡��  : pipim         �˼��жϲ���
** �䡡��  : NONE (�޷�ȷ������ֵ)
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _SmpCallIpiAllOther (PLW_IPI_MSG  pipim)
{
    ULONG   i;
    ULONG   ulCPUId;
    INT     iWaitSave = pipim->IPIM_iWait;
    
    ulCPUId = LW_CPU_GET_CUR_ID();
    
    KN_SMP_WMB();
    for (i = 0; i < LW_NCPUS; i++) {
        if (ulCPUId != i) {
            _SmpCallIpi(i, pipim);
            
            KN_SMP_MB();
            pipim->IPIM_iWait = iWaitSave;
            KN_SMP_WMB();
        }
    }
}
/*********************************************************************************************************
** ��������: _SmpCallFunc
** ��������: ���ú˼��ж���ָ���� CPU ����ָ���ĺ���
             ���ж�����±�����, ���뱣֤���� CPU �Ѿ�����.
** �䡡��  : ulCPUId       CPU ID
**           pfunc         ͬ��ִ�к���
**           pvArg         ͬ������
**           pfuncAsync    �첽ִ�к���
**           pvAsync       �첽ִ�в���
**           iOpt          ѡ�� IPIM_OPT_NORMAL / IPIM_OPT_NOKERN
** �䡡��  : ���÷���ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  _SmpCallFunc (ULONG        ulCPUId, 
                   FUNCPTR      pfunc, 
                   PVOID        pvArg,
                   VOIDFUNCPTR  pfuncAsync,
                   PVOID        pvAsync,
                   INT          iOpt)
{
    LW_IPI_MSG  ipim;
    
    ipim.IPIM_pfuncCall      = pfunc;
    ipim.IPIM_pvArg          = pvArg;
    ipim.IPIM_pfuncAsyncCall = pfuncAsync;
    ipim.IPIM_pvAsyncArg     = pvAsync;
    ipim.IPIM_iRet           = -1;
    ipim.IPIM_iOption        = iOpt;
    ipim.IPIM_iWait          = 1;
    
    return  (_SmpCallIpi(ulCPUId, &ipim));
}
/*********************************************************************************************************
** ��������: _SmpCallFunc
** ��������: ���ú˼��ж���ָ���� CPU ����ָ���ĺ���
             ���ж�����±�����, ���뱣֤���� CPU �Ѿ�����.
** �䡡��  : pfunc         ͬ��ִ�к���
**           pvArg         ͬ������
**           pfuncAsync    �첽ִ�к���
**           pvAsync       �첽ִ�в���
**           iOpt          ѡ�� IPIM_OPT_NORMAL / IPIM_OPT_NOKERN
** �䡡��  : NONE (�޷�ȷ������ֵ)
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpCallFuncAllOther (FUNCPTR      pfunc, 
                            PVOID        pvArg,
                            VOIDFUNCPTR  pfuncAsync,
                            PVOID        pvAsync,
                            INT          iOpt)
{
    LW_IPI_MSG  ipim;
    
    ipim.IPIM_pfuncCall      = pfunc;
    ipim.IPIM_pvArg          = pvArg;
    ipim.IPIM_pfuncAsyncCall = pfuncAsync;
    ipim.IPIM_pvAsyncArg     = pvAsync;
    ipim.IPIM_iRet           = -1;
    ipim.IPIM_iOption        = iOpt;
    ipim.IPIM_iWait          = 1;
    
    _SmpCallIpiAllOther(&ipim);
}
/*********************************************************************************************************
** ��������: _SmpProcFlushTlb
** ��������: ����˼��ж�ˢ�� TLB �Ĳ���
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0

static VOID  _SmpProcFlushTlb (PLW_CLASS_CPU  pcpuCur)
{
    INTREG          iregInterLevel;
    PLW_MMU_CONTEXT pmmuctx = __vmmGetCurCtx();

    iregInterLevel = KN_INT_DISABLE();
    __VMM_MMU_INV_TLB(pmmuctx);                                         /*  ��Ч���                    */
    KN_INT_ENABLE(iregInterLevel);

    LW_SPIN_LOCK_QUICK(&pcpuCur->CPU_slIpi, &iregInterLevel);           /*  ���� CPU                    */
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_FLUSH_TLB_MSK);                /*  ���                        */
    LW_SPIN_UNLOCK_QUICK(&pcpuCur->CPU_slIpi, iregInterLevel);          /*  ���� CPU                    */
    
    LW_SPINLOCK_NOTIFY();
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
** ��������: _SmpProcFlushCache
** ��������: ����˼��жϻ�д CACHE
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0

static VOID  _SmpProcFlushCache (PLW_CLASS_CPU  pcpuCur)
{
    INTREG  iregInterLevel;

    API_CacheFlush(DATA_CACHE, (PVOID)0, (size_t)~0);
    
    LW_SPIN_LOCK_QUICK(&pcpuCur->CPU_slIpi, &iregInterLevel);           /*  ���� CPU                    */
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_FLUSH_CACHE_MSK);              /*  ���                        */
    LW_SPIN_UNLOCK_QUICK(&pcpuCur->CPU_slIpi, iregInterLevel);          /*  ���� CPU                    */
    
    LW_SPINLOCK_NOTIFY();
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
** ��������: _SmpProcBoot
** ��������: ����˼��ж��������������� (��ǰδ����)
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _SmpProcBoot (PLW_CLASS_CPU  pcpuCur)
{
    INTREG  iregInterLevel;
    
    LW_SPIN_LOCK_QUICK(&pcpuCur->CPU_slIpi, &iregInterLevel);           /*  ���� CPU                    */
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_BOOT_MSK);                     /*  ���                        */
    LW_SPIN_UNLOCK_QUICK(&pcpuCur->CPU_slIpi, iregInterLevel);          /*  ���� CPU                    */
    
    LW_SPINLOCK_NOTIFY();
}
/*********************************************************************************************************
** ��������: __smpProcCallfunc
** ��������: ����˼��жϵ��ú���
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __smpProcCallfunc (PLW_CLASS_CPU  pcpuCur)
{
#define LW_KERNEL_OWN_CPU()     (PLW_CLASS_CPU)(_K_klKernel.KERN_pvCpuOwner)

    UINT            i, uiCnt;
    PLW_IPI_MSG     pipim;
    PLW_LIST_RING   pringTemp;
    PLW_LIST_RING   pringDelete;
    VOIDFUNCPTR     pfuncAsync;
    PVOID           pvAsync;
    
    LW_SPIN_LOCK_IGNIRQ(&pcpuCur->CPU_slIpi);                           /*  ���� CPU                    */
    
    pringTemp = pcpuCur->CPU_pringMsg;
    uiCnt     = pcpuCur->CPU_uiMsgCnt;
    
    for (i = 0; i < uiCnt; i++) {
        _BugHandle((!pcpuCur->CPU_pringMsg), LW_TRUE, "ipi call func error!\r\n");
        
        pipim = _LIST_ENTRY(pringTemp, LW_IPI_MSG, IPIM_ringManage);
        if ((LW_KERNEL_OWN_CPU() == pcpuCur) &&
            (pipim->IPIM_iOption & IPIM_OPT_NOKERN)) {                  /*  �˺����������ں�����״ִ̬��*/
            pringTemp = _list_ring_get_next(pringTemp);
            continue;
        }
        
        pringDelete = pringTemp;
        pringTemp   = _list_ring_get_next(pringTemp);
        _List_Ring_Del(pringDelete, &pcpuCur->CPU_pringMsg);            /*  ɾ��һ���ڵ�                */
        pcpuCur->CPU_uiMsgCnt--;
        
        if (pipim->IPIM_pfuncCall) {
            pipim->IPIM_iRet = pipim->IPIM_pfuncCall(pipim->IPIM_pvArg);/*  ִ��ͬ������                */
        }
        
        pfuncAsync = pipim->IPIM_pfuncAsyncCall;
        pvAsync    = pipim->IPIM_pvAsyncArg;
        
        KN_SMP_MB();
        pipim->IPIM_iWait = 0;                                          /*  ���ý���                    */
        KN_SMP_WMB();
        LW_SPINLOCK_NOTIFY();
        
        if (pfuncAsync) {
            pfuncAsync(pvAsync);                                        /*  ִ���첽����                */
        }
    }
    
    KN_SMP_MB();
    if (pcpuCur->CPU_pringMsg == LW_NULL) {
        LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_CALL_MSK);                 /*  ���                        */
    }
    
    LW_SPIN_UNLOCK_IGNIRQ(&pcpuCur->CPU_slIpi);                         /*  ���� CPU                    */
    
    LW_SPINLOCK_NOTIFY();
}
/*********************************************************************************************************
** ��������: _SmpProcCallfunc
** ��������: ����˼��жϵ��ú���
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _SmpProcCallfunc (PLW_CLASS_CPU  pcpuCur)
{
    INTREG  iregInterLevel;
    
    iregInterLevel = KN_INT_DISABLE();
    __smpProcCallfunc(pcpuCur);
    KN_INT_ENABLE(iregInterLevel);
}
/*********************************************************************************************************
** ��������: _SmpProcCallfuncIgnIrq
** ��������: ����˼��жϵ��ú��� (�Ѿ��ر��ж�)
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _SmpProcCallfuncIgnIrq (PLW_CLASS_CPU  pcpuCur)
{
    __smpProcCallfunc(pcpuCur);
}
/*********************************************************************************************************
** ��������: _SmpProcIpi
** ��������: ����˼��ж� (���ﲻ�����������Ϣ)
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpProcIpi (PLW_CLASS_CPU  pcpuCur)
{
    pcpuCur->CPU_iIPICnt++;                                             /*  �˼��ж����� ++             */

#if LW_CFG_VMM_EN > 0
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_FLUSH_TLB_MSK) {         /*  ���� MMU ���               */
        _SmpProcFlushTlb(pcpuCur);
    }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    
#if LW_CFG_CACHE_EN > 0
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_FLUSH_CACHE_MSK) {       /*  ��д CACHE                  */
        _SmpProcFlushCache(pcpuCur);
    }
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */

    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_BOOT_MSK) {              /*  ��������������              */
        _SmpProcBoot(pcpuCur);
    }
    
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_CALL_MSK) {              /*  �Զ������ ?                */
        _SmpProcCallfunc(pcpuCur);
    }
}
/*********************************************************************************************************
** ��������: _SmpTryProcIpi
** ��������: ���Դ���˼��ж� (�����������ִ�� FLUSH_TLB �� call ����)
** �䡡��  : pcpuCur       ��ǰ CPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpTryProcIpi (PLW_CLASS_CPU  pcpuCur)
{
#if LW_CFG_VMM_EN > 0
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_FLUSH_TLB_MSK) {         /*  ���� MMU ���               */
        _SmpProcFlushTlb(pcpuCur);
    }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */

    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_CALL_MSK) {              /*  �Զ������ ?                */
        _SmpProcCallfuncIgnIrq(pcpuCur);
    }
}
/*********************************************************************************************************
** ��������: _SmpUpdateIpi
** ��������: ����һ�� IPI
** �䡡��  : pcpuCur   CPU ���ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpUpdateIpi (PLW_CLASS_CPU  pcpu)
{
    if (!LW_CPU_IS_ACTIVE(pcpu)) {                                      /*  CPU ���뱻����              */
        return;
    }

    archMpInt(pcpu->CPU_ulCPUId);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
