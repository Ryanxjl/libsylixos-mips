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
** ��   ��   ��: lwip_sylixos.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 05 �� 06 ��
**
** ��        ��: sylixos inet (lwip)
*********************************************************************************************************/

#ifndef __LWIP_SYLIXOS_H
#define __LWIP_SYLIXOS_H

/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0

/*********************************************************************************************************
  �����ʼ����������������.
*********************************************************************************************************/

LW_API VOID         API_NetInit(VOID);                                  /*  ��װ�������                */

LW_API INT          API_NetJobAdd(VOIDFUNCPTR  pfunc, 
                                  PVOID        pvArg0,
                                  PVOID        pvArg1,
                                  PVOID        pvArg2,
                                  PVOID        pvArg3,
                                  PVOID        pvArg4,
                                  PVOID        pvArg5);                 /*  net job add                 */
                                  
LW_API size_t       API_NetJobGetLost(VOID);

#define netInit             API_NetInit
#define netJobAdd           API_NetJobAdd
#define netJobGetLost       API_NetJobGetLost

/*********************************************************************************************************
  SNMP private mib �����ӿ�.
  ע��: API_NetSnmpPriMibGet() ����ֵ��ǿת�� struct mib_array_node * ����
        API_NetSnmpSetPriMibInitHook() ��Ҫ�� API_NetInit() ����ǰ��ʼ��.
*********************************************************************************************************/

#if LW_CFG_LWIP_SNMP > 0

LW_API PVOID        API_NetSnmpPriMibGet(VOID);
LW_API VOID         API_NetSnmpSetPriMibInitHook(VOIDFUNCPTR  pfuncPriMibInit);

#define netSnmpPriMibGet                API_NetSnmpPriMibGet
#define netSnmpSetPriMibInitHook        API_NetSnmpSetPriMibInitHook

#endif                                                                  /*  LW_CFG_LWIP_SNMP > 0        */

#endif                                                                  /*  LW_CFG_NET_EN               */
#endif                                                                  /*  __LWIP_SYLIXOS_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
