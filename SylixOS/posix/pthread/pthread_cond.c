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
** ��   ��   ��: pthread_cond.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 12 �� 30 ��
**
** ��        ��: pthread �����������ݿ�. (cond �������ᷢ�� EINTR)

** BUG:
2012.12.13  ���� SylixOS ֧�ֽ�����Դ����, ���￪ʼ֧�־�̬��ʼ��.
2013.05.01  If successful, the pthread_cond_*() functions shall return zero; 
            otherwise, an error number shall be returned to indicate the error.
2014.07.04  ����ʱ�������������ȡ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../include/px_pthread.h"                                      /*  �Ѱ�������ϵͳͷ�ļ�        */
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0
/*********************************************************************************************************
  mutex ���δ���
*********************************************************************************************************/
extern void  __pthread_mutex_init_invisible(pthread_mutex_t  *pmutex);
/*********************************************************************************************************
** ��������: __pthread_cond_init_invisible
** ��������: �����������δ���. (��̬��ʼ��)
** �䡡��  : pcond         �����������ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static void  __pthread_cond_init_invisible (pthread_cond_t  *pcond)
{
    if (pcond && 
        !pcond->TCD_ulSignal &&
        !pcond->TCD_ulMutxe) {
        pthread_cond_init(pcond, LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: pthread_condattr_init
** ��������: ��ʼ�������������Կ�.
** �䡡��  : pcondattr     ����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_condattr_init (pthread_condattr_t  *pcondattr)
{
    API_ThreadCondAttrInit(pcondattr);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_condattr_destroy
** ��������: ���������������Կ�.
** �䡡��  : pcondattr     ����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_condattr_destroy (pthread_condattr_t  *pcondattr)
{
    API_ThreadCondAttrDestroy(pcondattr);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_condattr_setpshared
** ��������: ���������������Կ��Ƿ�Ϊ����.
** �䡡��  : pcondattr     ����
**           ishare        �Ƿ�Ϊ���̹���
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_condattr_setpshared (pthread_condattr_t  *pcondattr, int  ishare)
{
    API_ThreadCondAttrSetPshared(pcondattr, LW_THREAD_PROCESS_SHARED);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_condattr_getpshared
** ��������: ��������������Կ��Ƿ�Ϊ����.
** �䡡��  : pcondattr     ����
**           pishare       �Ƿ�Ϊ���̹���
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_condattr_getpshared (const pthread_condattr_t  *pcondattr, int  *pishare)
{
    API_ThreadCondAttrGetPshared(pcondattr, pishare);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_condattr_setclock
** ��������: ���� pthread ʱ������.
** �䡡��  : pcondattr     ����
**           clock_id      ʱ������ CLOCK_REALTIME only
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_condattr_setclock (pthread_condattr_t  *pcondattr, clockid_t  clock_id)
{
    if (!pcondattr) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if (clock_id != CLOCK_REALTIME) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_condattr_getclock
** ��������: ��ȡ pthread ʱ������.
** �䡡��  : pcondattr     ����
**           pclock_id     ʱ������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_condattr_getclock (const pthread_condattr_t  *pcondattr, clockid_t  *pclock_id)
{
    if (!pcondattr || !pclock_id) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    *pclock_id = CLOCK_REALTIME;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_cond_init
** ��������: ��ʼ����������.
** �䡡��  : pcond         �����������ƿ�
**           pcondattr     ����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cond_init (pthread_cond_t  *pcond, const pthread_condattr_t  *pcondattr)
{
    ULONG       ulAttr = LW_THREAD_PROCESS_SHARED;
    
    if (pcond == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    if (pcondattr) {
        ulAttr = *pcondattr;
    }

    if (API_ThreadCondInit(pcond, ulAttr)) {                            /*  ��ʼ����������              */
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_cond_destroy
** ��������: ������������.
** �䡡��  : pcond         �����������ƿ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cond_destroy (pthread_cond_t  *pcond)
{
    if (pcond == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if (API_ThreadCondDestroy(pcond)) {                                 /*  ������������                */
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_cond_signal
** ��������: ����һ������������Ч�ź�.
** �䡡��  : pcond         �����������ƿ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cond_signal (pthread_cond_t  *pcond)
{
    if (pcond == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    __pthread_cond_init_invisible(pcond);
    
    if (API_ThreadCondSignal(pcond)) {
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_cond_broadcast
** ��������: ����һ�����������㲥�ź�.
** �䡡��  : pcond         �����������ƿ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cond_broadcast (pthread_cond_t  *pcond)
{
    if (pcond == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    __pthread_cond_init_invisible(pcond);
    
    if (API_ThreadCondBroadcast(pcond)) {
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_cond_wait
** ��������: �ȴ�һ�����������㲥�ź�.
** �䡡��  : pcond         �����������ƿ�
**           pmutex        �����ź���
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cond_wait (pthread_cond_t  *pcond, pthread_mutex_t  *pmutex)
{
    if ((pcond == LW_NULL) || (pmutex == LW_NULL)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    __pthread_cond_init_invisible(pcond);
    __pthread_mutex_init_invisible(pmutex);
    
    if (API_ThreadCondWait(pcond, pmutex->PMUTEX_ulMutex, LW_OPTION_WAIT_INFINITE)) {
        errno = EINVAL;
        return  (EINVAL);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_cond_timedwait
** ��������: �ȴ�һ�����������㲥�ź�(���г�ʱ).
** �䡡��  : pcond         �����������ƿ�
**           pmutex        �����ź���
**           abs_timeout   ��ʱʱ�� (ע��: �����Ǿ���ʱ��, ��һ��ȷ������ʷʱ������: 2009.12.31 15:36:04)
                           ���߾���������Ǻ�ˬ, Ӧ���ټ�һ���������Եȴ����ʱ��.
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cond_timedwait (pthread_cond_t         *pcond, 
                             pthread_mutex_t        *pmutex,
                             const struct timespec  *abs_timeout)
{
    ULONG               ulTimeout;
    ULONG               ulError;
    struct timespec     tvNow;
    struct timespec     tvWait = {0, 0};
    
    if ((pcond == LW_NULL) || (pmutex == LW_NULL)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if ((abs_timeout == LW_NULL)    || 
        (abs_timeout->tv_nsec <  0) ||
        (abs_timeout->tv_nsec >= __TIMEVAL_NSEC_MAX)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    __pthread_cond_init_invisible(pcond);
    __pthread_mutex_init_invisible(pmutex);
    
    lib_clock_gettime(CLOCK_REALTIME, &tvNow);                          /*  ��õ�ǰϵͳʱ��            */
    if (__timespecLeftTime(&tvNow, abs_timeout)) {
        tvWait = *abs_timeout;
        __timespecSub(&tvWait, &tvNow);                                 /*  �����뵱ǰ�ȴ���ʱ����    */
    }
    /*
     *  ע��: �� tvWait ����ulong tick��Χʱ, ����Ȼ�������, ���³�ʱʱ�䲻׼ȷ.
     */
    ulTimeout = __timespecToTick(&tvWait);                              /*  �任Ϊ tick                 */
    
    ulError = API_ThreadCondWait(pcond, pmutex->PMUTEX_ulMutex, ulTimeout);
    if (ulError == ERROR_THREAD_WAIT_TIMEOUT) {
        errno = ETIMEDOUT;
        return  (ETIMEDOUT);
        
    } else if (ulError) {
        errno = EINVAL;
        return  (EINVAL);
        
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_cond_reltimedwait_np
** ��������: �ȴ�һ�����������㲥�ź�(���г�ʱ).
** �䡡��  : pcond         �����������ƿ�
**           pmutex        �����ź���
**           rel_timeout   ��Գ�ʱʱ��.
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_POSIXEX_EN > 0

LW_API 
int  pthread_cond_reltimedwait_np (pthread_cond_t         *pcond, 
                                   pthread_mutex_t        *pmutex,
                                   const struct timespec  *rel_timeout)
{
    ULONG               ulTimeout;
    ULONG               ulError;
    
    if ((pcond == LW_NULL) || (pmutex == LW_NULL)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if ((rel_timeout == LW_NULL)    || 
        (rel_timeout->tv_nsec <  0) ||
        (rel_timeout->tv_nsec >= __TIMEVAL_NSEC_MAX)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    __pthread_cond_init_invisible(pcond);
    __pthread_mutex_init_invisible(pmutex);
    
    /*
     *  ע��: �� rel_timeout ����ulong tick��Χʱ, ����Ȼ�������, ���³�ʱʱ�䲻׼ȷ.
     */
    ulTimeout = __timespecToTick(rel_timeout);                          /*  �任Ϊ tick                 */
    
    ulError = API_ThreadCondWait(pcond, pmutex->PMUTEX_ulMutex, ulTimeout);
    if (ulError == ERROR_THREAD_WAIT_TIMEOUT) {
        errno = ETIMEDOUT;
        return  (ETIMEDOUT);
        
    } else if (ulError) {
        errno = EINVAL;
        return  (EINVAL);
        
    } else {
        return  (ERROR_NONE);
    }
}

#endif                                                                  /*  LW_CFG_POSIXEX_EN > 0       */
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/