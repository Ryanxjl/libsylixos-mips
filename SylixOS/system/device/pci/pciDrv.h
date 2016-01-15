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
** ��   ��   ��: pciDrv.h
**
** ��   ��   ��: Gong.YuJian (�����)
**
** �ļ���������: 2015 �� 12 �� 23 ��
**
** ��        ��: PCI �����豸��������.
*********************************************************************************************************/

#ifndef __PCI_DRV_H
#define __PCI_DRV_H

/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_PCI_EN > 0)
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
#define PCI_DRV_NAME_MAX            (24 + 1)                            /*  �����������ֵ              */
#define PCI_DRV_FLAG_ACTIVE         0x01                                /*  �Ƿ񼤻�                    */


#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_PCI_EN > 0)         */
#endif                                                                  /*  __PCI_DRV_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
