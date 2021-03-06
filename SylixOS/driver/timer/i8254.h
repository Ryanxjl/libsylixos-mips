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
** 文   件   名: i8254.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2015 年 10 月 28 日
**
** 描        述: Intel 8254 定时器支持.
*********************************************************************************************************/

#ifndef __I8254_H
#define __I8254_H

#include "SylixOS.h"

/*********************************************************************************************************
  i8254 control
*********************************************************************************************************/

typedef struct i8254_ctl I8254_CTL;

struct i8254_ctl {
    /*
     *  user MUST set following members before calling this module api.
     */
    addr_t  iobase;                                                     /* 8254 I/O base address        */
                                                                        /* eg. ISA-BASE + 0x40          */
    unsigned int qcofreq;                                               /* default : 1193182            */
};

/*********************************************************************************************************
  i8254 functions
*********************************************************************************************************/

UINT16  i8254Init(I8254_CTL *pctl, UINT32  uiHz);
UINT16  i8254InitAsTick(I8254_CTL *pctl);
UINT16  i8254InitAsHtimer(I8254_CTL *pctl);
UINT16  i8254GetCnt(I8254_CTL *pctl);

#endif                                                                  /*  __I8259A_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
