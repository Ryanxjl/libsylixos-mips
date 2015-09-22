/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: s_system.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 02 �� 13 ��
**
** ��        ��: ����ϵͳ�ۺ�ͷ�ļ��⡣

** BUG:
2009.10.02  ������ͷ�ļ���˳��.
*********************************************************************************************************/

#ifndef __S_SYSTEM_H
#define __S_SYSTEM_H

/*********************************************************************************************************
  ϵͳ�ṹ����
*********************************************************************************************************/
#include "../SylixOS/system/include/s_type.h"
#include "../SylixOS/system/include/s_option.h"
#include "../SylixOS/system/include/s_error.h"
#include "../SylixOS/system/include/s_const.h"
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/include/s_class.h"
#endif
#include "../SylixOS/system/include/s_stat.h"
#include "../SylixOS/system/include/s_fcntl.h"
#include "../SylixOS/system/include/s_dirent.h"
#include "../SylixOS/system/include/s_utime.h"
#include "../SylixOS/system/include/s_api.h"
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/include/s_globalvar.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#include "../SylixOS/system/include/s_object.h"
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/include/s_internal.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#include "../SylixOS/system/include/s_systeminit.h"
/*********************************************************************************************************
  �м��
*********************************************************************************************************/
#include "../SylixOS/system/util/sioLib.h"

#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/util/bmsgLib.h"
#include "../SylixOS/system/util/rngLib.h"
#include "../SylixOS/system/excLib/excLib.h"
#include "../SylixOS/system/logLib/logLib.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */
/*********************************************************************************************************
  select
*********************************************************************************************************/
#include "../SylixOS/system/select/select.h"
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/select/selectDrv.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */
/*********************************************************************************************************
  ��Դ����
*********************************************************************************************************/
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/pm/pmAdapter.h"
#include "../SylixOS/system/pm/pmDev.h"
#include "../SylixOS/system/pm/pmIdle.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#include "../SylixOS/system/pm/pmSystem.h"
/*********************************************************************************************************
  ����ϵͳ��
*********************************************************************************************************/
#include "../SylixOS/system/bus/busSystem.h"
/*********************************************************************************************************
  �����豸ģ��
*********************************************************************************************************/
#include "../SylixOS/system/device/hwrtc/hwrtc.h"                       /*  real time clock             */
#include "../SylixOS/system/device/spipe/spipe.h"                       /*  stream pipe                 */
#include "../SylixOS/system/device/pipe/pipe.h"                         /*  vxworks pipe                */
#include "../SylixOS/system/device/ty/tty.h"                            /*  terminal device             */
#include "../SylixOS/system/device/pty/pty.h"                           /*  pseudo terminal             */
#include "../SylixOS/system/device/block/blockIo.h"                     /*  block device                */
#include "../SylixOS/system/device/block/ramDisk.h"                     /*  RAM disk                    */
#include "../SylixOS/system/device/can/can.h"                           /*  CAN bus device              */
#include "../SylixOS/system/device/graph/gmemDev.h"                     /*  graph memory device         */

#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/device/ata/ata.h"                           /*  ATA device                  */
#include "../SylixOS/system/device/dma/dma.h"                           /*  DMA device                  */
#include "../SylixOS/system/device/dma/dmaLib.h"
#include "../SylixOS/system/device/gpio/gpioLib.h"                      /*  GPIO �������               */
#include "../SylixOS/system/device/gpio/gpioDev.h"                      /*  GPIO �ÿ�̬�����豸�ӿ�     */
#include "../SylixOS/system/device/hstimerfd/hstimerfdDev.h"            /*  ���ٶ�ʱ���豸              */
#include "../SylixOS/system/device/rand/randDev.h"                      /*  random number generator     */
#include "../SylixOS/system/device/shm/shm.h"                           /*  shared memory device        */
#include "../SylixOS/system/device/i2c/i2cDev.h"                        /*  i2c ���߷��Ž����ں˿���    */
#include "../SylixOS/system/device/spi/spiDev.h"                        /*  spi ���߷��Ž����ں˿���    */
#include "../SylixOS/system/device/sd/sdDev.h"                          /*  sd ���߷��Ž����ں˿���     */
#include "../SylixOS/system/device/pci/pciDev.h"                        /*  pci ���߷��Ž����ں˿���    */
#include "../SylixOS/system/device/pci/pciScan.h"                       /*  pci �����Զ�ɨ�谲װ��Ӧ����*/
#include "../SylixOS/system/device/sdcard/include/sdcardLib.h"          /*  sd ������������           */
#include "../SylixOS/system/device/mem/memDev.h"                        /*  VxWorks memDev              */
#include "../SylixOS/system/device/mii/miiDev.h"                        /*  mii phy �ӿ�����            */
#include "../SylixOS/system/device/eventfd/eventfdDev.h"                /*  eventfd �豸                */
#endif                                                                  /*  __SYLIXOS_KERNEL            */
/*********************************************************************************************************
  Ӧ�ýӿ�
*********************************************************************************************************/
#include "../SylixOS/system/signal/signal.h"

#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/signal/signalLib.h"
#include "../SylixOS/system/signal/signalDev.h"
#endif

#include "../SylixOS/system/ptimer/ptimer.h"
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/ptimer/ptimerDev.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */

#include "../SylixOS/system/hotplugLib/hotplugLib.h"
/*********************************************************************************************************
  ��ʼ��
*********************************************************************************************************/
#ifdef   __SYLIXOS_KERNEL
#include "../SylixOS/system/sysInit/sysInit.h"
#endif                                                                  /*  __SYLIXOS_KERNEL            */

#endif                                                                  /*  __S_SYSTEM_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
