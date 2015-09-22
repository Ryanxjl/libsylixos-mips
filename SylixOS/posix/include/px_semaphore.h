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
** ��   ��   ��: px_semaphore.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 12 �� 30 ��
**
** ��        ��: posix ���ݿ��ź�������.
*********************************************************************************************************/

#ifndef __PX_SEMAPHORE_H
#define __PX_SEMAPHORE_H

#include "SylixOS.h"                                                    /*  ����ϵͳͷ�ļ�              */

/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0
/*********************************************************************************************************
  ���� sem_t �а��� __PX_NAME_NODE. �������������� posixLib.h ͷ�ļ�.
*********************************************************************************************************/
#include "posixLib.h"                                                   /*  posix �ڲ�������            */

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

#define SEM_FAILED              (LW_NULL)

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX           (__ARCH_INT_MAX)                        /*  posix sem is 'int' type     */
#endif                                                                  /*  SEM_VALUE_MAX               */

/*********************************************************************************************************
  sem handle
*********************************************************************************************************/

typedef struct {
    PVOID                   SEM_pvPxSem;                                /*  �ź����ڲ��ṹ              */
    PLW_RESOURCE_RAW        SEM_presraw;                                /*  ��Դ����ڵ�                */
    ULONG                   SEM_ulPad[5];
} sem_t;
#define SEMAPHORE_INITIALIZER   {LW_NULL, LW_NULL}

/*********************************************************************************************************
  sem api
*********************************************************************************************************/
LW_API int          sem_init(sem_t  *psem, int  pshared, unsigned int  value);
LW_API int          sem_destroy(sem_t  *psem);
LW_API sem_t       *sem_open(const char  *name, int  flag, ...);
LW_API int          sem_close(sem_t  *psem);
LW_API int          sem_unlink(const char *name);
LW_API int          sem_wait(sem_t  *psem);
LW_API int          sem_trywait(sem_t  *psem);
LW_API int          sem_timedwait(sem_t  *psem, const struct timespec *timeout);
#if LW_CFG_POSIXEX_EN > 0
LW_API int          sem_reltimedwait_np(sem_t  *psem, const struct timespec *rel_timeout);
#endif                                                                  /*  LW_CFG_POSIXEX_EN > 0       */
LW_API int          sem_post(sem_t  *psem);
LW_API int          sem_getvalue(sem_t  *psem, int  *pivalue);

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
#endif                                                                  /*  __PX_SEMAPHORE_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
