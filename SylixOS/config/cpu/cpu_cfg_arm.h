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
** 文   件   名: cpu_cfg_arm.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2009 年 09 月 29 日
**
** 描        述: ARM CPU 类型与功能配置.
*********************************************************************************************************/

#ifndef __CPU_CFG_ARM_H
#define __CPU_CFG_ARM_H

/*********************************************************************************************************
  CPU 体系结构
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_ARM             1                               /*  ARM family                  */

/*********************************************************************************************************
  整型大小端定义
*********************************************************************************************************/

#define LW_CFG_CPU_ENDIAN               0                               /*  0: 小端  1: 大端            */

/*********************************************************************************************************
  ARM 配置
*********************************************************************************************************/

#define LW_CFG_ARM_CP15                 1                               /*  ARM9 以上必须为 1           */
#define LW_CFG_ARM_PL330                1                               /*  是否允许 PL330 DMA 驱动     */
#define LW_CFG_ARM_CACHE_L2             1                               /*  是否允许管理 ARM 二级 CACHE */
                                                                        /*  PL210 / PL220 / PL310       */
/*********************************************************************************************************
  浮点运算单元
*********************************************************************************************************/

#define LW_CFG_CPU_FPU_EN               1                               /*  CPU 是否拥有 FPU            */

#endif                                                                  /*  __CPU_CFG_ARM_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
