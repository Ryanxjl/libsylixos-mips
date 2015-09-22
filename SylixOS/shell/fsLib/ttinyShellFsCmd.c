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
** ��   ��   ��: ttinyShellFsCmd.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 07 �� 27 ��
**
** ��        ��: ϵͳ�ڲ������ (�ļ�ϵͳ���).

** BUG
2008.11.11  __tshellFsCmdCp() ������Ŀ¼�ļ���ΪԴ�ļ�.
2008.12.04  mv �ƶ��ļ�ʱ, ����ͬһ����, ��Ҫ���и���, ɾ������.
2009.02.25  touch ���������ļ���СΪ��.
2009.02.21  �޸��� ls �� attrib ��Էǵ�ǰĿ¼�Ĵ���.
2009.03.10  cp �ļ�ʱ��Ҫͬ���ļ�ʱ��.
2009.03.11  ls �� attrib ����Ķ��ļ��Ĵ�С��ӡ����.
2009.03.14  ����� df ָ���֧��.
2009.03.21  �� attrib �����Ϊ ll.
            cp ����Ļ����������ļ���С����Ӧ.
2009.05.19  mv Ҫ��ֻд��ʽ���ļ�.
2009.06.30  cp ������Ҫ�� O_TRUNC ���ļ�.
2009.07.03  ������ GCC ����ʱ�ľ���.
2009.07.04  ���� shfile ָ��, ����ִ��ָ���� shell ����ļ�.
2009.07.08  mv ����ʱ��Ŀ�����ʱ, ��Ҫ��ʾ.
2009.07.28  �� mv ʧ��ʱ, ֱ�ӽ��� cp �� rm ���ϲ���.
2009.08.27  ls �� ll ������ stat() ����ʱ, ʹ��Ĭ��������ʾ.
2009.09.01  ������ ls ll ����Բ���Ŀ¼��β���� / �ַ�ʱ���ֵĴ���.
2009.10.02  �޷���ȡ stat ���ļ�Ĭ��Ϊ�ַ������ļ�.
2009.11.18  ���� df ��ʾ.
2009.12.14  __tshellFsCmdLl() ֧�ִ�ӡ�޸�ʱ��.
            �� ls ����Ĵ�ӡ����.
            ���� mount ����.
2009.12.15  ���� symbol link �ļ�����.
            ���������ļ��ɹ��жϵ�һ�� bug.
2009.12.29  ll �����ļ����Ե�һ���ַ���ʾ�����꾡.
2010.09.10  ls ��� d_type != DT_UNKNOWN ʱ, ����Ҫ���� stat() ����.
2011.03.04  cat ����֧���м���.
2011.03.22  ls ���������ļ���ɫ����.
2011.03.27  ���� mkfifo ����.
2011.05.16  __tshellFsCmdMv() ʹ�� rename() �滻 ioctl(... FIOMOVE ...) ��ʽ. 
2011.06.03  ���� varload �� varsave ����.
2011.06.23  ʹ���µ�ɫ��ת��.
2011.08.07  __tshellFsCmdLl() ֧����ʾ�����ļ�Ŀ��Ĺ���.
2011.11.17  cat ����ܴ�ӡ�� reg �ļ�.
2012.03.25  cd �� ch ������Ҫ�� stat �ж�.
2012.10.20  ����һ�� tmpname ����.
2012.10.25  cd ����ʹ�� chdir() ʵ��.
2013.01.22  ll ������ļ������ߵ���ʾ, chmod �����ͨ�����֧��.
2013.04.01  ���� GCC 4.7.3 �������� warning.
2013.06.24  ����� vfat �����֧��.
2014.05.30  ����鿴�ļ��д�С������.
2014.10.10  cp ���Ŀ���ļ�����Ϊ��ԭʼ�ļ���ͬ�� mode.
2014.10.29  ls ��ʾ�ĵ��п���ͨ�� TIOCGWINSZ ��ȡ.
2014.12.08  ln ֧�� -f ѡ��.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/shell/include/ttiny_shell.h"
#include "sys/ioctl.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0 && LW_CFG_MAX_VOLUMES > 0
#include "../SylixOS/shell/ttinyShell/ttinyShell.h"
#include "../SylixOS/shell/ttinyShell/ttinyShellLib.h"
#include "../SylixOS/shell/ttinyShell/ttinyShellSysCmd.h"
#include "../SylixOS/shell/ttinyVar/ttinyVarLib.h"
#include "../SylixOS/fs/include/fs_fs.h"
#if LW_CFG_POSIX_EN > 0
#include "fnmatch.h"
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static INT      __tshellFsCmdCp(INT  iArgC, PCHAR  ppcArgV[]);
LW_API time_t   API_RootFsTime(time_t  *time);
/*********************************************************************************************************
** ��������: __tshellFsCmdCd
** ��������: ϵͳ���� "cd"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdCd (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;

    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = cd(ppcArgV[1]);
    if (iError) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else if (errno == ENOTDIR) {
            fprintf(stderr, "not a directory!\n");
        } else {
            fprintf(stderr, "cd : error : %s\n", lib_strerror(errno));
        }
        return  (iError);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdCh
** ��������: ϵͳ���� "ch"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdCh (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;

    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = chdir(ppcArgV[1]);
    if (iError) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else if (errno == ENOTDIR) {
            fprintf(stderr, "not a directory!\n");
        } else {
            fprintf(stderr, "ch : error : %s\n", lib_strerror(errno));
        }
        return  (iError);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdPwd
** ��������: ϵͳ���� "pwd"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdPwd (INT  iArgC, PCHAR  ppcArgV[])
{
    CHAR   cFileName[MAX_FILENAME_LENGTH];
    
    ioDefPathGet(cFileName);
    lib_strcat(cFileName, "\n");
    printf(cFileName);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdTmpname
** ��������: ϵͳ���� "tmpname"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdTmpname (INT  iArgC, PCHAR  ppcArgV[])
{
    char cBuf[L_tmpnam];
    
    printf("can mktmp as name : %s\n", tmpnam(cBuf));
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdMkdir
** ��������: ϵͳ���� "mkdir"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdMkdir (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;
    
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = mkdir(ppcArgV[1], DEFAULT_DIR_PERM);
    if (iError) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "can not make this directory, error : %s\n", lib_strerror(errno));
        }
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdMkfifo
** ��������: ϵͳ���� "mkfifo"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdMkfifo (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;
    
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = mkfifo(ppcArgV[1], 0);
    if (iError) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "can not make this fifo, error : %s\n", lib_strerror(errno));
        }
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdRmdir
** ��������: ϵͳ���� "rmdir"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdRmdir (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;
    
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = rmdir(ppcArgV[1]);
    if (iError) {
        if (API_GetLastError() == ENOENT) {
            fprintf(stderr, "directory is not exist!\n");
        } else {
            fprintf(stderr, "can not remove directory, error : %s\n", lib_strerror(errno));
        }
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdRm
** ��������: ϵͳ���� "rm"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdRm (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;
    
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = unlink(ppcArgV[1]);
    if (iError) {
        if (API_GetLastError() == ENOENT) {
            fprintf(stderr, "file is not exist!\n");
        } else {
            fprintf(stderr, "can not remove this file, error : %s\n", lib_strerror(errno));
        }
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdMv
** ��������: ϵͳ���� "mv"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdMv (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError = PX_ERROR;
    REGISTER INT    iFd;
             CHAR   cTemp[16];
    
    if (iArgC != 3) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iFd = open(ppcArgV[1], O_RDONLY, 0);
    if (iFd < 0) {
        fprintf(stderr, "%s error : %s\n", ppcArgV[1], lib_strerror(errno));
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = access(ppcArgV[2], 0);                                     /*  ���Ŀ���ļ��Ƿ����        */
    if (iError != PX_ERROR) {
__re_select:
        printf("destination file is exist, overwrite? (Y/N)\n");
        read(0, cTemp, 16);
        if ((cTemp[0] == 'N') ||
            (cTemp[0] == 'n')) {                                        /*  ������                      */
            goto    __error_handle;
        } else if ((cTemp[0] == 'Y') ||
                   (cTemp[0] == 'y')) {                                 /*  ����                        */
            if (unlink(ppcArgV[2]) != ERROR_NONE) {
                goto    __error_handle;
            }
        } else {
            goto    __re_select;                                        /*  ѡ�����                    */
        }
    }
    close(iFd);                                                         /*  �ر�Դ�ļ�                  */
    
    iError = rename(ppcArgV[1], ppcArgV[2]);
    if (iError < 0) {
        iError = __tshellFsCmdCp(3, ppcArgV);                           /*  ��Ҫ���п���ɾ��            */
        if (iError >= 0) {
            __tshellFsCmdRm(2, ppcArgV);                                /*  ɾ��Դ�ļ�                  */
        }
    }
    return  (iError);
    
