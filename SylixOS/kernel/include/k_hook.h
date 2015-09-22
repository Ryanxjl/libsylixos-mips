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
** ��   ��   ��: k_hook.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2015 �� 05 �� 15 ��
**
** ��        ��: ����ϵͳ HOOK ���Ͷ���.
*********************************************************************************************************/

#ifndef __K_HOOK_H
#define __K_HOOK_H

/*********************************************************************************************************
  �̴߳��� HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_THREAD_CREATE_HOOK        1

/*********************************************************************************************************
  �߳�ɾ�� HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_THREAD_DELETE_HOOK        2

/*********************************************************************************************************
  �߳��л� HOOK
  �� HOOK �������л���϶������, ���Ҳ���ϵͳ���ô˺���ʱ���ڹ��ж�״̬.
*********************************************************************************************************/
#define LW_OPTION_THREAD_SWAP_HOOK          3

/*********************************************************************************************************
  ʱ���ж� HOOK
  �� HOOK ���ж������������������ں˺󱻵���.
*********************************************************************************************************/
#define LW_OPTION_THREAD_TICK_HOOK          4

/*********************************************************************************************************
  �̳߳�ʼ������ HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_THREAD_INIT_HOOK          5

/*********************************************************************************************************
  �����߳� HOOK
  �� HOOK �������������б�����, �����̻߳᲻��ϵĵ��ô� HOOK, ע��, �� HOOK ֻ����ϵͳ���������ǰ������.
*********************************************************************************************************/
#define LW_OPTION_THREAD_IDLE_HOOK          6

/*********************************************************************************************************
  ϵͳ��ʼ�� HOOK
  �� HOOK ����̭.
*********************************************************************************************************/
#define LW_OPTION_KERNEL_INITBEGIN          7

/*********************************************************************************************************
  ϵͳ��ʼ����� HOOK
  �� HOOK ��ϵͳ��û�н��������״̬ʱ������.
*********************************************************************************************************/
#define LW_OPTION_KERNEL_INITEND            8

/*********************************************************************************************************
  ϵͳ����(�ػ�) HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_KERNEL_REBOOT             9

/*********************************************************************************************************
  ϵͳ�������Ź� HOOK
  �� HOOK �ڹ��ж��ҽ����ں�״̬�±�����.
*********************************************************************************************************/
#define LW_OPTION_WATCHDOG_TIMER            10

/*********************************************************************************************************
  �ں˶��󴴽� HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_OBJECT_CREATE_HOOK        11

/*********************************************************************************************************
  �ں˶���ɾ�� HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_OBJECT_DELETE_HOOK        12

/*********************************************************************************************************
  �ļ����������� HOOK
  �� HOOK �� I/O ϵͳ����״̬�±�����.
*********************************************************************************************************/
#define LW_OPTION_FD_CREATE_HOOK            13

/*********************************************************************************************************
  �ļ�������ɾ�� HOOK
  �� HOOK �� I/O ϵͳ����״̬�±�����.
*********************************************************************************************************/
#define LW_OPTION_FD_DELETE_HOOK            14

/*********************************************************************************************************
  ��ǰ CPU ׼������/�ָ����� IDLE ���� HOOK
  �� HOOK �ڹ��ж��������ں�ʱ������.
*********************************************************************************************************/
#define LW_OPTION_CPU_IDLE_ENTER            15

/*********************************************************************************************************
  ��ǰ CPU IDLE ������ռ HOOK
  �� HOOK �ڹ��ж��������ں�ʱ������.
*********************************************************************************************************/
#define LW_OPTION_CPU_IDLE_EXIT             16

/*********************************************************************************************************
  ��ǰ CPU �����ж�, �ڵ����жϴ�������ǰ HOOK
  �� HOOK ���ж��������б�����.
*********************************************************************************************************/
#define LW_OPTION_CPU_INT_ENTER             17

/*********************************************************************************************************
  ��ǰ CPU �����ж�, �ڵ����жϴ��������� HOOK
  �� HOOK ���ж��������б�����.
*********************************************************************************************************/
#define LW_OPTION_CPU_INT_EXIT              18

/*********************************************************************************************************
  �̶߳�ջ��� HOOK
  �� HOOK ���ܷ������ں�����״̬�±�����.
*********************************************************************************************************/
#define LW_OPTION_STACK_OVERFLOW_HOOK       25

/*********************************************************************************************************
  �̳߳����������� (���յ��쳣�ź�) HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_FATAL_ERROR_HOOK          26

/*********************************************************************************************************
  ���̴��� HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_VPROC_CREATE_HOOK         30

/*********************************************************************************************************
  ����ɾ�� HOOK
  �� HOOK �������������б�����.
*********************************************************************************************************/
#define LW_OPTION_VPROC_DELETE_HOOK         31

#endif                                                                  /*  __K_HOOK_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/