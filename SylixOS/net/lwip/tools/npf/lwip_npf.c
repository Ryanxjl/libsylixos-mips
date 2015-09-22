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
** ��   ��   ��: lwip_npf.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2011 �� 02 �� 11 ��
**
** ��        ��: lwip net packet filter ����.

** BUG:
2011.07.07  ����һЩƴд����.
2013.09.11  ���� /proc/net/netfilter �ļ�.
            UDP/TCP �˲�����˿ڷ�Χ.
2013.09.12  ʹ�� bnprintf ר�û�������ӡ����.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/net/include/net_net.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_NET_EN > 0) && (LW_CFG_NET_NPF_EN > 0)
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "netif/etharp.h"
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define __NPF_NETIF_RULE_MAX                4                           /*  ���������                  */
#define __NPF_NETIF_HASH_SIZE               5                           /*  ����ӿ� hash ����С        */
#define __NPF_NETIF_DEFAULT_INPUT           tcpip_input                 /*  ����ӿ�Ĭ�����뺯��        */
/*********************************************************************************************************
  �����������
*********************************************************************************************************/
static LW_OBJECT_HANDLE                     _G_ulNpfLock;
#define __NPF_LOCK()                        API_SemaphoreBPend(_G_ulNpfLock, LW_OPTION_WAIT_INFINITE)
#define __NPF_UNLOCK()                      API_SemaphoreBPost(_G_ulNpfLock)
/*********************************************************************************************************
  ͳ����Ϣ (���治��͹�����ֹ, ������ɶ�����������1)
*********************************************************************************************************/
static ULONG                _G_ulNpfDropPacketCounter  = 0;             /*  �������ݱ��ĸ���            */
static ULONG                _G_ulNpfAllowPacketCounter = 0;             /*  �������ݱ��ĸ���            */

#define __NPF_PACKET_DROP_INC()             (_G_ulNpfDropPacketCounter++)
#define __NPF_PACKET_ALLOW_INC()            (_G_ulNpfAllowPacketCounter++)
#define __NPF_PACKET_DROP_GET()             (_G_ulNpfDropPacketCounter)
#define __NPF_PACKET_ALLOW_GET()            (_G_ulNpfAllowPacketCounter)
/*********************************************************************************************************
  ������ƽṹ

  ע��:
       �����Ҫ���� UDP/TCP ĳһ�˿�, ��ʼ�ͽ��� IP ��ַ�ֶξ���Ϊ 0.0.0.0 ~ FF.FF.FF.FF
       NPFR?_iRule �ֶ���Ϊ�˴Ӿ����ȡ���������
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            NPFRM_lineManage;                           /*  MAC �����������            */
    INT                     NPFRM_iRule;
    u8_t                    NPFRM_ucMac[NETIF_MAX_HWADDR_LEN];          /*  ���������յ� MAC ��ַ       */
} __NPF_RULE_MAC;
typedef __NPF_RULE_MAC     *__PNPF_RULE_MAC;

typedef struct {
    LW_LIST_LINE            NPFRI_lineManage;                           /*  IP �����������             */
    INT                     NPFRI_iRule;
    ip_addr_t               NPFRI_ipaddrStart;                          /*  ��ֹͨ�� IP ����ʼ IP ��ַ  */
    ip_addr_t               NPFRI_ipaddrEnd;                            /*  ��ֹͨ�� IP �ν��� IP ��ַ  */
} __NPF_RULE_IP;
typedef __NPF_RULE_IP      *__PNPF_RULE_IP;

typedef struct {
    LW_LIST_LINE            NPFRU_lineManage;                           /*  UDP �����������            */
    INT                     NPFRU_iRule;
    ip_addr_t               NPFRU_ipaddrStart;                          /*  ��ֹͨ�� IP ����ʼ IP ��ַ  */
    ip_addr_t               NPFRU_ipaddrEnd;                            /*  ��ֹͨ�� IP �ν��� IP ��ַ  */
    u16_t                   NPFRU_usPortStart;                          /*  ��ֹͨ�ŵĶ˿���ʼ ������   */
    u16_t                   NPFRU_usPortEnd;                            /*  ��ֹͨ�ŵĶ˿ڽ���          */
} __NPF_RULE_UDP;
typedef __NPF_RULE_UDP     *__PNPF_RULE_UDP;

typedef struct {
    LW_LIST_LINE            NPFRT_lineManage;                           /*  TCP �����������            */
    INT                     NPFRT_iRule;
    ip_addr_t               NPFRT_ipaddrStart;                          /*  ��ֹͨ�� IP ����ʼ IP ��ַ  */
    ip_addr_t               NPFRT_ipaddrEnd;                            /*  ��ֹͨ�� IP �ν��� IP ��ַ  */
    u16_t                   NPFRT_usPortStart;                          /*  ��ֹͨ�ŵĶ˿���ʼ ������   */
    u16_t                   NPFRT_usPortEnd;                            /*  ��ֹͨ�ŵĶ˿ڽ���          */
} __NPF_RULE_TCP;
typedef __NPF_RULE_TCP     *__PNPF_RULE_TCP;
/*********************************************************************************************************
  ����������ӿڽṹ

  ע��:
       ���������ӿ���Ϊ ab3 �ڵ�һ�δ�������ʱ�����¼ NPFNI_pfuncOrgInput ����, �� ab3 ���ڱ��Ƴ�, ����
  ������ ab3 input ������֮ǰ��ͬʱ, �������������� npf ����, ����ʹ NPFNI_pfuncOrgInput ��¼��ȷ�ĺ���,
  ��Ŀǰ���������, netif->input Ӧ��Ϊ tcpip_input() ����.
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            NPFNI_lineHash;                             /*  hash ��                     */
    LW_LIST_LINE_HEADER     NPFNI_npfrnRule[__NPF_NETIF_RULE_MAX];      /*  �����                      */

    CHAR                    NPFNI_cName[2];                             /*  ����ӿ���                  */
    UINT8                   NPFNI_ucNum;
    netif_input_fn          NPFNI_pfuncOrgInput;                        /*  ԭ�ȵ����뺯��              */
                                                                        /*  ����Ϊ: tcpip_input()       */

    UINT                    NPFNI_uiAttachCounter;                      /*  ���� attach ������          */
} __NPF_NETIF_CB;
typedef __NPF_NETIF_CB     *__PNPF_NETIF_CB;
/*********************************************************************************************************
  ���� hash ��
*********************************************************************************************************/
static LW_LIST_LINE_HEADER  _G_plineNpfHash[__NPF_NETIF_HASH_SIZE];     /*  ͨ�� name & num ����ɢ��    */
static ULONG                _G_ulNpfCounter = 0ul;                      /*  ���������                  */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static err_t  __npfInput(struct pbuf *p, struct netif *inp);

#if LW_CFG_SHELL_EN > 0
static INT    __tshellNetNpfShow(INT  iArgC, PCHAR  *ppcArgV);
static INT    __tshellNetNpfRuleAdd(INT  iArgC, PCHAR  *ppcArgV);
static INT    __tshellNetNpfRuleDel(INT  iArgC, PCHAR  *ppcArgV);
static INT    __tshellNetNpfAttach(INT  iArgC, PCHAR  *ppcArgV);
static INT    __tshellNetNpfDetach(INT  iArgC, PCHAR  *ppcArgV);
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */

