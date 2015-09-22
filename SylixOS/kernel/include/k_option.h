/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: k_option.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 13 ��
**
** ��        ��: ����ϵͳѡ��궨�塣

** BUG
2007.11.04  �� 0xFFFFFFFF ��Ϊ __ARCH_ULONG_MAX
2007.11.07  ���� LW_OPTION_THREAD_UNSELECT �������̲߳���Ҫ select ����, ���Խ�ʡ�ڴ�.
2008.03.02  ���� reboot �Ĺ���. LW_OPTION_KERNEL_REBOOT ���������ں˼����� HOOK.
2011.02.23  ���� LW_OPTION_SIGNAL_INTER ѡ��, �¼�����ѡ���Լ��Ƿ�ɱ��жϴ��.
2012.09.22  ����һЩ�µ� HOOK ��ʵ����ص͹��Ĺ���.
2013.08.29  ���� LW_OPTION_THREAD_NO_MONITOR �̴߳���ѡ��, ��ʾ�ں��¼����������Դ��߳���Ч.
*********************************************************************************************************/

#ifndef  __K_OPTION_H
#define  __K_OPTION_H

/*********************************************************************************************************
  DEFAULT
*********************************************************************************************************/

#define  LW_OPTION_NONE                                 0ul             /*  û���κ�ѡ��                */
#define  LW_OPTION_DEFAULT                              0ul             /*  Ĭ��ѡ��                    */

/*********************************************************************************************************
  GLOBAL (GLOBAL ��������ȫ�ֶ���, �����˳�ʱ�����մ������)
*********************************************************************************************************/

#define  LW_OPTION_OBJECT_GLOBAL                        0x80000000      /*  ȫ�ֶ���                    */
#define  LW_OPTION_OBJECT_LOCAL                         0x00000000      /*  ���ض���                    */

/*********************************************************************************************************
  HEAP
*********************************************************************************************************/

#define  LW_OPTION_HEAP_KERNEL                          0x00000000      /*  �ں˶�                      */
#define  LW_OPTION_HEAP_SYSTEM                          0x00000001      /*  ϵͳ��                      */

/*********************************************************************************************************
  THREAD 
     
  ע��: ����������������ָ���Ƿ���Ч, �� BSP FPU ���ʵ�־���, 
        ����ʹ�� LW_OPTION_THREAD_USED_FP ����ѡ��, ��ǿ�ƴ򿪶�Ӧ����� FPU ��֧��, 
        ��Ȼ, �������ϵͳ�����������Զ��жϸ������Ƿ�ʹ�ø���������, �����������Ӧ�����֧��.
        ���Խ����û�����ʹ�ô˴���ѡ��
*********************************************************************************************************/

#define  LW_OPTION_THREAD_STK_CHK                       0x00000003      /*  �����������ջ���м��      */
#define  LW_OPTION_THREAD_STK_CLR                       0x00000002      /*  ��������ʱ��ջ������������*/
#define  LW_OPTION_THREAD_USED_FP                       0x00000004      /*  ���渡��������              */
#define  LW_OPTION_THREAD_SUSPEND                       0x00000010      /*  �������������              */
#define  LW_OPTION_THREAD_UNPREEMPTIVE                  0x00000020      /*  ���񲻿���ռ (��ǰ������)   */
#define  LW_OPTION_THREAD_INIT                          0x00000040      /*  ��ʼ������                  */
#define  LW_OPTION_THREAD_SAFE                          0x00000080      /*  �������߳�Ϊ��ȫģʽ        */
#define  LW_OPTION_THREAD_DETACHED                      0x00000100      /*  �̲߳��������ϲ�            */
#define  LW_OPTION_THREAD_UNSELECT                      0x00000200      /*  ���̲߳�ʹ�� select ����    */
#define  LW_OPTION_THREAD_NO_MONITOR                    0x00000400      /*  �ں��¼����������Դ�������Ч*/
#define  LW_OPTION_THREAD_SCOPE_PROCESS                 0x00000800      /*  ���������ھ��� (��ǰ��֧��) */

/*********************************************************************************************************
  THREAD CANCLE
*********************************************************************************************************/

