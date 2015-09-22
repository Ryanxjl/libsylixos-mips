/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: px_initializer.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2012 �� 09 �� 18 ��
**
** ��        ��: pthread Ӧ�ó����п�ʹ�ñ�������ֲ��Ҫ��̬��ʼ���� posix ����.
                 ���ö���Ĺ�������������ʵ��
*********************************************************************************************************/

#ifndef __SYS_POSIX_INITIALIZER_H
#define __SYS_POSIX_INITIALIZER_H

#include "SylixOS.h"                                                    /*  ����ϵͳͷ�ļ�              */

/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0

#include "pthread.h"
#include "semaphore.h"

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

/*********************************************************************************************************
  SylixOS 1.0.0-rc35 �汾������֧���˽�����Դ����, ����֧���˾�̬����, �û����Բ���ʹ�ô˷���.
*********************************************************************************************************/

/*********************************************************************************************************
  MUTEX INITIALIZER
*********************************************************************************************************/

#define SYLIXOS_INITIALIZER_MUTEX(mutex)    \
        __attribute__((constructor)) static void __init_mutex_##mutex (void) \
        {   \
            pthread_mutex_init(&mutex, (void *)0); \
        }   \
        __attribute__((destructor)) static void __deinit_mutex_##mutex (void) \
        {   \
            pthread_mutex_destroy(&mutex); \
        }
        
/*********************************************************************************************************
  MUTEX INITIALIZER
*********************************************************************************************************/

#define SYLIXOS_INITIALIZER_COND(cond) \
        __attribute__((constructor)) static void __init_cond_##cond (void) \
        {   \
            pthread_cond_init(&cond, (void *)0); \
        }   \
        __attribute__((destructor)) static void __deinit_cond_##cond (void) \
        {   \
            pthread_cond_destroy(&cond); \
        }
        
/*********************************************************************************************************
  RWLOCK INITIALIZER
*********************************************************************************************************/

#define SYLIXOS_INITIALIZER_RWLOCK(rwlock) \
        __attribute__((constructor)) static void __init_cond_##rwlock (void) \
        {   \
            pthread_rwlock_init(&rwlock, (void *)0); \
        }   \
        __attribute__((destructor)) static void __deinit_cond_##rwlock (void) \
        {   \
            pthread_rwlock_destroy(&rwlock); \
        }

/*********************************************************************************************************
  SEMAPHORE INITIALIZER
*********************************************************************************************************/

#define SYLIXOS_INITIALIZER_SEMAPHORE(sem) \
        __attribute__((constructor)) static void __init_cond_##sem (void) \
        {   \
            sem_init(&sem, 0, 0); \
        }   \
        __attribute__((destructor)) static void __deinit_cond_##sem (void) \
        {   \
            sem_destroy(&sem); \
        }

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
#endif                                                                  /*  __SYS_POSIX_INITIALIZER_H   */
/*********************************************************************************************************
  END
*********************************************************************************************************/
