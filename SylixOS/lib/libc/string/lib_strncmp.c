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
** 文   件   名: lib_strncmp.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 02 月 13 日
**
** 描        述: 库

** BUG:
2013.06.09  strncmp 需要使用 unsigned char 比较大小.
*********************************************************************************************************/
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: lib_strncmp
** 功能描述: 
** 输　入  : 
** 输　出  : 
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT    lib_strncmp (CPCHAR  pcStr1, CPCHAR  pcStr2, size_t  stLen)
{
    REGISTER PUCHAR      pucStr1Reg = (PUCHAR)pcStr1;
    REGISTER PUCHAR      pucStr2Reg = (PUCHAR)pcStr2;

    for (; stLen > 0; stLen--) {
        if (*pucStr1Reg == *pucStr2Reg) {
            if (*pucStr1Reg == PX_EOS) {
                return  (0);
            }
        } else {
            break;
        }
        pucStr1Reg++;
        pucStr2Reg++;
    }
    
    if (stLen == 0) {
        return  (0);
    }
    
    if (*pucStr1Reg > *pucStr2Reg) {
        return  (1);
    }
    
    return  (-1);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
