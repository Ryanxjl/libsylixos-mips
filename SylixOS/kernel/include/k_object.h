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
** ��   ��   ��: k_object.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 18 ��
**
** ��        ��: ����ϵͳ�ڲ������塣

** BUG
2007.04.10  ȥ���� _OBJECT_KERNEL  _OBJECT_SCHED  _OBJECT_INTERRUPT  _OBJECT_ERR ��������
2014.07.19  ȥ���ϵĵ�Դ�����ڵ�����.
*********************************************************************************************************/

#ifndef  __K_OBJECT_H
#define  __K_OBJECT_H

/*********************************************************************************************************
          ����Ľṹ
          
	31             26 25                 16 15                                      0 bit
    +-------------------------------------------------------------------------------+
    |                |                     |                                        |
    |     CLASS      |        NODE         |                INDEX                   |
    |                |                     |                                        |
    +-------------------------------------------------------------------------------+
    
    CLASS   :   Type
    NODE    :   Processor Number (MPI)
    INDEX   :   Buffer Address
    
*********************************************************************************************************/

#define _OBJECT_THREAD          1                                       /*  �߳�                        */
#define _OBJECT_THREAD_POOL     2                                       /*  �̳߳�                      */
#define _OBJECT_SEM_C           3                                       /*  �������ź���                */
#define _OBJECT_SEM_B           4                                       /*  ��ֵ���ź���                */
#define _OBJECT_SEM_M           5                                       /*  �������ź���                */
#define _OBJECT_MSGQUEUE        7                                       /*  ��Ϣ����                    */
#define _OBJECT_EVENT_SET       8                                       /*  �¼���                      */
#define _OBJECT_SIGNAL          9                                       /*  �ź�                        */
#define _OBJECT_STDFILE        11                                       /*  stdio ��׼�ļ�              */
#define _OBJECT_TIME           13                                       /*  ʱ��                        */
#define _OBJECT_TIMER          14                                       /*  ��ʱ��                      */
#define _OBJECT_PARTITION      16                                       /*  �ڴ�����                  */
#define _OBJECT_REGION         17                                       /*  �ڴ�ɱ����                */
#define _OBJECT_RTC            18                                       /*  ʵʱʱ��                    */
#define _OBJECT_RMS            23                                       /*  ���ȵ�����                  */

#endif                                                                  /*  __K_OBJECT_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/