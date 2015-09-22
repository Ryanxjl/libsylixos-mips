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
** ��   ��   ��: diskPartition.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 03 �� 17 ��
**
** ��        ��: ���̷��������. (ע��: ����ʹ�� FAT �ļ�ϵͳ����).
*********************************************************************************************************/

#ifndef __DISKPARTITION_H
#define __DISKPARTITION_H

/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_MAX_VOLUMES > 0) && (LW_CFG_FATFS_EN > 0)
/*********************************************************************************************************
  ���̷�����ʹ��ʾ��: (����û���жϴ���״��)
  
  int                   i, partcnt;
  
  BLK_DEV               hdd;
  BLK_DEV               cache;
  BLK_DEV              *phddpart[...];
  
  DISKPART_TABLE        part;
  char                  cVolName[16];
  
  hdd = ataDiskCreate(...);                         ->���� HDD �豸
  diskCacheCreate(hdd, ..., &cache);                ->�����豸 CACHE
  partcnt  = diskPartitionScan(cache, &part);       ->�����豸������
  
  for (i = 0; i < partcnt; i++) {                   ->���ش��̸�������
      diskPartitionGet(&part, i, &phddpart[i]);
      sprintf(cVolName, "/hdd%d", i);
      fatFsDevCreate(cVolName, phddpart[i]);
  }
  
  ... (�����⵽��Ҫ�豸�Ƴ�)
  
  for (i = 0; i < partcnt; i++) {
      sprintf(cVolName, "/hdd%d", i);
      unlink(cVolName);
      diskPartitionFree(phddpart[i]);
  }
  diskCacheDelete(cache);
  ataDiskDelete(hdd);
  
  
  ע��:
  diskPartitionScan ���� -1 ���� 0 , �����豸�����ڴ��̷�����,
  ֱ��ʹ�� fatFsDevCreate(...) װ�ؾ���.
*********************************************************************************************************/
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
#define LW_DISK_PART_TYPE_EMPTY             0x00
#define LW_DISK_PART_TYPE_FAT12             0x01
#define LW_DISK_PART_TYPE_FAT16             0x04
#define LW_DISK_PART_TYPE_EXTENDED          0x05
#define LW_DISK_PART_TYPE_FAT16_BIG         0x06
#define LW_DISK_PART_TYPE_HPFS_NTFS         0x07
#define LW_DISK_PART_TYPE_WIN95_FAT32       0x0b
#define LW_DISK_PART_TYPE_WIN95_FAT32LBA    0x0c
#define LW_DISK_PART_TYPE_WIN95_FAT16LBA    0x0e
#define LW_DISK_PART_TYPE_WIN95_EXTENDED    0x0f
/*********************************************************************************************************
  ����������Ϣ
*********************************************************************************************************/
typedef struct {
    ULONG                    DPN_ulStartSector;                         /*  ������ʼ����                */
    ULONG                    DPN_ulNSector;                             /*  ������С                    */
    BOOL                     DPN_bIsActive;                             /*  �Ƿ�Ϊ�����              */
    BYTE                     DPN_ucPartType;                            /*  ��������                    */
} LW_DISKPART_NODE;
/*********************************************************************************************************
  �߼������������ƿ�
*********************************************************************************************************/
typedef struct {
    LW_BLK_DEV               DPO_blkdLogic;                             /*  ���̲�������                */
    LW_DISKPART_NODE         DPO_dpnEntry;                              /*  ����������Ϣ                */
    PLW_BLK_DEV              DPT_pblkdDisk;                             /*  �����豸�������ƿ�          */
} LW_DISKPART_OPERAT;
/*********************************************************************************************************
  ������̷�����
*********************************************************************************************************/
typedef struct {
    UINT                     DPT_ulNPart;                               /*  ��������                    */
    LW_DISKPART_OPERAT       DPT_dpoLogic[LW_CFG_MAX_DISKPARTS];        /*  �����������ƿ�              */
} LW_DISKPART_TABLE;
typedef LW_DISKPART_TABLE   *PLW_DISKPART_TABLE;
typedef LW_DISKPART_TABLE    DISKPART_TABLE;
/*********************************************************************************************************
  API
*********************************************************************************************************/
LW_API INT  API_DiskPartitionScan(PLW_BLK_DEV  pblkd, PLW_DISKPART_TABLE  pdptDisk);
LW_API INT  API_DiskPartitionGet(PLW_DISKPART_TABLE  pdptDisk, UINT  uiPart, PLW_BLK_DEV  *ppblkdLogic);
LW_API INT  API_DiskPartitionFree(PLW_BLK_DEV  pblkdLogic);
LW_API INT  API_DiskPartitionLinkNumGet(PLW_BLK_DEV  pblkdPhysical);

#define diskPartitionScan           API_DiskPartitionScan
#define diskPartitionGet            API_DiskPartitionGet
#define diskPartitionFree           API_DiskPartitionFree
#define diskPartitionLinkNumGet     API_DiskPartitionLinkNumGet

#endif                                                                  /*  (LW_CFG_MAX_VOLUMES > 0)    */
                                                                        /*  (LW_CFG_FATFS_EN > 0)       */
#endif                                                                  /*  __DISKPARTITION_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
