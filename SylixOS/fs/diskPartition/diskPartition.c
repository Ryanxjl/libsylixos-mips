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
** ��   ��   ��: diskPartition.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 03 �� 17 ��
**
** ��        ��: ���̷��������. (ע��: ����ʹ�� FAT �ļ�ϵͳ����).

** BUG:
2009.06.10  API_DiskPartitionScan() �ڷ���������ʧ��ʱ, �߼����̵� iNSector ��Ա��Ҫ��ֵΪ����������̵�
            ��С.
2009.06.19  �Ż�һ������ṹ.
2009.07.06  ������� Link ���������ڷ����������.
2009.11.09  ���������ͺ궨�����ͷ�ļ���.
2009.12.01  ���޷�����������ʱ, ��������������Ϊ LW_DISK_PART_TYPE_EMPTY ��־.
2011.11.21  �����ļ�ϵͳ, ������������ MBR_Table.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../fatFs/ff.h"
#include "diskPartition.h"
/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_MAX_VOLUMES > 0) && (LW_CFG_FATFS_EN > 0)
/*********************************************************************************************************
  һ��������̴�������߼������� BLK_DEV �ļ�ϵͳʾ���ṹ:

logic HDD FAT disk volume:
    +--------+          +--------+          +--------+          +--------+          +--------+
    | /hdd0  |          | /hdd1  |          | /hdd2  |          | /hdd3  |          | /hdd4  |
    |  FAT32 |          |  FAT32 |          |  FAT16 |          |  FAT32 |          |  NTFS  |
    +--------+          +--------+          +--------+          +--------+          +--------+
        |                   |                   |                   |                   |
        |                   |                   |                   |                   |
logic disk block device:    |                   |                   |                   |
    +--------+          +--------+          +--------+          +--------+          +--------+
    | BLK_DEV|          | BLK_DEV|          | BLK_DEV|          | BLK_DEV|          | BLK_DEV|
    +--------+          +--------+          +--------+          +--------+          +--------+
        \                   \                   |                  /                   /
         \___________________\__________________|_________________/___________________/
                                                |
                                                |
                                           +----------+
disk cache block device:                   |  dcache  |
                                           |  BLK_DEV |
                                           +----------+
                                                |
                                                |
                                           +----------+
physical disk block device:                | physical |
                                           |  BLK_DEV |
                                           +----------+
                                                |
                                                |
                                           +----------+
physical HDD:                              |   HDD    |
                                           +----------+
*********************************************************************************************************/
/*********************************************************************************************************
  �������־
*********************************************************************************************************/
#define __DISK_PART_ACTIVE                  0x80                        /*  ���������                */
#define __DISK_PART_UNACTIVE                0x00                        /*  �ǻ����                  */
/*********************************************************************************************************
  ����������Ҫ��Ϣ�ص�
*********************************************************************************************************/
#define __DISK_PART_TYPE                    0x4
#define __DISK_PART_STARTSECTOR             0x8
#define __DISK_PART_NSECTOR                 0xc
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT   __blockIoDevCreate(PLW_BLK_DEV  pblkdNew);
VOID  __blockIoDevDelete(INT  iIndex);
PLW_BLK_DEV  __blockIoDevGet(INT  iIndex);
INT   __blockIoDevReset(INT  iIndex);
INT   __blockIoDevIoctl(INT  iIndex, INT  iCmd, LONG  lArg);
INT   __blockIoDevRead(INT     iIndex, 
                       VOID   *pvBuffer, 
                       ULONG   ulStartSector, 
                       ULONG   ulSectorCount);
INT   __blockIoDevWrite(INT     iIndex, 
                        VOID   *pvBuffer, 
                        ULONG   ulStartSector, 
                        ULONG   ulSectorCount);
