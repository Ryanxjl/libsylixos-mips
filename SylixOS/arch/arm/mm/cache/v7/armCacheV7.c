/**********************************************************************************************************
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
** ��   ��   ��: armCacheV7.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 12 �� 09 ��
**
** ��        ��: ARMv7 ��ϵ���� CACHE ����.
**
** BUG:
2014.11.12  L2 CACHE ֻ�� CPU 0 ���ܲ���.
2015.08.21  ���� Invalidate ����������ַ�������.
2015.11.25  Text Update ����Ҫ���֧Ԥ��.
            Text Update ʹ�� armDCacheV7FlushPoU() ��д DCACHE.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0
#include "../armCacheCommon.h"
#include "../../mmu/armMmuCommon.h"
#include "../../../common/cp15/armCp15.h"
/*********************************************************************************************************
  L2 CACHE ֧��
*********************************************************************************************************/
#if LW_CFG_ARM_CACHE_L2 > 0
#include "../l2/armL2.h"
/*********************************************************************************************************
  L1 CACHE ״̬
*********************************************************************************************************/
static INT      iCacheStatus = 0;
#define L1_CACHE_I_EN   0x01
#define L1_CACHE_D_EN   0x02
#define L1_CACHE_EN     (L1_CACHE_I_EN | L1_CACHE_D_EN)
#define L1_CACHE_DIS    0x00
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
#define ARMV7_CSSELR_IND_DATA_UNIFIED   0
#define ARMV7_CSSELR_IND_INSTRUCTION    1

