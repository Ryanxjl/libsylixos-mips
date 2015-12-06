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
** ��   ��   ��: Lw_Api_Net.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 05 �� 10 ��
**
** ��        ��: ����ϵͳ�ṩ�� C / C++ �û����ں�Ӧ�ó���ӿڲ㡣
                 Ϊ����Ӧ��ͬ����ϰ�ߵ��ˣ�����ʹ���˺ܶ��ظ�����.
*********************************************************************************************************/

#ifndef __LW_API_NET_H
#define __LW_API_NET_H

/*********************************************************************************************************
  snmp
*********************************************************************************************************/

#define Lw_Net_SnmpPriMibGet            API_NetSnmpPriMibGet
#define Lw_Net_SnmpSetPriMibInitHook    API_NetSnmpSetPriMibInitHook

/*********************************************************************************************************
  tools
*********************************************************************************************************/
/*********************************************************************************************************
  ping
*********************************************************************************************************/

#define Lw_INet_PingInit                API_INetPingInit

/*********************************************************************************************************
  telnet
*********************************************************************************************************/

#define Lw_INet_TelnetInit              API_INetTelnetInit

/*********************************************************************************************************
  netbios
*********************************************************************************************************/

#define Lw_INet_NetBiosInit             API_INetNetBiosInit
#define Lw_INet_NetBiosSetName          API_INetNetBiosNameSet
#define Lw_INet_NetBiosNameGet          API_INetNetBiosNameGet

/*********************************************************************************************************
  tftp
*********************************************************************************************************/

#define Lw_INet_TftpReceive             API_INetTftpReceive
#define Lw_INet_TftpSend                API_INetTftpSend
#define Lw_INet_TftpServerInit          API_INetTftpServerInit
#define Lw_INet_TftpServerPath          API_INetTftpServerPath

/*********************************************************************************************************
  ftp
*********************************************************************************************************/

#define Lw_Inet_FtpServerInit           API_INetFtpServerInit
#define Lw_Inet_FtpServerShow           API_INetFtpServerShow
#define Lw_Inet_FtpServerPath           API_INetFtpServerPath

/*********************************************************************************************************
  vpn
*********************************************************************************************************/

#define Lw_Inet_VpnInit                 API_INetVpnInit
#define Lw_Inet_VpnClientCreate         API_INetVpnClientCreate
#define Lw_Inet_VpnClientDelete         API_INetVpnClientDelete

/*********************************************************************************************************
  nat
*********************************************************************************************************/

#define Lw_Inet_NatInit                 API_INetNatInit
#define Lw_Inet_NatStart                API_INetNatStart
#define Lw_Inet_NatStop                 API_INetNatStop

/*********************************************************************************************************
  npf
*********************************************************************************************************/

#define Lw_Inet_NpfInit                 API_INetNpfInit
#define Lw_Inet_NpfRuleAdd              API_INetNpfRuleAdd
#define Lw_Inet_NpfRuleDel              API_INetNpfRuleDel
#define Lw_Inet_NpfAttach               API_INetNpfAttach
#define Lw_Inet_NpfDetach               API_INetNpfDetach
#define Lw_Inet_NpfDropGet              API_INetNpfDropGet
#define Lw_Inet_NpfAllowGet             API_INetNpfAllowGet
#define Lw_Inet_NpfShow                 API_INetNpfShow

#endif                                                                  /*  __LW_API_NET_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
