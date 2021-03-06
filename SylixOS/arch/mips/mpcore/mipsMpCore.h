/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: mipsMpCore.h
**
** 创   建   人: Ryan.Xin (信金龙)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS 体系构架多核内部底层接口.
*********************************************************************************************************/

#ifndef __MIPSMPCORE_H
#define __MIPSMPCORE_H

/*********************************************************************************************************
  MIPS 自旋锁
*********************************************************************************************************/

VOID                     mipsSpinLock(volatile SPINLOCKTYPE  *psl);
volatile SPINLOCKTYPE    mipsSpinTryLock(volatile SPINLOCKTYPE  *psl);
VOID                     mipsSpinUnlock(volatile SPINLOCKTYPE  *psl);

#endif                                                                  /*  __MIPSMPCORE_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
