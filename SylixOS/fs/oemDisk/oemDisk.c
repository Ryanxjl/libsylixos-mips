/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: oemDisk.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2009 年 03 月 24 日
**
** 描        述: OEM 自动磁盘管理. 
                 由于多分区磁盘挂载, 控制, 卸载, 内存回收方面使用 API 过多, 操作繁琐, 这里将这些操作封装
                 为一个 OEM 磁盘操作库, 方便使用.
                 注意. oemDisk 必须在 hotplug 消息线程中串行化的工作.
                 
** BUG:
2009.03.25  增加对物理磁盘的电源管理.
2009.11.09  根据磁盘分区不同的文件系统类型, 装载不同文件系统.
2009.12.01  当无分区存在时, 默认使用 FAT 类型.
2009.12.14  缺少内存时, 打印错误.
2011.03.29  mount 时将自动规避有命名冲突的卷.
2012.09.01  使用 API_IosDevMatchFull() 预先判断卷标冲突.
            同时记录 oemDisk 所有 mount 的设备头, 这样确保卸载的安全.
2013.10.02  加入 API_OemDiskGetPath 获取 mount 后的设备路径.
2013.10.03  加入 API_OemDiskHotplugEventMessage 发送热插拔信息.
2015.12.25  加入 tpsFs 支持.
2016.01.12  oemDisk 支持彻底脱离具体的文件系统调用.
2016.05.10  oemDisk 遇到无法挂载的分区时, vol id 不增加.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/fs/include/fs_fs.h"
#include "../SylixOS/fs/fsCommon/fsCommon.h"
/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if LW_CFG_OEMDISK_EN > 0
#include "oemBlkIo.h"
/*********************************************************************************************************
  blk io 前缀
*********************************************************************************************************/
#define LW_BLKIO_PERFIX                 "/dev/blk/"
/*********************************************************************************************************
  后缀字符串长度 (一个磁盘不可超过 999 个分区)
*********************************************************************************************************/
#if LW_CFG_MAX_DISKPARTS > 9
#define __OEM_DISK_TAIL_LEN             2                               /*  最多 2 字节编号             */
#elif LW_CFG_MAX_DISKPARTS > 99
#define __OEM_DISK_TAIL_LEN             3                               /*  最多 3 字节编号             */
#else
#define __OEM_DISK_TAIL_LEN             1                               /*  最多 1 字节编号             */
#endif
/*********************************************************************************************************
** 函数名称: __oemDiskPartFree
** 功能描述: 释放 OEM 磁盘控制块的分区信息内存
** 输　入  : poemd             磁盘控制块
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
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
** 函数名称: __oemDiskForceDeleteEn
** 功能描述: OEM 磁盘强制删除
** 输　入  : poemd             磁盘控制块
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
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
** 函数名称: __oemDiskForceDeleteDis
** 功能描述: OEM 磁盘非强制删除
** 输　入  : poemd             磁盘控制块
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
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
** 函数名称: API_OemDiskMountEx2
** 功能描述: 自动挂载一个磁盘的所有分区. 可以使用指定的文件系统类型挂载
** 输　入  : pcVolName          根节点名字 (当前 API 将根据分区情况在末尾加入数字)
**           pblkdDisk          物理磁盘控制块 (必须是直接操作物理磁盘)
**           pvDiskCacheMem     磁盘 CACHE 缓冲区的内存起始地址  (为零表示动态分配磁盘缓冲)
**           stMemSize          磁盘 CACHE 缓冲区大小            (为零表示不需要 DISK CACHE)
**           iMaxRBurstSector   磁盘猝发读的最大扇区数
**           iMaxWBurstSector   磁盘猝发写的最大扇区数
**           pcFsName           文件系统类型, 例如: "vfat" "tpsfs" "iso9660" "ntfs" ...
**           bForceFsType       是否强制使用指定的文件系统类型
** 输　出  : OEM 磁盘控制块
** 全局变量: 
** 调用模块: 
** 注  意  : 挂载的文件系统不包含 yaffs 文件系统, yaffs 属于静态文件系统.
                                           API 函数
*********************************************************************************************************/
LW_API 
PLW_OEMDISK_CB  API_OemDiskMountEx2 (CPCHAR        pcVolName,
                                     PLW_BLK_DEV   pblkdDisk,
                                     PVOID         pvDiskCacheMem, 
                                     size_t        stMemSize, 
                                     INT           iMaxRBurstSector,
                                     INT           iMaxWBurstSector,
                                     CPCHAR        pcFsName,
                                     BOOL          bForceFsType)
{
             INT            i;
             INT            iErrLevel = 0;
             
    REGISTER ULONG          ulError;
             CHAR           cFullVolName[MAX_FILENAME_LENGTH];          /*  完整卷标名                  */
             
             INT            iBlkIo;
             INT            iBlkIoErr;
             PCHAR          pcTail;

             INT            iVolSeq;
    REGISTER INT            iNPart;
             DISKPART_TABLE dptPart;                                    /*  分区表                      */
             PLW_OEMDISK_CB poemd;
             
             FUNCPTR        pfuncFsCreate;
    
    /*
     *  挂载节点名检查
     */
    if (pcVolName == LW_NULL || *pcVolName != PX_ROOT) {                /*  名字错误                    */
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    i = (INT)lib_strnlen(pcVolName, (PATH_MAX - __OEM_DISK_TAIL_LEN));
    if (i >= (PATH_MAX - __OEM_DISK_TAIL_LEN)) {                        /*  名字过长                    */
        _ErrorHandle(ERROR_IO_NAME_TOO_LONG);
        return  (LW_NULL);
    }
    
    /*
     *  分配 OEM 磁盘控制块内存
     */
    poemd = (PLW_OEMDISK_CB)__SHEAP_ALLOC(sizeof(LW_OEMDISK_CB) + (size_t)i);
    if (poemd == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (LW_NULL);
    }
    lib_bzero(poemd, sizeof(LW_OEMDISK_CB) + i);                        /*  清空                        */
    
    poemd->OEMDISK_pblkdDisk = pblkdDisk;
    
    /* 
     *  分配磁盘缓冲内存
     */
    if ((pvDiskCacheMem == LW_NULL) && (stMemSize > 0)) {               /*  是否需要动态分配磁盘缓冲    */
        poemd->OEMDISK_pvCache = __SHEAP_ALLOC(stMemSize);
        if (poemd->OEMDISK_pvCache == LW_NULL) {
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);                      /*  系统缺少内存                */
            goto    __error_handle;
        }
        pvDiskCacheMem = poemd->OEMDISK_pvCache;
    }
    
    /*
     *  创建物理磁盘缓冲, 同时会初始化磁盘
     */
    if (stMemSize) {
        ulError = API_DiskCacheCreateEx(pblkdDisk, 
                                        pvDiskCacheMem, 
                                        stMemSize,
                                        iMaxRBurstSector, 
                                        iMaxWBurstSector, 
                                        &poemd->OEMDISK_pblkdCache);
        if (ulError) {
            iErrLevel = 1;
            goto    __error_handle;
        }
    } else {
        poemd->OEMDISK_pblkdCache = pblkdDisk;                          /*  不需要磁盘缓冲              */
    }
    
    /*
     *  创建 blk io 设备
     */
    pcTail = lib_rindex(pcVolName, PX_DIVIDER);
    if (pcTail == LW_NULL) {
        pcTail =  (PCHAR)pcVolName;
    } else {
        pcTail++;
    }

    for (iBlkIo = 0; iBlkIo < 64; iBlkIo++) {
        snprintf(cFullVolName, MAX_FILENAME_LENGTH, "%s%s%d", LW_BLKIO_PERFIX, pcTail, iBlkIo);
        if (API_IosDevMatchFull(cFullVolName) == LW_NULL) {
            iBlkIoErr = API_OemBlkIoCreate(cFullVolName,
                                           poemd->OEMDISK_pblkdCache);
            if (iBlkIoErr == ERROR_NONE) {
                break;

            } else if (errno != ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                iErrLevel = 2;
                goto    __error_handle;
            }
        }
    }

    /*
     *  扫面磁盘所有分区信息
     */
    iNPart = API_DiskPartitionScan(poemd->OEMDISK_pblkdCache, 
                                   &dptPart);                           /*  扫描分区表                  */
    if (iNPart < 1) {
        iErrLevel = 3;
        goto    __error_handle;
    }
    poemd->OEMDISK_uiNPart = (UINT)iNPart;                              /*  记录分区数量                */
    
    /*
     *  初始化所有的分区挂载失败
     */
    for (i = 0; i < iNPart; i++) {
        poemd->OEMDISK_iVolSeq[i] = PX_ERROR;                           /*  默认为挂载失败              */
    }
    
    /*
     *  分别挂载各个分区
     */
    iVolSeq = 0;
    for (i = 0; i < iNPart; i++) {                                      /*  装载各个分区                */
        if (API_DiskPartitionGet(&dptPart, i, 
                                 &poemd->OEMDISK_pblkdPart[i]) < 0) {   /*  获得分区 logic device       */
            break;
        }
        
__refined_seq:
        sprintf(cFullVolName, "%s%d", pcVolName, iVolSeq);              /*  获得完整卷名                */
        if (API_IosDevMatchFull(cFullVolName)) {                        /*  设备名重名预判              */
            iVolSeq++;
            goto    __refined_seq;                                      /*  重新确定卷序号              */
        }
        
        pfuncFsCreate = LW_NULL;
        
        switch (dptPart.DPT_dpoLogic[i].DPO_dpnEntry.DPN_ucPartType) {  /*  判断文件系统分区类型        */
            
        case LW_DISK_PART_TYPE_FAT12:                                   /*  FAT 文件系统类型            */
        case LW_DISK_PART_TYPE_FAT16:
        case LW_DISK_PART_TYPE_FAT16_BIG:
        case LW_DISK_PART_TYPE_WIN95_FAT32:
        case LW_DISK_PART_TYPE_WIN95_FAT32LBA:
        case LW_DISK_PART_TYPE_WIN95_FAT16LBA:
            if (bForceFsType) {                                         /*  是否强制指定文件系统类型    */
                pfuncFsCreate = __fsCreateFuncGet(pcFsName);
            } else {
                pfuncFsCreate = __fsCreateFuncGet("vfat");
            }
            break;
            
        case LW_DISK_PART_TYPE_TPS:
            if (bForceFsType) {                                         /*  是否强制指定文件系统类型    */
                pfuncFsCreate = __fsCreateFuncGet(pcFsName);
            } else {
                pfuncFsCreate = __fsCreateFuncGet("tpsfs");
            }
            break;
        
        default:                                                        /*  默认使用指定文件系统类型    */
            if (bForceFsType) {                                         /*  是否强制指定文件系统类型    */
                pfuncFsCreate = __fsCreateFuncGet(pcFsName);
            }
            break;
        }
        
        if (pfuncFsCreate) {                                            /*  存在支持的文件系统          */
            if (pfuncFsCreate(cFullVolName, 
                              poemd->OEMDISK_pblkdPart[i]) < 0) {       /*  挂载文件系统                */
                if (API_GetLastError() == ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                    iVolSeq++;
                    goto    __refined_seq;                              /*  重新确定卷序号              */
                } else {
                    goto    __mount_over;                               /*  挂载失败                    */
                }
            }
            poemd->OEMDISK_pdevhdr[i] = API_IosDevMatchFull(cFullVolName);
            poemd->OEMDISK_iVolSeq[i] = iVolSeq;                        /*  记录卷序号                  */
        
        } else {
            continue;                                                   /*  此分区无法加载              */
        }
        
        if (poemd->OEMDISK_iVolSeq[i] >= 0) {
            __oemDiskForceDeleteEn(cFullVolName);                       /*  默认为强制删除              */
        }
        
        iVolSeq++;                                                      /*  已处理完当前卷              */
    }

__mount_over:                                                           /*  所有分区挂载完毕            */
    lib_strcpy(poemd->OEMDISK_cVolName, pcVolName);                     /*  拷贝名字                    */
    
    return  (poemd);
    
__error_handle:
    if (iErrLevel > 2) {
        if (poemd->OEMDISK_pblkdCache) {
            API_OemBlkIoDelete(poemd->OEMDISK_pblkdCache);
        }
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
** 函数名称: API_OemDiskMountEx
** 功能描述: 自动挂载一个磁盘的所有分区. 可以使用指定的文件系统类型挂载
** 输　入  : pcVolName          根节点名字 (当前 API 将根据分区情况在末尾加入数字)
**           pblkdDisk          物理磁盘控制块 (必须是直接操作物理磁盘)
**           pvDiskCacheMem     磁盘 CACHE 缓冲区的内存起始地址  (为零表示动态分配磁盘缓冲)
**           stMemSize          磁盘 CACHE 缓冲区大小            (为零表示不需要 DISK CACHE)
**           iMaxBurstSector    磁盘猝发读写的最大扇区数
**           pcFsName           文件系统类型, 例如: "vfat" "tpsfs" "iso9660" "ntfs" ...
**           bForceFsType       是否强制使用指定的文件系统类型
** 输　出  : OEM 磁盘控制块
** 全局变量: 
** 调用模块: 
** 注  意  : 挂载的文件系统不包含 yaffs 文件系统, yaffs 属于静态文件系统.
                                           API 函数
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
    INT   iMaxRBurstSector;
    
    if (iMaxBurstSector > 2) {
        iMaxRBurstSector = iMaxBurstSector >> 1;                        /*  读猝发长度默认被写少一半    */
    }
    
    return  (API_OemDiskMountEx2(pcVolName, pblkdDisk,
                                 pvDiskCacheMem, stMemSize, 
                                 iMaxRBurstSector, iMaxBurstSector,
                                 pcFsName, bForceFsType));
}
/*********************************************************************************************************
** 函数名称: API_OemDiskMount2
** 功能描述: 自动挂载一个磁盘的所有分区. 当无法识别分区时, 使用 FAT 格式挂载.
** 输　入  : pcVolName          根节点名字 (当前 API 将根据分区情况在末尾加入数字)
**           pblkdDisk          物理磁盘控制块 (必须是直接操作物理磁盘)
**           pvDiskCacheMem     磁盘 CACHE 缓冲区的内存起始地址  (为零表示动态分配磁盘缓冲)
**           stMemSize          磁盘 CACHE 缓冲区大小            (为零表示不需要 DISK CACHE)
**           iMaxRBurstSector   磁盘猝发读的最大扇区数
**           iMaxWBurstSector   磁盘猝发写的最大扇区数
** 输　出  : OEM 磁盘控制块
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
PLW_OEMDISK_CB  API_OemDiskMount2 (CPCHAR        pcVolName,
                                   PLW_BLK_DEV   pblkdDisk,
                                   PVOID         pvDiskCacheMem, 
                                   size_t        stMemSize, 
                                   INT           iMaxRBurstSector,
                                   INT           iMaxWBurstSector)
{
             INT            i;
             INT            iErrLevel = 0;
             
    REGISTER ULONG          ulError;
             CHAR           cFullVolName[MAX_FILENAME_LENGTH];          /*  完整卷标名                  */
             
             INT            iBlkIo;
             INT            iBlkIoErr;
             PCHAR          pcTail;

             INT            iVolSeq;
    REGISTER INT            iNPart;
             DISKPART_TABLE dptPart;                                    /*  分区表                      */
             PLW_OEMDISK_CB poemd;
    
             FUNCPTR        pfuncFsCreate;
             
    /*
     *  挂载节点名检查
     */
    if (pcVolName == LW_NULL || *pcVolName != PX_ROOT) {                /*  名字错误                    */
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    i = (INT)lib_strnlen(pcVolName, (PATH_MAX - __OEM_DISK_TAIL_LEN));
    if (i >= (PATH_MAX - __OEM_DISK_TAIL_LEN)) {                        /*  名字过长                    */
        _ErrorHandle(ERROR_IO_NAME_TOO_LONG);
        return  (LW_NULL);
    }
    
    /*
     *  分配 OEM 磁盘控制块内存
     */
    poemd = (PLW_OEMDISK_CB)__SHEAP_ALLOC(sizeof(LW_OEMDISK_CB) + (size_t)i);
    if (poemd == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (LW_NULL);
    }
    lib_bzero(poemd, sizeof(LW_OEMDISK_CB) + i);                        /*  清空                        */
    
    poemd->OEMDISK_pblkdDisk = pblkdDisk;
    
    /* 
     *  分配磁盘缓冲内存
     */
    if ((pvDiskCacheMem == LW_NULL) && (stMemSize > 0)) {               /*  是否需要动态分配磁盘缓冲    */
        poemd->OEMDISK_pvCache = __SHEAP_ALLOC(stMemSize);
        if (poemd->OEMDISK_pvCache == LW_NULL) {
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);                      /*  系统缺少内存                */
            goto    __error_handle;
        }
        pvDiskCacheMem = poemd->OEMDISK_pvCache;
    }
    
    /*
     *  创建物理磁盘缓冲, 同时会初始化磁盘
     */
    if (stMemSize) {
        ulError = API_DiskCacheCreateEx(pblkdDisk, 
                                        pvDiskCacheMem, 
                                        stMemSize,
                                        iMaxRBurstSector, 
                                        iMaxWBurstSector, 
                                        &poemd->OEMDISK_pblkdCache);
        if (ulError) {
            iErrLevel = 1;
            goto    __error_handle;
        }
    } else {
        poemd->OEMDISK_pblkdCache = pblkdDisk;                          /*  不需要磁盘缓冲              */
    }
    
    /*
     *  创建 blk io 设备
     */
    pcTail = lib_rindex(pcVolName, PX_DIVIDER);
    if (pcTail == LW_NULL) {
        pcTail =  (PCHAR)pcVolName;
    } else {
        pcTail++;
    }

    for (iBlkIo = 0; iBlkIo < 64; iBlkIo++) {
        snprintf(cFullVolName, MAX_FILENAME_LENGTH, "%s%s%d", LW_BLKIO_PERFIX, pcTail, iBlkIo);
        if (API_IosDevMatchFull(cFullVolName) == LW_NULL) {
            iBlkIoErr = API_OemBlkIoCreate(cFullVolName,
                                           poemd->OEMDISK_pblkdCache);
            if (iBlkIoErr == ERROR_NONE) {
                break;

            } else if (errno != ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                iErrLevel = 2;
                goto    __error_handle;
            }
        }
    }

    /*
     *  扫面磁盘所有分区信息
     */
    iNPart = API_DiskPartitionScan(poemd->OEMDISK_pblkdCache, 
                                   &dptPart);                           /*  扫描分区表                  */
    if (iNPart < 1) {
        iErrLevel = 3;
        goto    __error_handle;
    }
    poemd->OEMDISK_uiNPart = (UINT)iNPart;                              /*  记录分区数量                */
    
    /*
     *  初始化所有的分区挂载失败
     */
    for (i = 0; i < iNPart; i++) {
        poemd->OEMDISK_iVolSeq[i] = PX_ERROR;                           /*  默认为挂载失败              */
    }
    
    /*
     *  分别挂载各个分区
     */
    iVolSeq = 0;
    for (i = 0; i < iNPart; i++) {                                      /*  装载各个分区                */
        if (API_DiskPartitionGet(&dptPart, i, 
                                 &poemd->OEMDISK_pblkdPart[i]) < 0) {   /*  获得分区 logic device       */
            break;
        }
        
__refined_seq:
        sprintf(cFullVolName, "%s%d", pcVolName, iVolSeq);              /*  获得完整卷名                */
        if (API_IosDevMatchFull(cFullVolName)) {                        /*  设备名重名预判              */
            iVolSeq++;
            goto    __refined_seq;                                      /*  重新确定卷序号              */
        }
        
        pfuncFsCreate = LW_NULL;
        
        switch (dptPart.DPT_dpoLogic[i].DPO_dpnEntry.DPN_ucPartType) {  /*  判断文件系统分区类型        */
        
        case LW_DISK_PART_TYPE_FAT12:                                   /*  FAT 文件系统类型            */
        case LW_DISK_PART_TYPE_FAT16:
        case LW_DISK_PART_TYPE_FAT16_BIG:
        case LW_DISK_PART_TYPE_WIN95_FAT32:
        case LW_DISK_PART_TYPE_WIN95_FAT32LBA:
        case LW_DISK_PART_TYPE_WIN95_FAT16LBA:
            pfuncFsCreate = __fsCreateFuncGet("vfat");                  /*  查询 VFAT 文件系统装载函数  */
            break;
        
        case LW_DISK_PART_TYPE_TPS:                                     /*  TPS 文件系统类型            */
            pfuncFsCreate = __fsCreateFuncGet("tpsfs");                 /*  查询 TPSFS 文件系统装载函数 */
            break;
        
        default:
            break;
        }
        
        if (pfuncFsCreate) {                                            /*  存在支持的文件系统          */
            if (pfuncFsCreate(cFullVolName, 
                              poemd->OEMDISK_pblkdPart[i]) < 0) {       /*  挂载文件系统                */
                if (API_GetLastError() == ERROR_IOS_DUPLICATE_DEVICE_NAME) {
                    iVolSeq++;
                    goto    __refined_seq;                              /*  重新确定卷序号              */
                } else {
                    goto    __mount_over;                               /*  挂载失败                    */
                }
            }
            poemd->OEMDISK_pdevhdr[i] = API_IosDevMatchFull(cFullVolName);
            poemd->OEMDISK_iVolSeq[i] = iVolSeq;                        /*  记录卷序号                  */
        
        } else {
            continue;                                                   /*  此分区无法加载              */
        }
        
        if (poemd->OEMDISK_iVolSeq[i] >= 0) {
            __oemDiskForceDeleteEn(cFullVolName);                       /*  默认为强制删除              */
        }
        
        iVolSeq++;                                                      /*  已处理完当前卷              */
    }

__mount_over:                                                           /*  所有分区挂载完毕            */
    lib_strcpy(poemd->OEMDISK_cVolName, pcVolName);                     /*  拷贝名字                    */
    
    return  (poemd);
    
__error_handle:
    if (iErrLevel > 2) {
        if (poemd->OEMDISK_pblkdCache) {
            API_OemBlkIoDelete(poemd->OEMDISK_pblkdCache);
        }
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
** 函数名称: API_OemDiskMount2
** 功能描述: 自动挂载一个磁盘的所有分区. 当无法识别分区时, 使用 FAT 格式挂载.
** 输　入  : pcVolName          根节点名字 (当前 API 将根据分区情况在末尾加入数字)
**           pblkdDisk          物理磁盘控制块 (必须是直接操作物理磁盘)
**           pvDiskCacheMem     磁盘 CACHE 缓冲区的内存起始地址  (为零表示动态分配磁盘缓冲)
**           stMemSize          磁盘 CACHE 缓冲区大小            (为零表示不需要 DISK CACHE)
**           iMaxBurstSector    磁盘猝发读写的最大扇区数
** 输　出  : OEM 磁盘控制块
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
PLW_OEMDISK_CB  API_OemDiskMount (CPCHAR        pcVolName,
                                  PLW_BLK_DEV   pblkdDisk,
                                  PVOID         pvDiskCacheMem, 
                                  size_t        stMemSize, 
                                  INT           iMaxBurstSector)
{
    INT   iMaxRBurstSector;
    
    if (iMaxBurstSector > 2) {
        iMaxRBurstSector = iMaxBurstSector >> 1;                        /*  读猝发长度默认被写少一半    */
    }
    
    return  (API_OemDiskMount2(pcVolName, pblkdDisk,
                               pvDiskCacheMem, stMemSize, 
                               iMaxRBurstSector, iMaxBurstSector));
}
/*********************************************************************************************************
** 函数名称: API_OemDiskUnmountEx
** 功能描述: 自动卸载一个物理 OEM 磁盘设备的所有卷标
** 输　入  : poemd              OEM 磁盘控制块
**           bForce             如果有文件占用是否强制卸载
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
INT  API_OemDiskUnmountEx (PLW_OEMDISK_CB  poemd, BOOL  bForce)
{
             INT            i;
             CHAR           cFullVolName[MAX_FILENAME_LENGTH];          /*  完整卷标名                  */
             PLW_BLK_DEV    pblkdDisk;
    REGISTER INT            iNPart;
    
    if (poemd == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    iNPart = (INT)poemd->OEMDISK_uiNPart;                               /*  获取分区数量                */
    
    for (i = 0; i < iNPart; i++) {
        if (poemd->OEMDISK_iVolSeq[i] != PX_ERROR) {                    /*  没有挂载文件系统            */
            sprintf(cFullVolName, "%s%d", 
                    poemd->OEMDISK_cVolName, 
                    poemd->OEMDISK_iVolSeq[i]);                         /*  获得完整卷名                */
            
            if (poemd->OEMDISK_pdevhdr[i] != API_IosDevMatchFull(cFullVolName)) {
                continue;                                               /*  不是此 oemDisk 的设备       */
            }
            
            if (bForce == LW_FALSE) {
                __oemDiskForceDeleteDis(cFullVolName);                  /*  不允许强制 umount 操作      */
            }
            
            if (unlink(cFullVolName) == ERROR_NONE) {                   /*  卸载所有相关卷              */
                poemd->OEMDISK_iVolSeq[i] = PX_ERROR;
            
            } else {
                return  (PX_ERROR);                                     /*  无法卸载卷                  */
            }
        }
    }
    
    __oemDiskPartFree(poemd);                                           /*  释放分区信息                */
    
    if (poemd->OEMDISK_pblkdCache != poemd->OEMDISK_pblkdDisk) {
        API_DiskCacheDelete(poemd->OEMDISK_pblkdCache);                 /*  释放 CACHE 内存             */
    }
    
    API_OemBlkIoDelete(poemd->OEMDISK_pblkdCache);

    if (poemd->OEMDISK_pvCache) {
        __SHEAP_FREE(poemd->OEMDISK_pvCache);                           /*  释放磁盘缓冲内存            */
    }
    __SHEAP_FREE(poemd);                                                /*  释放 OEM 磁盘设备内存       */
    
    /*
     *  物理磁盘掉电
     */
    pblkdDisk = poemd->OEMDISK_pblkdDisk;
    if (pblkdDisk->BLKD_pfuncBlkIoctl) {
        pblkdDisk->BLKD_pfuncBlkIoctl(pblkdDisk, LW_BLKD_CTRL_POWER, LW_BLKD_POWER_OFF);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_OemDiskUnmount
** 功能描述: 自动卸载一个物理 OEM 磁盘设备的所有卷标
** 输　入  : poemd              OEM 磁盘控制块
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
INT  API_OemDiskUnmount (PLW_OEMDISK_CB  poemd)
{
    return  (API_OemDiskUnmountEx(poemd, LW_TRUE));
}
/*********************************************************************************************************
** 函数名称: API_OemDiskGetPath
** 功能描述: 获得 OEM 磁盘设备指定下标的 mount 路径名
** 输　入  : poemd              OEM 磁盘控制块
**           iIndex             下标, 如果 0 代表第一个磁盘, 1 代表第二个磁盘...
**           pcPath             挂载路径名缓冲区
**           stSize             缓冲大小 (至少有 MAX_FILENAME_LENGTH)
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
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
    
    if (poemd->OEMDISK_iVolSeq[iIndex] != PX_ERROR) {                   /*  没有挂载文件系统            */
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
** 函数名称: API_OemDiskHotplugEventMessage
** 功能描述: 将指定的 OEM mount 控制块所有分区, 全部发送 hotplug 信息
** 输　入  : poemd              OEM 磁盘控制块
**           iMsg               hotplug 消息类型
**           bInsert            插入还是拔出
**           uiArg0~3           附加消息
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
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
             CHAR   cFullVolName[MAX_FILENAME_LENGTH];                  /*  完整卷标名                  */
    REGISTER INT    iNPart;
             INT    i;
    
    if (poemd == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    iNPart = (INT)poemd->OEMDISK_uiNPart;                               /*  获取分区数量                */
    
    for (i = 0; i < iNPart; i++) {
        if (poemd->OEMDISK_iVolSeq[i] != PX_ERROR) {                    /*  没有挂载文件系统            */
            sprintf(cFullVolName, "%s%d", 
                    poemd->OEMDISK_cVolName, 
                    poemd->OEMDISK_iVolSeq[i]);                         /*  获得完整卷名                */
            
            API_HotplugEventMessage(iMsg, bInsert, cFullVolName, 
                                    uiArg0, uiArg1, uiArg2, uiArg3);    /*  发送热插拔信息              */
        }
    }
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_HOTPLUG_EN > 0       */
#endif                                                                  /*  LW_CFG_OEMDISK_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
