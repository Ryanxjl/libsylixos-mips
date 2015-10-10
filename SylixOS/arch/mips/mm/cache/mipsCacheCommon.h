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
** ��   ��   ��: mipsCacheCommon.h
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 09 �� 01 ��
**
** ��        ��: MIPS ��ϵ�ܹ� CACHE ͨ�ú���֧��.
*********************************************************************************************************/

#ifndef __MIPSCACHECOMMON_H
#define __MIPSCACHECOMMON_H

/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0

UINT32  armCacheTypeReg(VOID);
VOID    armCacheRetireRR(VOID);
VOID    armCacheRetireDefault(VOID);
VOID    armICacheEnable(VOID);
VOID    armDCacheEnable(VOID);
VOID    armICacheDisable(VOID);
VOID    armICacheInvalidate(PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep);
VOID    armICacheInvalidateAll(VOID);
VOID    armDCacheInvalidate(PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep);
VOID    armDCacheFlush(PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep);
VOID    armDCacheClear(PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep);

/*********************************************************************************************************
  CACHE ��� pvAdrs �� pvEnd λ��
*********************************************************************************************************/

#define MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiLineSize)              \
        do {                                                                \
            ulEnd  = (addr_t)((size_t)pvAdrs + stBytes);                    \
            pvAdrs = (PVOID)((addr_t)pvAdrs & ~((addr_t)uiLineSize - 1));   \
        } while (0)

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
#endif                                                                  /*  __MIPSCACHECOMMON_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
