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
** ��   ��   ��: loader_affinity.c
**
** ��   ��   ��: Han.hui (����)
**
** �ļ���������: 2014 �� 11 �� 11 ��
**
** ��        ��: ���� CPU �׺Ͷ�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
#if LW_CFG_MODULELOADER_EN > 0
#include "../include/loader_vppatch.h"
/*********************************************************************************************************
** ��������: vprocSetAffinityCb
** ��������: ���ý��̵��ȵ� CPU ���ϻص�
** �䡡��  : pid           ����
**           stSize        CPU ���뼯�ڴ��С
**           pcpuset       CPU ����
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  vprocSetAffinityCb (LW_OBJECT_HANDLE  ulId, size_t  stSize, const PLW_CLASS_CPUSET  pcpuset)
{
    API_ThreadSetAffinity(ulId, stSize, pcpuset);
}
/*********************************************************************************************************
** ��������: vprocSetAffinity
** ��������: ���ý��̵��ȵ� CPU ����
** �䡡��  : pid           ����
**           stSize        CPU ���뼯�ڴ��С
**           pcpuset       CPU ����
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  vprocSetAffinity (pid_t  pid, size_t  stSize, const PLW_CLASS_CPUSET  pcpuset)
{
    return  (vprocThreadTraversal2(pid, vprocSetAffinityCb, (PVOID)stSize, (PLW_CLASS_CPUSET)pcpuset,
                                   LW_NULL, LW_NULL, LW_NULL, LW_NULL));
}
/*********************************************************************************************************
** ��������: vprocGetAffinity
** ��������: ��ȡ���̵��ȵ� CPU ����
** �䡡��  : pid           ����
**           stSize        CPU ���뼯�ڴ��С
**           pcpuset       CPU ����
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
INT  vprocGetAffinity (pid_t  pid, size_t  stSize, PLW_CLASS_CPUSET  pcpuset)
{
    LW_OBJECT_HANDLE  ulId;
    
    if ((stSize < sizeof(LW_CLASS_CPUSET)) || !pcpuset) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    ulId = vprocMainThread(pid);
    if (ulId == LW_OBJECT_HANDLE_INVALID) {
        _ErrorHandle(ESRCH);
        return  (PX_ERROR);
    }
    
    if (API_ThreadGetAffinity(ulId, stSize, pcpuset)) {
        _ErrorHandle(ESRCH);
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
