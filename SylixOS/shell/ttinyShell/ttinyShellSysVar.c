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
** ��   ��   ��: ttinyShellSysCmd.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 09 �� 05 ��
**
** ��        ��: һ����С�͵� shell ϵͳ, ϵͳ������������.

** BUG:
2009.12.11  ���� PATH ����, �Թ�Ӧ�ó����ȡ����·��.
2010.05.05  PATH ��ʼ��Ϊ /bin Ŀ¼(�� : ��Ϊ����������ֵͬ��ķָ���).
2011.03.05  ���ı�����
2011.03.10  ���� syslog socket ��������.
2011.06.10  ���� TZ ��������.
2011.07.08  ���� CALIBRATE ����.
2012.03.21  ���� NFS_CLIENT_AUTH ����, ָ�� NFS ʹ�� AUTH_NONE(windows) ���� AUTH_UNIX
2012.09.21  ����� locale ���������ĳ�ʼ��.
2013.01.23  ����� NFS_CLIENT_PROTO ����������ʼ��.
2013.06.12  SylixOS Ĭ�ϲ���ʹ�� ftk ͼ�ν���, ת��ʹ�� Qt ͼ�ν���.
2015.04.06  ȥ�� GUILIB GUIFONT ... Ĭ�ϻ�������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/shell/include/ttiny_shell.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0
#include "paths.h"
/*********************************************************************************************************
** ��������: __tshellSysVarInit
** ��������: ��ʼ��ϵͳ��������
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __tshellSysVarInit (VOID)
{
    /*
     *  ϵͳ��Ϣ
     */
    API_TShellExec("SYSTEM=\"" __SYLIXOS_VERINFO "\"");
    API_TShellExec("VERSION=\"" __SYLIXOS_VERSTR "\"");
    API_TShellExec("LICENSE=\"" __SYLIXOS_LICENSE "\"");

    API_TShellExec("TMPDIR=" LW_CFG_TMP_PATH);
    API_TShellExec("TZ=CST-8:00:00");                                   /*  Ĭ��Ϊ��8��                 */
    
    /*
     *  ͼ�ν���
     */
    API_TShellExec("KEYBOARD=/dev/input/keyboard0");                    /*  HID �豸                    */
    API_TShellExec("MOUSE=/dev/input/mouse0:/dev/input/touch0");
    
    API_TShellExec("TSLIB_TSDEVICE=/dev/input/touch0");                 /*  ������У׼�����豸          */
    API_TShellExec("TSLIB_CALIBFILE=/etc/pointercal");                  /*  ������У׼�ļ�              */
    
    /*
     *  �����װ�ص�Ӧ�ó���ģ��ʹ���� SylixOS ģ���ʼ����, 
     *  ��ģ����Զ�̬������ڴ��С���»�������ȷ��.
     */
    API_TShellExec("SO_MEM_PAGES=8192");                                /*  ��̬�ڴ�����ҳ������        */
                                                                        /*  Ĭ��Ϊ 32 MB                */
#if LW_CFG_FIO_FLOATING_POINT_EN > 0
    API_TShellExec("FIO_FLOAT=1");                                      /*  fio ֧�ָ���                */
#else
    API_TShellExec("FIO_FLOAT=0");                                      /*  fio ��֧�ָ���              */
#endif                                                                  /*  LW_CFG_FIO_FLOATING_POINT_EN*/

#if LW_CFG_POSIX_EN > 0
    API_TShellExec("SYSLOGD_HOST=0.0.0.0:514");                         /*  syslog ��������ַ           */
#endif

    API_TShellExec("NFS_CLIENT_AUTH=AUTH_UNIX");                        /*  NFS Ĭ��ʹ�� auth_unix      */
    API_TShellExec("NFS_CLIENT_PROTO=udp");                             /*  NFS Ĭ��ʹ�� udp Э��       */

    API_TShellExec("PATH=" _PATH_DEFPATH);                              /*  PATH ����ʱĬ��·��         */
    API_TShellExec("LD_LIBRARY_PATH=" _PATH_LIBPATH);                   /*  LD_LIBRARY_PATH Ĭ��ֵ      */
    
    /*
     *  ������������
     */
    API_TShellExec("LANG=C");                                           /*  ϵͳĬ�� locale Ϊ "C"      */
    API_TShellExec("LC_ALL=");                                          /*  �Ƽ���Ҫʹ�ô˱���          */
    API_TShellExec("PATH_LOCALE=" _PATH_LOCALE);                        /*  ע��:��Ҫ�� BSD ϵͳ�� UTF-8*/
                                                                        /*  Ŀ¼����������              */
    
    /*
     *  �ն�
     */                                                                    
    API_TShellExec("TERM=vt100");
    API_TShellExec("TERMCAP=/etc/termcap");                             /*  BSD �ն�ת��                */
}
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/