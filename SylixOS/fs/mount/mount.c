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
** ��   ��   ��: mount.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 09 �� 26 ��
**
** ��        ��: mount ���ؿ�, ���� SylixOS �ڶ�����豸���ط�ʽ.
                 ��ϸ�� blockIo.h (���Ƽ�ʹ��! ���ܴ�������)
** BUG:
2012.03.10  ����� NFS ��֧��.
2012.03.12  API_Mount() �ڵ��¼����·��, ɾ���ڵ�Ҳʹ�þ���·��.
2012.03.21  ���� API_MountShow() ����.
2012.04.11  ��������� mount ��, ���豸ж��ʧ��ʱ, umount ʧ��.
2012.06.27  �ڶ�����豸֧��ֱ�Ӷ�д���ݽӿ�.
            �������豸���ǿ��豸ʱ, ����ʹ�� __LW_MOUNT_DEFAULT_SECSIZE ���в���, ��������ֱ���޷����
            �ļ�ϵͳ�����ļ�, ����: romfs �ļ�.
2012.08.16  ʹ�� pread �� pwrite ���� lseek->read/write ����.
2012.09.01  ֧��ж�ط� mount �豸.
2012.12.07  ��Ĭ���ļ�ϵͳ����Ϊ vfat.
2012.12.08  �� block �ļ�������Ҫ���� ioctl ����.
2012.12.25  mount �� umount �������ں˿ռ�ִ��, �Ա�֤�����������ļ�������Ϊ�ں��ļ�������, �����������ں�
            �ռ�ʱ�����Է���.
2013.04.02  ���� sys/mount.h ֧��.
2013.06.25  logic �豸 BLKD_pvLink ����Ϊ NULL.
2014.05.24  ����� ramfs ֧��.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/fs/include/fs_fs.h"
#include "../SylixOS/fs/fsCommon/fsCommon.h"
/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_MAX_VOLUMES > 0) && (LW_CFG_MOUNT_EN > 0)
#include "sys/mount.h"
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define __LW_MOUNT_DEFAULT_FS       "vfat"                              /*  Ĭ�Ϲ����ļ�ϵͳ��ʽ        */
#define __LW_MOUNT_NFS_FS           "nfs"                               /*  nfs ����                    */
#define __LW_MOUNT_RAM_FS           "ramfs"                             /*  ram ����                    */
/*********************************************************************************************************
  Ĭ�ϲ���
*********************************************************************************************************/
#define __LW_MOUNT_DEFAULT_SECSIZE  512                                 /*  �� dev ���ǿ��豸ʱ������С */
/*********************************************************************************************************
  ���ؽڵ�
*********************************************************************************************************/
typedef struct {
    LW_BLK_DEV              MN_blkd;                                    /*  SylixOS ��һ����豸        */
    LW_LIST_LINE            MN_lineManage;                              /*  ��������                    */
    INT                     MN_iFd;                                     /*  �ڶ�����豸�ļ�������      */
    mode_t                  MN_mode;                                    /*  �ڶ����豸����              */
    CHAR                    MN_cVolName[1];                             /*  ���ؾ������                */
} __LW_MOUNT_NODE;
typedef __LW_MOUNT_NODE    *__PLW_MOUNT_NODE;
/*********************************************************************************************************
  ���ص�
*********************************************************************************************************/
static LW_LIST_LINE_HEADER  _G_plineMountDevHeader = LW_NULL;           /*  û�б�Ҫʹ�� hash ��ѯ      */
static LW_OBJECT_HANDLE     _G_ulMountLock = 0ul;

