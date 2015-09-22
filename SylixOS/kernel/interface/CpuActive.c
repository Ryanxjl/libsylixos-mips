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
** ��   ��   ��: CpuActive.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 07 �� 21 ��
**
** ��        ��: SMP ϵͳ����/�ر�һ�� CPU.
**
** ע        ��: �� CPU ��������ֹͣ��������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
** ��������: API_CpuUp
** ��������: ����һ�� CPU. (�� 0 �� CPU)
** �䡡��  : ulCPUId       CPU ID
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_CpuUp (ULONG  ulCPUId)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpu;

    if ((ulCPUId == 0) || (ulCPUId >= LW_NCPUS)) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    KN_SMP_MB();
    pcpu = LW_CPU_GET(ulCPUId);
    if (LW_CPU_IS_ACTIVE(pcpu) || 
        (LW_CPU_GET_IPI_PEND2(pcpu) & LW_IPI_DOWN_MSK)) {
        return  (ERROR_NONE);
    }
    
    iregInterLevel = KN_INT_DISABLE();
    bspCpuUp(ulCPUId);
    KN_INT_ENABLE(iregInterLevel);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_CpuDown
** ��������: �ر�һ�� CPU. (�� 0 �� CPU, ���ú󲢲������Ϲر�, ��Ҫ�ӳ�ʹ�� API_CpuIsUp ̽��)
** �䡡��  : ulCPUId       CPU ID
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_CpuDown (ULONG  ulCPUId)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpu;

    if ((ulCPUId == 0) || (ulCPUId >= LW_NCPUS)) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    __KERNEL_ENTER();
    pcpu = LW_CPU_GET(ulCPUId);
    if (!LW_CPU_IS_ACTIVE(pcpu) || 
        (LW_CPU_GET_IPI_PEND2(pcpu) & LW_IPI_DOWN_MSK)) {
        __KERNEL_EXIT();
        return  (ERROR_NONE);
    }
    
    LW_CPU_ADD_IPI_PEND2(pcpu, LW_IPI_DOWN_MSK);
    _ThreadOffAffinity(pcpu);                                           /*  �ر���� CPU �йص��׺Ͷ�   */
    __KERNEL_EXIT();
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    _SmpSendIpi(ulCPUId, LW_IPI_DOWN, 0);                               /*  ʹ�ú˼��ж�֪ͨ CPU ֹͣ   */
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_CpuIsUp
** ��������: ָ�� CPU �Ƿ�����.
** �䡡��  : ulCPUId       CPU ID
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
BOOL  API_CpuIsUp (ULONG  ulCPUId)
{
    PLW_CLASS_CPU   pcpu;
    
    if (ulCPUId >= LW_NCPUS) {
        _ErrorHandle(EINVAL);
        return  (LW_FALSE);
    }
    
    KN_SMP_MB();
    pcpu = LW_CPU_GET(ulCPUId);
    if (LW_CPU_IS_ACTIVE(pcpu)) {
        return  (LW_TRUE);
    
    } else {
        return  (LW_FALSE);
    }
}

#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
