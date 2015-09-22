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
** ��   ��   ��: pageTable.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 12 �� 09 ��
**
** ��        ��: ƽ̨�޹������ڴ����, ҳ������ز�����.

** BUG:
2009.02.24  �����ڲ�ȫ��ӳ�� __vmmLibGlobalInit() ����.
2009.03.03  �޸� __vmmLibGlobalInit() ע��.
2009.03.04  MMU ȫ��ӳ���ϵ�� __vmmLibGlobalMap() ȫȨ����, 
            __VMM_MMU_GLOBAL_INIT() ��������ʼ����ϵ��عؼ���Ϣ.
2009.03.05  __vmmLibPageMap() ��ҳ��������, ���ҳ������ cache ����, ��д cache �� MMU ����������.
            ���봦�������Ϊ������ַ���� cache ����Ҫ��д cache , ���� ARM1136.
2009.05.21  ��ʼ��ʱ�� iErrNo ���ж��д���.
2009.11.10  __vmmLibVirtualToPhysical() ����ʱ����ҳ����Ч����.
2009.12.29  __vmmLibGlobalMap() �����ӡ�����ִ�����.
2010.08.13  __vmmLibInit() �е��� __vmm_pgd_alloc() ��ַΪ 0 .
            __vmmLibPageMap() ʹ�� __vmm_pgd_alloc() ��ȡһ��ҳ����Ӧ�����.
2011.03.02  ���� __vmmLibGetFlag() ����.
2011.05.20  �� __vmmLibGetFlag() ��÷���Чӳ��ʱ, ���뷵��ӳ����Ч����.
2013.08.20  ��Ч���ʱ, ��Ҫ֪ͨ������ CPU ��Ч���.
2013.07.20  �������Ӻ˷���� MMU ��ʼ��.
2014.11.09  VMM LIB ��ʼ���������� MMU.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
/*********************************************************************************************************
  ��ϵ�ṹ��������
*********************************************************************************************************/
extern VOID   __ARCH_MMU_INIT(CPCHAR  pcMachineName);                   /*  BSP MMU ��ʼ������          */
/*********************************************************************************************************
  �ڲ�ȫ��ӳ�亯������
*********************************************************************************************************/
static INT    __vmmLibGlobalMap(PLW_MMU_CONTEXT   pmmuctx, PLW_MMU_GLOBAL_DESC  pmmugdesc);
/*********************************************************************************************************
** ��������: __vmmGetCurCtx
** ��������: ��õ�ǰ MMU ������ (�ڲ�ʹ�û���������ʹ��)
** �䡡��  : NONE
** �䡡��  : ��ǰ����ִ�е� MMU ������
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
PLW_MMU_CONTEXT __vmmGetCurCtx (VOID)
{
    static LW_MMU_CONTEXT   mmuctxGlobal;                               /*  ȫ�� MMU CTX                */

    return  (&mmuctxGlobal);                                            /*  Ŀǰ������֧��, ÿһ������  */
                                                                        /*  ӵ���Լ��������ַ�ռ�      */
}
/*********************************************************************************************************
** ��������: __vmmLibPrimaryInit
** ��������: ��ʼ�� MMU ����, CPU ������ء�(���ģʽ��, Ϊ���� MMU ��ʼ��)
** �䡡��  : pmmugdesc         ��ʼ��ʹ�õ�ȫ��ӳ���
**           pcMachineName     �������еĻ�������
** �䡡��  : BSP ��������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmLibPrimaryInit (PLW_MMU_GLOBAL_DESC  pmmugdesc, CPCHAR  pcMachineName)
{
    static   BOOL             bIsInit          = LW_FALSE;
             BOOL             bIsNeedGlobalMap = LW_FALSE;

    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();
    REGISTER INT              iError;
    REGISTER ULONG            ulError;
    
             INT              iErrLevel = 0;

    if (bIsInit == LW_FALSE) {
        __ARCH_MMU_INIT(pcMachineName);                                 /*  ��ʼ�� MMU ����             */
        bIsInit          = LW_TRUE;
        bIsNeedGlobalMap = LW_TRUE;
    }
    
    iError = __VMM_MMU_MEM_INIT(pmmuctx);                               /*  ����ҳ���ڴ滺����          */
    if (iError < ERROR_NONE) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "mmu memory init error.\r\n");
        return  (ERROR_KERNEL_MEMORY);
    }
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "mmu initialize. start memory pagination...\r\n");
    
    /*
     *  __vmm_pgd_alloc() ��ַΪ 0 , ��ʾҳ����ַ + 0 ƫ����, ���Է��ص�ҳ�������ҳ����ַ.
     */
    pmmuctx->MMUCTX_pgdEntry = LW_NULL;
    pmmuctx->MMUCTX_pgdEntry = __vmm_pgd_alloc(pmmuctx, 0ul);           /*  ���� PGD ����               */
    if (pmmuctx->MMUCTX_pgdEntry == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "mmu can not allocate pgd entry.\r\n");
        iErrLevel = 1;
        ulError   = ERROR_KERNEL_MEMORY;
        goto    __error_handle;
    }
    
    if (bIsNeedGlobalMap) {
        iError = __VMM_MMU_GLOBAL_INIT(pcMachineName);                  /*  ��ʼ����ϵ��عؼ�����      */
        if (iError < ERROR_NONE) {
            iErrLevel = 2;
            ulError   = errno;
            goto    __error_handle;
        }
        
        iError = __vmmLibGlobalMap(pmmuctx, pmmugdesc);                 /*  ȫ���ڴ��ϵӳ��            */
        if (iError < ERROR_NONE) {
            iErrLevel = 2;
            ulError   = errno;
            goto    __error_handle;
        }
    }
    
    __VMM_MMU_MAKE_CURCTX(pmmuctx);                                     /*  ����ҳ������ַ              */
    KN_SMP_MB();
    
    return  (ERROR_NONE);
    