#if LW_CFG_PROCFS_EN > 0
static VOID   __procFsNpfInit(VOID);
#endif                                                                  /*  LW_CFG_PROCFS_EN > 0        */
/*********************************************************************************************************
** ��������: __npfNetifFind
** ��������: ����ָ���Ĺ���������ӿڽṹ
** �䡡��  : pcName         ����ӿ����ֲ���
**           ucNum          ����ӿڱ��
** �䡡��  : �ҵ��Ĺ���������ӿڽṹ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static __PNPF_NETIF_CB  __npfNetifFind (CPCHAR  pcName, UINT8  ucNum)
{
    REGISTER INT                    iIndex;
             PLW_LIST_LINE          plineTemp;
             __PNPF_NETIF_CB        pnpfniTemp;

    iIndex = (pcName[0] + pcName[1] + ucNum) % __NPF_NETIF_HASH_SIZE;

    for (plineTemp  = _G_plineNpfHash[iIndex];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {

        pnpfniTemp = _LIST_ENTRY(plineTemp, __NPF_NETIF_CB, NPFNI_lineHash);
        if ((pnpfniTemp->NPFNI_cName[0] == pcName[0]) &&
            (pnpfniTemp->NPFNI_cName[1] == pcName[1]) &&
            (pnpfniTemp->NPFNI_ucNum    == ucNum)) {
            return  (pnpfniTemp);
        }
    }

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __npfNetifFind2
** ��������: ����ָ���Ĺ���������ӿڽṹ
** �䡡��  : pcNetifName      ����ӿ�����
** �䡡��  : �ҵ��Ĺ���������ӿڽṹ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static __PNPF_NETIF_CB  __npfNetifFind2 (CPCHAR  pcNetifName)
{
    REGISTER INT                    iIndex;
             PLW_LIST_LINE          plineTemp;
             __PNPF_NETIF_CB        pnpfniTemp;
             UINT8                  ucNum;

    if ((pcNetifName[2] < '0') || (pcNetifName[2] > '9')) {
        return  (LW_NULL);
    }
    ucNum  = (UINT8)(pcNetifName[2] - '0');
    iIndex = (pcNetifName[0] + pcNetifName[1] + ucNum) % __NPF_NETIF_HASH_SIZE;

    for (plineTemp  = _G_plineNpfHash[iIndex];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {

        pnpfniTemp = _LIST_ENTRY(plineTemp, __NPF_NETIF_CB, NPFNI_lineHash);
        if ((pnpfniTemp->NPFNI_cName[0] == pcNetifName[0]) &&
            (pnpfniTemp->NPFNI_cName[1] == pcNetifName[1]) &&
            (pnpfniTemp->NPFNI_ucNum    == ucNum)) {
            return  (pnpfniTemp);
        }
    }

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __npfNetifInsertHash
** ��������: ��ָ���Ĺ���������ӿڽṹ���� hash ��
** �䡡��  : pnpfni        ����������ӿڽṹ
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __npfNetifInsertHash (__PNPF_NETIF_CB  pnpfni)
{
    REGISTER INT                    iIndex;
    REGISTER LW_LIST_LINE_HEADER   *ppline;

    iIndex = (pnpfni->NPFNI_cName[0]
           +  pnpfni->NPFNI_cName[1]
           +  pnpfni->NPFNI_ucNum)
           %  __NPF_NETIF_HASH_SIZE;

    ppline = &_G_plineNpfHash[iIndex];

    _List_Line_Add_Ahead(&pnpfni->NPFNI_lineHash, ppline);
}
/*********************************************************************************************************
** ��������: __npfNetifDeleteHash
** ��������: ��ָ���Ĺ���������ӿڽṹ�� hash �����Ƴ�
** �䡡��  : pnpfni        ����������ӿڽṹ
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __npfNetifDeleteHash (__PNPF_NETIF_CB  pnpfni)
{
    REGISTER INT                    iIndex;
    REGISTER LW_LIST_LINE_HEADER   *ppline;

    iIndex = (pnpfni->NPFNI_cName[0]
           +  pnpfni->NPFNI_cName[1]
           +  pnpfni->NPFNI_ucNum)
           %  __NPF_NETIF_HASH_SIZE;

    ppline = &_G_plineNpfHash[iIndex];

    _List_Line_Del(&pnpfni->NPFNI_lineHash, ppline);
}
/*********************************************************************************************************
** ��������: __npfNetifCreate
** ��������: ����һ������������ӿڽṹ (���������ֱ�ӷ������е�)
** �䡡��  : pcNetifName       ����ӿ���
** �䡡��  : �������Ĺ���������ӿڽṹ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static __PNPF_NETIF_CB  __npfNetifCreate (CPCHAR  pcNetifName)
{
    INT                    i;
    __PNPF_NETIF_CB        pnpfni;

    pnpfni = __npfNetifFind2(pcNetifName);
    if (pnpfni == LW_NULL) {
        pnpfni =  (__PNPF_NETIF_CB)__SHEAP_ALLOC(sizeof(__NPF_NETIF_CB));
        if (pnpfni == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfni->NPFNI_cName[0] = pcNetifName[0];
        pnpfni->NPFNI_cName[1] = pcNetifName[1];
        
        /*
         *  TODO: lwip ��ʱʹ�����ַ�ʽ�Ľӿ�������, �����ܽӿڲ��ܳ��� 10 ��.
         */
        pnpfni->NPFNI_ucNum    = (UINT8)(pcNetifName[2] - '0');
        pnpfni->NPFNI_pfuncOrgInput = __NPF_NETIF_DEFAULT_INPUT;        /*  ʹ��Ĭ�ϵ�Э��ջ���뺯��    */

        pnpfni->NPFNI_uiAttachCounter = 0ul;

        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            pnpfni->NPFNI_npfrnRule[i] = LW_NULL;                       /*  û���κι���                */
        }
        __npfNetifInsertHash(pnpfni);
    }

    return  (pnpfni);
}
/*********************************************************************************************************
** ��������: __npfNetifDelete
** ��������: ɾ��һ������������ӿڽṹ
** �䡡��  : pnpfni        ����������ӿڽṹ
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __npfNetifDelete (__PNPF_NETIF_CB  pnpfni)
{
    __npfNetifDeleteHash(pnpfni);

    __SHEAP_FREE(pnpfni);
}
/*********************************************************************************************************
** ��������: __npfNetifRemoveHook
** ��������: ������ӿ�ɾ��ʱ, ���õ� hook ����
** �䡡��  : pcNetifName       ��Ӧ������ӿ���
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
VOID  __npfNetifRemoveHook (struct netif  *pnetif)
{
    INT                 i;
    __PNPF_NETIF_CB     pnpfni;

    if (pnetif->input != __npfInput) {                                  /*  ���ǹ��������뺯��          */
        return;
    }

    __NPF_LOCK();                                                       /*  ���� NPF ��                 */
    pnpfni = __npfNetifFind(pnetif->name, pnetif->num);
    if (pnpfni == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        return;
    }
    pnetif->input = pnpfni->NPFNI_pfuncOrgInput;                        /*  ����ʹ�� npf ���뺯��       */

    pnpfni->NPFNI_uiAttachCounter--;
    if (pnpfni->NPFNI_uiAttachCounter == 0) {                           /*  û���κ� attach             */
        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            if (pnpfni->NPFNI_npfrnRule[i]) {
                break;
            }
        }

        if (i >= __NPF_NETIF_RULE_MAX) {                                /*  ������ӿڿ��ƽ��û�й���  */
            __npfNetifDelete(pnpfni);
        }
    }
    __NPF_UNLOCK();                                                     /*  ���� NPF ��                 */
}
/*********************************************************************************************************
** ��������: __npfMacRuleCheck
** ��������: ��� MAC �����Ƿ��������ݱ�ͨ��
** �䡡��  : pnpfni        ����������ӿ�
             pethhdr       ��̫��ͷ
** �䡡��  : �Ƿ��������ݱ�ͨ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static BOOL  __npfMacRuleCheck (__PNPF_NETIF_CB  pnpfni, struct eth_hdr *pethhdr)
{
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_MAC     pnpfrm;

    for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_MAC];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  ���������                  */

        pnpfrm = _LIST_ENTRY(plineTemp, __NPF_RULE_MAC, NPFRM_lineManage);
        if (lib_memcmp(pnpfrm->NPFRM_ucMac,
                       pethhdr->src.addr,
                       ETHARP_HWADDR_LEN) == 0) {                       /*  �Ƚ� 6 ���ֽ�               */
            return  (LW_FALSE);                                         /*  ��ֹͨ��                    */
        }
    }

    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: __npfIpRuleCheck