#define  LW_THREAD_CANCEL_ASYNCHRONOUS                  0               /*  PTHREAD_CANCEL_ASYNCHRONOUS */
#define  LW_THREAD_CANCEL_DEFERRED                      1               /*  PTHREAD_CANCEL_DEFERRED     */

#define  LW_THREAD_CANCEL_ENABLE                        0               /*  �����߳� CANCEL             */
#define  LW_THREAD_CANCEL_DISABLE                       1

#define  LW_THREAD_CANCELED                             ((PVOID)-1)     /*  PTHREAD_CANCELED            */
                                                                        /*  �̱߳�cancel��ķ���ֵ      */

/*********************************************************************************************************
  THREAD NOTEPAD
*********************************************************************************************************/

#define  LW_OPTION_THREAD_NOTEPAD_0                     0x00            /*  �̼߳��±���                */
#define  LW_OPTION_THREAD_NOTEPAD_1                     0x01
#define  LW_OPTION_THREAD_NOTEPAD_2                     0x02
#define  LW_OPTION_THREAD_NOTEPAD_3                     0x03
#define  LW_OPTION_THREAD_NOTEPAD_4                     0x04
#define  LW_OPTION_THREAD_NOTEPAD_5                     0x05
#define  LW_OPTION_THREAD_NOTEPAD_6                     0x06
#define  LW_OPTION_THREAD_NOTEPAD_7                     0x07
#define  LW_OPTION_THREAD_NOTEPAD_8                     0x08
#define  LW_OPTION_THREAD_NOTEPAD_9                     0x09
#define  LW_OPTION_THREAD_NOTEPAD_10                    0x0A
#define  LW_OPTION_THREAD_NOTEPAD_11                    0x0B
#define  LW_OPTION_THREAD_NOTEPAD_12                    0x0C
#define  LW_OPTION_THREAD_NOTEPAD_13                    0x0D
#define  LW_OPTION_THREAD_NOTEPAD_14                    0x0E
#define  LW_OPTION_THREAD_NOTEPAD_15                    0x0F            /*  �̼߳��±���                */

/*********************************************************************************************************
  Semaphore & MsgQueue
*********************************************************************************************************/
/*********************************************************************************************************
  �ȴ�ʱ��ѡ��
*********************************************************************************************************/

#define  LW_OPTION_NOT_WAIT                             0x00000000      /*  ���ȴ������˳�              */
#define  LW_OPTION_WAIT_INFINITE                        __ARCH_ULONG_MAX
                                                                        /*  ��Զ�ȴ�                    */
#define  LW_OPTION_WAIT_A_TICK                          0x00000001      /*  �ȴ�һ��ʱ�����            */
#define  LW_OPTION_WAIT_A_SECOND                        CLOCKS_PER_SEC
                                                                        /*  �ȴ�һ��                    */
/*********************************************************************************************************
  ����ѡ��
*********************************************************************************************************/

#define  LW_OPTION_WAIT_PRIORITY                        0x00010000      /*  �����ȼ�˳��ȴ�            */
#define  LW_OPTION_WAIT_FIFO                            0x00000000      /*  ���Ƚ��ȳ�˳��ȴ�          */

#define  LW_OPTION_DELETE_SAFE                          0x00020000      /*  Mutex ��ȫɾ��ʹ��          */
#define  LW_OPTION_DELETE_UNSAFE                        0x00000000      /*  Mutex �ǰ�ȫ����ʹ��        */

/*********************************************************************************************************
  �����ź����ѡ�� (ͬ�������� EVENTSET)
*********************************************************************************************************/

#define  LW_OPTION_SIGNAL_INTER                         0x00040000      /*  �ɱ��źŴ�� (EINTR)        */
#define  LW_OPTION_SIGNAL_UNINTER                       0x00000000      /*  ���ɱ��źŴ��              */

/*********************************************************************************************************
  �����ź������ȼ��㷨����ѡ��
*********************************************************************************************************/

#define  LW_OPTION_INHERIT_PRIORITY                     0x00080000      /*  ���ȼ��̳��㷨              */
#define  LW_OPTION_PRIORITY_CEILING                     0x00000000      /*  ���ȼ��컨���㷨            */

/*********************************************************************************************************
  �����ź����Ƿ���������
*********************************************************************************************************/

