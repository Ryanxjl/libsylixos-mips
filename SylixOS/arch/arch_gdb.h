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
** ��   ��   ��: arch_gdb.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 05 �� 22 ��
**
** ��        ��: SylixOS ��ϵ���� GDB ���Խӿ�.
*********************************************************************************************************/

#ifndef __ARCH_GDB_H
#define __ARCH_GDB_H

#include "config/cpu/cpu_cfg.h"

#if (defined LW_CFG_CPU_ARCH_ARM)
#include "./arm/arm_gdb.h"

#elif (defined LW_CFG_CPU_ARCH_X86)
#include "./x86/x86_gdb.h"

#elif (defined LW_CFG_CPU_ARCH_MIPS)
#include "./mips/mips_gdb.h"

#elif (defined LW_CFG_CPU_ARCH_PPC)
#include "./ppc/ppc_gdb.h"
#endif                                                                  /*  LW_CFG_CPU_ARCH_ARM         */

/*********************************************************************************************************
  �Ĵ������Ͻṹ
*********************************************************************************************************/
typedef struct {
    INT         GDBR_iRegCnt;                                           /* �Ĵ�������                   */
    struct {
        ULONG   GDBRA_ulValue;                                          /* �Ĵ���ֵ                     */
    } regArr[GDB_MAX_REG_CNT];                                          /* �Ĵ�������                   */
} GDB_REG_SET;

/*********************************************************************************************************
  Xfer:features:read:target.xml �� Xfer:features:read:arch-core.xml ��Ӧ��
*********************************************************************************************************/
CPCHAR  archGdbTargetXml(VOID);

CPCHAR  archGdbCoreXml(VOID);

/*********************************************************************************************************
  gdb ��Ҫ�ĺ���ϵ�ṹ��صĹ���
*********************************************************************************************************/
INT     archGdbRegsGet(PVOID               pvDtrace,
                       LW_OBJECT_HANDLE    ulThread,
                       GDB_REG_SET        *pregset);                    /*  ��ȡϵͳ�Ĵ�����Ϣ          */

INT     archGdbRegsSet(PVOID               pvDtrace,
                       LW_OBJECT_HANDLE    ulThread,
                       GDB_REG_SET        *pregset);                    /*  ����ϵͳ�Ĵ�����Ϣ          */

INT     archGdbRegSetPc(PVOID              pvDtrace,
                        LW_OBJECT_HANDLE   ulThread,
                        ULONG              uiPc);                       /*  ���� pc �Ĵ���              */

ULONG   archGdbRegGetPc (GDB_REG_SET *pRegs);                           /*  ��ȡ pc �Ĵ���ֵ            */

ULONG   archGdbGetNextPc(PVOID pvDtrace,
                         LW_OBJECT_HANDLE ulThread,
                         GDB_REG_SET *pRegs);                           /*  ��ȡ��һ�� pc ֵ������֧Ԥ��*/

#endif                                                                  /*  __ARCH_GDB_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