** ��������: ��� IP �����Ƿ��������ݱ�ͨ��
** �䡡��  : pnpfni        ����������ӿ�
             piphdr        IP ��ͷ
** �䡡��  : �Ƿ��������ݱ�ͨ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static BOOL  __npfIpRuleCheck (__PNPF_NETIF_CB  pnpfni, struct ip_hdr *piphdr)
{
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_IP      pnpfri;

    for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_IP];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  ���������                  */

        pnpfri = _LIST_ENTRY(plineTemp, __NPF_RULE_IP, NPFRI_lineManage);
        if ((piphdr->src.addr >= pnpfri->NPFRI_ipaddrStart.addr) &&
            (piphdr->src.addr <= pnpfri->NPFRI_ipaddrEnd.addr)) {
            return  (LW_FALSE);                                         /*  ��ֹͨ��                    */
        }
    }

    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: __npfUdpRuleCheck
** ��������: ��� UDP �����Ƿ��������ݱ�ͨ��
** �䡡��  : pnpfni        ����������ӿ�
             piphdr        IP ��ͷ
             pudphdr       UDP ��ͷ
** �䡡��  : �Ƿ��������ݱ�ͨ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static BOOL  __npfUdpRuleCheck (__PNPF_NETIF_CB  pnpfni, struct ip_hdr *piphdr, struct udp_hdr *pudphdr)
{
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_UDP     pnpfru;

    for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_UDP];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  ���������                  */

        pnpfru = _LIST_ENTRY(plineTemp, __NPF_RULE_UDP, NPFRU_lineManage);
        if ((piphdr->src.addr >= pnpfru->NPFRU_ipaddrStart.addr) &&
            (piphdr->src.addr <= pnpfru->NPFRU_ipaddrEnd.addr)   &&
            (pudphdr->dest    >= pnpfru->NPFRU_usPortStart)      &&
            (pudphdr->dest    <= pnpfru->NPFRU_usPortEnd)) {
            return  (LW_FALSE);                                         /*  ��ֹͨ��                    */
        }
    }

    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: __npfTcpRuleCheck
** ��������: ��� TCP �����Ƿ��������ݱ�ͨ��
** �䡡��  : pnpfni        ����������ӿ�
             piphdr        IP ��ͷ
             ptcphdr       TCP ��ͷ
