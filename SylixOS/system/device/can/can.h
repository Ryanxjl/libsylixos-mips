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
** ��   ��   ��: can.h
**
** ��   ��   ��: Wang.Feng (����)
**
** �ļ���������: 2010 �� 02 �� 01 ��
**
** ��        ��: CAN �豸��.

** BUG
2010.02.01  ��ʼ�汾
2010.05.13  ���Ӽ����쳣����״̬�ĺ궨��
*********************************************************************************************************/

#ifndef __CAN_H
#define __CAN_H

/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_CAN_EN > 0)
/*********************************************************************************************************
  CAN ֡�ṹ�궨��
*********************************************************************************************************/
#define   CAN_MAX_DATA                             8                    /*  CAN ֡������󳤶�          */
/*********************************************************************************************************
  ����������װ�ص�����ʱ������
*********************************************************************************************************/
#define   CAN_CALLBACK_GET_TX_DATA                 1                    /*  ��װ��������ʱ�Ļص�        */
#define   CAN_CALLBACK_PUT_RCV_DATA                2                    /*  ��װ��������ʱ�Ļص�        */
#define   CAN_CALLBACK_PUT_BUS_STATE               3                    /*  ��װ����״̬�ı�ʱ�Ļص�    */
/*********************************************************************************************************
  ����״̬�궨��
*********************************************************************************************************/
#define   CAN_DEV_BUS_ERROR_NONE                   0X0000               /*  ����״̬                    */
#define   CAN_DEV_BUS_OVERRUN                      0X0001               /*  �������                    */
#define   CAN_DEV_BUS_OFF                          0X0002               /*  ���߹ر�                    */
#define   CAN_DEV_BUS_LIMIT                        0X0004               /*  �޶�����                    */
#define   CAN_DEV_BUS_PASSIVE                      0x0008               /*  ���󱻶�                    */
#define   CAN_DEV_BUS_RXBUFF_OVERRUN               0X0010               /*  ���ջ������                */
/*********************************************************************************************************
  CAN �豸 ioctl ����Ӧ��ʵ�������
*********************************************************************************************************/
#define   CAN_DEV_OPEN                             201                  /*  CAN �豸������            */
#define   CAN_DEV_CLOSE                            202                  /*  CAN �豸�ر�����            */
#define   CAN_DEV_GET_BUS_STATE                    203                  /*  ��ȡ CAN ������״̬         */
#define   CAN_DEV_REST_CONTROLLER                  205                  /*  ��λ CAN ������             */
#define   CAN_DEV_SET_BAUD                         206                  /*  ���� CAN ������             */
#define   CAN_DEV_SET_FLITER                       207                  /*  ���� CAN �����˲���         */
#define   CAN_DEV_STARTUP                          208                  /*  ���� CAN ������             */
/*********************************************************************************************************
  ע��: CAN �豸�� read() �� write() ����, �����������뷵��ֵΪ�ֽ���, ���� CAN ֡�ĸ���.
        ioctl() FIONREAD �� FIONWRITE ����ĵ�λ�����ֽ�.
  ����:
        CAN_FRAME   canframe[10];
        ssize_t     size;
        ssize_t     frame_num;
        ...
        size      = read(can_fd, canframe, 10 * sizeof(CAN_FRAME));
        frame_num = size / sizeof(CAN_FRAME);
        ...
        size      = write(can_fd, canframe, 10 * sizeof(CAN_FRAME));
        frame_num = size / sizeof(CAN_FRAME);
*********************************************************************************************************/
/*********************************************************************************************************
  CAN ֡�ṹ����
*********************************************************************************************************/
typedef struct {
    UINT            CAN_uiId;                                           /*  ��ʶ��                      */
    UINT            CAN_uiChannel;                                      /*  ͨ����                      */
    BOOL            CAN_bExtId;                                         /*  �Ƿ�����չ֡                */
    BOOL            CAN_bRtr;                                           /*  �Ƿ���Զ��֡                */
    UCHAR           CAN_ucLen;                                          /*  ���ݳ���                    */
    UCHAR           CAN_ucData[CAN_MAX_DATA];                           /*  ֡����                      */
} CAN_FRAME;
typedef CAN_FRAME  *PCAN_FRAME;                                         /*  CANָ֡������               */
/*********************************************************************************************************
  CAN ���������ṹ
*********************************************************************************************************/
#ifdef  __SYLIXOS_KERNEL

#ifdef  __cplusplus
typedef INT     (*CAN_CALLBACK)(...);
#else
typedef INT     (*CAN_CALLBACK)();
#endif

typedef struct __can_drv_funcs                       CAN_DRV_FUNCS;

typedef struct __can_chan {
    CAN_DRV_FUNCS    *pDrvFuncs;
} CAN_CHAN;                                                             /*  CAN �����ṹ��              */

struct __can_drv_funcs {
    INT               (*ioctl)
                      (
                      CAN_CHAN    *pcanchan,
                      INT          cmd,
                      PVOID        arg
                      );

    INT               (*txStartup)
                      (
                      CAN_CHAN    *pcanchan
                      );

    INT               (*callbackInstall)
                      (
                      CAN_CHAN          *pcanchan,
                      INT                callbackType,
                      CAN_CALLBACK       callback,
                      PVOID              callbackArg
                      );
};
/*********************************************************************************************************
  API
*********************************************************************************************************/

LW_API INT  API_CanDrvInstall(VOID);
LW_API INT  API_CanDevCreate(PCHAR     pcName,
                             CAN_CHAN *pcanchan,
                             UINT      uiRdFrameSize,
                             UINT      uiWrtFrameSize);
LW_API INT  API_CanDevRemove(PCHAR     pcName, BOOL  bForce);

#define canDrv          API_CanDrvInstall
#define canDevCreate    API_CanDevCreate
#define canDevRemove    API_CanDevRemove

#endif                                                                  /*  __SYLIXOS_KERNEL            */
#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0)      */
                                                                        /*  (LW_CFG_CAN_EN > 0)         */
#endif                                                                  /*  __CAN_H                     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