#define  LW_OPTION_NORMAL                               0x00100000      /*  �ݹ�ʱ����� (���Ƽ�)       */
#define  LW_OPTION_ERRORCHECK                           0x00200000      /*  �ݹ�ʱ����                  */
#define  LW_OPTION_RECURSIVE                            0x00000000      /*  ֧�ֵݹ����                */

/*********************************************************************************************************
  ��Ϣ���н���ѡ��
*********************************************************************************************************/

#define  LW_OPTION_NOERROR                              0x00400000      /*  ���ڻ���������Ϣ�Զ��ض�    */
                                                                        /*  Ĭ�Ͻ���Ϊ��ѡ��            */
/*********************************************************************************************************
  EVENT (��ǰ������)
*********************************************************************************************************/

                                                                        /*  ��ʹ����� 65535 ����Դ     */
                                                                        /*  �ں�ͨ��˳�������Ԥ������  */
#define  LW_OPTION_RESOURCE_1                           0x00000001      /*  ��Դ�� (Ԥ�������㷨ʹ��)   */
#define  LW_OPTION_RESOURCE_2                           0x00000002
#define  LW_OPTION_RESOURCE_3                           0x00000003
#define  LW_OPTION_RESOURCE_4                           0x00000004
#define  LW_OPTION_RESOURCE_5                           0x00000005
#define  LW_OPTION_RESOURCE_6                           0x00000006
#define  LW_OPTION_RESOURCE_7                           0x00000007
#define  LW_OPTION_RESOURCE_8                           0x00000008
#define  LW_OPTION_RESOURCE_9                           0x00000009
#define  LW_OPTION_RESOURCE_10                          0x0000000A
#define  LW_OPTION_RESOURCE_11                          0x0000000B
#define  LW_OPTION_RESOURCE_12                          0x0000000C
#define  LW_OPTION_RESOURCE_13                          0x0000000D
#define  LW_OPTION_RESOURCE_14                          0x0000000E
#define  LW_OPTION_RESOURCE_15                          0x0000000F
#define  LW_OPTION_RESOURCE_16                          0x00000010
#define  LW_OPTION_RESOURCE_17                          0x00000011
#define  LW_OPTION_RESOURCE_18                          0x00000012
#define  LW_OPTION_RESOURCE_19                          0x00000013
#define  LW_OPTION_RESOURCE_20                          0x00000014
#define  LW_OPTION_RESOURCE_21                          0x00000015
#define  LW_OPTION_RESOURCE_22                          0x00000016
#define  LW_OPTION_RESOURCE_23                          0x00000017
#define  LW_OPTION_RESOURCE_24                          0x00000018
#define  LW_OPTION_RESOURCE_25                          0x00000019
#define  LW_OPTION_RESOURCE_26                          0x0000001A
#define  LW_OPTION_RESOURCE_27                          0x0000001B
#define  LW_OPTION_RESOURCE_28                          0x0000001C
#define  LW_OPTION_RESOURCE_29                          0x0000001D
#define  LW_OPTION_RESOURCE_30                          0x0000001E
#define  LW_OPTION_RESOURCE_31                          0x0000001F
#define  LW_OPTION_RESOURCE_32                          0x00000020      /*  ��Դ�� (Ԥ�������㷨ʹ��)   */

/*********************************************************************************************************
  MSGQUEUE
*********************************************************************************************************/

#define  LW_OPTION_URGENT                               0x00000001      /*  ��Ϣ���н�����Ϣ����        */
#define  LW_OPTION_BROADCAST                            0x00000002      /*  ��Ϣ���й㲥����            */

/*********************************************************************************************************
  EVENT SET
*********************************************************************************************************/
/*********************************************************************************************************
  �ȴ�(����)ѡ��
*********************************************************************************************************/
#define  LW_OPTION_EVENTSET_WAIT_CLR_ALL                0               /*  ָ��λ��Ϊ0ʱ����           */
#define  LW_OPTION_EVENTSET_WAIT_CLR_ANY                1               /*  ָ��λ���κ�һλΪ0ʱ����   */
#define  LW_OPTION_EVENTSET_WAIT_SET_ALL                2               /*  ָ��λ��Ϊ1ʱ����           */
#define  LW_OPTION_EVENTSET_WAIT_SET_ANY                3               /*  ָ��λ���κ�һλΪ1ʱ����   */