** �䡡��  : �Ƿ��������ݱ�ͨ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static BOOL  __npfTcpRuleCheck (__PNPF_NETIF_CB  pnpfni, struct ip_hdr *piphdr, struct tcp_hdr *ptcphdr)
{
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_TCP     pnpfrt;

    for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_TCP];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  ���������                  */

        pnpfrt = _LIST_ENTRY(plineTemp, __NPF_RULE_TCP, NPFRT_lineManage);
        if ((piphdr->src.addr >= pnpfrt->NPFRT_ipaddrStart.addr) &&
            (piphdr->src.addr <= pnpfrt->NPFRT_ipaddrEnd.addr)   &&
            (ptcphdr->dest    >= pnpfrt->NPFRT_usPortStart)      &&
            (ptcphdr->dest    <= pnpfrt->NPFRT_usPortEnd)) {
            return  (LW_FALSE);                                         /*  ��ֹͨ��                    */
        }
    }

    return  (LW_TRUE);
}
/*********************************************************************************************************
** ��������: __npfInput
** ��������: ���������, �������������ݱ���������Э��ջ
** �䡡��  : p             ���ݰ�
**           inp           ����ӿ�
** �䡡��  : �����Ƿ���ȷ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static err_t  __npfInput (struct pbuf *p, struct netif *inp)
{
    __PNPF_NETIF_CB     pnpfni;                                         /*  ����������ӿ�              */
    INT                 iOffset = 0;                                    /*  �������ݵ���ʼƫ����        */

    struct eth_hdr      ethhdrChk;                                      /*  eth ͷ                      */
    struct ip_hdr       iphdrChk;                                       /*  ip ͷ                       */
    struct udp_hdr      udphdrChk;                                      /*  udp ͷ                      */
    struct tcp_hdr      tcphdrChk;                                      /*  tcp ͷ                      */


    __NPF_LOCK();                                                       /*  ���� NPF ��                 */
    pnpfni = __npfNetifFind(inp->name, inp->num);                       /*  ����������ӿ�              */
    if (pnpfni == LW_NULL) {                                            /*  û���ҵ���Ӧ�Ŀ��ƽṹ      */
        __NPF_PACKET_ALLOW_INC();
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        return  (__NPF_NETIF_DEFAULT_INPUT(p, inp));
    }

    /*
     *  ethernet ���˴���
     */
    if (inp->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET)) {       /*  ��̫���ӿ�                  */
        if (pbuf_copy_partial(p, (void *)&ethhdrChk,
                              sizeof(struct eth_hdr), (u16_t)iOffset) != sizeof(struct eth_hdr)) {
            goto    __allow_input;                                      /*  �޷���ȡ eth hdr ��������   */
        }

        if (__npfMacRuleCheck(pnpfni, &ethhdrChk)) {                    /*  ��ʼ�����Ӧ�� MAC ���˹��� */
            iOffset += sizeof(struct eth_hdr);                          /*  ����ͨ��                    */
        } else {
            __NPF_PACKET_DROP_INC();
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (ERR_IF);                                           /*  ����������                  */
        }
    }

    /*
     *  ip ���˴���
     */
    if (iOffset == sizeof(struct eth_hdr)) {                            /*  ǰ���� eth hdr �ֶ�         */
        if (ethhdrChk.type != PP_HTONS(ETHTYPE_IP)) {
            goto    __allow_input;                                      /*  ���� ip ���ݰ�, ����        */
        }
    }
    if (pbuf_copy_partial(p, (void *)&iphdrChk,
                          sizeof(struct ip_hdr), (u16_t)iOffset) != sizeof(struct ip_hdr)) {
        goto    __allow_input;                                          /*  �޷���ȡ ip hdr ����        */
    }
    if (IPH_V((&iphdrChk)) != 4) {                                      /*  �� ipv4 ���ݰ�              */
        goto    __allow_input;                                          /*  ����                        */
    }

    if (__npfIpRuleCheck(pnpfni, &iphdrChk)) {                          /*  ��ʼ�����Ӧ�� ip ���˹���  */
        iOffset += (IPH_HL((&iphdrChk)) * 4);                           /*  ����ͨ��                    */
    } else {
        __NPF_PACKET_DROP_INC();
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        return  (ERR_IF);                                               /*  ����������                  */
    }

    switch (IPH_PROTO((&iphdrChk))) {                                   /*  ��� ip ���ݱ�����          */

    case IP_PROTO_UDP:                                                  /*  udp ���˴���                */
    case IP_PROTO_UDPLITE:
        if (pbuf_copy_partial(p, (void *)&udphdrChk,
                              sizeof(struct udp_hdr), (u16_t)iOffset) != sizeof(struct udp_hdr)) {
            goto    __allow_input;                                      /*  �޷���ȡ udp hdr ����       */
        }
        if (__npfUdpRuleCheck(pnpfni, &iphdrChk, &udphdrChk) == LW_FALSE) {
            __NPF_PACKET_DROP_INC();
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (ERR_IF);                                           /*  ������                      */
        }
        break;

    case IP_PROTO_TCP:                                                  /*  tcp ���˴���                */
        if (pbuf_copy_partial(p, (void *)&tcphdrChk,
                              sizeof(struct tcp_hdr), (u16_t)iOffset) != sizeof(struct tcp_hdr)) {
            goto    __allow_input;                                      /*  �޷���ȡ tcp hdr ����       */
        }
        if (__npfTcpRuleCheck(pnpfni, &iphdrChk, &tcphdrChk) == LW_FALSE) {
            __NPF_PACKET_DROP_INC();
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (ERR_IF);                                           /*  ������                      */
        }
        break;

        /*
         *  TODO: δ������� ICMP �Ĺ��˹�����
         */
    default:
        break;
    }

