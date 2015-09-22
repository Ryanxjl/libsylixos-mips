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
** ��   ��   ��: ttniy_shell_option.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 07 �� 27 ��
**
** ��        ��: ���� ttniy_shell ����ѡ���.
*********************************************************************************************************/

#ifndef __TTINY_SHELL_OPTION_H
#define __TTINY_SHELL_OPTION_H

/*********************************************************************************************************
  ttniy_shell ��������
*********************************************************************************************************/
#define LW_OPTION_TSHELL_VT100          0x00000001                      /*  ʹ�� VT100 �жϿ����ַ�     */

#define LW_OPTION_TSHELL_AUTHEN         0x80000000                      /*  ʹ���û���֤                */
#define LW_OPTION_TSHELL_NOLOGO         0x40000000                      /*  �Ƿ���ʾ logo             */
#define LW_OPTION_TSHELL_NOECHO         0x20000000                      /*  �޻���                      */

#define LW_OPTION_TSHELL_PROMPT_FULL    0x10000000                      /*  ȫ����ʾ������ʾ��          */

#define LW_OPTION_TSHELL_CLOSE_FD       0x08000000                      /*  shell �˳�ʱ�ر� fd         */

/*********************************************************************************************************
  keyword �߼�����ѡ��
*********************************************************************************************************/

#define LW_OPTION_KEYWORD_SYNCBG        0x00000001                      /*  ���������ʹ��ͬ������ִ��  */
                                                                        /*  ������һ��������������ִ��  */
                                                                        /*  ���ҵȴ�ִ�н���            */
                                                                        
#define LW_OPTION_KEYWORD_ASYNCBG       0x00000002                      /*  ���������ʹ��ͬ������ִ��  */
                                                                        /*  ������һ��������������ִ��  */
                                                                        
#endif                                                                  /*  __TTINY_SHELL_OPTION_H      */
/*********************************************************************************************************
  END
*********************************************************************************************************/