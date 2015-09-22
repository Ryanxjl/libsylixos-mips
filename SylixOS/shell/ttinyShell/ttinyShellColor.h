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
** ��   ��   ��: ttinyShellColor.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 07 �� 08 ��
**
** ��        ��: tty ��ɫϵͳ.
*********************************************************************************************************/

#ifndef __TTINYSHELLCOLOR_H
#define __TTINYSHELLCOLOR_H

/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0

LW_API  VOID  API_TShellColorRefresh(VOID);
LW_API  VOID  API_TShellColorStart(CPCHAR  pcName, CPCHAR  pcLink, mode_t  mode, INT  iFd);
LW_API  VOID  API_TShellColorEnd(INT  iFd);

#define tshellColorRefresh  API_TShellColorRefresh
#define tshellColorStart    API_TShellColorStart
#define tshellColorEnd      API_TShellColorEnd

#ifdef __SYLIXOS_KERNEL
VOID          __tshellColorInit(VOID);
#endif                                                                  /*  __SYLIXOS_KERNEL            */

#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
#endif                                                                  /*  __TTINYSHELLCOLOR_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
