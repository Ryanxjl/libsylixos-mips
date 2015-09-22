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
** ��   ��   ��: sddrvm.h
**
** ��   ��   ��: Zeng.Bo (����)
**
** �ļ���������: 2014 �� 10 �� 24 ��
**
** ��        ��: sd drv manager layer

** BUG:
*********************************************************************************************************/

#ifndef __SDDRVM_H
#define __SDDRVM_H

#include "sdcore.h"

/*********************************************************************************************************
  SDM �¼����� for API_SdmEventNotify()
*********************************************************************************************************/

#define SDM_EVENT_DEV_INSERT            0
#define SDM_EVENT_DEV_REMOVE            1
#define SDM_EVENT_SDIO_INTERRUPT        2

/*********************************************************************************************************
  ǰ������
*********************************************************************************************************/
struct sd_drv;
struct sd_host;

typedef struct sd_drv     SD_DRV;
typedef struct sd_host    SD_HOST;

/*********************************************************************************************************
  sd ����(����sd memory �� sdio base)
*********************************************************************************************************/

struct sd_drv {
    LW_LIST_LINE  SDDRV_lineManage;                               /*  ����������                        */

    CPCHAR        SDDRV_cpcName;

    INT         (*SDDRV_pfuncDevCreate)(SD_DRV *psddrv, PLW_SDCORE_DEVICE psdcoredev, VOID **ppvDevPriv);
    INT         (*SDDRV_pfuncDevDelete)(SD_DRV *psddrv, VOID *pvDevPriv);

    atomic_t      SDDRV_atomicDevCnt;

    VOID         *SDDRV_pvSpec;
};

/*********************************************************************************************************
  sd host ��Ϣ�ṹ
*********************************************************************************************************/

#ifdef  __cplusplus
typedef INT     (*SD_CALLBACK)(...);
#else
typedef INT     (*SD_CALLBACK)();
#endif

struct sd_host {
    CPCHAR        SDHOST_cpcName;

    INT           SDHOST_iType;
#define SDHOST_TYPE_SD                  0
#define SDHOST_TYPE_SPI                 1

    INT           SDHOST_iCapbility;                                /*  ����֧�ֵ�����                  */
#define SDHOST_CAP_HIGHSPEED    (1 << 0)                            /*  ֧�ָ��ٴ���                    */
#define SDHOST_CAP_DATA_4BIT    (1 << 1)                            /*  ֧��4λ���ݴ���                 */
#define SDHOST_CAP_DATA_8BIT    (1 << 2)                            /*  ֧��8λ���ݴ���                 */

    VOID          (*SDHOST_pfuncSpicsEn)(SD_HOST *psdhost);
    VOID          (*SDHOST_pfuncSpicsDis)(SD_HOST *psdhost);
    INT           (*SDHOST_pfuncCallbackInstall)
                  (
                  SD_HOST          *psdhost,
                  INT               iCallbackType,                  /*  ��װ�Ļص�����������            */
                  SD_CALLBACK       callback,                       /*  �ص�����ָ��                    */
                  PVOID             pvCallbackArg                   /*  �ص������Ĳ���                  */
                  );

    INT           (*SDHOST_pfuncCallbackUnInstall)
                  (
                  SD_HOST          *psdhost,
                  INT               iCallbackType                   /*  ��װ�Ļص�����������            */
                  );
#define SDHOST_CALLBACK_CHECK_DEV       0                           /*  ��״̬���                      */
#define SDHOST_DEVSTA_UNEXIST           0                           /*  ��״̬:������                   */
#define SDHOST_DEVSTA_EXIST             1                           /*  ��״̬:����                     */

    VOID          (*SDHOST_pfuncSdioIntEn)(SD_HOST *psdhost, BOOL bEnable);
    BOOL          (*SDHOST_pfuncIsCardWp)(SD_HOST *psdhost);

    VOID          (*SDHOST_pfuncDevAttach)(SD_HOST *psdhost, CPCHAR cpcDevName);
    VOID          (*SDHOST_pfuncDevDetach)(SD_HOST *psdhost);
};

/*********************************************************************************************************
  API
*********************************************************************************************************/

LW_API INT   API_SdmLibInit(VOID);

LW_API PVOID API_SdmHostRegister(SD_HOST *psdhost);
LW_API INT   API_SdmHostUnRegister(PVOID  pvSdmHost);

LW_API INT   API_SdmHostCapGet(PLW_SDCORE_DEVICE psdcoredev, INT *piCapbility);
LW_API VOID  API_SdmHostInterEn(PLW_SDCORE_DEVICE psdcoredev, BOOL bEnable);
LW_API BOOL  API_SdmHostIsCardWp(PLW_SDCORE_DEVICE psdcoredev);

LW_API INT   API_SdmSdDrvRegister(SD_DRV *psddrv);
LW_API INT   API_SdmSdDrvUnRegister(SD_DRV *psddrv);

LW_API INT   API_SdmEventNotify(PVOID pvSdmHost, INT iEvtType);

#endif                                                              /*  __SDDRVM_H                      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