extern VOID     armDCacheV7Disable(VOID);
extern VOID     armDCacheV7FlushPoU(PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep);
extern VOID     armDCacheV7FlushAll(VOID);
extern VOID     armDCacheV7ClearAll(VOID);
extern UINT32   armCacheV7CCSIDR(VOID);
/*********************************************************************************************************
  CACHE ����
*********************************************************************************************************/
static UINT32                           uiArmV7CacheLineSize;
#define ARMv7_CACHE_LOOP_OP_MAX_SIZE    (32 * LW_CFG_KB_SIZE)
/*********************************************************************************************************
** ��������: armCacheV7Enable
** ��������: ʹ�� CACHE 
** �䡡��  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  armCacheV7Enable (LW_CACHE_TYPE  cachetype)
{
    if (cachetype == INSTRUCTION_CACHE) {
        armICacheEnable();
#if LW_CFG_ARM_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus |= L1_CACHE_I_EN;
        }
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
        armBranchPredictionEnable();

    } else {
        armDCacheEnable();
#if LW_CFG_ARM_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus |= L1_CACHE_D_EN;
        }
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    }
    
#if LW_CFG_ARM_CACHE_L2 > 0
    if ((LW_CPU_GET_CUR_ID() == 0) && 
        (iCacheStatus == L1_CACHE_EN)) {
        armL2Enable();
    }
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7Disable
** ��������: ���� CACHE 
** �䡡��  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  armCacheV7Disable (LW_CACHE_TYPE  cachetype)
{
    if (cachetype == INSTRUCTION_CACHE) {
        armICacheDisable();
#if LW_CFG_ARM_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus &= ~L1_CACHE_I_EN;
        }
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
        armBranchPredictionDisable();
        
    } else {
        armDCacheV7Disable();
#if LW_CFG_ARM_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus &= ~L1_CACHE_D_EN;
        }
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    }
    
#if LW_CFG_ARM_CACHE_L2 > 0
    if ((LW_CPU_GET_CUR_ID() == 0) && 
        (iCacheStatus == L1_CACHE_DIS)) {
        armL2Disable();
    }
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
     
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7Flush
** ��������: CACHE �����ݻ�д
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���� L2 Ϊ�����ַ tag ����������ʱʹ�� L2 ȫ����дָ��.
*********************************************************************************************************/
static INT	armCacheV7Flush (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;
    
    if (cachetype == DATA_CACHE) {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armDCacheV7FlushAll();                                      /*  ȫ����д                    */
        
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armDCacheFlush(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize); /*  ���ֻ�д                    */
        }
        
#if LW_CFG_ARM_CACHE_L2 > 0
        armL2FlushAll();
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7FlushPage
** ��������: CACHE �����ݻ�д
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           pvPdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT	armCacheV7FlushPage (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, PVOID  pvPdrs, size_t  stBytes)
{
    addr_t  ulEnd;
    
    if (cachetype == DATA_CACHE) {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armDCacheV7FlushAll();                                      /*  ȫ����д                    */
        
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armDCacheFlush(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize); /*  ���ֻ�д                    */
        }
        
#if LW_CFG_ARM_CACHE_L2 > 0
        armL2Flush(pvPdrs, stBytes);
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7Invalidate
** ��������: ָ�����͵� CACHE ʹ������Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ (pvAdrs ������������ַ)
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �˺���������� DCACHE pvAdrs �����ַ�������ַ������ͬ.
*********************************************************************************************************/
static INT	armCacheV7Invalidate (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armICacheInvalidateAll();                                   /*  ICACHE ȫ����Ч             */
        
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);
        }
    } else {
        if (stBytes > 0) {                                              /*  ���� > 0                    */
            addr_t  ulStart = (addr_t)pvAdrs;
                    ulEnd   = ulStart + stBytes;
            
            if (ulStart & ((addr_t)uiArmV7CacheLineSize - 1)) {         /*  ��ʼ��ַ�� cache line ����  */
                ulStart &= ~((addr_t)uiArmV7CacheLineSize - 1);
                armDCacheClear((PVOID)ulStart, (PVOID)ulStart, uiArmV7CacheLineSize);
                ulStart += uiArmV7CacheLineSize;
            }
            
            if (ulEnd & ((addr_t)uiArmV7CacheLineSize - 1)) {           /*  ������ַ�� cache line ����  */
                ulEnd &= ~((addr_t)uiArmV7CacheLineSize - 1);
                armDCacheClear((PVOID)ulEnd, (PVOID)ulEnd, uiArmV7CacheLineSize);
            }
                                                                        /*  ����Ч���벿��              */
            armDCacheInvalidate((PVOID)ulStart, (PVOID)ulEnd, uiArmV7CacheLineSize);
            
#if LW_CFG_ARM_CACHE_L2 > 0
            armL2Invalidate(pvAdrs, stBytes);                           /*  �����������ַ������ͬ      */
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
        } else {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "stBytes == 0.\r\n");
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7InvalidatePage
** ��������: ָ�����͵� CACHE ʹ������Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           pvPdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT	armCacheV7InvalidatePage (LW_CACHE_TYPE cachetype, PVOID pvAdrs, PVOID pvPdrs, size_t stBytes)
{
    addr_t  ulEnd;
    
    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armICacheInvalidateAll();                                   /*  ICACHE ȫ����Ч             */
        
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);
        }
    } else {
        if (stBytes > 0) {                                              /*  ���� > 0                    */
            addr_t  ulStart = (addr_t)pvAdrs;
                    ulEnd   = ulStart + stBytes;
                    
            if (ulStart & ((addr_t)uiArmV7CacheLineSize - 1)) {         /*  ��ʼ��ַ�� cache line ����  */
                ulStart &= ~((addr_t)uiArmV7CacheLineSize - 1);
                armDCacheClear((PVOID)ulStart, (PVOID)ulStart, uiArmV7CacheLineSize);
                ulStart += uiArmV7CacheLineSize;
            }
            
            if (ulEnd & ((addr_t)uiArmV7CacheLineSize - 1)) {           /*  ������ַ�� cache line ����  */
                ulEnd &= ~((addr_t)uiArmV7CacheLineSize - 1);
                armDCacheClear((PVOID)ulEnd, (PVOID)ulEnd, uiArmV7CacheLineSize);
            }
                                                                        /*  ����Ч���벿��              */
            armDCacheInvalidate((PVOID)ulStart, (PVOID)ulEnd, uiArmV7CacheLineSize);
            
#if LW_CFG_ARM_CACHE_L2 > 0
            armL2Invalidate(pvPdrs, stBytes);                           /*  �����������ַ������ͬ      */
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
        } else {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "stBytes == 0.\r\n");
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7Clear
** ��������: ָ�����͵� CACHE ʹ���ֻ�ȫ�����(��д�ڴ�)����Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���� L2 Ϊ�����ַ tag ����������ʱʹ�� L2 ȫ����д����Чָ��.
*********************************************************************************************************/
static INT	armCacheV7Clear (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armICacheInvalidateAll();                                   /*  ICACHE ȫ����Ч             */
            
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);
        }
    } else {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armDCacheV7ClearAll();                                      /*  ȫ����д����Ч              */
        
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armDCacheClear(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize); /*  ���ֻ�д����Ч              */
        }
        