__allow_input:
    __NPF_PACKET_ALLOW_INC();
    __NPF_UNLOCK();                                                     /*  ���� NPF ��                 */
    return  (pnpfni->NPFNI_pfuncOrgInput(p, inp));                      /*  ��������                    */
}
/*********************************************************************************************************
** ��������: API_INetNpfRuleAdd
** ��������: net packet filter ����һ������
** �䡡��  : pcNetifName       ��Ӧ������ӿ���
**           iRule             ��Ӧ�Ĺ���, MAC/IP/UDP/TCP/...
**           pucMac            ��ֹͨ�ŵ� MAC ��ַ����,
**           pcAddrStart       ��ֹͨ�� IP ��ַ��ʼ, Ϊ IP ��ַ�ַ���, ��ʽΪ: ???.???.???.???
**           pcAddrEnd         ��ֹͨ�� IP ��ַ����, Ϊ IP ��ַ�ַ���, ��ʽΪ: ???.???.???.???
**           usPortStart       ��ֹͨ�ŵı�����ʼ�˿ں�(�����ֽ���), �������� UDP/TCP ����
**           usPortEnd         ��ֹͨ�ŵı��ؽ����˿ں�(�����ֽ���), �������� UDP/TCP ����
** �䡡��  : ������
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
PVOID  API_INetNpfRuleAdd (CPCHAR  pcNetifName,
                           INT     iRule,
                           UINT8   pucMac[],
                           CPCHAR  pcAddrStart,
                           CPCHAR  pcAddrEnd,
                           UINT16  usPortStart,
                           UINT16  usPortEnd)
{
    __PNPF_NETIF_CB        pnpfni;

    if (!pcNetifName) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }

    if (iRule == LWIP_NPF_RULE_MAC) {                                   /*  ���� MAC ����               */
        __PNPF_RULE_MAC   pnpfrm;

        if (!pucMac) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfrm = (__PNPF_RULE_MAC)__SHEAP_ALLOC(sizeof(__NPF_RULE_MAC));/*  ��������                    */
        if (pnpfrm == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfrm->NPFRM_iRule = iRule;
        lib_memcpy(&pnpfrm->NPFRM_ucMac, pucMac, ETHARP_HWADDR_LEN);    /*  ���� 6 �ֽ�                 */

        __NPF_LOCK();                                                   /*  ���� NPF ��                 */
        pnpfni = __npfNetifCreate(pcNetifName);                         /*  �������ƿ�                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (LW_NULL);
        }
        _List_Line_Add_Ahead(&pnpfrm->NPFRM_lineManage,
                             &pnpfni->NPFNI_npfrnRule[iRule]);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */

        return  ((PVOID)pnpfrm);

    } else if (iRule == LWIP_NPF_RULE_IP) {                             /*  ���� IP ����                */
        __PNPF_RULE_IP      pnpfri;

        if (!pcAddrStart || !pcAddrEnd) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfri = (__PNPF_RULE_IP)__SHEAP_ALLOC(sizeof(__NPF_RULE_IP));  /*  ��������                    */
        if (pnpfri == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfri->NPFRI_iRule = iRule;
        
        pnpfri->NPFRI_ipaddrStart.addr = ipaddr_addr(pcAddrStart);
        pnpfri->NPFRI_ipaddrEnd.addr   = ipaddr_addr(pcAddrEnd);

        __NPF_LOCK();                                                   /*  ���� NPF ��                 */
        pnpfni = __npfNetifCreate(pcNetifName);                         /*  �������ƿ�                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (LW_NULL);
        }
        _List_Line_Add_Ahead(&pnpfri->NPFRI_lineManage,
                             &pnpfni->NPFNI_npfrnRule[iRule]);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */

        return  ((PVOID)pnpfri);

    } else if (iRule == LWIP_NPF_RULE_UDP) {                            /*  ���� UDP ����               */
        __PNPF_RULE_UDP   pnpfru;

        if (!pcAddrStart || !pcAddrEnd) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfru = (__PNPF_RULE_UDP)__SHEAP_ALLOC(sizeof(__NPF_RULE_UDP));/*  ��������                    */
        if (pnpfru == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfru->NPFRU_iRule = iRule;
        
        pnpfru->NPFRU_ipaddrStart.addr = ipaddr_addr(pcAddrStart);
        pnpfru->NPFRU_ipaddrEnd.addr   = ipaddr_addr(pcAddrEnd);
        pnpfru->NPFRU_usPortStart      = usPortStart;
        pnpfru->NPFRU_usPortEnd        = usPortEnd;

        __NPF_LOCK();                                                   /*  ���� NPF ��                 */
        pnpfni = __npfNetifCreate(pcNetifName);                         /*  �������ƿ�                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (LW_NULL);
        }
        _List_Line_Add_Ahead(&pnpfru->NPFRU_lineManage,
                             &pnpfni->NPFNI_npfrnRule[iRule]);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */

        return  ((PVOID)pnpfru);

    } else if (iRule == LWIP_NPF_RULE_TCP) {                            /*  ���� TCP ����               */
        __PNPF_RULE_TCP   pnpfrt;

        if (!pcAddrStart || !pcAddrEnd) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfrt = (__PNPF_RULE_TCP)__SHEAP_ALLOC(sizeof(__NPF_RULE_TCP));/*  ��������                    */
        if (pnpfrt == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfrt->NPFRT_iRule = iRule;
        
        pnpfrt->NPFRT_ipaddrStart.addr = ipaddr_addr(pcAddrStart);
        pnpfrt->NPFRT_ipaddrEnd.addr   = ipaddr_addr(pcAddrEnd);
        pnpfrt->NPFRT_usPortStart      = usPortStart;
        pnpfrt->NPFRT_usPortEnd        = usPortEnd;

        __NPF_LOCK();                                                   /*  ���� NPF ��                 */
        pnpfni = __npfNetifCreate(pcNetifName);                         /*  �������ƿ�                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            return  (LW_NULL);
        }
        _List_Line_Add_Ahead(&pnpfrt->NPFRT_lineManage,
                             &pnpfni->NPFNI_npfrnRule[iRule]);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */

        return  ((PVOID)pnpfrt);

    } else {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: API_INetNpfRuleDel
** ��������: net packet filter ɾ��һ������
** �䡡��  : pcNetifName       ��Ӧ������ӿ���
**           pvRule            ������ (����Ϊ NULL, Ϊ NULL ʱ��ʾʹ�ù������к�)
**           iSeqNum           ָ������ӿڵĹ������к� (�� 0 ��ʼ)
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
INT  API_INetNpfRuleDel (CPCHAR  pcNetifName,
                         PVOID   pvRule,
                         INT     iSeqNum)
{
    INT                    i;
    __PNPF_NETIF_CB        pnpfni;
    __PNPF_RULE_MAC        pnpfrm;

    if (!pcNetifName) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    if (!pvRule && (iSeqNum < 0)) {                                     /*  ��������������ͬʱ��Ч      */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    __NPF_LOCK();                                                       /*  ���� NPF ��                 */
    pnpfni = __npfNetifFind2(pcNetifName);
    if (pnpfni == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    if (pvRule) {                                                       /*  ͨ�����ɾ��                */
        pnpfrm = _LIST_ENTRY(pvRule, __NPF_RULE_MAC, NPFRM_lineManage);
        _List_Line_Del(&pnpfrm->NPFRM_lineManage,
                       &pnpfni->NPFNI_npfrnRule[pnpfrm->NPFRM_iRule]);
        _G_ulNpfCounter--;
        
    } else {                                                            /*  ͨ�����ɾ��                */
        PLW_LIST_LINE          plineTemp;

        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            for (plineTemp  = pnpfni->NPFNI_npfrnRule[i];
                 plineTemp != LW_NULL;
                 plineTemp  = _list_line_get_next(plineTemp)) {
                if (iSeqNum == 0) {
                    goto    __rule_find;
                }
                iSeqNum--;
            }
        }

__rule_find:
        if (iSeqNum || (plineTemp == LW_NULL)) {
            __NPF_UNLOCK();                                             /*  ���� NPF ��                 */
            _ErrorHandle(EINVAL);                                       /*  iSeqNum ��������            */
            return  (PX_ERROR);
        }
        _List_Line_Del(plineTemp,
                       &pnpfni->NPFNI_npfrnRule[i]);
        _G_ulNpfCounter--;
        
        pvRule = plineTemp;
    }

    /*
     *  ��� pnpfni ��û���κι���, ͬʱҲû���κ���������. pnpfni ���Ա�ɾ��.
     */
    if (pnpfni->NPFNI_uiAttachCounter == 0) {                           /*  û�����ӵ�����              */
        INT     i;

        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            if (pnpfni->NPFNI_npfrnRule[i]) {
                break;
            }
        }

        if (i >= __NPF_NETIF_RULE_MAX) {                                /*  ������ӿڿ��ƽ��û�й���  */
            __npfNetifDelete(pnpfni);
        }
    }
    __NPF_UNLOCK();                                                     /*  ���� NPF ��                 */

    __SHEAP_FREE(pvRule);                                               /*  �ͷ��ڴ�                    */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_INetNpfAttach
** ��������: �� net packet filter �󶨵�������, ʹ�������߱� npf ����.
** �䡡��  : pcNetifName       ��Ӧ������ӿ���
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
INT  API_INetNpfAttach (CPCHAR  pcNetifName)
{
    struct netif       *pnetif;
    __PNPF_NETIF_CB     pnpfni;

    if (!pcNetifName) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    __NPF_LOCK();                                                       /*  ���� NPF ��                 */
    pnetif = netif_find((PCHAR)pcNetifName);
    if (pnetif == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    pnpfni = __npfNetifCreate(pcNetifName);                             /*  �������ƿ�                  */
    if (pnpfni == LW_NULL) {
        return  (PX_ERROR);
    }
    pnpfni->NPFNI_pfuncOrgInput = pnetif->input;                        /*  ��¼��������뺯��          */
    pnetif->input = __npfInput;                                         /*  ʹ�� npf ���뺯��           */

    pnpfni->NPFNI_uiAttachCounter++;
    __NPF_UNLOCK();                                                     /*  ���� NPF ��                 */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_INetNpfDetach
** ��������: �� net packet filter �Ӱ󶨵������Ϸֿ�, ʹ���������߱� npf ����.
** �䡡��  : pcNetifName       ��Ӧ������ӿ���
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
INT  API_INetNpfDetach (CPCHAR  pcNetifName)
{
    INT                 i;
    struct netif       *pnetif;
    __PNPF_NETIF_CB     pnpfni;

    if (!pcNetifName) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    __NPF_LOCK();                                                       /*  ���� NPF ��                 */
    pnetif = netif_find((PCHAR)pcNetifName);
    if (pnetif == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    if (pnetif->input != __npfInput) {                                  /*  ���ǹ��������뺯��          */
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    pnpfni = __npfNetifFind2(pcNetifName);
    if (pnpfni == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    pnetif->input = pnpfni->NPFNI_pfuncOrgInput;                        /*  ����ʹ�� npf ���뺯��       */

    pnpfni->NPFNI_uiAttachCounter--;
    if (pnpfni->NPFNI_uiAttachCounter == 0) {                           /*  û���κ� attach             */
        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            if (pnpfni->NPFNI_npfrnRule[i]) {
                break;
            }
        }

        if (i >= __NPF_NETIF_RULE_MAX) {                                /*  ������ӿڿ��ƽ��û�й���  */
            __npfNetifDelete(pnpfni);
        }
    }
    __NPF_UNLOCK();                                                     /*  ���� NPF ��                 */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_INetNpfInit
** ��������: net packet filter ��ʼ��
** �䡡��  : NONE
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
INT  API_INetNpfInit (VOID)
{
    static BOOL bIsInit = LW_FALSE;

    if (bIsInit) {
        return  (ERROR_NONE);
    }

    _G_ulNpfLock = API_SemaphoreBCreate("sem_npflcok", LW_TRUE, 
                                        LW_OPTION_WAIT_FIFO | LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    if (_G_ulNpfLock == LW_OBJECT_HANDLE_INVALID) {
        return  (PX_ERROR);
    }

#if LW_CFG_SHELL_EN > 0
    /*
     *  ���� SHELL ����.
     */
    API_TShellKeywordAdd("npfs", __tshellNetNpfShow);
    API_TShellHelpAdd("npfs",    "show net packet filter rule(s).\n");

    API_TShellKeywordAdd("npfruleadd", __tshellNetNpfRuleAdd);
    API_TShellFormatAdd("npfruleadd",  " [netifname] [rule] [args...]");
    API_TShellHelpAdd("npfruleadd",    "add a rule into net packet filter.\n"
                                       "eg. npfruleadd en1 mac 11:22:33:44:55:66\n"
                                       "    npfruleadd en1 ip 192.168.0.5 192.168.0.10\n"
                                       "    npfruleadd lo0 udp 0.0.0.0 255.255.255.255 433 500\n"
                                       "    npfruleadd wl2 tcp 192.168.0.1 192.168.0.200 169 169\n");

    API_TShellKeywordAdd("npfruledel", __tshellNetNpfRuleDel);
    API_TShellFormatAdd("npfruledel",  " [netifname] [rule sequence num]");
    API_TShellHelpAdd("npfruledel",    "del a rule into net packet filter.\n");

    API_TShellKeywordAdd("npfattach", __tshellNetNpfAttach);
    API_TShellFormatAdd("npfattach",  " [netifname]");
    API_TShellHelpAdd("npfattach",    "attach net packet filter to a netif.\n");

    API_TShellKeywordAdd("npfdetach", __tshellNetNpfDetach);
    API_TShellFormatAdd("npfdetach",  " [netifname]");
    API_TShellHelpAdd("npfdetach",    "detach net packet filter from a netif.\n");
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */

#if LW_CFG_PROCFS_EN > 0
    __procFsNpfInit();
#endif                                                                  /*  LW_CFG_PROCFS_EN > 0        */

    bIsInit = LW_TRUE;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_INetNpfDropGet
** ��������: net packet filter ���������ݰ����� (���������Թ��˺ͻ��治����ɵĶ���)
** �䡡��  : NONE
** �䡡��  : ���������ݰ�����
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
ULONG  API_INetNpfDropGet (VOID)
{
    return  (__NPF_PACKET_DROP_GET());
}
/*********************************************************************************************************
** ��������: API_INetNpfAllowGet
** ��������: net packet filter ����ͨ�������ݰ�����
** �䡡��  : NONE
** �䡡��  : ����ͨ�������ݰ�����
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
ULONG  API_INetNpfAllowGet (VOID)
{
    return  (__NPF_PACKET_ALLOW_GET());
}
/*********************************************************************************************************
** ��������: API_INetNpfShow
** ��������: net packet filter ��ʾ��ǰ���еĹ�����Ŀ
** �䡡��  : iFd           ��ӡĿ���ļ�������
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
INT  API_INetNpfShow (INT  iFd)
{
    INT     iFilterFd;
    CHAR    cBuffer[512];
    ssize_t sstNum;
    
    iFilterFd = open("/proc/net/netfilter", O_RDONLY);
    if (iFilterFd < 0) {
        fprintf(stderr, "can not open /proc/net/netfilter : %s\n", lib_strerror(errno));
        return  (PX_ERROR);
    }
    
    do {
        sstNum = read(iFilterFd, cBuffer, sizeof(cBuffer));
        if (sstNum > 0) {
            write(iFd, cBuffer, (size_t)sstNum);
        }
    } while (sstNum > 0);
    
    close(iFilterFd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellNetNpfShow
** ��������: ϵͳ���� "npfs"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0

static INT  __tshellNetNpfShow (INT  iArgC, PCHAR  *ppcArgV)
{
    return  (API_INetNpfShow(STD_OUT));
}
/*********************************************************************************************************
** ��������: __tshellNetNpfRuleAdd
** ��������: ϵͳ���� "npfruleadd"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  __tshellNetNpfRuleAdd (INT  iArgC, PCHAR  *ppcArgV)
{
#define __NPF_TSHELL_RADD_ARG_NETIF     1
#define __NPF_TSHELL_RADD_ARG_RULE      2
#define __NPF_TSHELL_RADD_ARG_MAC       3
#define __NPF_TSHELL_RADD_ARG_IPS       3
#define __NPF_TSHELL_RADD_ARG_IPE       4
#define __NPF_TSHELL_RADD_ARG_PORTS     5
#define __NPF_TSHELL_RADD_ARG_PORTE     6
    PVOID    pvRule;

    if (iArgC < 4) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "mac") == 0) {
        INT         i;
        UINT8       ucMac[NETIF_MAX_HWADDR_LEN];
        INT         iMac[NETIF_MAX_HWADDR_LEN];

        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_MAC], "%x:%x:%x:%x:%x:%x",
                   &iMac[0], &iMac[1], &iMac[2], &iMac[3], &iMac[4], &iMac[5]) != 6) {
            fprintf(stderr, "mac argment error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        for (i = 0; i < ETHARP_HWADDR_LEN; i++) {
            ucMac[i] = (UINT8)iMac[i];
        }
        pvRule = API_INetNpfRuleAdd(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                    LWIP_NPF_RULE_MAC,
                                    ucMac, LW_NULL, LW_NULL, 0, 0);
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add mac rule, error : %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "ip") == 0) {
        if (iArgC != 5) {
            fprintf(stderr, "argments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        pvRule = API_INetNpfRuleAdd(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                    LWIP_NPF_RULE_IP,
                                    LW_NULL,
                                    ppcArgV[__NPF_TSHELL_RADD_ARG_IPS],
                                    ppcArgV[__NPF_TSHELL_RADD_ARG_IPE], 0, 0);
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add ip rule, error : %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "udp") == 0) {
        INT     iPortS = -1;
        INT     iPortE = -1;

        if (iArgC != 7) {
            fprintf(stderr, "argments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTS], "%i", &iPortS) != 1) {
            fprintf(stderr, "port argment error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTE], "%i", &iPortE) != 1) {
            fprintf(stderr, "port argment error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        
        pvRule = API_INetNpfRuleAdd(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                    LWIP_NPF_RULE_UDP,
                                    LW_NULL,
                                    ppcArgV[__NPF_TSHELL_RADD_ARG_IPS],
                                    ppcArgV[__NPF_TSHELL_RADD_ARG_IPE],
                                    htons((u16_t)iPortS),
                                    htons((u16_t)iPortE));
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add udp rule, error : %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "tcp") == 0) {
        INT     iPortS = -1;
        INT     iPortE = -1;

        if (iArgC != 7) {
            fprintf(stderr, "argments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTS], "%i", &iPortS) != 1) {
            fprintf(stderr, "port argment error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTE], "%i", &iPortE) != 1) {
            fprintf(stderr, "port argment error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        
        pvRule = API_INetNpfRuleAdd(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                    LWIP_NPF_RULE_TCP,
                                    LW_NULL,
                                    ppcArgV[__NPF_TSHELL_RADD_ARG_IPS],
                                    ppcArgV[__NPF_TSHELL_RADD_ARG_IPE],
                                    htons((u16_t)iPortS),
                                    htons((u16_t)iPortE));
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add tcp rule, error : %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else {
        fprintf(stderr, "rule type argment error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    printf("rule add ok\n");
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellNetNpfRuleDel
** ��������: ϵͳ���� "npfruledel"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  __tshellNetNpfRuleDel (INT  iArgC, PCHAR  *ppcArgV)
{
    INT     iError;
    INT     iSeqNum = -1;

    if (iArgC != 3) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    if (sscanf(ppcArgV[2], "%i", &iSeqNum) != 1) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    iError = API_INetNpfRuleDel(ppcArgV[1], LW_NULL, iSeqNum);
    if (iError) {
        if (errno == EINVAL) {
            fprintf(stderr, "argments error!\n");
        } else {
            fprintf(stderr, "can not delete rule, error : %s\n", lib_strerror(errno));
        }
    } else {
        printf("delete.\n");
    }

    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellNetNpfAttach
** ��������: ϵͳ���� "npfattach"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  __tshellNetNpfAttach (INT  iArgC, PCHAR  *ppcArgV)
{
    INT     iError;

    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    iError = API_INetNpfAttach(ppcArgV[1]);
    if (iError < 0) {
        if (errno == EINVAL) {
            fprintf(stderr, "can not find the netif!\n");
        } else if (errno == ERROR_SYSTEM_LOW_MEMORY) {
            fprintf(stderr, "system low memory!\n");
        } else {
            fprintf(stderr, "can not attach netif, error : %s\n", lib_strerror(errno));
        }
    } else {
        printf("attached.\n");
    }

    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellNetNpfDetach
** ��������: ϵͳ���� "npfdetach"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  __tshellNetNpfDetach (INT  iArgC, PCHAR  *ppcArgV)
{
    INT     iError;

    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    iError = API_INetNpfDetach(ppcArgV[1]);
    if (iError < 0) {
        if (errno == EINVAL) {
            fprintf(stderr, "netif error!\n");
        } else {
            fprintf(stderr, "can not detach netif, error : %s\n", lib_strerror(errno));
        }
    } else {
        printf("detached.\n");
    }

    return  (iError);
}
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
/*********************************************************************************************************
  /proc/net/netfilter
*********************************************************************************************************/
#if LW_CFG_PROCFS_EN > 0
/*********************************************************************************************************
  ���� proc �ļ���������
*********************************************************************************************************/
static ssize_t  __procFsNetFilterRead(PLW_PROCFS_NODE  p_pfsn, 
                                      PCHAR            pcBuffer, 
                                      size_t           stMaxBytes,
                                      off_t            oft);
/*********************************************************************************************************
  ���� proc �ļ�����������
*********************************************************************************************************/
static LW_PROCFS_NODE_OP    _G_pfsnoNetFilterFuncs = {
    __procFsNetFilterRead, LW_NULL
};
/*********************************************************************************************************
  ���� proc �ļ��ڵ�
*********************************************************************************************************/
static LW_PROCFS_NODE       _G_pfsnNetFilter[] = 
{
    LW_PROCFS_INIT_NODE("netfilter", 
                        (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH), 
                        &_G_pfsnoNetFilterFuncs, 
                        "F",
                        0),
};
/*********************************************************************************************************
** ��������: __procFsNetFilterPrint
** ��������: ��ӡ���� netfilter �ļ�
** �䡡��  : pcBuffer      ����
**           stTotalSize   ��������С
**           pstOft        ��ǰƫ����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __procFsNetFilterPrint (PCHAR  pcBuffer, size_t  stTotalSize, size_t *pstOft)
{
    INT                    i;
    INT                    iSeqNum;
    CHAR                   cIpBuffer1[INET_ADDRSTRLEN];
    CHAR                   cIpBuffer2[INET_ADDRSTRLEN];

    __PNPF_NETIF_CB        pnpfni;
    PLW_LIST_LINE          plineTempNpfni;
    PLW_LIST_LINE          plineTemp;
    
    for (i = 0; i < __NPF_NETIF_HASH_SIZE; i++) {
        for (plineTempNpfni  = _G_plineNpfHash[i];
             plineTempNpfni != LW_NULL;
             plineTempNpfni  = _list_line_get_next(plineTempNpfni)) {

            pnpfni = _LIST_ENTRY(plineTempNpfni, __NPF_NETIF_CB, NPFNI_lineHash);

            iSeqNum = 0;

            /*
             *  ���� MAC �����
             */
            for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_MAC];
                 plineTemp != LW_NULL;
                 plineTemp  = _list_line_get_next(plineTemp)) {
                __PNPF_RULE_MAC        pnpfrm;

                pnpfrm = _LIST_ENTRY(plineTemp, __NPF_RULE_MAC, NPFRM_lineManage);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%c%c%d   %-6s %6d %-4s NO    "
                         "%02x:%02x:%02x:%02x:%02x:%02x %-15s %-15s %-6s %-6s\n",
                         pnpfni->NPFNI_cName[0], pnpfni->NPFNI_cName[1], pnpfni->NPFNI_ucNum,
                         (pnpfni->NPFNI_uiAttachCounter) ? "YES" : "NO",
                         iSeqNum,
                         "MAC",
                         pnpfrm->NPFRM_ucMac[0],
                         pnpfrm->NPFRM_ucMac[1],
                         pnpfrm->NPFRM_ucMac[2],
                         pnpfrm->NPFRM_ucMac[3],
                         pnpfrm->NPFRM_ucMac[4],
                         pnpfrm->NPFRM_ucMac[5],
                         "N/A", "N/A", "N/A", "N/A");
                iSeqNum++;
            }

            /*
             *  ���� IP �����
             */
            for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_IP];
                 plineTemp != LW_NULL;
                 plineTemp  = _list_line_get_next(plineTemp)) {
                __PNPF_RULE_IP         pnpfri;

                pnpfri = _LIST_ENTRY(plineTemp, __NPF_RULE_IP, NPFRI_lineManage);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%c%c%d   %-6s %6d %-4s NO    %-17s %-15s %-15s %-6s %-6s\n",
                         pnpfni->NPFNI_cName[0], pnpfni->NPFNI_cName[1], pnpfni->NPFNI_ucNum,
                         (pnpfni->NPFNI_uiAttachCounter) ? "YES" : "NO",
                         iSeqNum,
                         "IP", "N/A",
                         ipaddr_ntoa_r(&pnpfri->NPFRI_ipaddrStart, cIpBuffer1, INET_ADDRSTRLEN),
                         ipaddr_ntoa_r(&pnpfri->NPFRI_ipaddrEnd,   cIpBuffer2, INET_ADDRSTRLEN),
                         "N/A", "N/A");

                iSeqNum++;
            }

            /*
             *  ���� UDP �����
             */
            for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_UDP];
                 plineTemp != LW_NULL;
                 plineTemp  = _list_line_get_next(plineTemp)) {
                __PNPF_RULE_UDP        pnpfru;

                pnpfru = _LIST_ENTRY(plineTemp, __NPF_RULE_UDP, NPFRU_lineManage);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%c%c%d   %-6s %6d %-4s NO    %-17s %-15s %-15s %-6d %-6d\n",
                         pnpfni->NPFNI_cName[0], pnpfni->NPFNI_cName[1], pnpfni->NPFNI_ucNum,
                         (pnpfni->NPFNI_uiAttachCounter) ? "YES" : "NO",
                         iSeqNum,
                         "UDP", "N/A",
                         ipaddr_ntoa_r(&pnpfru->NPFRU_ipaddrStart, cIpBuffer1, INET_ADDRSTRLEN),
                         ipaddr_ntoa_r(&pnpfru->NPFRU_ipaddrEnd,   cIpBuffer2, INET_ADDRSTRLEN),
                         ntohs(pnpfru->NPFRU_usPortStart),
                         ntohs(pnpfru->NPFRU_usPortEnd));

                iSeqNum++;
            }

            /*
             *  ���� TCP �����
             */
            for (plineTemp  = pnpfni->NPFNI_npfrnRule[LWIP_NPF_RULE_TCP];
                 plineTemp != LW_NULL;
                 plineTemp  = _list_line_get_next(plineTemp)) {
                __PNPF_RULE_TCP        pnpfrt;

                pnpfrt = _LIST_ENTRY(plineTemp, __NPF_RULE_TCP, NPFRT_lineManage);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%c%c%d   %-6s %6d %-4s NO    %-17s %-15s %-15s %-6d %-6d\n",
                         pnpfni->NPFNI_cName[0], pnpfni->NPFNI_cName[1], pnpfni->NPFNI_ucNum,
                         (pnpfni->NPFNI_uiAttachCounter) ? "YES" : "NO",
                         iSeqNum,
                         "TCP", "N/A",
                         ipaddr_ntoa_r(&pnpfrt->NPFRT_ipaddrStart, cIpBuffer1, INET_ADDRSTRLEN),
                         ipaddr_ntoa_r(&pnpfrt->NPFRT_ipaddrEnd,   cIpBuffer2, INET_ADDRSTRLEN),
                         ntohs(pnpfrt->NPFRT_usPortStart),
                         ntohs(pnpfrt->NPFRT_usPortEnd));

                iSeqNum++;
            }
        }
    }
    
    *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                       "\ndrop:%d  allow:%d\n", 
                       __NPF_PACKET_DROP_GET(), __NPF_PACKET_ALLOW_GET());
}
/*********************************************************************************************************
** ��������: __procFsNetFilterRead
** ��������: procfs ��һ����ȡ���� netfilter �ļ�
** �䡡��  : p_pfsn        �ڵ���ƿ�
**           pcBuffer      ������
**           stMaxBytes    ��������С
**           oft           �ļ�ָ��
** �䡡��  : ʵ�ʶ�ȡ����Ŀ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static ssize_t  __procFsNetFilterRead (PLW_PROCFS_NODE  p_pfsn, 
                                       PCHAR            pcBuffer, 
                                       size_t           stMaxBytes,
                                       off_t            oft)
{
    const CHAR      cFilterInfoHdr[] = 
    "NETIF ATTACH SEQNUM RULE ALLOW MAC               IPs             IPe             PORTs  PORTe\n";
    
          PCHAR     pcFileBuffer;
          size_t    stRealSize;                                         /*  ʵ�ʵ��ļ����ݴ�С          */
          size_t    stCopeBytes;
    
    /*
     *  ����Ԥ���ڴ��СΪ 0 , ���Դ򿪺��һ�ζ�ȡ��Ҫ�ֶ������ڴ�.
     */
    pcFileBuffer = (PCHAR)API_ProcFsNodeBuffer(p_pfsn);
    if (pcFileBuffer == LW_NULL) {                                      /*  ��û�з����ڴ�              */
        size_t  stNeedBufferSize = 0;
        
        __NPF_LOCK();                                                   /*  ���� NPF ��                 */
        stNeedBufferSize = (size_t)(_G_ulNpfCounter * 128);
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        
        stNeedBufferSize += sizeof(cFilterInfoHdr) + 64;
        
        if (API_ProcFsAllocNodeBuffer(p_pfsn, stNeedBufferSize)) {
            _ErrorHandle(ENOMEM);
            return  (0);
        }
        pcFileBuffer = (PCHAR)API_ProcFsNodeBuffer(p_pfsn);             /*  ���»���ļ���������ַ      */
        
        stRealSize = bnprintf(pcFileBuffer, stNeedBufferSize, 0, cFilterInfoHdr);
                                                                        /*  ��ӡͷ��Ϣ                  */
                                                                        
        __NPF_LOCK();                                                   /*  ���� NPF ��                 */
        __procFsNetFilterPrint(pcFileBuffer, stNeedBufferSize, &stRealSize);
        __NPF_UNLOCK();                                                 /*  ���� NPF ��                 */
        
        API_ProcFsNodeSetRealFileSize(p_pfsn, stRealSize);
    } else {
        stRealSize = API_ProcFsNodeGetRealFileSize(p_pfsn);
    }
    if (oft >= stRealSize) {
        _ErrorHandle(ENOSPC);
        return  (0);
    }
    
    stCopeBytes  = __MIN(stMaxBytes, (size_t)(stRealSize - oft));       /*  ����ʵ�ʿ������ֽ���        */
    lib_memcpy(pcBuffer, (CPVOID)(pcFileBuffer + oft), (UINT)stCopeBytes);
    
    return  ((ssize_t)stCopeBytes);
}
/*********************************************************************************************************
** ��������: __procFsNpfInit
** ��������: procfs ��ʼ������ netfilter �ļ�
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __procFsNpfInit (VOID)
{
    API_ProcFsMakeNode(&_G_pfsnNetFilter[0],  "/net");
}
#endif                                                                  /*  LW_CFG_PROCFS_EN > 0        */

#endif                                                                  /*  LW_CFG_NET_EN > 0           */
                                                                        /*  LW_CFG_NET_NPF_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/