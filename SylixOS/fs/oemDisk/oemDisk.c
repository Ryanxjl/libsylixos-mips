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
** ��   ��   ��: oemDisk.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 03 �� 24 ��
**
** ��        ��: OEM �Զ����̹���. 
                 ���ڶ�������̹���, ����, ж��, �ڴ���շ���ʹ�� API ����, ��������, ���ｫ��Щ������װ
                 Ϊһ�� OEM ���̲�����, ����ʹ��.
                 ע��. oemDisk ������ hotplug ��Ϣ�߳��д��л��Ĺ���.
                 
** BUG:
2009.03.25  ���Ӷ��������̵ĵ�Դ����.
2009.11.09  ���ݴ��̷�����ͬ���ļ�ϵͳ����, װ�ز�ͬ�ļ�ϵͳ.
2009.12.01  ���޷�������ʱ, Ĭ��ʹ�� FAT ����.
2009.12.14  ȱ���ڴ�ʱ, ��ӡ����.
2011.03.29  mount ʱ���Զ������������ͻ�ľ�.
2012.09.01  ʹ�� API_IosDevMatchFull() Ԥ���жϾ����ͻ.
            ͬʱ��¼ oemDisk ���� mount ���豸ͷ, ����ȷ��ж�صİ�ȫ.