#if LW_CFG_ARM_CACHE_L2 > 0
        armL2ClearAll();
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7ClearPage
** ��������: ָ�����͵� CACHE ʹ���ֻ�ȫ�����(��д�ڴ�)����Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           pvPdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT	armCacheV7ClearPage (LW_CACHE_TYPE cachetype, PVOID pvAdrs, PVOID pvPdrs, size_t stBytes)
{
    addr_t  ulEnd;
    
    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armICacheInvalidateAll();                                   /*  ICACHE ȫ����Ч             */
            
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);
        }
    } else {
        if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
            armDCacheV7ClearAll();                                      /*  ȫ����д����Ч              */
        
        } else {
            ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
            armDCacheClear(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize); /*  ���ֻ�д����Ч              */
        }
        
#if LW_CFG_ARM_CACHE_L2 > 0
        armL2Clear(pvPdrs, stBytes);
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: armCacheV7Lock
** ��������: ����ָ�����͵� CACHE 
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT	armCacheV7Lock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: armCacheV7Unlock
** ��������: ����ָ�����͵� CACHE 
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT	armCacheV7Unlock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: armCacheV7TextUpdate
** ��������: ���(��д�ڴ�) D CACHE ��Ч(���ʲ�����) I CACHE
** �䡡��  : pvAdrs                        �����ַ
**           stBytes                       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : L2 cache Ϊͳһ CACHE ���� text update ����Ҫ���� L2 cache.
*********************************************************************************************************/
static INT	armCacheV7TextUpdate (PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;
    
    if (stBytes >= ARMv7_CACHE_LOOP_OP_MAX_SIZE) {
        armDCacheV7FlushAll();                                          /*  DCACHE ȫ����д             */
        armICacheInvalidateAll();                                       /*  ICACHE ȫ����Ч             */
        
    } else {
        ARM_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiArmV7CacheLineSize);
        
#if LW_CFG_ARM_CACHE_L2 > 0
        armDCacheV7FlushPoU(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);
#else
        armDCacheFlush(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);     /*  ���ֻ�д                    */
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */
        armICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiArmV7CacheLineSize);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: archCacheV7Init
