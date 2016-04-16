;/*********************************************************************************************************
;**
;**                                    �й�������Դ��֯
;**
;**                                   Ƕ��ʽʵʱ����ϵͳ
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------�ļ���Ϣ--------------------------------------------------------------------------------
;**
;** ��   ��   ��: ppcMmuAsm.S
;**
;** ��   ��   ��: Jiao.JinXing (������)
;**
;** �ļ���������: 2016 �� 01 �� 14 ��
;**
;** ��        ��: PowerPC ��ϵ���� MMU ����.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/vmm_cfg.h>

#if LW_CFG_VMM_EN > 0

    FILE_BEGIN()

    EXPORT_LABEL(ppcMmuEnable)
    EXPORT_LABEL(ppcMmuDisable)

    EXPORT_LABEL(ppcMmuInvalidateTLBNr)
    EXPORT_LABEL(ppcMmuInvalidateTLBEA)

    EXPORT_LABEL(ppcMmuSetSDR1)
    EXPORT_LABEL(ppcMmuSetSR)

    EXPORT_LABEL(ppcMmuGetSRR1)
    EXPORT_LABEL(ppcMmuGetDSISR)

    EXPORT_LABEL(ppcHashPageTblPteSet)

;/*********************************************************************************************************
;  ʹ�� MMU
;*********************************************************************************************************/

FUNC_DEF(ppcMmuEnable)
    MFMSR   R3
    ISYNC
#if defined(ARCH_PPC_MSR_DR)
    ORI     R3 , R3 , ARCH_PPC_MSR_DR | ARCH_PPC_MSR_IR
#elif defined(ARCH_PPC_MSR_DS)
    ORI     R3 , R3 , ARCH_PPC_MSR_DS | ARCH_PPC_MSR_IS
#else
#error "bit to enable mmu is not defined"
#endif
    ISYNC
    MTMSR   R3
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ���� MMU
;*********************************************************************************************************/

FUNC_DEF(ppcMmuDisable)
    MFMSR   R3
    ISYNC
    ARCH_PPC_MMU_MASK(R3 , R3)
    ISYNC
    MTMSR   R3
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ��Ч���� TLB
;*********************************************************************************************************/

FUNC_DEF(ppcMmuInvalidateTLBNr)
    ISYNC
    MTCTR   R3                                                          ;/*  CTR = R3                    */
    XOR     R4 , R4 , R4                                                ;/*  R4 = 0                      */

LINE_LABEL(tlbloop)
    ISYNC
    TLBIE   R4
    EIEIO
    TLBSYNC
    SYNC
    ADDI    R4 , R4 , 0x1000                                            ;/*  Increment bits 15-19        */
    BDNZ    tlbloop                                                     ;/*  Dec CTR, branch if CTR != 0 */
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ��Чָ�� EA �� TLB
;*********************************************************************************************************/

FUNC_DEF(ppcMmuInvalidateTLBEA)
    SYNC
    TLBIE   R3
    EIEIO
    TLBSYNC
    SYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ���� PTE������Ч TLB
;  VOID  ppcHashPageTblPteSet(PTE    *pPte,
;                             UINT32  uiWord0,
;                             UINT32  uiWord1,
;                             UINT32  uiEffectiveAddr);
;*********************************************************************************************************/

FUNC_DEF(ppcHashPageTblPteSet)
    SYNC
    STW     R5, 4(R3)
    EIEIO
    STW     R4, 0(R3)
    SYNC
    TLBIE   R6
    EIEIO
    TLBSYNC
    SYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ���� SDR1 �Ĵ���
;*********************************************************************************************************/

FUNC_DEF(ppcMmuSetSDR1)
    SYNC
    MTSPR   SDR1 , R3
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ���öμĴ���
;  VOID  ppcMmuSetSR(UINT32  uiSRn, UINT32  uiValue);
;*********************************************************************************************************/

FUNC_DEF(ppcMmuSetSR)
    SYNC
    RLWINM  R3 , R3 , 28 , 0 , 3
    ISYNC
    MTSRIN  R4 , R3
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ��� SRR1 �Ĵ���
;*********************************************************************************************************/

FUNC_DEF(ppcMmuGetSRR1)
    MFSPR   R3 , SRR1
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  ��� DSISR �Ĵ���
;*********************************************************************************************************/

FUNC_DEF(ppcMmuGetDSISR)
    MFSPR   R3 , DSISR
    ISYNC
    BLR
    FUNC_END()

    FILE_END()
#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/