__error_handle:
    close(iFd);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdCat
** ��������: ϵͳ���� "cat"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdCat (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT            iError;
    REGISTER ssize_t        sstNum;
    REGISTER INT            iFd;
             CHAR           cBuffer[MAX_FILENAME_LENGTH];
             struct stat    statFile;
             
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iFd = open(ppcArgV[1], O_RDONLY, 0);
    if (iFd < 0) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "file error %s!\n", lib_strerror(errno));
        }
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    iError = fstat(iFd, &statFile);
    if (iError < 0) {
        fprintf(stderr, "file stat error!\n");
        close(iFd);
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    if (S_ISDIR(statFile.st_mode)) {
        fprintf(stderr, "file read error: Is a directory!\n");
        close(iFd);
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    API_ThreadCleanupPush((VOIDFUNCPTR)close, (PVOID)iFd);
    
    do {
        sstNum = read(iFd, cBuffer, MAX_FILENAME_LENGTH);
        if (sstNum > 0) {
            write(1, cBuffer, (size_t)sstNum);
        }
    } while (sstNum > 0);
    
    API_ThreadCleanupPop(LW_TRUE);
    
    printf("\n");
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __checkIsTime
** ��������: ����ִ��Ƿ�Ϊʱ��
** �䡡��  : pcStr         �ַ���
**           time          ʱ��
** �䡡��  : 0  ��ʾת����ȷ
**           -1 ��ʾ����
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __checkIsTime (PCHAR  pcStr, time_t  *time)
{
    /*
     *  ����Ŀǰ������ʱ��
     */
    if (lib_strlen(pcStr) == 10) {
    }
    
    if (lib_strlen(pcStr) == 8) {
    }
    
    return  (-1);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdTouch
** ��������: ϵͳ���� "touch"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdTouch (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT        iAflag = 0;
    REGISTER INT        iMflag = 0;
    REGISTER INT        iCflag = 0;
    
    REGISTER INT        i;
    REGISTER INT        iFileNameIndex = -1;
    
    REGISTER INT        iOc;
             time_t     timeNew;
             
    REGISTER INT        iFd;
    
    while ((iOc = getopt(iArgC, ppcArgV, "amc")) != -1) {
        switch (iOc) {
        case 'a':
            iAflag = 1;                                                 /*  ֻ�ı����ʱ��              */
            break;
        case 'm':
            iMflag = 1;                                                 /*  ֻ�ı��޸�ʱ��              */
            break;
        case 'c':
            iCflag = 1;                                                 /*  ���ļ������ڣ���������      */
            break;
        }
    }
    getopt_free();
    
    for (i = 1; i < iArgC; i++) {
        if (*ppcArgV[i] != '-') {
            if (__checkIsTime(ppcArgV[i], &timeNew)) {
                iFileNameIndex = i;
            }
        }
    }
    
    if (iFileNameIndex == -1) {
        fprintf(stderr, "file error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    (VOID)iAflag;                                                       /*  ��ʱ��ʹ��                  */
    (VOID)iMflag;
    
    if (iCflag == 0) {
        iFd = open(ppcArgV[iFileNameIndex], O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_FILE_PERM);
        if (iFd < 0) {
            if (errno == EACCES) {
                fprintf(stderr, "insufficient permissions.\n");
            } else {
                fprintf(stderr, "can not create file! error : %s\n", lib_strerror(errno));
            }
            return  (-1);
        }
        close(iFd);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __fillWhite
** ��������: �������ɸ��ո�
** �䡡��  : stLen      �ո�ĳ���
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __fillWhite (size_t  stLen)
{
    CHAR    cFmt[16];
    
    sprintf(cFmt, "%%-%zds", stLen);
    printf(cFmt, "");                                                   /*  ����ո�                    */
}
/*********************************************************************************************************
** ��������: __tshellFsCmdLs
** ��������: ϵͳ���� "ls"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdLs (INT  iArgC, PCHAR  ppcArgV[])
{
#define __TSHELL_BYTES_PERLINE          80                              /*  ���� 80 �ַ�                */
#define __TSHELL_BYTES_PERFILE          16                              /*  ÿ���ļ�����ʾ�񳤶�        */

             CHAR            cDirName[MAX_FILENAME_LENGTH] = ".";
             size_t          stDirLen = 1;
    
    REGISTER DIR            *pdir;
    REGISTER struct dirent  *pdirent;
             struct stat     statGet;
             struct winsize  winsz;
             
             INT             iError;
             
             size_t          stTotalLen = 0;
             size_t          stPrintLen;
             size_t          stPad;
             
    if ((iArgC > 1) && (lib_strcmp(ppcArgV[1], "-l") == 0)) {
        printf("you can use 'll' command.\n");
        return  (ERROR_NONE);
    }
    
    if ((iArgC == 2) && lib_strcmp(ppcArgV[1], ".")) {                  /*  ָ��Ŀ¼                    */
        lib_strcpy(cDirName, ppcArgV[1]);
        stDirLen = lib_strlen(cDirName);
        pdir     = opendir(cDirName);
        if (stDirLen > 0) {
            if (cDirName[stDirLen - 1] != PX_DIVIDER) {                 /*  ����Ŀ¼������ / ��β       */
                cDirName[stDirLen++] = PX_DIVIDER;                      /*  ����һ�� /                  */
            }
        }
    } else {
        pdir = opendir(cDirName);                                       /*  ��ǰĿ¼                    */
    }
    
    if (!pdir) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "can not open dir! %s\n", lib_strerror(errno));
        }
        return  (-1);
    }
    
    if (ioctl(STD_OUT, TIOCGWINSZ, &winsz)) {                           /*  ��ô�����Ϣ                */
        winsz.ws_col = __TSHELL_BYTES_PERLINE;
    } else {
        winsz.ws_col = (unsigned short)ROUND_DOWN(winsz.ws_col, __TSHELL_BYTES_PERFILE);
    }
    
    do {
        pdirent = readdir(pdir);
        if (!pdirent) {
            break;
        } else {
            
            if ((pdirent->d_type == DT_UNKNOWN) ||
                (pdirent->d_type == DT_REG)) {
                if ((stDirLen > 1) || 
                    ((stDirLen == 1) && (cDirName[0] == PX_ROOT))) {
                    lib_strcpy(&cDirName[stDirLen], pdirent->d_name);   /*  ����ָ��Ŀ¼                */
                    iError = stat(cDirName, &statGet);
                } else {
                    iError = stat(pdirent->d_name, &statGet);           /*  ʹ�õ�ǰĿ¼                */
                }
                if (iError < 0) {                                       /*  �豸�ļ�Ĭ��ʹ�����������  */
                    statGet.st_mode = S_IRUSR | S_IFREG;                /*  Ĭ������                    */
                }
            } else {
                statGet.st_mode = DTTOIF(pdirent->d_type);
            }
            
            API_TShellColorStart(pdirent->d_name, "", statGet.st_mode, STD_OUT);
            stPrintLen = printf("%-15s ", pdirent->d_name);             /*  ��ӡ�ļ���                  */
            if (stPrintLen > __TSHELL_BYTES_PERFILE) {
                stPad = ROUND_UP(stPrintLen, __TSHELL_BYTES_PERFILE)
                      - stPrintLen;                                     /*  �����������                */
                __fillWhite(stPad);
            } else {
                stPad = 0;
            }
            stTotalLen += stPrintLen + stPad;
            API_TShellColorEnd(STD_OUT);
            
            if (stTotalLen >= winsz.ws_col) {
                printf("\n");                                           /*  ����                        */
                stTotalLen = 0;
            }
        }
    } while (1);
    
    if (stTotalLen) {
        printf("\n");                                                   /*  ��������                    */
    }
    
    closedir(pdir);                                                     /*  �ر��ļ���                  */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdCmp
** ��������: ϵͳ���� "cmp"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0:  �ļ���ͬ
**           1:  �ļ���ͬ
**           -1: ���ִ���
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdCmp (INT  iArgC, PCHAR  ppcArgV[])
{
             INT        iError = ERROR_NONE;
    REGISTER INT        iFdSrc;
    REGISTER INT        iFdDst = PX_ERROR;
    
    REGISTER ssize_t    sstRdSrcNum;
    REGISTER ssize_t    sstRdDstNum;
            
             CHAR       cTempSrc[128];
             CHAR       cTempDst[128];
             
             PCHAR      pcBufferSrc = LW_NULL;
             PCHAR      pcBufferDst = LW_NULL;
             
             size_t     stBuffer;
             
    struct   stat       statFile;
             off_t      oftSize;

    if (iArgC != 3) {
        fprintf(stderr, "param error!\n");
        return  (-1);
    }
    if (lib_strcmp(ppcArgV[1], ppcArgV[2]) == 0) {                      /*  �ļ���ͬ                    */
        printf("file same!\n");
        return  (0);
    }
    
    iFdSrc = open(ppcArgV[1], O_RDONLY);
    if (iFdSrc < 0) {
        fprintf(stderr, "can not open %s!\n", ppcArgV[1]);
        return  (-1);
    }
    iFdDst = open(ppcArgV[2], O_RDONLY);
    if (iFdDst < 0) {
        close(iFdSrc);
        fprintf(stderr, "can not open %s!\n", ppcArgV[2]);
        return  (-1);
    }
    
    iError = fstat(iFdSrc, &statFile);                                  /*  ���ȱȽ��ļ���С            */
    if (iError != ERROR_NONE) {
        fprintf(stderr, "%s get stat error!\n", ppcArgV[1]);
        goto    __error_handle;
    }
    oftSize = statFile.st_size;
    iError = fstat(iFdDst, &statFile);
    if (iError != ERROR_NONE) {
        fprintf(stderr, "%s get stat error!\n", ppcArgV[2]);
        goto    __error_handle;
    }
    if (oftSize != statFile.st_size) {                                  /*  �ļ���С��ͬ                */
        printf("file not same!\n");
        goto    __error_handle;
    }
    
    pcBufferSrc = (PCHAR)__SHEAP_ALLOC((16 * LW_CFG_KB_SIZE));          /*  ���ٱȽ��ڴ�                */
    pcBufferDst = (PCHAR)__SHEAP_ALLOC((16 * LW_CFG_KB_SIZE));
    if (!pcBufferSrc || !pcBufferDst) {
        if (pcBufferSrc) {
            __SHEAP_FREE(pcBufferSrc);                                  /*  ��������ͬʱ����ɹ�        */
        }
        if (pcBufferDst) {
            __SHEAP_FREE(pcBufferDst);
        }
        pcBufferSrc = cTempSrc;
        pcBufferDst = cTempDst;
        stBuffer    = sizeof(cTempSrc);
    } else {
        stBuffer    = (16 * LW_CFG_KB_SIZE);
    }
    
    for (;;) {                                                          /*  ��ʼ�Ƚ��ļ�                */
        sstRdSrcNum = read(iFdSrc, pcBufferSrc, stBuffer);
        sstRdDstNum = read(iFdDst, pcBufferDst, stBuffer);
        if (sstRdSrcNum != sstRdDstNum) {
            fprintf(stderr, "file read error!\n");
            goto    __error_handle;
        }
        if (sstRdSrcNum <= 0) {                                         /*  �ļ���ȡ���                */
            break;
        }
        if (lib_memcmp(pcBufferSrc, pcBufferDst, (UINT)sstRdSrcNum) != 0) {
            printf("file not same!\n");
            goto    __error_handle;
        }
    }
    close(iFdSrc);
    close(iFdDst);                                                      /*  �ر��ļ�                    */
    if (stBuffer == (16 * LW_CFG_KB_SIZE)) {
        __SHEAP_FREE(pcBufferSrc);
        __SHEAP_FREE(pcBufferDst);                                      /*  �ͷŻ�����                  */
    }
    printf("file same!\n");
    
    return  (ERROR_NONE);
    
__error_handle:
    close(iFdSrc);
    close(iFdDst);
    
    if (pcBufferSrc && (pcBufferSrc != cTempSrc)) {
        __SHEAP_FREE(pcBufferSrc);
    }
    if (pcBufferDst && (pcBufferDst != cTempDst)) {
        __SHEAP_FREE(pcBufferDst);
    }
    
    return  (-1);
}
/*********************************************************************************************************
** ��������: __buildDstFileName
** ��������: ����Ŀ���ļ���
** �䡡��  : pcSrc         Դ�ļ�
**           pcDstDir      Ŀ��Ŀ¼
**           pcBuffer      �������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID __buildDstFileName (CPCHAR  pcSrc, CPCHAR  pcDstDir, PCHAR  pcBuffer)
{
    size_t  stDstDirLen   = lib_strlen(pcDstDir);
    PCHAR   pcSrcFileName = lib_rindex(pcSrc, PX_DIVIDER);

    if (pcSrcFileName == LW_NULL) {
        pcSrcFileName =  (PCHAR)pcSrc;
    } else {
        pcSrcFileName++;
    }

    lib_strlcpy(pcBuffer, pcDstDir, MAX_FILENAME_LENGTH - 1);

    if (pcBuffer[stDstDirLen - 1] != PX_DIVIDER) {
        pcBuffer[stDstDirLen]      = PX_DIVIDER;
        pcBuffer[stDstDirLen + 1]  = PX_EOS;
    }

    lib_strlcat(pcBuffer, pcSrcFileName, MAX_FILENAME_LENGTH);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdCp
** ��������: ϵͳ���� "cp"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdCp (INT  iArgC, PCHAR  ppcArgV[])
{
             INT        iError = PX_ERROR;
    REGISTER INT        iFdSrc;
    REGISTER INT        iFdDst = PX_ERROR;
    
             CHAR       cDstFile[MAX_FILENAME_LENGTH] = "\0";

    REGISTER ssize_t    sstRdNum;
    REGISTER ssize_t    sstWrNum;
            
             CHAR       cTemp[128];
             PCHAR      pcBuffer;
             size_t     stBuffer;
             
    struct   stat       statFile;
    struct   stat       statDst;
    
             time_t     timeStart;
             time_t     timeEnd;
             time_t     timeDiff;

    if (iArgC != 3) {
        fprintf(stderr, "param error!\n");
        return  (-1);
    }
    if (lib_strcmp(ppcArgV[1], ppcArgV[2]) == 0) {                      /*  �ļ��ظ�                    */
        fprintf(stderr, "param error!\n");
        return  (-1);
    }
    
    iFdSrc = open(ppcArgV[1], O_RDONLY, 0);                             /*  ��Դ�ļ�                  */
    if (iFdSrc < 0) {
        fprintf(stderr, "can not open source file!\n");
        return  (-1);
    }
    
    iError = fstat(iFdSrc, &statFile);                                  /*  ���Դ�ļ�����              */
    if (iError < 0) {
        goto    __error_handle;
    }
    if (S_ISDIR(statFile.st_mode)) {                                    /*  ���ܸ���Ŀ¼�ļ�            */
        fprintf(stderr, "can not copy dir file!\n");
        errno  = EISDIR;
        iError = PX_ERROR;
        goto    __error_handle;
    }
    
    iError = stat(ppcArgV[2], &statDst);
    if (iError == ERROR_NONE) {
        if (S_ISDIR(statDst.st_mode)) {
            __buildDstFileName(ppcArgV[1], ppcArgV[2], cDstFile);       /*  ����Ŀ���ļ�·��            */
        }
    }
    if (cDstFile[0] == PX_EOS) {
        lib_strlcpy(cDstFile, ppcArgV[2], MAX_FILENAME_LENGTH);
    }

    iError = access(cDstFile, 0);                                       /*  ���Ŀ���ļ��Ƿ����        */
    if (iError == ERROR_NONE) {
__re_select:
        printf("destination file is exist, overwrite? (Y/N)\n");
        read(0, cTemp, 128);
        if ((cTemp[0] == 'N') ||
            (cTemp[0] == 'n')) {                                        /*  ������                      */
            iError = PX_ERROR;
            goto    __error_handle;
        } else if ((cTemp[0] != 'Y') &&
                   (cTemp[0] != 'y')) {                                 /*  ѡ�����                    */
            goto    __re_select;
        }
    }
                                                                        /*  ����Ŀ���ļ�                */
    iFdDst = open(cDstFile, (O_WRONLY | O_CREAT | O_TRUNC), DEFFILEMODE);
    if (iFdDst < 0) {
        close(iFdSrc);
        fprintf(stderr, "can not open destination file!\n");
        return  (-1);
    }
    
    stBuffer = (UINT)__MIN((64 * LW_CFG_KB_SIZE), statFile.st_size);    /*  ���㻺����                  */
    if (stBuffer < 1) {
        stBuffer = 128;                                                 /*  ����Ϊ 128 �ֽ�             */
    }
    pcBuffer = (PCHAR)__SHEAP_ALLOC(stBuffer);
    if (pcBuffer == LW_NULL) {                                          /*  �����ڴ��Ƿ�ʧ��            */
        pcBuffer =  cTemp;                                              /*  ʹ����ʱ�ڴ�                */
        stBuffer =  128;
    }
    
    lib_time(&timeStart);                                               /*  ��¼��ʼʱ��                */
    
    for (;;) {                                                          /*  ��ʼ�����ļ�                */
        sstRdNum = read(iFdSrc, pcBuffer, stBuffer);
        if (sstRdNum > 0) {
            sstWrNum = write(iFdDst, pcBuffer, (size_t)sstRdNum);
            if (sstWrNum != sstRdNum) {                                 /*  д���ļ�����                */
                fprintf(stderr, "can not write destination file! error : %s\n", lib_strerror(errno));
                iError = PX_ERROR;
                break;
            }
        } else if (sstRdNum == 0) {                                     /*  �������                    */
            iError = ERROR_NONE;
            break;
        } else {
            iError = PX_ERROR;                                          /*  ��ȡ���ݴ���                */
            break;
        }
    }
    
    if (iError == ERROR_NONE) {
        lib_time(&timeEnd);                                             /*  ��¼����ʱ��                */
        timeDiff = timeEnd - timeStart;
        
        printf("copy complete. size:%lld(Bytes) time:%lld(s) speed:%lld(Bps)\n", 
               (INT64)statFile.st_size,
               (INT64)timeDiff,
               (INT64)(statFile.st_size / (timeDiff ? timeDiff : 1)));
    }
        
    if (pcBuffer != cTemp) {
        __SHEAP_FREE(pcBuffer);                                         /*  �ͷŻ���                    */
    }
    
__error_handle:
    close(iFdSrc);
    if (iFdDst >= 0) {
        fchmod(iFdDst, statFile.st_mode);                               /*  ����Ϊ��Դ�ļ���ͬ�� mode   */
        close(iFdDst);
    }
    
    if (iError == ERROR_NONE) {                                         /*  �������                    */
        /*
         *  �޸ĸ��ƺ��ļ���ʱ����Ϣ
         */
        struct utimbuf  utimbDst;
        
        utimbDst.actime  = statFile.st_atime;
        utimbDst.modtime = statFile.st_mtime;
        
        (VOID)utime(cDstFile, &utimbDst);                               /*  �����ļ�ʱ��                */
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellFsShowMode
** ��������: ��ʾ�ļ� mode ѡ��
** �䡡��  : pstat     �ļ�ѡ��
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID __tshellFsShowMode (struct stat  *pstat)
{
    CHAR    cBuffer[11];
    
    if (S_ISLNK(pstat->st_mode)) {
        cBuffer[0] = 'l';
    } else if (S_ISDIR(pstat->st_mode)) {
        cBuffer[0] = 'd';
    } else if (S_ISCHR(pstat->st_mode)) {
        cBuffer[0] = 'c';
    } else if (S_ISBLK(pstat->st_mode)) {
        cBuffer[0] = 'b';
    } else if (S_ISSOCK(pstat->st_mode)) {
        cBuffer[0] = 's';
    } else if (S_ISFIFO(pstat->st_mode)) {
        cBuffer[0] = 'f';
    } else {
        cBuffer[0] = '-';
    }
    
    if (pstat->st_mode & S_IRUSR) {
        cBuffer[1] = 'r';
    } else {
        cBuffer[1] = '-';
    }
    
    if (pstat->st_mode & S_IWUSR) {
        cBuffer[2] = 'w';
    } else {
        cBuffer[2] = '-';
    }
    
    if (pstat->st_mode & S_IXUSR) {
        if (pstat->st_mode & S_ISUID) {
            cBuffer[3] = 's';
        } else {
            cBuffer[3] = 'x';
        }
    } else {
        if (pstat->st_mode & S_ISUID) {
            cBuffer[3] = 'S';
        } else {
            cBuffer[3] = '-';
        }
    }
    
    if (pstat->st_mode & S_IRGRP) {
        cBuffer[4] = 'r';
    } else {
        cBuffer[4] = '-';
    }
    
    if (pstat->st_mode & S_IWGRP) {
        cBuffer[5] = 'w';
    } else {
        cBuffer[5] = '-';
    }
    
    if (pstat->st_mode & S_IXGRP) {
        if (pstat->st_mode & S_ISGID) {
            cBuffer[6] = 's';
        } else {
            cBuffer[6] = 'x';
        }
    } else {
        if (pstat->st_mode & S_ISGID) {
            cBuffer[6] = 'S';
        } else {
            cBuffer[6] = '-';
        }
    }
    
    if (pstat->st_mode & S_IROTH) {
        cBuffer[7] = 'r';
    } else {
        cBuffer[7] = '-';
    }
    
    if (pstat->st_mode & S_IWOTH) {
        cBuffer[8] = 'w';
    } else {
        cBuffer[8] = '-';
    }
    
    if (pstat->st_mode & S_IXOTH) {
        cBuffer[9] = 'x';
    } else {
        cBuffer[9] = '-';
    }
    cBuffer[10] = PX_EOS;
    
    printf(cBuffer);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdLl
** ��������: ϵͳ���� "ll"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdLl (INT  iArgC, PCHAR  ppcArgV[])
{
             PCHAR           pcStat;
             CHAR            cDirName[MAX_FILENAME_LENGTH] = ".";
             CHAR            cTimeBuf[32];                              /*  bigger than sizeof(ASCBUF)  */
             PCHAR           pcN;
             struct tm       tmBuf;
             
             size_t          stDirLen = 1;
             
    REGISTER DIR            *pdir;
    REGISTER struct dirent  *pdirent;
             struct stat     statGet;
             
             INT             iError;
             INT             iItemNum = 0;
    
    if ((iArgC == 2) && lib_strcmp(ppcArgV[1], ".")) {                  /*  ָ��Ŀ¼                    */
        lib_strcpy(cDirName, ppcArgV[1]);
        stDirLen = lib_strlen(cDirName);
        pdir     = opendir(cDirName);
        if (stDirLen > 0) {
            if (cDirName[stDirLen - 1] != PX_DIVIDER) {                 /*  ����Ŀ¼������ / ��β       */
                cDirName[stDirLen++] = PX_DIVIDER;                      /*  ����һ�� /                  */
            }
        }
    } else {
        pdir = opendir(cDirName);                                       /*  ��ǰĿ¼                    */
    }
    
    if (!pdir) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "can not open dir %s!\n", lib_strerror(errno));
        }
        return  (-1);
    }
    
    do {
        pdirent = readdir(pdir);
        if (!pdirent) {
            break;
        } else {
            CHAR    cBuffer[MAX_FILENAME_LENGTH];
            
            if ((stDirLen > 1) || 
                ((stDirLen == 1) && (cDirName[0] == PX_ROOT))) {
                lib_strcpy(&cDirName[stDirLen], pdirent->d_name);       /*  ����ָ��Ŀ¼                */
                pcStat = cDirName;
                iError = lstat(cDirName, &statGet);
            } else {
                pcStat = pdirent->d_name;
                iError = lstat(pdirent->d_name, &statGet);              /*  ʹ�õ�ǰĿ¼                */
            }
            if (iError < 0) {                                           /*  �豸�ļ�Ĭ��ʹ�����������  */
                statGet.st_dev     = 0;
                statGet.st_ino     = 0;
                statGet.st_mode    = 0666 | S_IFCHR;                    /*  Ĭ������                    */
                statGet.st_nlink   = 0;
                statGet.st_uid     = 0;
                statGet.st_gid     = 0;
                statGet.st_rdev    = 1;
                statGet.st_size    = 0;
                statGet.st_blksize = 0;
                statGet.st_blocks  = 0;
                statGet.st_atime   = API_RootFsTime(LW_NULL);           /*  Ĭ��ʹ�� root fs ��׼ʱ��   */
                statGet.st_mtime   = API_RootFsTime(LW_NULL);
                statGet.st_ctime   = API_RootFsTime(LW_NULL);
            }
            
            __tshellFsShowMode(&statGet);                               /*  ��ʾ�ļ�����                */
            
            if (API_TShellGetUserName(statGet.st_uid,
                                      cBuffer, sizeof(cBuffer))) {      /*  ����ļ������û���          */
                printf(" %-8d", statGet.st_uid);
            } else {
                printf(" %-8s", cBuffer);
            }
            
            if (API_TShellGetGrpName(statGet.st_gid,
                                     cBuffer, sizeof(cBuffer))) {       /*  ����ļ������û���          */
                printf(" %-8d", statGet.st_gid);
            } else {
                printf(" %-8s", cBuffer);
            }
            
            lib_localtime_r(&statGet.st_mtime, &tmBuf);                 /*  ת��Ϊ tm ��ʽ              */
            lib_asctime_r(&tmBuf, cTimeBuf);                            /*  �����ַ���                  */
            pcN = lib_index(cTimeBuf, '\n');
            if (pcN) {
                *pcN = PX_EOS;
            }
            
            printf(" %s", cTimeBuf);                                    /*  ��ӡ�޸�ʱ��                */
            
            if (S_ISDIR(statGet.st_mode)) {
                API_TShellColorStart(pdirent->d_name, "", statGet.st_mode, STD_OUT);
                printf("           %s/\n", pdirent->d_name);
                API_TShellColorEnd(STD_OUT);
                                      
            } else if (S_ISLNK(statGet.st_mode)) {                      /*  �����ļ�                    */
                CHAR            cDstName[MAX_FILENAME_LENGTH] = "<unknown>";
                struct stat     statDst;
                ssize_t         sstLen;
                
                statDst.st_mode = 0;
                sstLen = readlink(pcStat, cDstName, MAX_FILENAME_LENGTH);
                if (sstLen >= 0) {
                    cDstName[sstLen] = PX_EOS;                          /*  ���������                  */
                }
                stat(cDstName, &statDst);
                API_TShellColorStart(pdirent->d_name, cDstName, statGet.st_mode, STD_OUT);
                printf("           %s/ -> ", pdirent->d_name);
                API_TShellColorStart(cDstName, "", statDst.st_mode, STD_OUT);
                printf("%s\n", cDstName);
                API_TShellColorEnd(STD_OUT);
            
            } else {
                if (statGet.st_size > (10 * LW_CFG_MB_SIZE)) {
                    printf(" %6zdMB, ", (size_t)(statGet.st_size / LW_CFG_MB_SIZE));
                } else if (statGet.st_size > (10 * LW_CFG_KB_SIZE)) {
                    printf(" %6zdKB, ", (size_t)(statGet.st_size / LW_CFG_KB_SIZE));
                } else {
                    printf(" %6zd B, ", (size_t)(statGet.st_size));
                }
                
                API_TShellColorStart(pdirent->d_name, "", statGet.st_mode, STD_OUT);
                printf("%s\n", pdirent->d_name);
                API_TShellColorEnd(STD_OUT);
            }
            
            iItemNum++;
        }
    } while (1);
    
    closedir(pdir);
    
    printf("      total items : %d\n", iItemNum);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __getDsize
** ��������: �ݹ��ȡ�ļ��е��ļ������ʹ�С
** �䡡��  : pcDirName     Ŀ¼��
**           stDirLen      ��ǰĿ¼������
**           pulFileCnt    �ļ�����
**           poftSize      �ܴ�С
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __getDsize (PCHAR  pcDirName, size_t  stDirLen, ULONG  *pulFileCnt, off_t  *poftSize)
{
    DIR            *pdir;
    struct dirent  *pdirent;
    struct stat     statGet;
    
    pdir = opendir(pcDirName);
    if (pdir) {
        do {
            pdirent = readdir(pdir);
            if (pdirent) {
                bnprintf(pcDirName, MAX_FILENAME_LENGTH, stDirLen, "/%s", pdirent->d_name);
                if (lstat(pcDirName, &statGet) == ERROR_NONE) {
                    if (S_ISDIR(statGet.st_mode)) {
                        __getDsize(pcDirName, lib_strlen(pcDirName), pulFileCnt, poftSize);
                    
                    } else {
                        (*pulFileCnt) += 1;
                        (*poftSize)   += statGet.st_size;
                    }
                }
                pcDirName[stDirLen] = PX_EOS;
            }
        } while (pdirent);
        closedir(pdir);
    }
}
/*********************************************************************************************************
** ��������: __tshellFsCmdDsize
** ��������: ϵͳ���� "dsize"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdDsize (INT  iArgC, PCHAR  ppcArgV[])
{
    ULONG       ulFiles = 0ul;
    off_t       oftSize = 0;
    struct stat statGet;
    CHAR        cDirName[MAX_FILENAME_LENGTH];
    
    if (iArgC < 2) {
        fprintf(stderr, "df argments error, (dsize directory)\n");
        return  (-1);
    }
    
    if (stat(ppcArgV[1], &statGet)) {
        fprintf(stderr, "can not get %s stat().\n", ppcArgV[1]);
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    if (S_ISDIR(statGet.st_mode)) {
        lib_strlcpy(cDirName, ppcArgV[1], MAX_FILENAME_LENGTH);
        printf("scanning...\n");
        __getDsize(cDirName, lib_strlen(cDirName), &ulFiles, &oftSize);
        printf("total file %lu size %llu\n", ulFiles, oftSize);
        
    } else {
        printf("total file 1 size %llu\n", statGet.st_size);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdDf
** ��������: ϵͳ���� "df"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdDf (INT  iArgC, PCHAR  ppcArgV[])
{
    static PCHAR   pcVolumeInfoHdr = \
                    "    VOLUME         TOTAL        FREE     USED RO       FS TYPE\n"
                    "-------------- ------------ ------------ ---- -- --------------------\n";

    struct statfs       statfsGet;
           
           /*
            *  ������Ϣ
            */
           UINT64       ullFree;
           UINT64       ullTotal;
           
           /*
            *  ��ʾ�ñ���
            */
           ULONG        ulTotalDisp;
           ULONG        ulTotalPoint;
           ULONG        ulFreeDisp;
           ULONG        ulFreePoint;
           
           /*
            *  ��ʾ��λ�ı�ֵ
            */
           ULONG        ulTotalDiv;
           ULONG        ulFreeDiv;
           
           /*
            *  ��ʾ��λ
            */
           PCHAR        pcTotalUnit = "";
           PCHAR        pcFreeUnit  = "";
           
           /*
            *  �ļ�ϵͳ����
            */
           PCHAR        pcFsType = "unknown";
           
           /*
            *  ʹ�ðٷֱ�
            */
           INT          iUseagePercent;
           
           /*
            *  �Ƿ�Ϊֻ��
            */
           PCHAR        pcRo = "n";

    if (iArgC != 2) {
        fprintf(stderr, "df argments error, (df volname)\n");
        return  (-1);
    }
    
    if (statfs(ppcArgV[1], &statfsGet) < 0) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            perror("volume state error");
        }
        return  (-1);
    }
    
    ullFree  = ((UINT64)statfsGet.f_bfree  * statfsGet.f_bsize);
    ullTotal = ((UINT64)statfsGet.f_blocks * statfsGet.f_bsize);
    
    if (ullTotal > (2ul * LW_CFG_GB_SIZE)) {
        ulTotalDiv  = LW_CFG_GB_SIZE;
        pcTotalUnit = "GB";
    } else if (ullTotal > (2ul * LW_CFG_MB_SIZE)) {
        ulTotalDiv  = LW_CFG_MB_SIZE;
        pcTotalUnit = "MB";
    } else {
        ulTotalDiv  = LW_CFG_KB_SIZE;
        pcTotalUnit = "KB";
    }
    
    if (ullFree > (2ul * LW_CFG_GB_SIZE)) {
        ulFreeDiv  = LW_CFG_GB_SIZE;
        pcFreeUnit = "GB";
    } else if (ullFree > (2ul * LW_CFG_MB_SIZE)) {
        ulFreeDiv  = LW_CFG_MB_SIZE;
        pcFreeUnit = "MB";
    } else {
        ulFreeDiv  = LW_CFG_KB_SIZE;
        pcFreeUnit = "KB";
    }
    
    ulTotalDisp  = (ULONG)(ullTotal / ulTotalDiv);
    if (ullTotal % ulTotalDiv) {
        ulTotalPoint = (ULONG)(ullTotal / (UINT64)(ulTotalDiv / 100));
        ulTotalPoint = (ulTotalPoint % 100);
    } else {
        ulTotalPoint = 0;
    }
    
    ulFreeDisp   = (ULONG)(ullFree / ulFreeDiv);
    if (ullFree % ulFreeDiv) {
        ulFreePoint = (ULONG)(ullFree / (UINT64)(ulFreeDiv / 100));
        ulFreePoint = (ulFreePoint % 100);
    } else {
        ulFreePoint = 0;
    }
    
    {
        INT  iFd = open(ppcArgV[1], O_RDONLY, 0);
        if (iFd >= 0) {
            ioctl(iFd, FIOFSTYPE, (LONG)&pcFsType);
            close(iFd);
        }
    }
    
    printf(pcVolumeInfoHdr);
    
    if (statfsGet.f_flag & ST_RDONLY) {
        pcRo = "y";
    }
    
    if (ullFree > ullTotal) {
        printf("%-14s %7lu.%02lu%-2s        unknown---%% %-2s %s\n", ppcArgV[1], 
               ulTotalDisp, ulTotalPoint, 
               pcTotalUnit, pcRo, pcFsType);
    } else {
        UINT64      ullUseBlocks = (UINT64)(statfsGet.f_blocks - statfsGet.f_bfree);
    
        ullUseBlocks *= 100;
        iUseagePercent = (INT)(ullUseBlocks / statfsGet.f_blocks);
        printf("%-14s %7lu.%02lu%-2s %7lu.%02lu%-2s %3d%% %-2s %s\n", ppcArgV[1], 
               ulTotalDisp, ulTotalPoint, pcTotalUnit, 
               ulFreeDisp,  ulFreePoint,  pcFreeUnit, 
               iUseagePercent, pcRo, pcFsType);
    }
           
    return  (0);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdChmod
** ��������: ϵͳ���� "chmod"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdChmod (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER size_t     stLen;
             INT        i;
             INT        iNewMode = DEFAULT_FILE_PERM;

    if (iArgC != 3) {
        fprintf(stderr, "chmod argments error, (chmod newmode filename)\n");
        return  (-1);
    }
    
    stLen = lib_strnlen(ppcArgV[1], 5);
    if (stLen > 4) {
        fprintf(stderr, "newmode error, (eg. 777 ... octal)\n");
        return  (-1);
    }
    
    for (i = 0; i < stLen; i++) {                                       /*  ������ 8���Ƶ�              */
        if ((ppcArgV[1][i] > '7') || (ppcArgV[1][i] < '0')) {
            fprintf(stderr, "newmode error, (eg. 777 ... octal)\n");
            return  (-1);
        }
    }
    
    sscanf(ppcArgV[1], "%o", &iNewMode);
    
    if (lib_strchr(ppcArgV[2], '*') ||
        lib_strchr(ppcArgV[2], '?')) {                                  /*  ���� shell ͨ���           */

#if LW_CFG_POSIX_EN > 0
        DIR             *pdir;
        struct dirent   *pdirent;
        CHAR             cName[MAX_FILENAME_LENGTH];
        size_t           stDirLen;
        PCHAR            pcTail;
        INT              iRet;
        
        pcTail = lib_strrchr(ppcArgV[2], PX_DIVIDER);
        if (pcTail) {
            stDirLen = pcTail - ppcArgV[2];
            lib_memcpy(cName, ppcArgV[2], stDirLen);
            cName[stDirLen] = PX_EOS;
            pcTail++;                                                   /*  ָ���ļ�������              */
        
        } else {
            stDirLen = 1;
            lib_strcpy(cName, ".");                                     /*  ��ǰĿ¼                    */
            pcTail = ppcArgV[2];                                        /*  ָ���ļ�������              */
        }
        
        pdir = opendir(cName);
        if (!pdir) {
            fprintf(stderr, "can not open dir %s error : %s\n", cName, lib_strerror(errno));
            return  (-1);
        }
        
        pdirent = readdir(pdir);
        while (pdirent) {
            iRet = fnmatch(pcTail, pdirent->d_name, FNM_PATHNAME);
            if (iRet == ERROR_NONE) {
                bnprintf(cName, MAX_FILENAME_LENGTH, stDirLen, "/%s", pdirent->d_name);
                i = chmod(cName, iNewMode);                             /*  ת���ļ�ģʽ                */
                if (i < 0) {
                    if (errno == EACCES) {
                        fprintf(stderr, "%s insufficient permissions.\n", 
                                pdirent->d_name);
                    } else {
                        fprintf(stderr, "%s can not change file mode, error : %s\n", 
                                pdirent->d_name, lib_strerror(errno));
                    }
                }
            }
            pdirent = readdir(pdir);
        }
        closedir(pdir);
#else
        printf("sylixos do not have fnmatch().\n");
        return  (-1);
#endif
    } else {
        i = chmod(ppcArgV[2], iNewMode);                                /*  ת���ļ�ģʽ                */
        if (i < 0) {
            if (errno == EACCES) {
                fprintf(stderr, "insufficient permissions.\n");
            } else {
                perror("can not change file mode error");
            }
            return  (-1);
        }
    }
    
    return  (0);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdMkfs
** ��������: ϵͳ���� "mkfs"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdMkfs (INT  iArgC, PCHAR  ppcArgV[])
{
    REGISTER INT    iError;
    
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    printf("now format media, please wait...\n");
    iError = mkfs(ppcArgV[1]);
    if (iError) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "can not format media : error : %s\n", lib_strerror(errno));
        }
        return  (-errno);
    }
    printf("disk format ok.\n");
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdShfile
** ��������: ϵͳ���� "shfile"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdShfile (INT  iArgC, PCHAR  ppcArgV[])
{
    FILE  *fileShell;
    PCHAR  pcCmd       = LW_NULL;
    PCHAR  pcCmdBuffer = LW_NULL;                                       /*  �����                    */
    
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    fileShell = fopen(ppcArgV[1], "r");                                 /*  �� shell �ļ�             */
    if (fileShell == LW_NULL) {
        if (errno == EACCES) {
            fprintf(stderr, "insufficient permissions.\n");
        } else {
            fprintf(stderr, "can not open %s : %s\n", ppcArgV[1], lib_strerror(errno));
        }
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    pcCmdBuffer = (PCHAR)__SHEAP_ALLOC(LW_CFG_SHELL_MAX_COMMANDLEN + 1);/*  ���������                */
    if (pcCmdBuffer == LW_NULL) {
        fclose(fileShell);
        fprintf(stderr, "system low memory.\n");
        return  (PX_ERROR);
    }
    
    do {
        pcCmd = fgets(pcCmdBuffer, LW_CFG_SHELL_MAX_COMMANDLEN, 
                      fileShell);                                       /*  ���һ��ָ��                */
        if (pcCmd) {
            API_TShellExec(pcCmd);                                      /*  ִ��ָ������                */
        }
    } while (pcCmd);
    
    __SHEAP_FREE(pcCmdBuffer);                                          /*  �ͷŻ���                    */
    fclose(fileShell);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdMount
** ��������: ϵͳ���� "mount"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_MOUNT_EN > 0

static INT  __tshellFsCmdMount (INT  iArgC, PCHAR  ppcArgV[])
{
    PCHAR       pcType   = LW_NULL;
    PCHAR       pcDev    = LW_NULL;
    PCHAR       pcFs     = LW_NULL;
    PCHAR       pcOption = LW_NULL;

    INT         iC;
    INT         iOptInd;
    
    if (iArgC < 3) {
        fprintf(stderr, "option error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    while ((iC = getopt(iArgC, ppcArgV, "t:o:")) != EOF) {
        switch (iC) {
        
        case 't':
            pcType = optarg;
            break;
            
        case 'o':
            pcOption = optarg;
            break;
        }
    }
    
    iOptInd = optind;
    if (iOptInd > (iArgC - 2)) {
        fprintf(stderr, "option error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    getopt_free();
    
    pcDev = ppcArgV[iOptInd];
    pcFs  = ppcArgV[iOptInd + 1];
    
    if (API_MountEx(pcDev, pcFs, pcType, pcOption) != ERROR_NONE) {
        fprintf(stderr, "mount error, error : %s\n", lib_strerror(errno));
        return  (PX_ERROR);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: __tshellFsCmdUmount
** ��������: ϵͳ���� "umount"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdUmount (INT  iArgC, PCHAR  ppcArgV[])
{
    if (iArgC != 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    if (API_Unmount(ppcArgV[1]) == ERROR_NONE) {
        return  (ERROR_NONE);
    } else {
        fprintf(stderr, "umount error, error : %s\n", lib_strerror(errno));
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: __tshellFsCmdShowmount
** ��������: ϵͳ���� "showmount"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdShowmount (INT  iArgC, PCHAR  ppcArgV[])
{
    API_MountShow();
    
    return  (ERROR_NONE);
}
#endif                                                                  /*  LW_CFG_MOUNT_EN > 0         */
/*********************************************************************************************************
** ��������: __tshellFsCmdLn
** ��������: ϵͳ���� "ln"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdLn (INT  iArgC, PCHAR  ppcArgV[])
{
    if (iArgC == 3) {
        if (ppcArgV[1][0] == '-') {
            fprintf(stderr, "argments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (symlink(ppcArgV[1], ppcArgV[2]) != ERROR_NONE) {
            fprintf(stderr, "symlink error : %s\n", lib_strerror(errno));
        }
    } else if (iArgC == 4) {
        if (ppcArgV[1][0] != '-') {
            fprintf(stderr, "argments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (!lib_strchr(ppcArgV[1], 's')) {
            fprintf(stderr, "must use -s to create a symbol link.\n");
            return  (-ERROR_TSHELL_EPARAM);
            
        } else {
            if (lib_strchr(ppcArgV[1], 'f')) {
                struct stat  statGet;
                if (lstat(ppcArgV[3], &statGet) == ERROR_NONE) {
                    if (S_ISLNK(statGet.st_mode)) {
                        unlink(ppcArgV[3]);
                    }
                }
            }
            if (symlink(ppcArgV[2], ppcArgV[3]) != ERROR_NONE) {
                fprintf(stderr, "symlink error : %s\n", lib_strerror(errno));
            }
        }
    } else {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdDosfslabel
** ��������: ϵͳ���� "dosfslabel"
** �䡡��  : iArgC         ��������
**           ppcArgV       ������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __tshellFsCmdDosfslabel (INT  iArgC, PCHAR  ppcArgV[])
{
    INT  iFd;
    INT  iError;
    CHAR cLabel[MAX_FILENAME_LENGTH];
    
    if (iArgC < 2) {
        fprintf(stderr, "argments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    if (iArgC == 2) {                                                   /*  ��ȡ����                    */
        iFd = open(ppcArgV[1], O_RDONLY);
    } else {
        iFd = open(ppcArgV[1], O_RDWR);                                 /*  ���þ���                    */
    }
    
    if (iFd < 0) {
        fprintf(stderr, "can not open device, error : %s\n", lib_strerror(errno));
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    if (iArgC == 2) {
        iError = ioctl(iFd, FIOLABELGET, cLabel);
        if (iError < ERROR_NONE) {
            fprintf(stderr, "can not get label, error : %s\n", lib_strerror(errno));
        } else {
            printf("%s\n", cLabel);
        }
    } else {
        iError = ioctl(iFd, FIOLABELSET, ppcArgV[2]);
        if (iError < ERROR_NONE) {
            fprintf(stderr, "can not set label, error : %s\n", lib_strerror(errno));
        }
    }
    
    close(iFd);
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: __tshellFsCmdInit
** ��������: ��ʼ���ļ�ϵͳ���
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __tshellFsCmdInit (VOID)
{
    API_TShellKeywordAdd("cd", __tshellFsCmdCd);
    API_TShellFormatAdd("cd", " path");
    API_TShellHelpAdd("cd", "set current working path\n");
    
    API_TShellKeywordAdd("ch", __tshellFsCmdCh);
    API_TShellFormatAdd("ch", " dir");
    API_TShellHelpAdd("ch", "change dir\n");
    
    API_TShellKeywordAdd("pwd", __tshellFsCmdPwd);
    API_TShellFormatAdd("pwd", " ");
    API_TShellHelpAdd("pwd", "print working directory\n");
    
    API_TShellKeywordAdd("df", __tshellFsCmdDf);
    API_TShellFormatAdd("df", " volume name");
    API_TShellHelpAdd("df", "display volume useage information\n");
    
    API_TShellKeywordAdd("tmpname", __tshellFsCmdTmpname);
    API_TShellHelpAdd("tmpname", "calculate a temp file name.\n");
    
    API_TShellKeywordAdd("mkdir", __tshellFsCmdMkdir);
    API_TShellFormatAdd("mkdir", " directory");
    API_TShellHelpAdd("mkdir", "make a new directory\n");
    
    API_TShellKeywordAdd("mkfifo", __tshellFsCmdMkfifo);
    API_TShellFormatAdd("mkfifo", " [fifo name]");
    API_TShellHelpAdd("mkfifo", "make a new fifo\n");
    
    API_TShellKeywordAdd("rmdir", __tshellFsCmdRmdir);
    API_TShellFormatAdd("rmdir", " directory");
    API_TShellHelpAdd("rmdir", "remove a directory\n");
    
    API_TShellKeywordAdd("rm", __tshellFsCmdRm);
    API_TShellFormatAdd("rm", " file name");
    API_TShellHelpAdd("rm", "remove a file\n");
    
    API_TShellKeywordAdd("mv", __tshellFsCmdMv);
    API_TShellFormatAdd("mv", " SRC file name, DST file name");
    API_TShellHelpAdd("mv", "move a file\n");
    
    API_TShellKeywordAdd("cat", __tshellFsCmdCat);
    API_TShellFormatAdd("cat", " file name");
    API_TShellHelpAdd("cat", "display file\n");
    
    API_TShellKeywordAdd("cp", __tshellFsCmdCp);
    API_TShellFormatAdd("cp", " scr file name dst file name");
    API_TShellHelpAdd("cp", "copy file\n");
    
    API_TShellKeywordAdd("cmp", __tshellFsCmdCmp);
    API_TShellFormatAdd("cmp", " [file one] [file two]");
    API_TShellHelpAdd("cmp", "compare two file\n");
    
    API_TShellKeywordAdd("touch", __tshellFsCmdTouch);
    API_TShellFormatAdd("touch", " [-amc] file name");
    API_TShellHelpAdd("touch", "touch a file\n");
    
    API_TShellKeywordAdd("ls", __tshellFsCmdLs);
    API_TShellFormatAdd("ls", " [path name]");
    API_TShellHelpAdd("ls", "list file(s)\n");
    
    API_TShellKeywordAdd("ll", __tshellFsCmdLl);
    API_TShellFormatAdd("ll", " [path name]");
    API_TShellHelpAdd("ll", "get file(s) attrib\n");
    
    API_TShellKeywordAdd("dsize", __tshellFsCmdDsize);
    API_TShellFormatAdd("dsize", " [path name]");
    API_TShellHelpAdd("dsize", "get directory size\n");
    
    API_TShellKeywordAdd("chmod", __tshellFsCmdChmod);
    API_TShellFormatAdd("chmod", " newmode filename");
    API_TShellHelpAdd("chmod", "change file attrib\n"
                               "eg. chmod 777 file\n");
    
    API_TShellKeywordAdd("mkfs", __tshellFsCmdMkfs);
    API_TShellFormatAdd("mkfs", " media name");
    API_TShellHelpAdd("mkfs", "make a fs format in a disk(format disk).\n");
    
    API_TShellKeywordAdd("shfile", __tshellFsCmdShfile);
    API_TShellFormatAdd("shfile", " shell file");
    API_TShellHelpAdd("shfile", "execute a shell file.\n");
    
#if LW_CFG_MOUNT_EN > 0
    API_TShellKeywordAdd("mount", __tshellFsCmdMount);
    API_TShellFormatAdd("mount", " [-t fstype] [-o option] [blk dev] [mount path]");
    API_TShellHelpAdd("mount",  "mount a volume.\n"
                                "eg. mount /dev/sda1 /mnt/usb\n"
                                "    mount -t vfat /dev/sda1 /mnt/usb\n"
                                "    mount -t romfs /dev/rom0 /mnt/rom0\n"
                                "    mount -t ramfs 100000 /mnt/ram\n"
                                "    mount -t nfs -o ro 192.168.0.123:/nfstest /mnt/nfs\n"
                                "-o \n"
                                "ro (read-only file system)\n"
                                "rw (read-write file system)\n");
                                
    API_TShellKeywordAdd("umount", __tshellFsCmdUmount);
    API_TShellFormatAdd("umount", " [mount path]");
    API_TShellHelpAdd("umount",  "unmount a volume.\n"
                                 "eg. mount /mnt/usb\n");

    API_TShellKeywordAdd("showmount", __tshellFsCmdShowmount);
    API_TShellHelpAdd("showmount",  "show all mount point.\n");
#endif                                                                  /*  LW_CFG_MOUNT_EN > 0         */

    API_TShellKeywordAdd("ln", __tshellFsCmdLn);
    API_TShellFormatAdd("ln", " [-s | -f] [actualpath] [sympath]");
    API_TShellHelpAdd("ln",   "create a symbol link file (must use -s).\n"
                              "eg. ln -s /tmp/dir /dir\n");
                              
    API_TShellKeywordAdd("dosfslabel", __tshellFsCmdDosfslabel);
    API_TShellFormatAdd("dosfslabel", " [[vol newlabel] [vol]]");
    API_TShellHelpAdd("dosfslabel",   "get or set volumn label.\n"
                                      "eg. dosfslabel /usb/ms0\n"
                                      "    dosfslabel /usb/ms1 newlabel\n");
}
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
                                                                        /*  LW_CFG_MAX_VOLUMES > 0      */
/*********************************************************************************************************
  END
*********************************************************************************************************/