#define  LW_OPTION_EVENTSET_RETURN_ALL                  0x00000040      /*  ����¼��󷵻�������Ч���¼�*/
#define  LW_OPTION_EVENTSET_RESET                       0x00000080      /*  ����¼����Ƿ��Զ�����¼�  */
#define  LW_OPTION_EVENTSET_RESET_ALL                   0x00000100      /*  ����¼�����������¼�      */
/*********************************************************************************************************
  ����(����)ѡ��
*********************************************************************************************************/
#define  LW_OPTION_EVENTSET_SET                         0x1             /*  ��ָ���¼���Ϊ 1            */
#define  LW_OPTION_EVENTSET_CLR                         0x0             /*  ��ָ���¼���Ϊ 0            */
/*********************************************************************************************************
  �¼����
*********************************************************************************************************/
#define  LW_OPTION_EVENT_0                              0x00000001      /*  �¼����������¼�λ          */
#define  LW_OPTION_EVENT_1                              0x00000002
#define  LW_OPTION_EVENT_2                              0x00000004
#define  LW_OPTION_EVENT_3                              0x00000008
#define  LW_OPTION_EVENT_4                              0x00000010
#define  LW_OPTION_EVENT_5                              0x00000020
#define  LW_OPTION_EVENT_6                              0x00000040
#define  LW_OPTION_EVENT_7                              0x00000080
#define  LW_OPTION_EVENT_8                              0x00000100
#define  LW_OPTION_EVENT_9                              0x00000200
#define  LW_OPTION_EVENT_10                             0x00000400
#define  LW_OPTION_EVENT_11                             0x00000800
#define  LW_OPTION_EVENT_12                             0x00001000
#define  LW_OPTION_EVENT_13                             0x00002000
#define  LW_OPTION_EVENT_14                             0x00004000
#define  LW_OPTION_EVENT_15                             0x00008000
#define  LW_OPTION_EVENT_16                             0x00010000
#define  LW_OPTION_EVENT_17                             0x00020000
#define  LW_OPTION_EVENT_18                             0x00040000
#define  LW_OPTION_EVENT_19                             0x00080000
#define  LW_OPTION_EVENT_20                             0x00100000
#define  LW_OPTION_EVENT_21                             0x00200000
#define  LW_OPTION_EVENT_22                             0x00400000
#define  LW_OPTION_EVENT_23                             0x00800000
#define  LW_OPTION_EVENT_24                             0x01000000
#define  LW_OPTION_EVENT_25                             0x02000000
#define  LW_OPTION_EVENT_26                             0x04000000
#define  LW_OPTION_EVENT_27                             0x08000000
#define  LW_OPTION_EVENT_28                             0x10000000
#define  LW_OPTION_EVENT_29                             0x20000000
#define  LW_OPTION_EVENT_30                             0x40000000
#define  LW_OPTION_EVENT_31                             0x80000000
#define  LW_OPTION_EVENT_ALL                           (0xFFFFFFFF)     /*  �¼����������¼�λ          */

/*********************************************************************************************************
  SCHEDLER
*********************************************************************************************************/

#define  LW_OPTION_SCHED_FIFO                           0x01            /*  ������ FIFO                 */
#define  LW_OPTION_SCHED_RR                             0x00            /*  ������ RR                   */

#define  LW_OPTION_RESPOND_IMMIEDIA                     0x00            /*  ������Ӧ�߳� (��������ʹ��) */
#define  LW_OPTION_RESPOND_STANDARD                     0x01            /*  ��ͨ��Ӧ�߳�                */
#define  LW_OPTION_RESPOND_AUTO                         0x02            /*  �Զ�                        */

/*********************************************************************************************************
  TIME
*********************************************************************************************************/

#define  LW_OPTION_ONE_TICK         0x00000001                          /*  �ȴ�һ�� TICK               */
#define  LW_OPTION_ONE_SECOND       CLOCKS_PER_SEC                      /*  һ��                        */
#define  LW_OPTION_ONE_MINUTE       (60 * LW_OPTION_ONE_SECOND)         /*  һ����                      */
#define  LW_OPTION_ONE_HOUR         (60 * LW_OPTION_ONE_MINUTE)         /*  һСʱ                      */

