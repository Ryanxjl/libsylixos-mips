/**********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: mipsMmuCommon.c
**
** ��   ��   ��: Ryan.Xin (����)
**
** �ļ���������: 2015 �� 09 �� 01 ��
**
** ��        ��: MIPS ��ϵ�ܹ� MMU ͨ�ú���֧��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "mipsMmuCommon.h"
/*********************************************************************************************************
** ��������: mipsGetAbtAddr
** ��������: MMU ϵͳ�������ݷ����쳣ʱ�ĵ�ַ
** �䡡��  : NONE
** �䡡��  : �����쳣��ַ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
addr_t  mipsGetAbtAddr (VOID)
{
    return  ((addr_t)0ul);
}
/*********************************************************************************************************
** ��������: mipsGetAbtType
** ��������: MMU ϵͳ�������ݷ����쳣ʱ������
** �䡡��  : NONE
** �䡡��  : �����쳣����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
ULONG   mipsGetAbtType (VOID)
{
    return  (LW_VMM_ABORT_TYPE_TERMINAL);
}
/*********************************************************************************************************
** ��������: mipsGetPreAddr
** ��������: MMU ϵͳ����ָ������쳣ʱ�ĵ�ַ
** �䡡��  : NONE
** �䡡��  : ָ���쳣��ַ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
addr_t  mipsGetPreAddr (addr_t  ulRetLr)
{
    return  (ulRetLr);
}
/*********************************************************************************************************
** ��������: mipsGetPreType
** ��������: MMU ϵͳ����ָ������쳣ʱ������
** �䡡��  : NONE
** �䡡��  : ָ���쳣����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
ULONG   mipsGetPreType (VOID)
{
    return  (LW_VMM_ABORT_TYPE_TERMINAL);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/