__error_handle:
    if (iErrLevel > 1) {
        __vmm_pgd_free(pmmuctx->MMUCTX_pgdEntry);
    }
    
    _ErrorHandle(ulError);
    return  (ulError);
}
/*********************************************************************************************************
** ��������: __vmmLibSecondaryInit
** ��������: ��ʼ�� MMU ����, CPU ������ء�(���ģʽ��, Ϊ���� MMU ��ʼ��)
** �䡡��  : pcMachineName     �������еĻ�������
** �䡡��  : BSP ��������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

ULONG  __vmmLibSecondaryInit (CPCHAR  pcMachineName)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();
    REGISTER INT              iError;
    REGISTER ULONG            ulError;
    
    iError = __VMM_MMU_GLOBAL_INIT(pcMachineName);                      /*  ��ʼ����ϵ��عؼ�����      */
    if (iError < ERROR_NONE) {
        ulError = errno;
        return  (ulError);
    }
    
    __VMM_MMU_MAKE_CURCTX(pmmuctx);                                     /*  ����ҳ��λ��                */
    KN_SMP_MB();
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
** ��������: __vmmLibGlobalMap
** ��������: ȫ��ҳ��ӳ���ϵ����
** �䡡��  : pmmuctx        MMU ������
**           pmmugdesc      ��ʼ��ʹ�õ�ȫ��ӳ���
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __vmmLibGlobalMap (PLW_MMU_CONTEXT   pmmuctx, PLW_MMU_GLOBAL_DESC  pmmugdesc) 
{
    INT     i;
    ULONG   ulPageNum;

    for (i = 0; pmmugdesc[i].stSize; i++) {
        ulPageNum = (ULONG)(pmmugdesc[i].stSize >> LW_CFG_VMM_PAGE_SHIFT);
        if (pmmugdesc[i].stSize & ~LW_CFG_VMM_PAGE_MASK) {
            ulPageNum++;
        }
        
        if (__vmmLibPageMap(pmmugdesc[i].ulPhysicalAddr, 
                            pmmugdesc[i].ulVirtualAddr,
                            ulPageNum,
                            pmmugdesc[i].ulFlag)) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "vmm global map fail.\r\n");
            return  (PX_ERROR);
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vmmLibPageMap
** ��������: ������ҳ������ӳ��
** �䡡��  : ulPhysicalAddr        ����ҳ���ַ
**           ulVirtualAddr         ��Ҫӳ��������ַ
**           ulPageNum             ��Ҫӳ���ҳ�����
**           ulFlag                ҳ���־
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmLibPageMap (addr_t  ulPhysicalAddr, 
                        addr_t  ulVirtualAddr, 
                        ULONG   ulPageNum, 
                        ULONG   ulFlag)
{
    INTREG                   iregInterLevel;
    ULONG                    i;
    PLW_MMU_CONTEXT          pmmuctx = __vmmGetCurCtx();
    LW_PGD_TRANSENTRY       *p_pgdentry;
    LW_PMD_TRANSENTRY       *p_pmdentry;
    LW_PTE_TRANSENTRY       *p_pteentry;
        
    for (i = 0; i < ulPageNum; i++) {
        p_pgdentry = __vmm_pgd_alloc(pmmuctx, ulVirtualAddr);
        if (p_pgdentry == LW_NULL) {
            return  (ERROR_VMM_LOW_LEVEL);
        }
        
        p_pmdentry = __vmm_pmd_alloc(pmmuctx, p_pgdentry, ulVirtualAddr);
        if (p_pmdentry == LW_NULL) {
            return  (ERROR_VMM_LOW_LEVEL);
        }
        
        p_pteentry = __vmm_pte_alloc(pmmuctx, p_pmdentry, ulVirtualAddr);
        if (p_pteentry == LW_NULL) {
            return  (ERROR_VMM_LOW_LEVEL);
        }
        
        iregInterLevel = KN_INT_DISABLE();                              /*  �ر��ж�                    */
        __VMM_MMU_MAKE_TRANS(pmmuctx, p_pteentry, 
                             ulPhysicalAddr, ulFlag);                   /*  ����ӳ���ϵ                */
        KN_INT_ENABLE(iregInterLevel);                                  /*  ���ж�                    */
        
        ulPhysicalAddr += LW_CFG_VMM_PAGE_SIZE;
        ulVirtualAddr  += LW_CFG_VMM_PAGE_SIZE;
    }
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */

#if LW_CFG_SMP_EN > 0
    if (LW_SYS_STATUS_IS_RUNNING() && 
        (__VMM_MMU_OPTION() & LW_VMM_MMU_FLUSH_TLB_MP)) {
        _SmpSendIpiAllOther(LW_IPI_FLUSH_TLB, 1);                       /*  ͬ��ˢ������ CPU TLB        */
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

    __VMM_MMU_INV_TLB(pmmuctx);                                         /*  ��Ч���                    */
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vmmLibGetFlag
** ��������: ��ȡָ���߼���ַ�ķ���Ȩ��
** �䡡��  : ulVirtualAddr         ��Ҫӳ��������ַ
**           pulFlag               ҳ���־
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmLibGetFlag (addr_t  ulVirtualAddr, ULONG  *pulFlag)
{
    PLW_MMU_CONTEXT          pmmuctx = __vmmGetCurCtx();
    ULONG                    ulFlag;

    ulFlag = __VMM_MMU_FLAG_GET(pmmuctx, ulVirtualAddr);
    if (pulFlag) {
        *pulFlag = ulFlag;
    }
    
    if (ulFlag & LW_VMM_FLAG_VALID) {
        return  (ERROR_NONE);
    } else {
        _ErrorHandle(ERROR_VMM_PAGE_INVAL);
        return  (ERROR_VMM_PAGE_INVAL);
    }
}
/*********************************************************************************************************
** ��������: __vmmLibSetFlag
** ��������: ����ָ���߼���ַ�ķ���Ȩ��
** �䡡��  : ulVirtualAddr         ��Ҫӳ��������ַ
**           ulFlag                ҳ���־
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmLibSetFlag (addr_t  ulVirtualAddr, ULONG  ulFlag)
{
    INTREG                   iregInterLevel;
    PLW_MMU_CONTEXT          pmmuctx = __vmmGetCurCtx();
    INT                      iError;

    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    iError = __VMM_MMU_FLAG_SET(pmmuctx, ulVirtualAddr, ulFlag);
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */

    if (iError < ERROR_NONE) {
        return  (ERROR_VMM_LOW_LEVEL);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: __vmmLibVirtualToPhysical
** ��������: ͨ�������ַ��ѯ��Ӧ��������ַ
** �䡡��  : ulVirtualAddr      �����ַ
**           pulPhysicalAddr    ���ص�������ַ
** �䡡��  : �����ַ, ���󷵻� (void *)-1 ��ַ.
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmLibVirtualToPhysical (addr_t  ulVirtualAddr, addr_t  *pulPhysicalAddr)
{
    PLW_MMU_CONTEXT          pmmuctx = __vmmGetCurCtx();
    LW_PGD_TRANSENTRY       *p_pgdentry;
    LW_PMD_TRANSENTRY       *p_pmdentry;
    LW_PTE_TRANSENTRY       *p_pteentry;
    
    INT                      iError;
    
    p_pgdentry = __VMM_MMU_PGD_OFFSET(pmmuctx, ulVirtualAddr);
    if (__VMM_MMU_PGD_NONE((*p_pgdentry))) {                            /*  �ж� PGD ��Ŀ��ȷ��         */
        goto    __error_handle;
    }
    p_pmdentry = __VMM_MMU_PMD_OFFSET(p_pgdentry, ulVirtualAddr);
    if (__VMM_MMU_PMD_NONE((*p_pmdentry))) {                            /*  �ж� PMD ��Ŀ��ȷ��         */
        goto    __error_handle;
    }
    p_pteentry = __VMM_MMU_PTE_OFFSET(p_pmdentry, ulVirtualAddr);
    if (__VMM_MMU_PTE_NONE((*p_pteentry))) {                            /*  �ж� PTE ��Ŀ��ȷ��         */
        goto    __error_handle;
    }
    
    iError = __VMM_MMU_PHYS_GET((*p_pteentry), pulPhysicalAddr);        /*  ��ѯҳ�����ַ              */
    if (iError < 0) {
        return  (ERROR_VMM_LOW_LEVEL);
    
    } else {
        *pulPhysicalAddr = (ulVirtualAddr & (LW_CFG_VMM_PAGE_SIZE - 1))
                         + (*pulPhysicalAddr);                          /*  ����ҳ��ƫ����              */
        return  (ERROR_NONE);
    }
    
__error_handle:
    _ErrorHandle(ERROR_VMM_PAGE_INVAL);
    return  (ERROR_VMM_PAGE_INVAL);
}
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/