/*********************************************************************************************************
  TIMER ����ѡ��
*********************************************************************************************************/

#define  LW_OPTION_ITIMER                               0x00000001      /*  ��ͨ                        */
#define  LW_OPTION_HTIMER                               0x00000000      /*  ����                        */

/*********************************************************************************************************
  TIMER ����ѡ��
*********************************************************************************************************/

#define  LW_OPTION_AUTO_RESTART                         0x00000001      /*  ��ʱ���Զ�����              */
#define  LW_OPTION_MANUAL_RESTART                       0x00000000      /*  ��ʱ���ֶ�����              */

/*********************************************************************************************************
  IRQ
*********************************************************************************************************/

#define  LW_IRQ_0                                       0               /*  �ж���������                */
#define  LW_IRQ_1                                       1
#define  LW_IRQ_2                                       2
#define  LW_IRQ_3                                       3
#define  LW_IRQ_4                                       4
#define  LW_IRQ_5                                       5
#define  LW_IRQ_6                                       6
#define  LW_IRQ_7                                       7
#define  LW_IRQ_8                                       8
#define  LW_IRQ_9                                       9
#define  LW_IRQ_10                                      10
#define  LW_IRQ_11                                      11
#define  LW_IRQ_12                                      12
#define  LW_IRQ_13                                      13
#define  LW_IRQ_14                                      14
#define  LW_IRQ_15                                      15
#define  LW_IRQ_16                                      16
#define  LW_IRQ_17                                      17
#define  LW_IRQ_18                                      18
#define  LW_IRQ_19                                      19
#define  LW_IRQ_20                                      20
#define  LW_IRQ_21                                      21
#define  LW_IRQ_22                                      22
#define  LW_IRQ_23                                      23
#define  LW_IRQ_24                                      24
#define  LW_IRQ_25                                      25
#define  LW_IRQ_26                                      26
#define  LW_IRQ_27                                      27
#define  LW_IRQ_28                                      28
#define  LW_IRQ_29                                      29
#define  LW_IRQ_30                                      30
#define  LW_IRQ_31                                      31

/*********************************************************************************************************
  IRQ FLAG (����Ϊ QUEUE ģʽ������, ϵͳ�޷��ٷ��ط� QUEUE ģʽ����)
*********************************************************************************************************/

#define  LW_IRQ_FLAG_QUEUE                              0x0001          /*  ������, �����              */
#define  LW_IRQ_FLAG_PREEMPTIVE                         0x0002          /*  �Ƿ�������ռ (arch����ʵ��) */
#define  LW_IRQ_FLAG_SAMPLE_RAND                        0x0004          /*  �Ƿ������ϵͳ���������    */

/*********************************************************************************************************
  RTC
*********************************************************************************************************/

#define  LW_OPTION_RTC_SOFT                             0               /*  RTC ʹ��ϵͳ TICK ����ʱ��Դ*/
#define  LW_OPTION_RTC_HARD                             1               /*  RTC ʹ��Ӳ�� RTC ʱ��Դ     */

/*********************************************************************************************************
  REBOOT
*********************************************************************************************************/

#define  LW_REBOOT_FORCE                                (-1)            /*  �����������������(��������)*/
#define  LW_REBOOT_WARM                                 0               /*  ������ (BSP����������Ϊ)    */
#define  LW_REBOOT_COLD                                 1               /*  ������ (BSP����������Ϊ)    */
#define  LW_REBOOT_SHUTDOWN                             2               /*  �ر� (BSP����������Ϊ)      */
#define  LW_REBOOT_POWEROFF                             3               /*  �رպ�ϵ� (BSP����������Ϊ)*/

/*********************************************************************************************************
  CPU �ܺ�
*********************************************************************************************************/

#define  LW_CPU_POWERLEVEL_TOP                          0               /*  CPU ����ٶ�����            */
#define  LW_CPU_POWERLEVEL_FAST                         1               /*  CPU ��������                */
#define  LW_CPU_POWERLEVEL_NORMAL                       2               /*  CPU ��������                */
#define  LW_CPU_POWERLEVEL_SLOW                         3               /*  CPU ��������                */

#endif                                                                  /*  __K_OPTION_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/