** ��������: ��ʼ�� CACHE 
** �䡡��  : pcacheop       CACHE ����������
**           uiInstruction  ָ�� CACHE ����
**           uiData         ���� CACHE ����
**           pcMachineName  ��������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  armCacheV7Init (LW_CACHE_OP *pcacheop, 
                      CACHE_MODE   uiInstruction, 
                      CACHE_MODE   uiData, 
                      CPCHAR       pcMachineName)
{
    UINT32  uiCCSIDR;

#define ARMv7_CCSIDR_LINESIZE_MASK      0x7
#define ARMv7_CCSIDR_LINESIZE(x)        ((x) & ARMv7_CCSIDR_LINESIZE_MASK)
#define ARMv7_CACHE_LINESIZE(x)         (16 << ARMv7_CCSIDR_LINESIZE(x))

#if LW_CFG_ARM_CACHE_L2 > 0
    armL2Init(uiInstruction, uiData, pcMachineName);
#endif                                                                  /*  LW_CFG_ARM_CACHE_L2 > 0     */

#if LW_CFG_SMP_EN > 0
    pcacheop->CACHEOP_ulOption = CACHE_TEXT_UPDATE_MP;
#else
    pcacheop->CACHEOP_ulOption = 0ul;
#endif                                                                  /*  LW_CFG_SMP_EN               */

    uiCCSIDR                     = armCacheV7CCSIDR();
    pcacheop->CACHEOP_iCacheLine = ARMv7_CACHE_LINESIZE(uiCCSIDR);
    uiArmV7CacheLineSize         = (UINT32)pcacheop->CACHEOP_iCacheLine;

    _DebugFormat(__LOGMESSAGE_LEVEL, "ARMv7 Cache line size = %d byte.\r\n",
                 pcacheop->CACHEOP_iCacheLine);

    if ((lib_strcmp(pcMachineName, ARM_MACHINE_A5) == 0) ||
        (lib_strcmp(pcMachineName, ARM_MACHINE_A7) == 0)) {
        pcacheop->CACHEOP_iILoc      = CACHE_LOCATION_VIPT;
        pcacheop->CACHEOP_iDLoc      = CACHE_LOCATION_PIPT;
        
    } else if (lib_strcmp(pcMachineName, ARM_MACHINE_A9) == 0) {
        pcacheop->CACHEOP_iILoc      = CACHE_LOCATION_VIPT;
        pcacheop->CACHEOP_iDLoc      = CACHE_LOCATION_PIPT;
        armAuxControlFeatureEnable(AUX_CTRL_A9_L1_PREFETCH);            /*  Cortex-A9 ʹ�� L1 Ԥȡ      */
    
    } else if (lib_strcmp(pcMachineName, ARM_MACHINE_A8) == 0) {
        pcacheop->CACHEOP_iILoc      = CACHE_LOCATION_VIPT;
        pcacheop->CACHEOP_iDLoc      = CACHE_LOCATION_PIPT;
        armAuxControlFeatureEnable(AUX_CTRL_A8_FORCE_ETM_CLK |
                                   AUX_CTRL_A8_FORCE_MAIN_CLK |
                                   AUX_CTRL_A8_L1NEON |
                                   AUX_CTRL_A8_FORCE_NEON_CLK |
                                   AUX_CTRL_A8_FORCE_NEON_SIGNAL);
    
    } else if (lib_strcmp(pcMachineName, ARM_MACHINE_A15) == 0) {
        pcacheop->CACHEOP_iILoc      = CACHE_LOCATION_PIPT;
        pcacheop->CACHEOP_iDLoc      = CACHE_LOCATION_PIPT;
        armAuxControlFeatureEnable(AUX_CTRL_A15_FORCE_MAIN_CLK |
                                   AUX_CTRL_A15_FORCE_NEON_CLK);
   
    } else if ((lib_strcmp(pcMachineName, ARM_MACHINE_A53)     == 0) ||
               (lib_strcmp(pcMachineName, ARM_MACHINE_A57)     == 0) ||
               (lib_strcmp(pcMachineName, ARM_MACHINE_FT1500A) == 0)) { /*  ARMv8 32 λģʽ             */
        pcacheop->CACHEOP_iILoc      = CACHE_LOCATION_PIPT;
        pcacheop->CACHEOP_iDLoc      = CACHE_LOCATION_PIPT;
    }
    
    pcacheop->CACHEOP_pfuncEnable  = armCacheV7Enable;
    pcacheop->CACHEOP_pfuncDisable = armCacheV7Disable;
    
    pcacheop->CACHEOP_pfuncLock    = armCacheV7Lock;                    /*  ��ʱ��֧����������          */
    pcacheop->CACHEOP_pfuncUnlock  = armCacheV7Unlock;
    
    pcacheop->CACHEOP_pfuncFlush          = armCacheV7Flush;
    pcacheop->CACHEOP_pfuncFlushPage      = armCacheV7FlushPage;
    pcacheop->CACHEOP_pfuncInvalidate     = armCacheV7Invalidate;
    pcacheop->CACHEOP_pfuncInvalidatePage = armCacheV7InvalidatePage;
    pcacheop->CACHEOP_pfuncClear          = armCacheV7Clear;
    pcacheop->CACHEOP_pfuncClearPage      = armCacheV7ClearPage;
    pcacheop->CACHEOP_pfuncTextUpdate     = armCacheV7TextUpdate;
    
#if LW_CFG_VMM_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = API_VmmDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = API_VmmDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = API_VmmDmaFree;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** ��������: archCacheV7Reset
** ��������: ��λ CACHE 
** �䡡��  : pcMachineName  ��������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ����� lockdown �������� unlock & invalidate ��������
*********************************************************************************************************/
VOID  armCacheV7Reset (CPCHAR  pcMachineName)
{
    armICacheInvalidateAll();
    armDCacheV7Disable();
    armICacheDisable();
    armBranchPredictorInvalidate();
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
