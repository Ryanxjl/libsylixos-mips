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
** ��   ��   ��: lwip_vpnnetif.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2010 �� 05 �� 25 ��
**
** ��        ��: SSL VPN Ӧ�ýӿ�.
*********************************************************************************************************/

#ifndef __LWIP_VPN_H
#define __LWIP_VPN_H

/*********************************************************************************************************
  VPN ֤����֤��ʽ
*********************************************************************************************************/

#define LW_VPN_SSL_VERIFY_NONE       0                                  /*  SSL_VERIFY_NONE             */
#define LW_VPN_SSL_VERIFY_OPT        1                                  /*  SSL_VERIFY_OPTIONAL         */
#define LW_VPN_SSL_VERIFY_REQUIRED   2                                  /*  SSL_VERIFY_REQUIRED         */

/*********************************************************************************************************
  VPN API
*********************************************************************************************************/

LW_API VOID     API_INetVpnInit(VOID);
LW_API INT      API_INetVpnClientCreate(CPCHAR          cpcCACrtFile,
                                        CPCHAR          cpcPrivateCrtFile,
                                        CPCHAR          cpcKeyFile,
                                        CPCHAR          cpcKeyPassword,
                                        CPCHAR          cpcServerIp,
                                        CPCHAR          cpcServerClientIp,
                                        CPCHAR          cpcServerClientMask,
                                        CPCHAR          cpcServerClientGw,
                                        UINT16          usPort,
                                        INT             iSSLTimeoutSec,
                                        INT             iVerifyOpt,
                                        UINT8          *pucMac);
LW_API INT      API_INetVpnClientDelete(CPCHAR          pcNetifName);

#define inetVpnInit                 API_INetVpnInit
#define inetVpnClientCreate         API_INetVpnClientCreate
#define inetVpnClientDelete         API_INetVpnClientDelete

/*********************************************************************************************************
  shell ���� "vpnopen" ���������ļ���ʽʾ������:
  (����˳���� API_INetVpnClientCreate ��ͬ, ע��: �ļ��޿���, �����޿ո�)
  (����֤ʱ, ֤���ļ�����Կ�ļ���Ϊ null, ����Կ�ļ�����������ʱ, �����ֶ�Ϊ null)

  /mnt/ata0/vpn/config/ca_crt.pem
  /mnt/ata0/vpn/config/client_crt.pem
  /mnt/ata0/vpn/config/client_key.pem
  123456
  61.186.128.33
  192.168.0.123
  255.255.255.0
  192.168.0.1
  4443
  500
  1
  e2-22-32-a5-8f-94
*********************************************************************************************************/
#endif                                                                  /*  __LWIP_VPN_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
