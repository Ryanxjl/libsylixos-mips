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
** ��   ��   ��: mips32Cache.c
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 10 �� 12 ��
**
** ��        ��: MIPS32 ��ϵ���� CACHE ����.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0
#include "../mipsCacheCommon.h"
#include "../../../common/cp0/mipsCp0.h"

static UINT32                           uiMIPS32CacheLineSize;
#define MIPS32_CACHE_LOOP_OP_MAX_SIZE   (32 * LW_CFG_KB_SIZE)
#define CACHE_SIZE                      16*1024
#define CACHE_LINE_SIZE                 32
/*********************************************************************************************************
** ��������: mips32CacheEnable
** ��������: ʹ�� CACHE
** �䡡��  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheEnable (LW_CACHE_TYPE  cachetype)
{
    if (cachetype == INSTRUCTION_CACHE) {
        mipsICacheEnable();
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus |= L1_CACHE_I_EN;
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */

    } else {
        mipsDCacheEnable();
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus |= L1_CACHE_D_EN;
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
    }

#if LW_CFG_MIPS_CACHE_L2 > 0
    if ((LW_CPU_GET_CUR_ID() == 0) &&
        (iCacheStatus == L1_CACHE_EN)) {
        mips32L2Enable();
    }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheDisable
** ��������: ���� CACHE
** �䡡��  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheDisable (LW_CACHE_TYPE  cachetype)
{
    if (cachetype == INSTRUCTION_CACHE) {
        mipsICacheDisable();
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus &= ~L1_CACHE_I_EN;
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */

    } else {
        mipsDCacheDisable();
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (LW_CPU_GET_CUR_ID() == 0) {
            iCacheStatus &= ~L1_CACHE_D_EN;
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
    }

#if LW_CFG_MIPS_CACHE_L2 > 0
    if ((LW_CPU_GET_CUR_ID() == 0) &&
        (iCacheStatus == L1_CACHE_DIS)) {
        mips32L2Disable();
    }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheLock
** ��������: ����ָ�����͵� CACHE
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheLock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: mips32CacheUnlock
** ��������: ����ָ�����͵� CACHE
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheUnlock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}

/*********************************************************************************************************
** ��������: mips32CacheFlush
** ��������: CACHE �����ݻ�д
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : ���� L2 Ϊ�����ַ tag ����������ʱʹ�� L2 ȫ����дָ��.
*********************************************************************************************************/
static INT  mips32CacheFlush (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (cachetype == DATA_CACHE) {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
                                                                        /*  ȫ����д                    */
            mipsDCacheFlush(pvAdrs, (PVOID)(pvAdrs + CACHE_SIZE), uiMIPS32CacheLineSize);

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
                                                                        /*  ���ֻ�д                    */
            mipsDCacheFlush(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }

#if LW_CFG_MIPS_CACHE_L2 > 0
        mipsL2FlushAll();
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheFlushPage
** ��������: CACHE �����ݻ�д
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           pvPdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheFlushPage (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, PVOID  pvPdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (cachetype == DATA_CACHE) {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
                                                                        /*  ȫ����д                    */
            mipsDCacheFlush(pvAdrs, (PVOID)(pvAdrs + CACHE_SIZE), uiMIPS32CacheLineSize);

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
                                                                        /*  ���ֻ�д                    */
            mipsDCacheFlush(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }

#if LW_CFG_MIPS_CACHE_L2 > 0
        mipsL2Flush(pvPdrs, stBytes);
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheInvalidate
** ��������: ָ�����͵� CACHE ʹ������Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ (pvAdrs ������������ַ)
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : �˺���������� DCACHE pvAdrs �����ַ�������ַ������ͬ.
*********************************************************************************************************/
static INT  mips32CacheInvalidate (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheInvalidateAll();                                   /*  ICACHE ȫ����Ч            */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
            mipsICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }
    } else {
        if (stBytes > 0) {                                              /*  ���� > 0                   */
            addr_t  ulStart = (addr_t)pvAdrs;
                    ulEnd   = ulStart + stBytes;

            if (ulStart & ((addr_t)uiMIPS32CacheLineSize - 1)) {        /*  ��ʼ��ַ�� cache line ���� */
                ulStart &= ~((addr_t)uiMIPS32CacheLineSize - 1);
                mipsDCacheClear((PVOID)ulStart, (PVOID)ulStart, uiMIPS32CacheLineSize);
                ulStart += uiMIPS32CacheLineSize;
            }

            if (ulEnd & ((addr_t)uiMIPS32CacheLineSize - 1)) {          /*  ������ַ�� cache line ���� */
                ulEnd &= ~((addr_t)uiMIPS32CacheLineSize - 1);
                mipsDCacheClear((PVOID)ulEnd, (PVOID)ulEnd, uiMIPS32CacheLineSize);
            }
                                                                        /*  ����Ч���벿��              */
            mipsICacheInvalidate((PVOID)ulStart, (PVOID)ulEnd, uiMIPS32CacheLineSize);

#if LW_CFG_MIPS_CACHE_L2 > 0
            mipsL2Invalidate(pvAdrs, stBytes);                          /*  �����������ַ������ͬ     */
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        } else {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "stBytes == 0.\r\n");
        }
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheInvalidatePage
** ��������: ָ�����͵� CACHE ʹ������Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           pvPdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheInvalidatePage (LW_CACHE_TYPE cachetype, PVOID pvAdrs, PVOID pvPdrs, size_t stBytes)
{
    addr_t  ulEnd;

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheInvalidateAll();                                  /*  ICACHE ȫ����Ч             */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
            mipsICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }
    } else {
        if (stBytes > 0) {                                              /*  ���� > 0                    */
            addr_t  ulStart = (addr_t)pvAdrs;
                    ulEnd   = ulStart + stBytes;

            if (ulStart & ((addr_t)uiMIPS32CacheLineSize - 1)) {        /*  ��ʼ��ַ�� cache line ����  */
                ulStart &= ~((addr_t)uiMIPS32CacheLineSize - 1);
                mipsDCacheClear((PVOID)ulStart, (PVOID)ulStart, uiMIPS32CacheLineSize);
                ulStart += uiMIPS32CacheLineSize;
            }

            if (ulEnd & ((addr_t)uiMIPS32CacheLineSize - 1)) {          /*  ������ַ�� cache line ����  */
                ulEnd &= ~((addr_t)uiMIPS32CacheLineSize - 1);
                mipsDCacheClear((PVOID)ulEnd, (PVOID)ulEnd, uiMIPS32CacheLineSize);
            }
                                                                        /*  ����Ч���벿��              */
            mipsDCacheInvalidate((PVOID)ulStart, (PVOID)ulEnd, uiMIPS32CacheLineSize);

#if LW_CFG_MIPS_CACHE_L2 > 0
            mipsL2Invalidate(pvPdrs, stBytes);                          /*  �����������ַ������ͬ      */
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        } else {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "stBytes == 0.\r\n");
        }
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheClear
** ��������: ָ�����͵� CACHE ʹ���ֻ�ȫ�����(��д�ڴ�)����Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : ���� L2 Ϊ�����ַ tag ����������ʱʹ�� L2 ȫ����д����Чָ��.
*********************************************************************************************************/
static INT  mips32CacheClear (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheInvalidateAll();                                  /*  ICACHE ȫ����Ч             */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
            mipsICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }
    } else {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
                                                                        /*  ȫ����д����Ч              */
            mipsDCacheClear(pvAdrs, (PVOID)(pvAdrs + CACHE_SIZE), uiMIPS32CacheLineSize);

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
                                                                        /*  ���ֻ�д����Ч              */
            mipsDCacheClear(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }

#if LW_CFG_MIPS_CACHE_L2 > 0
        mipsL2ClearAll();
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheClearPage
** ��������: ָ�����͵� CACHE ʹ���ֻ�ȫ�����(��д�ڴ�)����Ч(���ʲ�����)
** �䡡��  : cachetype     CACHE ����
**           pvAdrs        �����ַ
**           pvPdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  mips32CacheClearPage (LW_CACHE_TYPE cachetype, PVOID pvAdrs, PVOID pvPdrs, size_t stBytes)
{
    addr_t  ulEnd;

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheInvalidateAll();                                  /*  ICACHE ȫ����Ч             */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
            mipsICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }
    } else {
        if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
                                                                        /*  ȫ����д����Ч              */
            mipsDCacheClear(pvAdrs, (PVOID)(pvAdrs + CACHE_SIZE), uiMIPS32CacheLineSize);

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
                                                                        /*  ���ֻ�д����Ч              */
            mipsDCacheClear(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
        }

#if LW_CFG_MIPS_CACHE_L2 > 0
        mipsL2Clear(pvPdrs, stBytes);
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheTextUpdate
** ��������: ���(��д�ڴ�) D CACHE ��Ч(���ʲ�����) I CACHE
** �䡡��  : pvAdrs        �����ַ
**           stBytes       ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : L2 cache Ϊͳһ CACHE ���� text update ����Ҫ���� L2 cache.
*********************************************************************************************************/
static INT  mips32CacheTextUpdate (PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (stBytes >= MIPS32_CACHE_LOOP_OP_MAX_SIZE) {
                                                                        /*  DCACHE ȫ����д             */
        mipsDCacheFlush(pvAdrs, (PVOID)(pvAdrs + CACHE_SIZE), uiMIPS32CacheLineSize);
        mipsICacheInvalidateAll();                                      /*  ICACHE ȫ����Ч             */

    } else {
        MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiMIPS32CacheLineSize);
        mipsDCacheFlush(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);   /*  ���ֻ�д                    */
        mipsICacheInvalidate(pvAdrs, (PVOID)ulEnd, uiMIPS32CacheLineSize);
    }
#if 0
    if (LW_NCPUS > 1) {
        mipsBranchPredictorInvalidateInnerShareable();                  /*  ���к������֧Ԥ��          */
    }
#endif
    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: mips32CacheInit
** ��������: ��ʼ�� CACHE
** �䡡��  : pcacheop       CACHE ����������
**           uiInstruction  ָ�� CACHE ����
**           uiData         ���� CACHE ����
**           pcMachineName  ��������
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  mips32CacheInit (LW_CACHE_OP *pcacheop,
                      CACHE_MODE   uiInstruction,
                      CACHE_MODE   uiData,
                      CPCHAR       pcMachineName)
{
    UINT32  uiCP0COFG1;

#define MIPS32_CP0CFG1IL_LINESIZE(x)    ((x) & M_Config1IL)
#define MIPS32_CACHE_LINESIZE(x)        (2 << MIPS32_CP0CFG1IL_LINESIZE(x))

#if LW_CFG_MIPS_CACHE_L2 > 0
    mipsL2Init(uiInstruction, uiData, pcMachineName);
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */

    uiCP0COFG1                   = mipsCp0Config1Read();
    pcacheop->CACHEOP_iCacheLine = MIPS32_CACHE_LINESIZE(uiCP0COFG1);
    uiMIPS32CacheLineSize         = (UINT32)pcacheop->CACHEOP_iCacheLine;

    _DebugFormat(__LOGMESSAGE_LEVEL, "MIPS Cache line size = %d byte.\r\n",
                 pcacheop->CACHEOP_iCacheLine);

    if ((lib_strcmp(pcMachineName, MIPS_MACHINE_NONE) == 0) ||
        (lib_strcmp(pcMachineName, MIPS_MACHINE_24KF) == 0)) {
        pcacheop->CACHEOP_iILoc      = CACHE_LOCATION_VIPT;
        pcacheop->CACHEOP_iDLoc      = CACHE_LOCATION_VIPT;

    }

    pcacheop->CACHEOP_pfuncEnable  = mips32CacheEnable;
    pcacheop->CACHEOP_pfuncDisable = mips32CacheDisable;

    pcacheop->CACHEOP_pfuncLock    = mips32CacheLock;                   /*  ��ʱ��֧����������          */
    pcacheop->CACHEOP_pfuncUnlock  = mips32CacheUnlock;

    pcacheop->CACHEOP_pfuncFlush          = mips32CacheFlush;
    pcacheop->CACHEOP_pfuncFlushPage      = mips32CacheFlushPage;
    pcacheop->CACHEOP_pfuncInvalidate     = mips32CacheInvalidate;
    pcacheop->CACHEOP_pfuncInvalidatePage = mips32CacheInvalidatePage;
    pcacheop->CACHEOP_pfuncClear          = mips32CacheClear;
    pcacheop->CACHEOP_pfuncClearPage      = mips32CacheClearPage;
    pcacheop->CACHEOP_pfuncTextUpdate     = mips32CacheTextUpdate;

#if LW_CFG_VMM_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = API_VmmDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = API_VmmDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = API_VmmDmaFree;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** ��������: mips32CacheReset
** ��������: ��λ CACHE
** �䡡��  : pcMachineName  ��������
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : ����� lockdown �������� unlock & invalidate ��������
*********************************************************************************************************/
VOID  mips32CacheReset (CPCHAR  pcMachineName)
{
    mipsICacheInvalidateAll();
    mipsDCacheDisable();
    mipsICacheDisable();
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
