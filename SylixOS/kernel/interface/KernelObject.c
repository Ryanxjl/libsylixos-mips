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
** ��   ��   ��: KernelObject.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: �ں˶���.

** BUG:
2012.03.30  �������ں˶�������ϲ�������ļ�.
2012.12.07  API_ObjectIsGlobal() ʹ����Դ�������ж�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_ObjectGetClass
** ��������: ��ö�������
** �䡡��  : 
** �䡡��  : CLASS
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
UINT8  API_ObjectGetClass (LW_OBJECT_HANDLE  ulId)
{
    REGISTER UINT8    ucClass;

#if LW_CFG_ARG_CHK_EN > 0
    if (!ulId) {
        _ErrorHandle(ERROR_KERNEL_OBJECT_NULL);
        return  (0);
    }
#endif
    
    ucClass = (UINT8)_ObjectGetClass(ulId);
    
    return  (ucClass);
}
/*********************************************************************************************************
** ��������: API_ObjectIsGlobal
** ��������: ��ö����Ƿ�Ϊȫ�ֶ���
** �䡡��  : 
** �䡡��  : Ϊȫ�ֶ��󣬷��أ�LW_TRUE  ���򣬷��أ�LW_FALSE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
BOOL  API_ObjectIsGlobal (LW_OBJECT_HANDLE  ulId)
{
#if LW_CFG_MODULELOADER_EN > 0
    return  (__resHandleIsGlobal(ulId));
#else
    return  (LW_TRUE);
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
}
/*********************************************************************************************************
** ��������: API_ObjectGetNode
** ��������: ��ö������ڵĴ�������
** �䡡��  : 
** �䡡��  : ��������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_ObjectGetNode (LW_OBJECT_HANDLE  ulId)
{
    REGISTER ULONG    ulNode;
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!ulId) {
        _ErrorHandle(ERROR_KERNEL_OBJECT_NULL);
        return  ((unsigned)(PX_ERROR));
    }
#endif

    ulNode = _ObjectGetNode(ulId);
    
    return  (ulNode);
}
/*********************************************************************************************************
** ��������: API_ObjectGetIndex
** ��������: ��ö��󻺳����ڵ�ַ
** �䡡��  : 
** �䡡��  : ��ö��󻺳����ڵ�ַ
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_ObjectGetIndex (LW_OBJECT_HANDLE  ulId)
{
    REGISTER ULONG    ulIndex;
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!ulId) {
        _ErrorHandle(ERROR_KERNEL_OBJECT_NULL);
        return  ((unsigned)(PX_ERROR));
    }
#endif

    ulIndex = _ObjectGetIndex(ulId);
    
    return  (ulIndex);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/


