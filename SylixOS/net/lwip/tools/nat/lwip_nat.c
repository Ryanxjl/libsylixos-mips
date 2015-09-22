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
** ��   ��   ��: lwip_nat.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2010 �� 03 �� 19 ��
**
** ��        ��: lwip NAT ֧�ְ�.

** BUG:
2011.03.06  ���� gcc 4.5.1 ��� warning.
2011.07.30  nats ������������ն�(����: telnet)�б�����, ��Ϊ��Ҫʹ�� NAT ��, printf ������մ�ӡ������
            ��, ����� netproto ������ռ NAT ��.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/net/include/net_net.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_NET_EN > 0) && (LW_CFG_NET_NAT_EN > 0)
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip_natlib.h"
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
VOID        __natPoolCreate(VOID);
VOID        __natTimer(VOID);
VOID        __natShow(ip_addr_t  *pipaddr);

#if LW_CFG_SHELL_EN > 0
static INT  __tshellNat(INT  iArgC, PCHAR  ppcArgV[]);
static INT  __tshellNatShow(INT  iArgC, PCHAR  ppcArgV[]);
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
/*********************************************************************************************************
  NAT ���������� AP ��������ӿ�
*********************************************************************************************************/
extern struct netif        *_G_pnetifNatLocal;
extern struct netif        *_G_pnetifNatAp;
/*********************************************************************************************************
  NAT ������
*********************************************************************************************************/
extern LW_OBJECT_HANDLE     _G_ulNatOpLock;
/*********************************************************************************************************
  NAT ˢ�¶�ʱ��
*********************************************************************************************************/
static LW_OBJECT_HANDLE     _G_ulNatTimer;
/*********************************************************************************************************
** ��������: API_INetNatInit
** ��������: internet NAT ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
VOID  API_INetNatInit (VOID)
{
    static   BOOL   bIsInit = LW_FALSE;
    
    if (bIsInit) {
        return;
    }
    
    __natPoolCreate();                                                  /*  ���� NAT ���ƿ黺���       */
    
    _G_ulNatOpLock = API_SemaphoreMCreate("nat_lock", LW_PRIO_DEF_CEILING, 
                                          LW_OPTION_INHERIT_PRIORITY |
                                          LW_OPTION_OBJECT_GLOBAL,
                                          LW_NULL);                     /*  ���� NAT ������             */
                                          
    _G_ulNatTimer = API_TimerCreate("nat_timer", LW_OPTION_ITIMER | LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    
    API_TimerStart(_G_ulNatTimer, 
                   (LW_TICK_HZ * 60),
                   LW_OPTION_AUTO_RESTART,
                   (PTIMER_CALLBACK_ROUTINE)__natTimer,
                   LW_NULL);                                            /*  ÿ����ִ��һ��              */
    
#if LW_CFG_SHELL_EN > 0
    API_TShellKeywordAdd("nat", __tshellNat);
    API_TShellFormatAdd("nat", " [-stop] / {[LAN netif] [WAN netif]}");
    API_TShellHelpAdd("nat",   "start or stop NAT network.\n"
                               "eg. nat wl2 en1   (start NAT network use wl2 as LAN, en1 as WAN)\n"
                               "    nat -stop     (stop NAT network)\n");
                               
    API_TShellKeywordAdd("nats", __tshellNatShow);
    API_TShellFormatAdd("nats", " [ip addr]");
    API_TShellHelpAdd("nats",   "show NAT network.\n"
                                "warning: do not use this commond in networking terminal!\n");
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
    
    bIsInit = LW_TRUE;
}
/*********************************************************************************************************
** ��������: API_INetNatStart
** ��������: ���� NAT (��������ӿڱ���Ϊ lwip Ĭ�ϵ�·�ɽӿ�)
** �䡡��  : pcLocalNetif          ������������ӿ���
**           pcApNetif             ��������ӿ���
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_INetNatStart (CPCHAR  pcLocalNetif, CPCHAR  pcApNetif)
{
    struct netif   *pnetifLocal;
    struct netif   *pnetifAp;

    if (!pcLocalNetif || !pcApNetif) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (_G_pnetifNatLocal || _G_pnetifNatAp) {                          /*  NAT ���ڹ���                */
        _ErrorHandle(EISCONN);
        return  (PX_ERROR);
    }
    
    __NAT_OP_LOCK();                                                    /*  ���� NAT ����               */
    pnetifLocal = netif_find((PCHAR)pcLocalNetif);
    pnetifAp    = netif_find((PCHAR)pcApNetif);
    
    if (!pnetifLocal || !pnetifAp) {                                    /*  ������ӿڲ�����            */
        __NAT_OP_UNLOCK();                                              /*  ���� NAT ����               */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    __KERNEL_ENTER();
    _G_pnetifNatLocal = pnetifLocal;                                    /*  ��������ӿ�                */
    _G_pnetifNatAp    = pnetifAp;
    __KERNEL_EXIT();
    
    __NAT_OP_UNLOCK();                                                  /*  ���� NAT ����               */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_INetNatStop
** ��������: ֹͣ NAT (ע��: ɾ�� NAT ����ӿ�ʱ, ����Ҫ��ֹͣ NAT ����ӿ�)
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_INetNatStop (VOID)
{
    if (!_G_pnetifNatLocal || !_G_pnetifNatAp) {                        /*  NAT û�й���                */
        _ErrorHandle(ENOTCONN);
        return  (PX_ERROR);
    }
    
    __NAT_OP_LOCK();                                                    /*  ���� NAT ����               */
    
    __KERNEL_ENTER();
    _G_pnetifNatLocal = LW_NULL;
    _G_pnetifNatAp    = LW_NULL;
    __KERNEL_EXIT();
    
    __NAT_OP_UNLOCK();                                                  /*  ���� NAT ����               */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellNat
** ��������: ϵͳ���� "nat"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0

static INT  __tshellNat (INT  iArgC, PCHAR  ppcArgV[])
{
    if (iArgC == 3) {
        if (API_INetNatStart(ppcArgV[1], ppcArgV[2]) != ERROR_NONE) {
            fprintf(stderr, "can not start NAT network, errno : %s\n", lib_strerror(errno));
        } else {
            printf("NAT network started, [LAN : %s] [WAN : %s]\n", ppcArgV[1], ppcArgV[2]);
        }
    } else if (iArgC == 2) {
        if (lib_strcmp(ppcArgV[1], "-stop") == 0) {
            API_INetNatStop();
            printf("NAT network stoped.\n");
        }
    } else {
        fprintf(stderr, "option error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellNatShow
** ��������: ϵͳ���� "nats"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellNatShow (INT  iArgC, PCHAR  ppcArgV[])
{
    ip_addr_t       ipaddr;
    
    if (iArgC == 1) {
        ipaddr.addr = IPADDR_ANY;
        __natShow(&ipaddr);
    } else if (iArgC == 2) {
        ipaddr.addr = inet_addr(ppcArgV[2]);
        __natShow(&ipaddr);
    } else {
        fprintf(stderr, "option error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    return  (ERROR_NONE);
}
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */

#endif                                                                  /*  LW_CFG_NET_EN > 0           */
                                                                        /*  LW_CFG_NET_NAT_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
