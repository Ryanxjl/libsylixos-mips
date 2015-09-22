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
** ��   ��   ��: lwip_ifparam.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2015 �� 09 �� 20 ��
**
** ��        ��: ����ӿ����ò�����ȡ.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0
#include "iniparser/iniparser.h"
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
/*********************************************************************************************************
  ��������ļ���ʽ���� /etc/ifparam.ini

  [en1]
  enable=1
  ipaddr=192.168.1.2
  netmask=255.255.255.0
  gateway=192.168.1.1
  default=1
  mac=00:11:22:33:44:55

  resolver ��������ļ����� /etc/resolv.conf

  nameserver x.x.x.x
*********************************************************************************************************/
/*********************************************************************************************************
  �����ļ�λ��
*********************************************************************************************************/
#define LW_IFPARAM_PATH     "/etc/ifparam.ini"
#define LW_RESCONF_PATH     "/etc/resolv.conf"
#define LW_IFPARAM_ENABLE   "enable"
#define LW_IFPARAM_IPADDR   "ipaddr"
#define LW_IFPARAM_MASK     "netmask"
#define LW_IFPARAM_GW       "gateway"
#define LW_IFPARAM_MAC      "mac"
#define LW_IFPARAM_DEFAULT  "default"
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
#define LW_IFPARAM_MAX_NAME 32

typedef struct {
    dictionary      *IFP_pdict;
    CHAR             IFP_pcName[LW_IFPARAM_MAX_NAME];
} LW_IFPARAM;
typedef LW_IFPARAM  *PLW_IFPARAM;
/*********************************************************************************************************
** ��������: if_param_load
** ��������: װ��ָ������ӿ�����
** �䡡��  : name          ��������
** �䡡��  : ���þ��
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
void  *if_param_load (const char *name)
{
    PLW_IFPARAM     pifp;

    if (!name) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }

    if (lib_strlen(name) >= LW_IFPARAM_MAX_NAME) {
        _ErrorHandle(ENAMETOOLONG);
        return  (LW_NULL);
    }

    pifp = (PLW_IFPARAM)__SHEAP_ALLOC(sizeof(LW_IFPARAM));
    if (!pifp) {
        _ErrorHandle(ENOMEM);
        return  (LW_NULL);
    }

    pifp->IFP_pdict = iniparser_load(LW_IFPARAM_PATH);
    if (!pifp->IFP_pdict) {
        __SHEAP_FREE(pifp);
        return  (LW_NULL);
    }

    lib_strcpy(pifp->IFP_pcName, name);

    return  ((void *)pifp);
}
/*********************************************************************************************************
** ��������: if_param_unload
** ��������: ж��ָ������ӿ�����
** �䡡��  : pifparam      ���þ��
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
void  if_param_unload (void *pifparam)
{
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict) {
        return;
    }

    iniparser_freedict(pifp->IFP_pdict);
    __SHEAP_FREE(pifp);
}
/*********************************************************************************************************
** ��������: if_param_getenable
** ��������: ��ȡ�����Ƿ�ʹ������. (���δ�ҵ�����Ĭ��Ϊʹ��)
** �䡡��  : pifparam      ���þ��
**           enable        �Ƿ�ʹ��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getenable (void *pifparam, int *enable)
{
    char            key[128];
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict || !enable) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    snprintf(key, sizeof(key), "%s:" LW_IFPARAM_ENABLE, pifp->IFP_pcName);

    *enable = iniparser_getint(pifp->IFP_pdict, key, 1);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_getdefault
** ��������: ��ȡ�����Ƿ�ΪĬ��·������. (���δ�ҵ�����Ĭ��Ϊʹ��)
** �䡡��  : pifparam      ���þ��
**           def           �Ƿ�ΪĬ��·��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getdefault (void *pifparam, int *def)
{
    char            key[128];
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict || !def) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    snprintf(key, sizeof(key), "%s:" LW_IFPARAM_DEFAULT, pifp->IFP_pcName);

    *def = iniparser_getint(pifp->IFP_pdict, key, 1);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_getipaddr
** ��������: ��ȡ IP ��ַ����.
** �䡡��  : pifparam      ���þ��
**           ipaddr        IP ��ַ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getipaddr (void *pifparam, ip_addr_t *ipaddr)
{
    char            key[128];
    const char     *value;
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict || !ipaddr) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    snprintf(key, sizeof(key), "%s:" LW_IFPARAM_IPADDR, pifp->IFP_pcName);

    value = iniparser_getstring(pifp->IFP_pdict, key, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (ipaddr_aton(value, ipaddr)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getnetmask
** ��������: ��ȡ������������.
** �䡡��  : pifparam      ���þ��
**           mask          ��������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getnetmask (void *pifparam, ip_addr_t *mask)
{
    char            key[128];
    const char     *value;
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict || !mask) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    snprintf(key, sizeof(key), "%s:" LW_IFPARAM_MASK, pifp->IFP_pcName);

    value = iniparser_getstring(pifp->IFP_pdict, key, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (ipaddr_aton(value, mask)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getgw
** ��������: ��ȡ������������.
** �䡡��  : pifparam      ���þ��
**           gw            ���ص�ַ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getgw (void *pifparam, ip_addr_t *gw)
{
    char            key[128];
    const char     *value;
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict || !gw) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    snprintf(key, sizeof(key), "%s:" LW_IFPARAM_GW, pifp->IFP_pcName);

    value = iniparser_getstring(pifp->IFP_pdict, key, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (ipaddr_aton(value, gw)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getmac
** ��������: ��ȡ MAC ����.
** �䡡��  : pifparam      ���þ��
**           mac           MAC ��ַ�ַ���
**           sz            ��������С
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getmac (void *pifparam, char *mac, size_t  sz)
{
    char            key[128];
    const char     *value;
    PLW_IFPARAM     pifp = (PLW_IFPARAM)pifparam;

    if (!pifp || !pifp->IFP_pdict) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    snprintf(key, sizeof(key), "%s:" LW_IFPARAM_MAC, pifp->IFP_pcName);

    value = iniparser_getstring(pifp->IFP_pdict, key, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    lib_strlcpy(mac, value, sz);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_syncdns
** ��������: ͬ�� DNS ����.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
void  if_param_syncdns (void)
{
#define MATCH(line, name) \
    (!lib_strncmp(line, name, sizeof(name) - 1) && \
    (line[sizeof(name) - 1] == ' ' || \
     line[sizeof(name) - 1] == '\t'))

    FILE  *fp = fopen(LW_RESCONF_PATH, "r");
    char   buf[128];
    char  *cp;
    u8_t   numdns = 0;

    if (!fp) {
        return;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        if (*buf == ';' || *buf == '#') {
            continue;
        }
        if (MATCH(buf, "nameserver")) {
            cp = buf + sizeof("nameserver") - 1;
            while (*cp == ' ' || *cp == '\t'){
                cp++;
            }

            cp[lib_strcspn(cp, ";# \t\n")] = '\0';
            if ((*cp != '\0') && (*cp != '\n')) {
                ip_addr_t   addr;
                if (ipaddr_aton(cp, &addr)) {
                    if (numdns < DNS_MAX_SERVERS) {
                        dns_setserver(numdns, &addr);
                        numdns++;
                    }
                }
            }
        }
    }

    fclose(fp);
}
#endif                                                                  /*  LW_CFG_NET_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/

