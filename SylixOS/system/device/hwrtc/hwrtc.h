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
** ��   ��   ��: hwrtc.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2010 �� 01 �� 04 ��
**
** ��        ��: Ӳ�� RTC �豸�������. (ע��: Ӳ�� RTC �ӿ�Ӧ��Ϊ UTC ʱ��)
*********************************************************************************************************/

#ifndef __HWRTC_H
#define __HWRTC_H

/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_RTC_EN > 0)

/*********************************************************************************************************
  rtc ����Ӳ�������ӿ�
*********************************************************************************************************/
typedef struct {
    VOIDFUNCPTR             RTC_pfuncInit;                              /*  ��ʼ�� RTC                  */
    FUNCPTR                 RTC_pfuncSet;                               /*  ����Ӳ�� RTC ʱ��           */
    FUNCPTR                 RTC_pfuncGet;                               /*  ��ȡӲ�� RTC ʱ��           */
    FUNCPTR                 RTC_pfuncIoctl;                             /*  ���ิ�ӵ� RTC ����         */
                                                                        /*  �������û��������жϵȵ�    */
} LW_RTC_FUNCS;
typedef LW_RTC_FUNCS       *PLW_RTC_FUNCS;

/*********************************************************************************************************
  rtc api
*********************************************************************************************************/
LW_API INT          API_RtcDrvInstall(VOID);
LW_API INT          API_RtcDevCreate(PLW_RTC_FUNCS    prtcfuncs);


LW_API INT          API_RtcSet(time_t  time);
LW_API INT          API_RtcGet(time_t  *ptime);
LW_API INT          API_SysToRtc(VOID);
LW_API INT          API_RtcToSys(VOID);
LW_API INT          API_RtcToRoot(VOID);

#define rtcDrv              API_RtcDrvInstall
#define rtcDevCreate        API_RtcDevCreate
#define rtcSet              API_RtcSet
#define rtcGet              API_RtcGet
#define sysToRtc            API_SysToRtc
#define rtcToSys            API_RtcToSys
#define rtcToRoot           API_RtcToRoot

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_RTC_EN > 0)         */
#endif                                                                  /*  __HWRTC_H                   */
/*********************************************************************************************************
  END
*********************************************************************************************************/