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
** 文   件   名: ppcSpr.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 12 月 17 日
**
** 描        述: PowerPC 体系构架特殊功能寄存器接口.
*********************************************************************************************************/

#ifndef __ARCH_PPCSPR_H
#define __ARCH_PPCSPR_H

extern UINT32  ppcGetMSR(VOID);
extern UINT32  ppcGetDAR(VOID);

extern VOID    ppcSetDEC(UINT32);
extern UINT32  ppcGetDEC(VOID);

#endif                                                                  /*  __ARCH_PPCSPR_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