2013.10.02  ���� API_OemDiskGetPath ��ȡ mount ����豸·��.
2013.10.03  ���� API_OemDiskHotplugEventMessage �����Ȳ����Ϣ.
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
#if (LW_CFG_MAX_VOLUMES > 0) && (LW_CFG_DISKCACHE_EN > 0) && (LW_CFG_FATFS_EN > 0)
/*********************************************************************************************************
  ��׺�ַ������� (һ�����̲��ɳ��� 999 ������)
*********************************************************************************************************/
#if LW_CFG_MAX_DISKPARTS > 9
#define __OEM_DISK_TAIL_LEN             2                               /*  ��� 2 �ֽڱ��             */
#elif LW_CFG_MAX_DISKPARTS > 99
#define __OEM_DISK_TAIL_LEN             3                               /*  ��� 3 �ֽڱ��             */
#else
#define __OEM_DISK_TAIL_LEN             1                               /*  ��� 1 �ֽڱ��             */
#endif
/*********************************************************************************************************
** ��������: __oemDiskPartFree
** ��������: �ͷ� OEM ���̿��ƿ�ķ�����Ϣ�ڴ�
** �䡡��  : poemd             ���̿��ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __oemDiskPartFree (PLW_OEMDISK_CB  poemd)
{
    REGISTER INT     i;
    
    for (i = 0; i < (INT)poemd->OEMDISK_uiNPart; i++) {
        if (poemd->OEMDISK_pblkdPart[i]) {
            API_DiskPartitionFree(poemd->OEMDISK_pblkdPart[i]);
        }
    }
}
/*********************************************************************************************************
** ��������: __oemDiskForceDeleteEn
** ��������: OEM ����ǿ��ɾ��
** �䡡��  : poemd             ���̿��ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID __oemDiskForceDeleteEn (CPCHAR  pcVolName)
{
    INT  iFd = open(pcVolName, O_RDONLY);
    
    if (iFd >= 0) {
        ioctl(iFd, FIOSETFORCEDEL, LW_TRUE);
        close(iFd);
    }
}
/*********************************************************************************************************
** ��������: __oemDiskForceDeleteDis
** ��������: OEM ���̷�ǿ��ɾ��
** �䡡��  : poemd             ���̿��ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID __oemDiskForceDeleteDis (CPCHAR  pcVolName)
{
    INT  iFd = open(pcVolName, O_RDONLY);
    
    if (iFd >= 0) {
        ioctl(iFd, FIOSETFORCEDEL, LW_FALSE);
        close(iFd);
    }
}
/*********************************************************************************************************
** ��������: API_OemDiskMountEx
** ��������: �Զ�����һ�����̵����з���. ����ʹ��ָ�����ļ�ϵͳ���͹���
** �䡡��  : pcVolName          ���ڵ����� (��ǰ API �����ݷ��������ĩβ��������)
**           pblkdDisk          �������̿��ƿ� (������ֱ�Ӳ�����������)
**           pvDiskCacheMem     ���� CACHE ���������ڴ���ʼ��ַ  (Ϊ���ʾ��̬������̻���)
**           stMemSize          ���� CACHE ��������С            (Ϊ���ʾ����Ҫ DISK CACHE)
**           iMaxBurstSector    ����⧷���д�����������
**           pcFsName           �ļ�ϵͳ����, ����: "vfat" "iso9660" "ntfs" ...
**           bForceFsType       �Ƿ�ǿ��ʹ��ָ�����ļ�ϵͳ����
** �䡡��  : OEM ���̿��ƿ�
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���ص��ļ�ϵͳ������ yaffs �ļ�ϵͳ, yaffs ���ھ�̬�ļ�ϵͳ.
                                           API ����
*********************************************************************************************************/
LW_API 
PLW_OEMDISK_CB  API_OemDiskMountEx (CPCHAR        pcVolName,
                                    PLW_BLK_DEV   pblkdDisk,
                                    PVOID         pvDiskCacheMem, 
                                    size_t        stMemSize, 
                                    INT           iMaxBurstSector,
                                    CPCHAR        pcFsName,
                                    BOOL          bForceFsType)
{
             INT            i;
             INT            iErrLevel = 0;
             
    REGISTER ULONG          ulError;
             CHAR           cFullVolName[MAX_FILENAME_LENGTH];          /*  ����������                  */
             
             INT            iVolSeq;
    REGISTER INT            iNPart;
             DISKPART_TABLE dptPart;                                    /*  ������                      */
             PLW_OEMDISK_CB poemd;
             
             FUNCPTR        pfuncFsCreate = __fsCreateFuncGet(pcFsName);/*  �ļ�ϵͳ��������            */
    
    if (pfuncFsCreate == LW_NULL) {
        _ErrorHandle(ERROR_IO_NO_DRIVER);                               /*  û���ļ�ϵͳ����            */
        return  (LW_NULL);
    }
    
    /*
     *  ���ؽڵ������
     */
    if (pcVolName == LW_NULL || *pcVolName != PX_ROOT) {                /*  ���ִ���                    */
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    i = (INT)lib_strnlen(pcVolName, (PATH_MAX - __OEM_DISK_TAIL_LEN));
    if (i >= (PATH_MAX - __OEM_DISK_TAIL_LEN)) {                        /*  ���ֹ���                    */
        _ErrorHandle(ERROR_IO_NAME_TOO_LONG);
        return  (LW_NULL);
    }
    
    /*
     *  ���� OEM ���̿��ƿ��ڴ�
     */
    poemd = (PLW_OEMDISK_CB)__SHEAP_ALLOC(sizeof(LW_OEMDISK_CB) + (size_t)i);
    if (poemd == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (LW_NULL);
    }
    lib_bzero(poemd, sizeof(LW_OEMDISK_CB) + i);                        /*  ���                        */
    
    poemd->OEMDISK_pblkdDisk = pblkdDisk;
    
    /* 
     *  ������̻����ڴ�
     */
    if ((pvDiskCacheMem == LW_NULL) && (stMemSize > 0)) {               /*  �Ƿ���Ҫ��̬������̻���    */
        poemd->OEMDISK_pvCache = __SHEAP_ALLOC(stMemSize);
        if (poemd->OEMDISK_pvCache == LW_NULL) {
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);                      /*  ϵͳȱ���ڴ�                */
            goto    __error_handle;
        }
        pvDiskCacheMem = poemd->OEMDISK_pvCache;
    }
    
    /*
     *  �����������̻���, ͬʱ���ʼ������
     */
    if (stMemSize) {
        ulError = API_DiskCacheCreate(pblkdDisk, 
                                      pvDiskCacheMem, 
                                      stMemSize,
                                      iMaxBurstSector, 
                                      &poemd->OEMDISK_pblkdCache);
        if (ulError) {
            iErrLevel = 1;
            goto    __error_handle;
        }
    } else {
        poemd->OEMDISK_pblkdCache = pblkdDisk;                          /*  ����Ҫ���̻���              */
    }
    
    /*
     *  ɨ��������з�����Ϣ
     */
    iNPart = API_DiskPartitionScan(poemd->OEMDISK_pblkdCache, 
                                   &dptPart);                           /*  ɨ�������                  */
    if (iNPart < 1) {
        iErrLevel = 2;
        goto    __error_handle;
    }
    poemd->OEMDISK_uiNPart = (UINT)iNPart;                              /*  ��¼��������                */
    
    /*
     *  ��ʼ�����еķ�������ʧ��
     */
    for (i = 0; i < iNPart; i++) {
        poemd->OEMDISK_iVolSeq[i] = PX_ERROR;                           /*  Ĭ��Ϊ����ʧ��              */
    }
    
    /*
     *  �ֱ���ظ�������
     */
    iVolSeq = 0;
    for (i = 0; i < iNPart; i++) {                                      /*  װ�ظ�������                */
        if (API_DiskPartitionGet(&dptPart, i, 
                                 &poemd->OEMDISK_pblkdPart[i]) < 0) {   /*  ��÷��� logic device       */
            break;
        }
        
__refined_seq:
        sprintf(cFullVolName, "%s%d", pcVolName, iVolSeq);              /*  �����������                */
        
        switch (dptPart.DPT_dpoLogic[i].DPO_dpnEntry.DPN_ucPartType) {  /*  �ж��ļ�ϵͳ��������        */
            
        case LW_DISK_PART_TYPE_FAT12:                                   /*  FAT �ļ�ϵͳ����            */
        case LW_DISK_PART_TYPE_FAT16:
        case LW_DISK_PART_TYPE_FAT16_BIG:
        case LW_DISK_PART_TYPE_WIN95_FAT32:
        case LW_DISK_PART_TYPE_WIN95_FAT32LBA:
        case LW_DISK_PART_TYPE_WIN95_FAT16LBA:
            if (API_IosDevMatchFull(cFullVolName)) {                    /*  �豸������Ԥ��              */
                iVolSeq++;
                goto    __refined_seq;                                  /*  ����ȷ�������              */
            }
            if (bForceFsType) {                                         /*  �Ƿ�ǿ��ָ���ļ�ϵͳ����    */
                if (pfuncFsCreate(cFullVolName, 
                                  poemd->OEMDISK_pblkdPart[i]) < 0) {   /*  �����ļ�ϵͳ                */
                    if (API_GetLastError() == ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                        iVolSeq++;
                        goto    __refined_seq;                          /*  ����ȷ�������              */
                    } else {
                        goto    __mount_over;                           /*  ����ʧ��                    */
                    }
                }
            } else {
                if (API_FatFsDevCreate(cFullVolName,                    /*  ���� FAT �ļ�ϵͳ           */
                                       poemd->OEMDISK_pblkdPart[i]) < 0) {
                    if (API_GetLastError() == ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                        iVolSeq++;
                        goto    __refined_seq;                          /*  ����ȷ�������              */
                    } else {
                        goto    __mount_over;                           /*  ����ʧ��                    */
                    }
                }
            }
            poemd->OEMDISK_pdevhdr[i] = API_IosDevMatchFull(cFullVolName);
            poemd->OEMDISK_iVolSeq[i] = iVolSeq;                        /*  ��¼�����                  */
            break;
        
        default:                                                        /*  Ĭ��ʹ��ָ���ļ�ϵͳ����    */
            if (pfuncFsCreate(cFullVolName, 
                              poemd->OEMDISK_pblkdPart[i]) < 0) {       /*  �����ļ�ϵͳ                */
                if (API_GetLastError() == ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                    iVolSeq++;
                    goto    __refined_seq;                              /*  ����ȷ�������              */
                } else {
                    goto    __mount_over;                               /*  ����ʧ��                    */
                }
            }
            poemd->OEMDISK_iVolSeq[i] = iVolSeq;                        /*  ��¼�����                  */
            break;
        }
        
        if (poemd->OEMDISK_iVolSeq[i] >= 0) {
            __oemDiskForceDeleteEn(cFullVolName);                       /*  Ĭ��Ϊǿ��ɾ��              */
        }
        
        iVolSeq++;                                                      /*  �Ѵ����굱ǰ��              */
    }

__mount_over:                                                           /*  ���з����������            */
    if (i == 0) {                                                       /*  һ��������û�й��سɹ� ?    */
        iErrLevel = 3;
        goto    __error_handle;
    }
    lib_strcpy(poemd->OEMDISK_cVolName, pcVolName);                     /*  ��������                    */
    
    return  (poemd);
    
__error_handle:
    if (iErrLevel > 2) {
        __oemDiskPartFree(poemd);
    }
    if (iErrLevel > 1) {
        if (poemd->OEMDISK_pblkdCache != pblkdDisk) {
            API_DiskCacheDelete(poemd->OEMDISK_pblkdCache);
        }
    }
    if (iErrLevel > 0) {
        if (poemd->OEMDISK_pvCache) {
            __SHEAP_FREE(poemd->OEMDISK_pvCache);
        }
    }
    __SHEAP_FREE(poemd);
    
    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: API_OemDiskMount
** ��������: �Զ�����һ�����̵����з���. ���޷�ʶ�����ʱ, ʹ�� FAT ��ʽ����.
** �䡡��  : pcVolName          ���ڵ����� (��ǰ API �����ݷ��������ĩβ��������)
**           pblkdDisk          �������̿��ƿ� (������ֱ�Ӳ�����������)
**           pvDiskCacheMem     ���� CACHE ���������ڴ���ʼ��ַ  (Ϊ���ʾ��̬������̻���)
**           stMemSize          ���� CACHE ��������С            (Ϊ���ʾ����Ҫ DISK CACHE)
**           iMaxBurstSector    ����⧷���д�����������
** �䡡��  : OEM ���̿��ƿ�
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
PLW_OEMDISK_CB  API_OemDiskMount (CPCHAR        pcVolName,
                                  PLW_BLK_DEV   pblkdDisk,
                                  PVOID         pvDiskCacheMem, 
                                  size_t        stMemSize, 
                                  INT           iMaxBurstSector)
{
             INT            i;
             INT            iErrLevel = 0;
             
    REGISTER ULONG          ulError;
             CHAR           cFullVolName[MAX_FILENAME_LENGTH];          /*  ����������                  */
             
             INT            iVolSeq;
    REGISTER INT            iNPart;
             DISKPART_TABLE dptPart;                                    /*  ������                      */
             PLW_OEMDISK_CB poemd;
    
    /*
     *  ���ؽڵ������
     */
    if (pcVolName == LW_NULL || *pcVolName != PX_ROOT) {                /*  ���ִ���                    */
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    i = (INT)lib_strnlen(pcVolName, (PATH_MAX - __OEM_DISK_TAIL_LEN));
    if (i >= (PATH_MAX - __OEM_DISK_TAIL_LEN)) {                        /*  ���ֹ���                    */
        _ErrorHandle(ERROR_IO_NAME_TOO_LONG);
        return  (LW_NULL);
    }
    
    /*
     *  ���� OEM ���̿��ƿ��ڴ�
     */
    poemd = (PLW_OEMDISK_CB)__SHEAP_ALLOC(sizeof(LW_OEMDISK_CB) + (size_t)i);
    if (poemd == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (LW_NULL);
    }
    lib_bzero(poemd, sizeof(LW_OEMDISK_CB) + i);                        /*  ���                        */
    
    poemd->OEMDISK_pblkdDisk = pblkdDisk;
    
    /* 
     *  ������̻����ڴ�
     */
    if ((pvDiskCacheMem == LW_NULL) && (stMemSize > 0)) {               /*  �Ƿ���Ҫ��̬������̻���    */
        poemd->OEMDISK_pvCache = __SHEAP_ALLOC(stMemSize);
        if (poemd->OEMDISK_pvCache == LW_NULL) {
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);                      /*  ϵͳȱ���ڴ�                */
            goto    __error_handle;
        }
        pvDiskCacheMem = poemd->OEMDISK_pvCache;
    }
    
    /*
     *  �����������̻���, ͬʱ���ʼ������
     */
    if (stMemSize) {
        ulError = API_DiskCacheCreate(pblkdDisk, 
                                      pvDiskCacheMem, 
                                      stMemSize,
                                      iMaxBurstSector, 
                                      &poemd->OEMDISK_pblkdCache);
        if (ulError) {
            iErrLevel = 1;
            goto    __error_handle;
        }
    } else {
        poemd->OEMDISK_pblkdCache = pblkdDisk;                          /*  ����Ҫ���̻���              */
    }
    
    /*
     *  ɨ��������з�����Ϣ
     */
    iNPart = API_DiskPartitionScan(poemd->OEMDISK_pblkdCache, 
                                   &dptPart);                           /*  ɨ�������                  */
    if (iNPart < 1) {
        iErrLevel = 2;
        goto    __error_handle;
    }
    poemd->OEMDISK_uiNPart = (UINT)iNPart;                              /*  ��¼��������                */
    
    /*
     *  ��ʼ�����еķ�������ʧ��
     */
    for (i = 0; i < iNPart; i++) {
        poemd->OEMDISK_iVolSeq[i] = PX_ERROR;                           /*  Ĭ��Ϊ����ʧ��              */
    }
    
    /*
     *  �ֱ���ظ�������
     */
    iVolSeq = 0;
    for (i = 0; i < iNPart; i++) {                                      /*  װ�ظ�������                */
        if (API_DiskPartitionGet(&dptPart, i, 
                                 &poemd->OEMDISK_pblkdPart[i]) < 0) {   /*  ��÷��� logic device       */
            break;
        }
        
__refined_seq:
        sprintf(cFullVolName, "%s%d", pcVolName, iVolSeq);              /*  �����������                */
        
        switch (dptPart.DPT_dpoLogic[i].DPO_dpnEntry.DPN_ucPartType) {  /*  �ж��ļ�ϵͳ��������        */
        
        case LW_DISK_PART_TYPE_EMPTY:                                   /*  Ĭ��ʹ�� FAT ����           */
        
        case LW_DISK_PART_TYPE_FAT12:                                   /*  FAT �ļ�ϵͳ����            */
        case LW_DISK_PART_TYPE_FAT16:
        case LW_DISK_PART_TYPE_FAT16_BIG:
        case LW_DISK_PART_TYPE_WIN95_FAT32:
        case LW_DISK_PART_TYPE_WIN95_FAT32LBA:
        case LW_DISK_PART_TYPE_WIN95_FAT16LBA:
            if (API_IosDevMatchFull(cFullVolName)) {                    /*  �豸������Ԥ��              */
                iVolSeq++;
                goto    __refined_seq;                                  /*  ����ȷ�������              */
            }
            if (API_FatFsDevCreate(cFullVolName, 
                                   poemd->OEMDISK_pblkdPart[i]) < 0) {  /*  ���� FAT �ļ�ϵͳ           */
                if (API_GetLastError() == ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                    iVolSeq++;
                    goto    __refined_seq;                              /*  ����ȷ�������              */
                } else {
                    goto    __mount_over;                               /*  ����ʧ��                    */
                }
            }
            poemd->OEMDISK_pdevhdr[i] = API_IosDevMatchFull(cFullVolName);
            poemd->OEMDISK_iVolSeq[i] = iVolSeq;                        /*  ��¼�����                  */
            break;
            
        case LW_DISK_PART_TYPE_HPFS_NTFS:                               /*  NTFS �ļ�ϵͳ����           */
            break;
        
        default:
            break;
        }
        
        if (poemd->OEMDISK_iVolSeq[i] >= 0) {
            __oemDiskForceDeleteEn(cFullVolName);                       /*  Ĭ��Ϊǿ��ɾ��              */
        }
        
        iVolSeq++;                                                      /*  �Ѵ����굱ǰ��              */
    }

__mount_over:                                                           /*  ���з����������            */
    if (i == 0) {                                                       /*  һ��������û�й��سɹ� ?    */
        iErrLevel = 3;
        goto    __error_handle;
    }
    lib_strcpy(poemd->OEMDISK_cVolName, pcVolName);                     /*  ��������                    */
    
    return  (poemd);
    
__error_handle:
    if (iErrLevel > 2) {
        __oemDiskPartFree(poemd);
    }
    if (iErrLevel > 1) {
        if (poemd->OEMDISK_pblkdCache != pblkdDisk) {
            API_DiskCacheDelete(poemd->OEMDISK_pblkdCache);
        }
    }
    if (iErrLevel > 0) {
        if (poemd->OEMDISK_pvCache) {
            __SHEAP_FREE(poemd->OEMDISK_pvCache);
        }
    }
    __SHEAP_FREE(poemd);
    
    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: API_OemDiskUnmountEx
** ��������: �Զ�ж��һ������ OEM �����豸�����о���
** �䡡��  : poemd              OEM ���̿��ƿ�
**           bForce             ������ļ�ռ���Ƿ�ǿ��ж��
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_OemDiskUnmountEx (PLW_OEMDISK_CB  poemd, BOOL  bForce)
{
             INT            i;
             CHAR           cFullVolName[MAX_FILENAME_LENGTH];          /*  ����������                  */
             PLW_BLK_DEV    pblkdDisk;
    REGISTER INT            iNPart;
    
    if (poemd == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    iNPart = (INT)poemd->OEMDISK_uiNPart;                               /*  ��ȡ��������                */
    
    for (i = 0; i < iNPart; i++) {
        if (poemd->OEMDISK_iVolSeq[i] != PX_ERROR) {                    /*  û�й����ļ�ϵͳ            */
            sprintf(cFullVolName, "%s%d", 
                    poemd->OEMDISK_cVolName, 
                    poemd->OEMDISK_iVolSeq[i]);                         /*  �����������                */
            
            if (poemd->OEMDISK_pdevhdr[i] != API_IosDevMatchFull(cFullVolName)) {
                continue;                                               /*  ���Ǵ� oemDisk ���豸       */
            }
            
            if (bForce == LW_FALSE) {
                __oemDiskForceDeleteDis(cFullVolName);                  /*  ������ǿ�� umount ����      */
            }
            
            if (unlink(cFullVolName) == ERROR_NONE) {                   /*  ж��������ؾ�              */
                poemd->OEMDISK_iVolSeq[i] = PX_ERROR;
            
            } else {
                return  (PX_ERROR);                                     /*  �޷�ж�ؾ�                  */
            }
        }
    }
    
    __oemDiskPartFree(poemd);                                           /*  �ͷŷ�����Ϣ                */
    
    if (poemd->OEMDISK_pblkdCache != poemd->OEMDISK_pblkdDisk) {
        API_DiskCacheDelete(poemd->OEMDISK_pblkdCache);                 /*  �ͷ� CACHE �ڴ�             */
    }
    
    if (poemd->OEMDISK_pvCache) {
        __SHEAP_FREE(poemd->OEMDISK_pvCache);                           /*  �ͷŴ��̻����ڴ�            */
    }
    __SHEAP_FREE(poemd);                                                /*  �ͷ� OEM �����豸�ڴ�       */
    
    /*
     *  �������̵���
     */
    pblkdDisk = poemd->OEMDISK_pblkdDisk;
    if (pblkdDisk->BLKD_pfuncBlkIoctl) {
        pblkdDisk->BLKD_pfuncBlkIoctl(pblkdDisk, LW_BLKD_CTRL_POWER, LW_BLKD_POWER_OFF);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_OemDiskUnmount
** ��������: �Զ�ж��һ������ OEM �����豸�����о���
** �䡡��  : poemd              OEM ���̿��ƿ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_OemDiskUnmount (PLW_OEMDISK_CB  poemd)
{
    return  (API_OemDiskUnmountEx(poemd, LW_TRUE));
}
/*********************************************************************************************************
** ��������: API_OemDiskGetPath
** ��������: ��� OEM �����豸ָ���±�� mount ·����
** �䡡��  : poemd              OEM ���̿��ƿ�
**           iIndex             �±�, ��� 0 ������һ������, 1 �����ڶ�������...
**           pcPath             ����·����������
**           stSize             �����С (������ MAX_FILENAME_LENGTH)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_OemDiskGetPath (PLW_OEMDISK_CB  poemd, INT  iIndex, PCHAR  pcPath, size_t stSize)
{
    if (poemd == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (iIndex >= LW_CFG_MAX_DISKPARTS) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (poemd->OEMDISK_iVolSeq[iIndex] != PX_ERROR) {                   /*  û�й����ļ�ϵͳ            */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    bnprintf(pcPath, stSize, 0, "%s%d", 
             poemd->OEMDISK_cVolName, 
             poemd->OEMDISK_iVolSeq[iIndex]);
             
    if (poemd->OEMDISK_pdevhdr[iIndex] != API_IosDevMatchFull(pcPath)) {
        return  (PX_ERROR);
    }
             
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_OemDiskHotplugEventMessage
** ��������: ��ָ���� OEM mount ���ƿ����з���, ȫ������ hotplug ��Ϣ
** �䡡��  : poemd              OEM ���̿��ƿ�
**           iMsg               hotplug ��Ϣ����
**           bInsert            ���뻹�ǰγ�
**           uiArg0~3           ������Ϣ
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_HOTPLUG_EN > 0

LW_API 
INT  API_OemDiskHotplugEventMessage (PLW_OEMDISK_CB  poemd, 
                                     INT             iMsg, 
                                     BOOL            bInsert,
                                     UINT32          uiArg0,
                                     UINT32          uiArg1,
                                     UINT32          uiArg2,
                                     UINT32          uiArg3)
{
             CHAR   cFullVolName[MAX_FILENAME_LENGTH];                  /*  ����������                  */
    REGISTER INT    iNPart;
             INT    i;
    
    if (poemd == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    iNPart = (INT)poemd->OEMDISK_uiNPart;                               /*  ��ȡ��������                */
    
    for (i = 0; i < iNPart; i++) {
        if (poemd->OEMDISK_iVolSeq[i] != PX_ERROR) {                    /*  û�й����ļ�ϵͳ            */
            sprintf(cFullVolName, "%s%d", 
                    poemd->OEMDISK_cVolName, 
                    poemd->OEMDISK_iVolSeq[i]);                         /*  �����������                */
            
            API_HotplugEventMessage(iMsg, bInsert, cFullVolName, 
                                    uiArg0, uiArg1, uiArg2, uiArg3);    /*  �����Ȳ����Ϣ              */
        }
    }
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_HOTPLUG_EN > 0       */
#endif                                                                  /*  (LW_CFG_MAX_VOLUMES > 0)    */
                                                                        /*  (LW_CFG_DISKCACHE_EN > 0)   */
                                                                        /*  (LW_CFG_FATFS_EN > 0)       */
/*********************************************************************************************************
  END
*********************************************************************************************************/