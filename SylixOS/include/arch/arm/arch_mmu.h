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
** 文   件   名: arch_mmu.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 05 月 04 日
**
** 描        述: ARM 内存管理相关.
**
** BUG:
2015.04.01 将 VMM 页面尺寸相关的配置移动到这里.
*********************************************************************************************************/

#ifndef __ARM_ARCH_MMU_H
#define __ARM_ARCH_MMU_H

/*********************************************************************************************************
  虚拟内存页表相关配置
*********************************************************************************************************/

#define LW_CFG_VMM_PAGE_SHIFT                 12                        /*  2^12 = 4096                 */
#define LW_CFG_VMM_PAGE_SIZE                  (1ul << LW_CFG_VMM_PAGE_SHIFT)
#define LW_CFG_VMM_PAGE_MASK                  (~(LW_CFG_VMM_PAGE_SIZE - 1))

#define LW_CFG_VMM_PMD_SHIFT                  20                        /*  NO PMD same as PGD          */
#define LW_CFG_VMM_PMD_SIZE                   (1ul << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_MASK                   (~(LW_CFG_VMM_PMD_SIZE - 1))

#define LW_CFG_VMM_PGD_SHIFT                  20                        /*  2^20 = 1MB                  */
#define LW_CFG_VMM_PGD_SIZE                   (1ul << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_MASK                   (~(LW_CFG_VMM_PGD_SIZE - 1))

/*********************************************************************************************************
  转换条目相关, 每一层所包含的表项个数
*********************************************************************************************************/

#define LW_CFG_VMM_PTRS_PER_PGD               256                       /*  PGD 拥有的转换条目数        */
#define LW_CFG_VMM_PTRS_PER_PMD               1                         /*  PMD 拥有的转换条目数        */
#define LW_CFG_VMM_PTRS_PER_PTE               1024                      /*  PTE 拥有的转换条目数        */

/*********************************************************************************************************
  MMU 转换条目类型
*********************************************************************************************************/

typedef UINT32  LW_PGD_TRANSENTRY;                                      /*  页目录类型                  */
typedef UINT32  LW_PMD_TRANSENTRY;                                      /*  中间页目录类型              */
typedef UINT32  LW_PTE_TRANSENTRY;                                      /*  页表条目类型                */

#endif                                                                  /*  __ARM_ARCH_MMU_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
