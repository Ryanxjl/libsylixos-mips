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
** ��   ��   ��: armDbg.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 05 �� 14 ��
**
** ��        ��: ARM ��ϵ���ܵ������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_GDB_EN > 0
#include "dtrace.h"
#if LW_CFG_CACHE_EN > 0
#include "../mm/cache/armCacheCommon.h"
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
  ARM �ϵ�ʹ��δ����ָ���쳣, ARM �ϵ�ָ��� 16 λΪ thumb ָ��ϵ�,
  ���ڲ�֪����ָ���Ƿ�Ϊ thumb ָ��ʱ������,
  ������ next ����ִ��ʱ, ���Բ��� ARM �� thumb ģʽ, һ������ ARM_BREAKPOINT_INS.
*********************************************************************************************************/
#define ARM_BREAKPOINT_INS          0xE7FFDEFE
#define ARM_ABORTPOINT_INS          0xE7FFDEFF
#define ARM_BREAKPOINT_INS_TMB      0xDEFE
#define ARM_ABORTPOINT_INS_TMB      0xDEFF
/*********************************************************************************************************
** ��������: archDbgBpInsert
** ��������: ����һ���ϵ�.
** �䡡��  : ulAddr         �ϵ��ַ
**           stSize         �ϵ��С
**           pulIns         ���ص�֮ǰ��ָ��
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  archDbgBpInsert (addr_t  ulAddr, size_t stSize, ULONG  *pulIns)
{
    ULONG ulIns = ARM_BREAKPOINT_INS;

    lib_memcpy((PCHAR)pulIns, (PCHAR)ulAddr, stSize);                   /*  memcpy ���� arm ��������    */
    lib_memcpy((PCHAR)ulAddr, (PCHAR)&ulIns, stSize);
    
#if LW_CFG_CACHE_EN > 0
    API_CacheTextUpdate((PVOID)ulAddr, stSize);
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
}
/*********************************************************************************************************
** ��������: archDbgAbInsert
** ��������: ����һ���쳣��.
** �䡡��  : ulAddr         �ϵ��ַ
**           pulIns         ���ص�֮ǰ��ָ��
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �����쳣�ϵ�ʱ, ������ CPU ����ģʽ, ֱ�Ӳ��� 32 λ�ϵ㼴��.
*********************************************************************************************************/
VOID  archDbgAbInsert (addr_t  ulAddr, ULONG  *pulIns)
{
    *pulIns = *(ULONG *)ulAddr;
    *(ULONG *)ulAddr = ARM_ABORTPOINT_INS;
    
#if LW_CFG_CACHE_EN > 0
    API_CacheTextUpdate((PVOID)ulAddr, sizeof(ULONG));
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
}
/*********************************************************************************************************
** ��������: archDbgBpRemove
** ��������: ɾ��һ���ϵ�.
** �䡡��  : ulAddr         �ϵ��ַ
**           stSize         �ϵ��С
**           pulIns         ���ص�֮ǰ��ָ��
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  archDbgBpRemove (addr_t  ulAddr, size_t stSize, ULONG  ulIns)
{
    lib_memcpy((PCHAR)ulAddr, (PCHAR)&ulIns, stSize);
    
#if LW_CFG_CACHE_EN > 0
    API_CacheTextUpdate((PVOID)ulAddr, stSize);
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
}
/*********************************************************************************************************
** ��������: archDbgBpPrefetch
** ��������: Ԥȡһ��ָ��.
             ��ָ��� MMU ���������ʱ, ָ��ռ�Ϊ����ֻ��, ������Ҫ����һ��ȱҳ�ж�, ��¡һ������ҳ��.
** �䡡��  : ulAddr         �ϵ��ַ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  archDbgBpPrefetch (addr_t  ulAddr)
{
    volatile UINT8  ucByte = *(UINT8 *)ulAddr;                          /*  ��ȡ�ϵ㴦����              */
    
    *(UINT8 *)ulAddr = ucByte;                                          /*  ִ��һ��д����, ����ҳ���ж�*/
}
/*********************************************************************************************************
** ��������: archDbgTrapType
** ��������: ��ȡ trap ����.
** �䡡��  : ulAddr         �ϵ��ַ
**           pvArch         ��ϵ�ṹ��ز��� (ARM ������Ϊ CPSR)
** �䡡��  : LW_TRAP_INVAL / LW_TRAP_BRKPT / LW_TRAP_ABORT
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
UINT  archDbgTrapType (addr_t  ulAddr, PVOID   pvArch)
{
    REGISTER UINT32  uiCpsr = (UINT32)pvArch;

    if (uiCpsr & 0x20) {                                                /*  Thumb ģʽ                  */
        switch (*(UINT16 *)ulAddr) {

        case ARM_BREAKPOINT_INS_TMB:
            return  (LW_TRAP_BRKPT);

        case ARM_ABORTPOINT_INS_TMB:
            return  (LW_TRAP_ABORT);

        default:
            return  (LW_TRAP_INVAL);
        }
    } else {                                                            /*  ARM ģʽ                    */
        switch (*(ULONG *)ulAddr) {
        
        case ARM_BREAKPOINT_INS:
            return  (LW_TRAP_BRKPT);
            
        case ARM_ABORTPOINT_INS:
            return  (LW_TRAP_ABORT);
            
        default:
            return  (LW_TRAP_INVAL);
        }
    }
}
#endif                                                                  /*  LW_CFG_GDB_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