#define __LW_MOUNT_LOCK()   API_SemaphoreMPend(_G_ulMountLock, LW_OPTION_WAIT_INFINITE)
#define __LW_MOUNT_UNLOCK() API_SemaphoreMPost(_G_ulMountLock)
/*********************************************************************************************************
** ��������: __mountDevReset
** ��������: ��λ���豸.
** �䡡��  : pmnDev             mount �ڵ�
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mountDevReset (__PLW_MOUNT_NODE  pmnDev)
{
    if (!S_ISBLK(pmnDev->MN_mode)) {
        return  (ERROR_NONE);
    }
    
    return  (ioctl(pmnDev->MN_iFd, LW_BLKD_CTRL_RESET, 0));
}
/*********************************************************************************************************
** ��������: __mountDevStatusChk
** ��������: �����豸.
** �䡡��  : pmnDev             mount �ڵ�
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mountDevStatusChk (__PLW_MOUNT_NODE  pmnDev)
{
    if (!S_ISBLK(pmnDev->MN_mode)) {
        return  (ERROR_NONE);
    }
    
    return  (ioctl(pmnDev->MN_iFd, LW_BLKD_CTRL_STATUS, 0));
}
/*********************************************************************************************************
** ��������: __mountDevIoctl
** ��������: ���ƿ��豸.
** �䡡��  : pmnDev            mount �ڵ�
**           iCmd              ��������
**           lArg              ���Ʋ���
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mountDevIoctl (__PLW_MOUNT_NODE  pmnDev, INT  iCmd, LONG  lArg)
{
    if (iCmd == LW_BLKD_CTRL_GETFD) {
        if (lArg) {
            *((INT *)lArg) = pmnDev->MN_iFd;
        }
        return  (ERROR_NONE);
    }
    
    if (!S_ISBLK(pmnDev->MN_mode)) {
        if ((iCmd != FIOFLUSH) && 
            (iCmd != FIOSYNC) &&
            (iCmd != FIODATASYNC)) {                                    /*  �� BLK �豸ֻ��ִ����Щ���� */
            _ErrorHandle(ENOSYS);
            return  (PX_ERROR);
        }
    }
    
    return  (ioctl(pmnDev->MN_iFd, iCmd, lArg));
}
/*********************************************************************************************************
** ��������: __mountDevWrt
** ��������: д���豸.
** �䡡��  : pmnDev            mount �ڵ�
**           pvBuffer          ������
**           ulStartSector     ��ʼ������
**           ulSectorCount     ��������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mountDevWrt (__PLW_MOUNT_NODE  pmnDev, 
                           VOID             *pvBuffer, 
                           ULONG             ulStartSector, 
                           ULONG             ulSectorCount)
{
    LW_BLK_CTL      blkc;
    
    if (S_ISBLK(pmnDev->MN_mode)) {
        blkc.BLKC_bIsRead       = LW_FALSE;
        blkc.BLKC_pvBuffer      = pvBuffer;
        blkc.BLKC_ulStartSector = ulStartSector;
        blkc.BLKC_ulSectorCount = ulSectorCount;
        return  (ioctl(pmnDev->MN_iFd, LW_BLKD_CTRL_WRITE, &blkc));
    
    } else {
        size_t  stBytes  = (size_t)ulSectorCount * __LW_MOUNT_DEFAULT_SECSIZE;
        off_t   oftStart = (off_t)ulStartSector * __LW_MOUNT_DEFAULT_SECSIZE;
        
        if (pwrite(pmnDev->MN_iFd, pvBuffer, stBytes, oftStart) == stBytes) {
            return  (ERROR_NONE);
        }
    
        _ErrorHandle(ENOTBLK);
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: __mountDevRd
** ��������: �����豸.
** �䡡��  : pmnDev            mount �ڵ�
**           pvBuffer          ������
**           ulStartSector     ��ʼ������
**           ulSectorCount     ��������
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mountDevRd (__PLW_MOUNT_NODE   pmnDev,
                          VOID              *pvBuffer, 
                          ULONG              ulStartSector, 
                          ULONG              ulSectorCount)
{
    LW_BLK_CTL      blkc;
    
    if (S_ISBLK(pmnDev->MN_mode)) {
        blkc.BLKC_bIsRead       = LW_TRUE;
        blkc.BLKC_pvBuffer      = pvBuffer;
        blkc.BLKC_ulStartSector = ulStartSector;
        blkc.BLKC_ulSectorCount = ulSectorCount;
        return  (ioctl(pmnDev->MN_iFd, LW_BLKD_CTRL_READ, &blkc));
        
    } else {
        size_t  stBytes  = (size_t)ulSectorCount * __LW_MOUNT_DEFAULT_SECSIZE;
        off_t   oftStart = (off_t)ulStartSector * __LW_MOUNT_DEFAULT_SECSIZE;
    
        if (pread(pmnDev->MN_iFd, pvBuffer, stBytes, oftStart) == stBytes) {
            return  (ERROR_NONE);
        }
        
        _ErrorHandle(ENOTBLK);
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: __mountInit
** ��������: ��ʼ�� mount ��.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID __mountInit (VOID)
{
    if (_G_ulMountLock == 0) {
        _IosLock();
        if (_G_ulMountLock == 0) {
            _G_ulMountLock = API_SemaphoreMCreate("mount_lock", LW_PRIO_DEF_CEILING, 
                                LW_OPTION_DELETE_SAFE | LW_OPTION_OBJECT_GLOBAL, LW_NULL);
        }
        _IosUnlock();
    }
}
/*********************************************************************************************************
** ��������: __mount
** ��������: ����һ������(�ڲ�����)
** �䡡��  : pcDevName         ���豸��   ����: /dev/sda1
**           pcVolName         ����Ŀ��   ����: /mnt/usb (����ʹ�����·��, �����޷�ж��)
**           pcFileSystem      �ļ�ϵͳ��ʽ "vfat" "iso9660" "ntfs" "nfs" "romfs" "ramfs" ... 
                               NULL ��ʾʹ��Ĭ���ļ�ϵͳ
**           pcOption          ѡ��, ��ǰ֧�� ro ���� rw
** �䡡��  : < 0 ��ʾʧ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mount (CPCHAR  pcDevName, CPCHAR  pcVolName, CPCHAR  pcFileSystem, CPCHAR  pcOption)
{
#define __LW_MOUNT_OPT_RO   "ro"
#define __LW_MOUNT_OPT_RW   "rw"

    REGISTER PCHAR      pcFs;
             FUNCPTR    pfuncFsCreate;
             INT        iFd;
             INT        iOpenFlag = O_RDWR;
             size_t     stLen;
             
      struct stat       statBuf;
    __PLW_MOUNT_NODE    pmnDev;
    
             CHAR           cVolNameBuffer[MAX_FILENAME_LENGTH];

    if (!pcDevName || !pcVolName) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    __mountInit();
    
    if (pcOption) {                                                     /*  �ļ�ϵͳ����ѡ��            */
        if (lib_strcasecmp(__LW_MOUNT_OPT_RO, pcOption) == 0) {
            iOpenFlag = O_RDONLY;
        } else if (lib_strcasecmp(__LW_MOUNT_OPT_RW, pcOption) == 0) {
            iOpenFlag = O_RDWR;
        }
    }
    
    pcFs = (!pcFileSystem) ? __LW_MOUNT_DEFAULT_FS : (PCHAR)pcFileSystem;
    pfuncFsCreate = __fsCreateFuncGet(pcFs);                            /*  �ļ�ϵͳ��������            */
    if (pfuncFsCreate == LW_NULL) {
        _ErrorHandle(ERROR_IO_NO_DRIVER);                               /*  û���ļ�ϵͳ����            */
        return  (PX_ERROR);
    }
    
    if ((lib_strcmp(pcFs, __LW_MOUNT_NFS_FS) == 0) ||
        (lib_strcmp(pcFs, __LW_MOUNT_RAM_FS) == 0)) {                   /*  NFS ���� RAM FS             */
        iFd = -1;                                                       /*  ����Ҫ�����豸�ļ�          */
    
    } else {
        iFd = open(pcDevName, iOpenFlag);                               /*  �򿪿��豸                  */
        if (iFd < 0) {
            iOpenFlag = O_RDONLY;
            iFd = open(pcDevName, iOpenFlag);                           /*  ֻ����ʽ��                */
            if (iFd < 0) {
                return  (PX_ERROR);
            }
        }
        if (fstat(iFd, &statBuf) < 0) {                                 /*  ����豸����                */
            close(iFd);
            return  (PX_ERROR);
        }
    }
    
    _PathGetFull(cVolNameBuffer, MAX_FILENAME_LENGTH, pcVolName);
    
    pcVolName = cVolNameBuffer;                                         /*  ʹ�þ���·��                */
    
    stLen = lib_strlen(pcVolName);
    pmnDev = (__PLW_MOUNT_NODE)__SHEAP_ALLOC(sizeof(__LW_MOUNT_NODE) + stLen);
    if (pmnDev == LW_NULL) {
        if (iFd >= 0) {
            close(iFd);
        }
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (PX_ERROR);
    }
    lib_bzero(pmnDev, sizeof(__LW_MOUNT_NODE));
    
    pmnDev->MN_blkd.BLKD_pcName = (PCHAR)__SHEAP_ALLOC(lib_strlen(pcDevName) + 1);
    if (pmnDev->MN_blkd.BLKD_pcName == LW_NULL) {
        if (iFd >= 0) {
            close(iFd);
        }
        __SHEAP_FREE(pmnDev);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (PX_ERROR);
    }
    lib_strcpy(pmnDev->MN_blkd.BLKD_pcName, pcDevName);                 /*  ��¼�豸��(nfs ram rom ʹ��)*/
    lib_strcpy(pmnDev->MN_cVolName, pcVolName);                         /*  ����������                */
    
    pmnDev->MN_blkd.BLKD_pfuncBlkRd        = __mountDevRd;
    pmnDev->MN_blkd.BLKD_pfuncBlkWrt       = __mountDevWrt;
    pmnDev->MN_blkd.BLKD_pfuncBlkIoctl     = __mountDevIoctl;
    pmnDev->MN_blkd.BLKD_pfuncBlkReset     = __mountDevReset;
    pmnDev->MN_blkd.BLKD_pfuncBlkStatusChk = __mountDevStatusChk;
    
    
    if (S_ISBLK(statBuf.st_mode)) {                                     /*  ��׼���豸                  */
        pmnDev->MN_blkd.BLKD_ulNSector        = 0;                      /*  ͨ�� ioctl ��ȡ             */
        pmnDev->MN_blkd.BLKD_ulBytesPerSector = 0;
        pmnDev->MN_blkd.BLKD_ulBytesPerBlock  = 0;
    
    } else {                                                            /*  �ǿ��豸ʹ��Ĭ��������С    */
        pmnDev->MN_blkd.BLKD_ulBytesPerSector = __LW_MOUNT_DEFAULT_SECSIZE;
        pmnDev->MN_blkd.BLKD_ulNSector        = (ULONG)(statBuf.st_size 
                                              / __LW_MOUNT_DEFAULT_SECSIZE);
        pmnDev->MN_blkd.BLKD_ulBytesPerBlock  = __LW_MOUNT_DEFAULT_SECSIZE;
    }
    
    pmnDev->MN_blkd.BLKD_bRemovable       = LW_TRUE;
    pmnDev->MN_blkd.BLKD_bDiskChange      = LW_FALSE;
    pmnDev->MN_blkd.BLKD_iRetry           = 3;                          /*  default 3 times             */
    pmnDev->MN_blkd.BLKD_iFlag            = iOpenFlag;
    pmnDev->MN_blkd.BLKD_iLogic           = 1;                          /*  ��ʽ��������������          */
    pmnDev->MN_blkd.BLKD_pvLink           = (PLW_BLK_DEV)&pmnDev->MN_blkd;
    /*
     *  ���²�����ʼ��Ϊ 0 
     */
    
    pmnDev->MN_iFd  = iFd;
    pmnDev->MN_mode = statBuf.st_mode;                                  /*  �豸�ļ�����                */
    
    if (pfuncFsCreate(pcVolName, (PLW_BLK_DEV)pmnDev) < 0) {            /*  �����ļ�ϵͳ                */
        if (iFd >= 0) {
            close(iFd);
        }
        __SHEAP_FREE(pmnDev->MN_blkd.BLKD_pcName);
        __SHEAP_FREE(pmnDev);                                           /*  �ͷſ��ƿ�                  */
        return  (PX_ERROR);
    }
    
    __LW_MOUNT_LOCK();
    _List_Line_Add_Ahead(&pmnDev->MN_lineManage,
                         &_G_plineMountDevHeader);                      /*  ��������                    */
    __LW_MOUNT_UNLOCK();
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __unmount
** ��������: ж��һ������(�ڲ�����)
** �䡡��  : pcVolName         ����Ŀ��   ����: /mnt/usb
** �䡡��  : < 0 ��ʾʧ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __unmount (CPCHAR  pcVolName)
{
    INT                 iError;
    __PLW_MOUNT_NODE    pmnDev;
    PLW_LIST_LINE       plineTemp;
    CHAR                cVolNameBuffer[MAX_FILENAME_LENGTH];
    
    __mountInit();
    
    _PathGetFull(cVolNameBuffer, MAX_FILENAME_LENGTH, pcVolName);
    
    pcVolName = cVolNameBuffer;                                         /*  ʹ�þ���·��                */
    
    __LW_MOUNT_LOCK();
    for (plineTemp  = _G_plineMountDevHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
        
        pmnDev = _LIST_ENTRY(plineTemp, __LW_MOUNT_NODE, MN_lineManage);
        if (lib_strcmp(pmnDev->MN_cVolName, pcVolName) == 0) {
            break;
        }
    }
    if (plineTemp == LW_NULL) {                                         /*  û���ҵ�                    */
        INT iError = PX_ERROR;
        
        if (API_IosDevMatchFull(pcVolName)) {                           /*  ������豸, �����ж���豸  */
            iError = unlink(pcVolName);
            __LW_MOUNT_UNLOCK();
            return  (iError);
        }
        
        __LW_MOUNT_UNLOCK();
        _ErrorHandle(ENOENT);
        return  (PX_ERROR);
    
    } else {
        iError = unlink(pmnDev->MN_cVolName);                           /*  ж�ؾ�                      */
        if (iError < 0) {
            if (errno != ENOENT) {                                      /*  ������Ǳ�ж�ع���          */
                __LW_MOUNT_UNLOCK();
                return  (PX_ERROR);                                     /*  ж��ʧ��                    */
            }
        }
        if (pmnDev->MN_iFd >= 0) {
            close(pmnDev->MN_iFd);                                      /*  �رտ��豸                  */
        }
        _List_Line_Del(&pmnDev->MN_lineManage,
                       &_G_plineMountDevHeader);                        /*  �˳���������                */
    }
    __LW_MOUNT_UNLOCK();
    
    __SHEAP_FREE(pmnDev->MN_blkd.BLKD_pcName);
    __SHEAP_FREE(pmnDev);                                               /*  �ͷſ��ƿ�                  */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_MountEx
** ��������: ����һ������
** �䡡��  : pcDevName         ���豸��   ����: /dev/sda1
**           pcVolName         ����Ŀ��   ����: /mnt/usb (����ʹ�����·��, �����޷�ж��)
**           pcFileSystem      �ļ�ϵͳ��ʽ "vfat" "iso9660" "ntfs" "nfs" "romfs" "ramfs" ... 
                               NULL ��ʾʹ��Ĭ���ļ�ϵͳ
**           pcOption          ѡ��, ��ǰ֧�� ro ���� rw
** �䡡��  : < 0 ��ʾʧ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_MountEx (CPCHAR  pcDevName, CPCHAR  pcVolName, CPCHAR  pcFileSystem, CPCHAR  pcOption)
{
    INT     iRet;
    
    __KERNEL_SPACE_ENTER();
    iRet = __mount(pcDevName, pcVolName, pcFileSystem, pcOption);
    __KERNEL_SPACE_EXIT();
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_Mount
** ��������: ����һ������
** �䡡��  : pcDevName         ���豸��   ����: /dev/sda1
**           pcVolName         ����Ŀ��   ����: /mnt/usb (����ʹ�����·��, �����޷�ж��)
**           pcFileSystem      �ļ�ϵͳ��ʽ "vfat" "iso9660" "ntfs" "nfs" "romfs" "ramfs" .. 
                               NULL ��ʾʹ��Ĭ���ļ�ϵͳ
** �䡡��  : < 0 ��ʾʧ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_Mount (CPCHAR  pcDevName, CPCHAR  pcVolName, CPCHAR  pcFileSystem)
{
    INT     iRet;
    
    __KERNEL_SPACE_ENTER();
    iRet = __mount(pcDevName, pcVolName, pcFileSystem, LW_NULL);
    __KERNEL_SPACE_EXIT();
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_Unmount
** ��������: ж��һ������
** �䡡��  : pcVolName         ����Ŀ��   ����: /mnt/usb
** �䡡��  : < 0 ��ʾʧ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_Unmount (CPCHAR  pcVolName)
{
    INT     iRet;
    
    __KERNEL_SPACE_ENTER();
    iRet = __unmount(pcVolName);
    __KERNEL_SPACE_EXIT();
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_MountShow
** ��������: ��ʾ��ǰ���ص���Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
VOID  API_MountShow (VOID)
{
    PCHAR               pcMountInfoHdr = "       VOLUME                    BLK NAME\n"
                                         "-------------------- --------------------------------\n";
    __PLW_MOUNT_NODE    pmnDev;
    PLW_LIST_LINE       plineTemp;
    
    CHAR                cBlkNameBuffer[MAX_FILENAME_LENGTH];
    PCHAR               pcBlkName;

    if (LW_CPU_GET_CUR_NESTING()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return;
    }
    
    __mountInit();
    
    printf("all mount point show >>\n");
    printf(pcMountInfoHdr);                                             /*  ��ӡ��ӭ��Ϣ                */
    
    __LW_MOUNT_LOCK();
    for (plineTemp  = _G_plineMountDevHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
        
        pmnDev = _LIST_ENTRY(plineTemp, __LW_MOUNT_NODE, MN_lineManage);
        
        if (pmnDev->MN_blkd.BLKD_pcName) {
            pcBlkName = pmnDev->MN_blkd.BLKD_pcName;
        } else {
            INT     iRet;
            __KERNEL_SPACE_ENTER();                                     /*  ���ļ������������ں�        */
            iRet = API_IosFdGetName(pmnDev->MN_iFd, cBlkNameBuffer, MAX_FILENAME_LENGTH);
            __KERNEL_SPACE_EXIT();
            if (iRet < ERROR_NONE) {
                pcBlkName = "<unknown>";
            } else {
                pcBlkName = cBlkNameBuffer;
            }
        }
        printf("%-20s %-32s\n", pmnDev->MN_cVolName, pcBlkName);
    }
    __LW_MOUNT_UNLOCK();
}
/*********************************************************************************************************
** ��������: mount
** ��������: linux ���� mount.
** �䡡��  : pcDevName     �豸��
**           pcVolName     ����Ŀ��
**           pcFileSystem  �ļ�ϵͳ
**           ulFlag        ���ز���
**           pvData        ������Ϣ(δʹ��)
** �䡡��  : ���ؽ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  mount (CPCHAR  pcDevName, CPCHAR  pcVolName, CPCHAR  pcFileSystem, 
            ULONG   ulFlag, CPVOID pvData)
{
    PCHAR   pcOption = "rw";

    if (ulFlag & MS_RDONLY) {
        pcOption = "ro";
    }
    
    (VOID)pvData;
    
    return  (API_MountEx(pcDevName, pcVolName, pcFileSystem, pcOption));
}
/*********************************************************************************************************
** ��������: umount
** ��������: linux ���� umount.
** �䡡��  : pcVolName     ���ؽڵ�
** �䡡��  : ������ؽ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  umount (CPCHAR  pcVolName)
{
    return  (API_Unmount(pcVolName));
}
/*********************************************************************************************************
** ��������: umount2
** ��������: linux ���� umount2.
** �䡡��  : pcVolName     ���ؽڵ�
**           iFlag         MNT_FORCE ��ʾ�������
** �䡡��  : ������ؽ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  umount2 (CPCHAR  pcVolName, INT iFlag)
{
    INT iFd;

    if (iFlag & MNT_FORCE) {
        iFd = open(pcVolName, O_RDONLY);
        if (iFd >= 0) {
            ioctl(iFd, FIOSETFORCEDEL, LW_TRUE);                        /*  ��������ж���豸            */
            close(iFd);
        }
    }
    
    return  (API_Unmount(pcVolName));
}
#endif                                                                  /*  LW_CFG_MAX_VOLUMES > 0      */
                                                                        /*  LW_CFG_MOUNT_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
