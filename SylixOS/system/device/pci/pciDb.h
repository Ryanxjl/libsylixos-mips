/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: pciDb.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 09 �� 30 ��
**
** ��        ��: PCI ������Ϣ���ݿ�.
*********************************************************************************************************/

#ifndef __PCIDB_H
#define __PCIDB_H

/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_PCI_EN > 0)

VOID  __pciDbInit(VOID);
VOID  __pciDbGetClassStr(UINT8 ucClass, UINT8 ucSubClass, UINT8 ucProgIf, PCHAR pcBuffer, size_t  stSize);

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_PCI_EN > 0)         */
#endif                                                                  /*  __PCIDB_H                   */
/*********************************************************************************************************
  END
*********************************************************************************************************/