/*********************************************************************************************************
** ��������: __logicDiskWrt
** ��������: д�������߼�����
** �䡡��  : pdpoLogic         �߼����̿��ƿ�
**           pvBuffer          ������
**           ulStartSector     ��ʼ������
**           ulSectorCount     ��������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __logicDiskWrt (LW_DISKPART_OPERAT    *pdpoLogic, 
                            VOID                  *pvBuffer, 
                            ULONG                  ulStartSector, 
                            ULONG                  ulSectorCount)
{
    ulStartSector += pdpoLogic->DPO_dpnEntry.DPN_ulStartSector;         /*  �߼�����ƫ����              */
    
    return  (pdpoLogic->DPT_pblkdDisk->BLKD_pfuncBlkWrt(pdpoLogic->DPT_pblkdDisk, 
                                                        pvBuffer,
                                                        ulStartSector,
                                                        ulSectorCount));
}
/*********************************************************************************************************
** ��������: __logicDiskRd
** ��������: ���������߼�����
** �䡡��  : pdpoLogic         �߼����̿��ƿ�
**           pvBuffer          ������
**           ulStartSector     ��ʼ������
**           ulSectorCount     ��������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __logicDiskRd (LW_DISKPART_OPERAT    *pdpoLogic,
                           VOID                  *pvBuffer, 
                           ULONG                  ulStartSector, 
                           ULONG                  ulSectorCount)
{
    ulStartSector += pdpoLogic->DPO_dpnEntry.DPN_ulStartSector;         /*  �߼�����ƫ����              */
    
    return  (pdpoLogic->DPT_pblkdDisk->BLKD_pfuncBlkRd(pdpoLogic->DPT_pblkdDisk, 
                                                       pvBuffer,
                                                       ulStartSector,
                                                       ulSectorCount));
}
/*********************************************************************************************************
** ��������: __logicDiskIoctl
** ��������: ���Ƶ������߼�����
** �䡡��  : pdpoLogic         �߼����̿��ƿ�
**           iCmd              ��������
**           lArg              ���Ʋ���
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __logicDiskIoctl (LW_DISKPART_OPERAT    *pdpoLogic, INT  iCmd, LONG  lArg)
{
    return  (pdpoLogic->DPT_pblkdDisk->BLKD_pfuncBlkIoctl(pdpoLogic->DPT_pblkdDisk, 
                                                          iCmd,
                                                          lArg));
}
/*********************************************************************************************************
** ��������: __logicDiskReset
** ��������: ��λ�������߼����� (��ǰϵͳ������õ�����)
** �䡡��  : pdpoLogic         �߼����̿��ƿ�
** �䡡��  : 0
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __logicDiskReset (LW_DISKPART_OPERAT    *pdpoLogic)
{
    return  (pdpoLogic->DPT_pblkdDisk->BLKD_pfuncBlkReset(pdpoLogic->DPT_pblkdDisk));
}
/*********************************************************************************************************
** ��������: __logicDiskStatusChk
** ��������: ��ⵥ�����߼�����
** �䡡��  : pdpoLogic         �߼����̿��ƿ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __logicDiskStatusChk (LW_DISKPART_OPERAT    *pdpoLogic)
{
    return  (pdpoLogic->DPT_pblkdDisk->BLKD_pfuncBlkStatusChk(pdpoLogic->DPT_pblkdDisk));
}
/*********************************************************************************************************
** ��������: __logicDiskInit
** ��������: ��ʼ���߼����̿��ƿ�
** �䡡��  : pblkdLogic        �߼������豸
**           pblkdPhysical     ��������豸
**           ulNSector         ������������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __logicDiskInit (PLW_BLK_DEV  pblkdLogic, PLW_BLK_DEV  pblkdPhysical, ULONG  ulNSector)
{
    pblkdLogic->BLKD_pfuncBlkRd        = __logicDiskRd;
    pblkdLogic->BLKD_pfuncBlkWrt       = __logicDiskWrt;
    pblkdLogic->BLKD_pfuncBlkIoctl     = __logicDiskIoctl;
    pblkdLogic->BLKD_pfuncBlkReset     = __logicDiskReset;
    pblkdLogic->BLKD_pfuncBlkStatusChk = __logicDiskStatusChk;
    
    pblkdLogic->BLKD_ulNSector         = ulNSector;
    pblkdLogic->BLKD_ulBytesPerSector  = pblkdPhysical->BLKD_ulBytesPerSector;
    pblkdLogic->BLKD_ulBytesPerBlock   = pblkdPhysical->BLKD_ulBytesPerBlock;
    
    pblkdLogic->BLKD_bRemovable        = pblkdPhysical->BLKD_bRemovable;
    pblkdLogic->BLKD_iRetry            = pblkdPhysical->BLKD_iRetry;
    pblkdLogic->BLKD_iFlag             = pblkdPhysical->BLKD_iFlag;
    
    pblkdLogic->BLKD_iLogic            = 1;
    pblkdLogic->BLKD_uiLinkCounter     = 0;
    
    /*
     *  ������ײ������������ƿ�
     */
    while (pblkdPhysical->BLKD_pvLink && (pblkdPhysical != (PLW_BLK_DEV)pblkdPhysical->BLKD_pvLink)) {
        pblkdPhysical = (PLW_BLK_DEV)pblkdPhysical->BLKD_pvLink;
    }
    
    pblkdLogic->BLKD_pvLink = (PVOID)pblkdPhysical;
}
/*********************************************************************************************************
** ��������: __diskPartitionScan
** ��������: ����������������Ϣ
** �䡡��  : iIndex            ���豸��������
**           pdpt              ������Ϣ
**           uiCounter         ����������
**           ulStartSector     ��ʼ����
**           ulExtStartSector  ��չ������ʼ����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __diskPartitionScan (INT                 iIndex, 
                                 ULONG               ulBytesPerSector,
                                 PLW_DISKPART_TABLE  pdpt, 
                                 UINT                uiCounter,
                                 ULONG               ulStartSector,
                                 ULONG               ulExtStartSector)
{
#ifndef MBR_Table
#define MBR_Table			446	                                        /* MBR: Partition table offset  */
#endif

    INT                     i;
    INT                     iPartInfoStart;
    BYTE                    ucActiveFlag;
    BYTE                    ucPartType;
    LW_DISKPART_OPERAT     *pdoLogic;
    
    INT                     iError;
    
    BYTE                   *pucBuffer = (BYTE *)__SHEAP_ALLOC((size_t)ulBytesPerSector);
    
    if (pucBuffer == LW_NULL) {
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);                          /*  �ڴ治��                    */
        return  (PX_ERROR);
    }
    lib_bzero(pucBuffer, (size_t)ulBytesPerSector);
    
    if (__blockIoDevRead(iIndex, (PVOID)pucBuffer, 
                         ulStartSector, 1) < 0) {                       /*  ��ȡ������                  */
        __SHEAP_FREE(pucBuffer);
        return  (PX_ERROR);
    }
    
    if ((pucBuffer[ulBytesPerSector - 2] != 0x55) ||
        (pucBuffer[ulBytesPerSector - 1] != 0xaa)) {                    /*  ����������־�Ƿ���ȷ        */
        __SHEAP_FREE(pucBuffer);
        return  (PX_ERROR);
    }
    
    for (i = 0; i < 4; i++) {                                           /*  �鿴����������Ϣ            */
        iPartInfoStart = MBR_Table + (i * 16);                          /*  ��ȡ������Ϣ��ʼ��          */
        
        ucActiveFlag = pucBuffer[iPartInfoStart];                       /*  �����־                    */
        ucPartType   = pucBuffer[iPartInfoStart + __DISK_PART_TYPE];    /*  �����ļ�ϵͳ����            */
        
        switch (ucPartType) {
        
        case LW_DISK_PART_TYPE_EMPTY:                                   /*  ���з���                    */
            break;
            
        case LW_DISK_PART_TYPE_FAT12:                                   /*  ��Ч�ļ�ϵͳ����            */
        case LW_DISK_PART_TYPE_FAT16:
        case LW_DISK_PART_TYPE_FAT16_BIG:
        case LW_DISK_PART_TYPE_HPFS_NTFS:
        case LW_DISK_PART_TYPE_WIN95_FAT32:
        case LW_DISK_PART_TYPE_WIN95_FAT32LBA:
        case LW_DISK_PART_TYPE_WIN95_FAT16LBA:
            pdoLogic = &pdpt->DPT_dpoLogic[uiCounter];                  /*  �߼�������Ϣ                */
            
            /*
             *  ��¼��Ҫ������Ϣ
             */
            pdoLogic->DPO_dpnEntry.DPN_ulStartSector = ulStartSector +
                LD_DWORD(&pucBuffer[iPartInfoStart + __DISK_PART_STARTSECTOR]);
            pdoLogic->DPO_dpnEntry.DPN_ulNSector = 
                LD_DWORD(&pucBuffer[iPartInfoStart + __DISK_PART_NSECTOR]);
            
            if (ucActiveFlag == __DISK_PART_ACTIVE) {                   /*  ��¼�Ƿ�Ϊ�����          */
                pdoLogic->DPO_dpnEntry.DPN_bIsActive = LW_TRUE;
            } else {
                pdoLogic->DPO_dpnEntry.DPN_bIsActive = LW_FALSE;
            }
            pdoLogic->DPO_dpnEntry.DPN_ucPartType = ucPartType;         /*  ��¼��������                */
            
            pdoLogic->DPT_pblkdDisk = __blockIoDevGet(iIndex);          /*  ��¼�²��豸                */
            
            __logicDiskInit(&pdoLogic->DPO_blkdLogic,
                            pdoLogic->DPT_pblkdDisk,
                            pdoLogic->DPO_dpnEntry.DPN_ulNSector);      /*  ��ʼ���߼��豸���ƿ�        */
            
            uiCounter++;                                                /*  ��������++                  */
            break;
            
        case LW_DISK_PART_TYPE_EXTENDED:                                /*  ��չ��������                */
        case LW_DISK_PART_TYPE_WIN95_EXTENDED:
            /*
             *  ע��, һ�ŷ����������ֻ��ӵ�� 1 ����չ����.
             */
            if (ulStartSector == 0ul) {                                 /*  �Ƿ�λ����������            */
                ulExtStartSector = LD_DWORD(&pucBuffer[iPartInfoStart + __DISK_PART_STARTSECTOR]);
                ulStartSector    = ulExtStartSector;
            } else {                                                    /*  λ����չ����������          */
                ulStartSector    = LD_DWORD(&pucBuffer[iPartInfoStart + __DISK_PART_STARTSECTOR]);
                ulStartSector   += ulExtStartSector;
            }
            
            iError = __diskPartitionScan(iIndex, ulBytesPerSector, 
                                         pdpt, uiCounter, 
                                         ulStartSector, 
                                         ulExtStartSector);             /*  �ݹ��ѯ��չ����            */
            if (iError < 0) {
                __SHEAP_FREE(pucBuffer);
                return  ((INT)uiCounter);
            } else {
                uiCounter = (UINT)iError;                               /*  iError Ϊ�µ� uiCounter     */
            }
            break;
            
        default:                                                        /*  �������ͺ���                */
            break;
        }
        
        if (uiCounter >= LW_CFG_MAX_DISKPARTS) {                        /*  �����ٱ�����������Ϣ      */
            __SHEAP_FREE(pucBuffer);
            return  ((INT)uiCounter);
        }
    }
    
    __SHEAP_FREE(pucBuffer);
    if (uiCounter == 0) {                                               /*  û�м�⵽�κη���          */
        return  (PX_ERROR);
    } else {
        return  ((INT)uiCounter);                                       /*  ��⵽�ķ�������            */
    }
}
/*********************************************************************************************************
** ��������: API_DiskPartitionScan
** ��������: ����ָ��������̵ķ������, (ͨ��������)
** �䡡��  : pblkd             ������豸����
**           pdptDisk          ���̷�����Ϣ
** �䡡��  : ERROR CODE        ��ȷʱ, ���ط�������
**                             ������û�з�������߷�����Ϣ����ʱ, ���� -1.
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_DiskPartitionScan (PLW_BLK_DEV  pblkd, PLW_DISKPART_TABLE  pdptDisk)
{
    REGISTER INT                    iBlkdIndex;
    REGISTER INT                    iError;
             ULONG                  ulBytesPerSector;
             
    if (pblkd == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    lib_bzero(pdptDisk, sizeof(LW_DISKPART_TABLE));
    
    iBlkdIndex = __blockIoDevCreate(pblkd);                             /*  ������豸������            */
    if (iBlkdIndex == -1) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "block device invalidate.\r\n");
        _ErrorHandle(ERROR_IOS_DEVICE_NOT_FOUND);
        return  (PX_ERROR);
    } else if (iBlkdIndex == -2) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "block device table full.\r\n");
        _ErrorHandle(ERROR_IOS_DRIVER_GLUT);
        return  (PX_ERROR);
    }
    __blockIoDevIoctl(iBlkdIndex, LW_BLKD_CTRL_POWER, LW_BLKD_POWER_ON);/*  �򿪵�Դ                    */
    
    __blockIoDevReset(iBlkdIndex);                                      /*  ��λ���̽ӿ�                */
    
    iError = __blockIoDevIoctl(iBlkdIndex, FIODISKINIT, 0);             /*  ��ʼ������                  */
    if (iError < 0) {
        __blockIoDevDelete(iBlkdIndex);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "can not initialize disk.\r\n");
        _ErrorHandle(ERROR_IO_DEVICE_ERROR);
        return  (iError);
    }
    
    iError = __blockIoDevIoctl(iBlkdIndex, LW_BLKD_GET_SECSIZE, 
                               (LONG)&ulBytesPerSector);                /*  ��ô��� sector size        */
    if (iError < 0) {
        __blockIoDevDelete(iBlkdIndex);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "sector size invalidate.\r\n");
        _ErrorHandle(ERROR_IO_DEVICE_ERROR);
        return  (iError);
    }
    
    iError = __diskPartitionScan(iBlkdIndex, ulBytesPerSector, 
                                 pdptDisk, 0, 0, 0);                    /*  �������̷�����              */
    
    __blockIoDevDelete(iBlkdIndex);                                     /*  �ӿ��豸��������ж��        */
    
    if (iError >= 0) {
        pdptDisk->DPT_ulNPart = (ULONG)iError;                          /*  ��¼��������                */
    
    } else {
        LW_DISKPART_OPERAT  *pdoLogic;
        
        pdptDisk->DPT_ulNPart = 1;                                      /*  Ĭ��Ϊ�������޷��������    */
        pdoLogic = &pdptDisk->DPT_dpoLogic[0];                          /*  �߼�������Ϣ                */
        
        pdoLogic->DPO_dpnEntry.DPN_ulStartSector = 0ul;
        pdoLogic->DPO_dpnEntry.DPN_ulNSector     = pblkd->BLKD_ulNSector;
        pdoLogic->DPO_dpnEntry.DPN_bIsActive     = LW_FALSE;
        pdoLogic->DPO_dpnEntry.DPN_ucPartType    = LW_DISK_PART_TYPE_EMPTY;
                                                                        /*  ������Ч                    */
        pdoLogic->DPT_pblkdDisk = pblkd;                                /*  ��¼�²��豸                */
        
        __logicDiskInit(&pdoLogic->DPO_blkdLogic,
                        pblkd,
                        pblkd->BLKD_ulNSector);                         /*  ��ʼ���߼��豸���ƿ�        */
        iError = 1;                                                     /*  ������                      */
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: API_DiskPartitionGet
** ��������: ͨ����ǰ�����ķ�����Ϣ, ���һ���߼������Ĳ��� BLK_DEV
** �䡡��  : ppdptDisk         ������̵ķ�����Ϣ
**           uiPart            �ڼ�������
**           ppblkdLogic       �߼������Ĳ��� BLK_DEV
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_DiskPartitionGet (PLW_DISKPART_TABLE  pdptDisk, UINT  uiPart, PLW_BLK_DEV  *ppblkdLogic)
{
    if (pdptDisk == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    if (ppblkdLogic == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    if (uiPart >= pdptDisk->DPT_ulNPart) {
        _ErrorHandle(ERROR_IO_VOLUME_ERROR);
        return  (PX_ERROR);
    }
    
    *ppblkdLogic = (PLW_BLK_DEV)__SHEAP_ALLOC(sizeof(LW_DISKPART_OPERAT));
    if (*ppblkdLogic == LW_NULL) {
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (PX_ERROR);
    }
    
    lib_memcpy(*ppblkdLogic, 
               &pdptDisk->DPT_dpoLogic[uiPart].DPO_blkdLogic, 
               sizeof(LW_DISKPART_OPERAT));                             /*  �������ƿ���Ϣ              */
    
    return  (ERROR_NONE);                                               /*  ���ط����߼��豸            */
}
/*********************************************************************************************************
** ��������: API_DiskPartitionFree
** ��������: ��һ���������ϵͳ�б��Ƴ�, ��Ҫ�ͷ�һ�������߼���Ϣ.
** �䡡��  : pblkdLogic        �����߼��豸���ƿ�
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_DiskPartitionFree (PLW_BLK_DEV  pblkdLogic)
{
    if (pblkdLogic == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    if ((pblkdLogic->BLKD_iLogic == 0) ||
        (pblkdLogic->BLKD_pvLink == LW_NULL)) {
        _ErrorHandle(ERROR_IO_DISK_NOT_PRESENT);
        return  (PX_ERROR);
    }
    __SHEAP_FREE(pblkdLogic);                                           /*  �ͷ��ڴ�                    */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_DiskPartitionLinkNumGet
** ��������: ���һ�������豸��ǰ���ص��߼���������. (�û�ж���������ʱ�ж�)
** �䡡��  : pblkdPhysical       ������ƿ�
** �䡡��  : ��������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT  API_DiskPartitionLinkNumGet (PLW_BLK_DEV  pblkdPhysical)
{
    if (pblkdPhysical == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    return  ((INT)pblkdPhysical->BLKD_uiLinkCounter);
}
#endif                                                                  /*  (LW_CFG_MAX_VOLUMES > 0)    */
                                                                        /*  (LW_CFG_FATFS_EN > 0)       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
