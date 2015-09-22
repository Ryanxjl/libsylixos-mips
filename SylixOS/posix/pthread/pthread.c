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
** ��   ��   ��: pthread.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 12 �� 30 ��
**
** ��        ��: pthread ���ݿ�.

** BUG:
2010.01.13  �����߳�ʹ��Ĭ������ʱ, ���ȼ�Ӧʹ�� POSIX Ĭ�����ȼ�.
2010.05.07  pthread_create() Ӧ�ڵ��� API_ThreadStart() ֮ǰ���߳� ID д�������ַ.
2010.10.04  ���� pthread_create() ���� attr Ϊ NULL ʱ���߳�������������.
            �����̲߳��л��������.
2010.10.06  ���� pthread_set_name_np() ����.
2011.06.24  pthread_create() ��û�� attr ʱ, ʹ�� POSIX Ĭ�϶�ջ��С.
2012.06.18  �� posix �̵߳����ȼ�����, ����Ҫת�����ȼ�Ϊ sylixos ��׼. 
            posix ���ȼ�����Խ��, ���ȼ�Խ��, sylixos �պ��෴.
2013.05.01  If successful, the pthread_*() function shall return zero; 
            otherwise, an error number shall be returned to indicate the error.
2013.05.03  pthread_create() ������� attr ���� attr ָ���Ķ�ջ��СΪ 0, ��̳д����ߵĶ�ջ��С.
2013.05.07  ���� pthread_getname_np() ����.
2013.09.18  pthread_create() ����Զ�ջ�������Ĵ���.
2013.12.02  ���� pthread_yield().
2014.01.01  ���� pthread_safe_np() �� pthread_unsafe_np().
2014.07.04  ���� pthread_setaffinity_np �� pthread_getaffinity_np();
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../include/px_pthread.h"                                      /*  �Ѱ�������ϵͳͷ�ļ�        */
#include "../include/posixLib.h"                                        /*  posix �ڲ�������            */
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0
/*********************************************************************************************************
** ��������: pthread_atfork
** ��������: �����߳��� fork() ʱ��Ҫִ�еĺ���.
** �䡡��  : prepare       prepare ����ָ��
**           parent        ������ִ�к���ָ��
**           child         �ӽ���ִ�к���ָ��
**           arg           �߳���ڲ���
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_atfork (void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
    errno = ENOSYS;
    return  (ENOSYS);
}
/*********************************************************************************************************
** ��������: pthread_create
** ��������: ����һ�� posix �߳�.
** �䡡��  : pthread       �߳� id (����).
**           pattr         ��������
**           start_routine �߳����
**           arg           �߳���ڲ���
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_create (pthread_t              *pthread, 
                     const pthread_attr_t   *pattr, 
                     void                   *(*start_routine)(void *),
                     void                   *arg)
{
    LW_OBJECT_HANDLE        ulId;
    LW_CLASS_THREADATTR     lwattr;
    PLW_CLASS_TCB           ptcbCur;
    PCHAR                   pcName = "pthread";
    
    if (start_routine == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    lwattr = API_ThreadAttrGetDefault();                                /*  ���Ĭ���߳�����            */
    
    if (pattr) {
        if (!(pattr->PTHREADATTR_ulOption & LW_OPTION_THREAD_STK_CHK)) {/*  ���Կ�û�г�ʼ��            */
            errno = EINVAL;
            return  (EINVAL);
        }
        pcName = pattr->PTHREADATTR_pcName;                             /*  ʹ�� attr ��Ϊ�����߳���    */
        
        if (pattr->PTHREADATTR_stStackGuard > pattr->PTHREADATTR_stStackByteSize) {
            lwattr.THREADATTR_stGuardSize = LW_CFG_THREAD_DEFAULT_GUARD_SIZE;
        } else {
            lwattr.THREADATTR_stGuardSize = pattr->PTHREADATTR_stStackGuard;
        }
        
        if (pattr->PTHREADATTR_stStackByteSize == 0) {                  /*  �̳д�����                  */
            lwattr.THREADATTR_stStackByteSize = ptcbCur->TCB_stStackSize * sizeof(LW_STACK);
        } else {
            lwattr.THREADATTR_pstkLowAddr     = (PLW_STACK)pattr->PTHREADATTR_pvStackAddr;
            lwattr.THREADATTR_stStackByteSize = pattr->PTHREADATTR_stStackByteSize;
        }
        
        if (pattr->PTHREADATTR_iInherit == PTHREAD_INHERIT_SCHED) {     /*  �Ƿ�̳����ȼ�              */
            lwattr.THREADATTR_ucPriority =  ptcbCur->TCB_ucPriority;
        } else {
            lwattr.THREADATTR_ucPriority = 
                (UINT8)PX_PRIORITY_CONVERT(pattr->PTHREADATTR_schedparam.sched_priority);
        }
        
        lwattr.THREADATTR_ulOption = pattr->PTHREADATTR_ulOption;
    
    } else {                                                            /*  ��ջ��С�����ȼ�ȫ���̳�    */
        lwattr.THREADATTR_stStackByteSize = ptcbCur->TCB_stStackSize * sizeof(LW_STACK);
        lwattr.THREADATTR_ucPriority  = ptcbCur->TCB_ucPriority;
    }
    
    lwattr.THREADATTR_pvArg = arg;                                      /*  ��¼����                    */
    
    /*
     *  ��ʼ���߳�.
     */
    ulId = API_ThreadInit(pcName, (PTHREAD_START_ROUTINE)start_routine, &lwattr, LW_NULL);
    if (ulId == 0) {
        return  (errno);
    }
    
    if (pattr) {
        UINT8       ucPolicy        = (UINT8)pattr->PTHREADATTR_iSchedPolicy;
        UINT8       ucActivatedMode = LW_OPTION_RESPOND_AUTO;           /*  Ĭ����Ӧ��ʽ                */
        
        if (pattr->PTHREADATTR_iInherit == PTHREAD_INHERIT_SCHED) {     /*  �̳е��Ȳ���                */
            API_ThreadGetSchedParam(ptcbCur->TCB_ulId, &ucPolicy, &ucActivatedMode);
        }
        
        API_ThreadSetSchedParam(ulId, ucPolicy, ucActivatedMode);       /*  ���õ��Ȳ���                */
    }
    
    if (pthread) {
        *pthread = ulId;                                                /*  �����߳̾��                */
    }
    
    API_ThreadStart(ulId);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_cancel
** ��������: cancel һ�� posix �߳�.
** �䡡��  : thread       �߳� id.
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_cancel (pthread_t  thread)
{
    PX_ID_VERIFY(thread, pthread_t);
    
    if (API_ThreadCancel(&thread)) {
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_testcancel
** ��������: testcancel ��ǰ posix �߳�.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
void  pthread_testcancel (void)
{
    API_ThreadTestCancel();
}
/*********************************************************************************************************
** ��������: pthread_join
** ��������: join һ�� posix �߳�.
** �䡡��  : thread       �߳� id.
**           ppstatus     ��ȡ�̷߳���ֵ
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_join (pthread_t  thread, void **ppstatus)
{
    if (API_ThreadJoin(thread, ppstatus)) {
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_detach
** ��������: detach һ�� posix �߳�.
** �䡡��  : thread       �߳� id.
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_detach (pthread_t  thread)
{
    PX_ID_VERIFY(thread, pthread_t);
    
    if (API_ThreadDetach(thread)) {
        return  (errno);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_equal
** ��������: �ж����� posix �߳��Ƿ���ͬ.
** �䡡��  : thread1       �߳� id.
**           thread2       �߳� id.
** �䡡��  : true or false
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_equal (pthread_t  thread1, pthread_t  thread2)
{
    PX_ID_VERIFY(thread1, pthread_t);
    PX_ID_VERIFY(thread2, pthread_t);

    return  (thread1 == thread2);
}
/*********************************************************************************************************
** ��������: pthread_exit
** ��������: ɾ����ǰ posix �߳�.
** �䡡��  : status        exit ����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
void  pthread_exit (void *status)
{
    LW_OBJECT_HANDLE    ulId = API_ThreadIdSelf();

    API_ThreadDelete(&ulId, status);
}
/*********************************************************************************************************
** ��������: pthread_self
** ��������: ��õ�ǰ posix �߳̾��.
** �䡡��  : NONE
** �䡡��  : ���
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
pthread_t  pthread_self (void)
{
    return  (API_ThreadIdSelf());
}
/*********************************************************************************************************
** ��������: pthread_yield
** ��������: ����ǰ������뵽ͬ���ȼ���������������, �����ó�һ�� CPU ����.
** �䡡��  : NONE
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_yield (void)
{
    API_ThreadYield(API_ThreadIdSelf());
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_kill
** ��������: ��ָ�� posix �̷߳���һ���ź�.
** �䡡��  : thread        �߳̾��
**           signo         �ź�
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_SIGNAL_EN > 0

LW_API 
int  pthread_kill (pthread_t  thread, int signo)
{
    int  error;

    PX_ID_VERIFY(thread, pthread_t);
    
    error = kill(thread, signo);
    if (error < ERROR_NONE) {
        return  (errno);
    } else {
        return  (error);
    }
}
/*********************************************************************************************************
** ��������: pthread_sigmask
** ��������: �޸� posix �߳��ź�����.
** �䡡��  : how           ����
**           newmask       �µ��ź�����
**           oldmask       �ɵ��ź�����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_sigmask (int  how, const sigset_t  *newmask, sigset_t *oldmask)
{
    int  error;
    
    error = sigprocmask(how, newmask, oldmask);
    if (error < ERROR_NONE) {
        return  (errno);
    } else {
        return  (error);
    }
}

#endif                                                                  /*  LW_CFG_SIGNAL_EN > 0        */
/*********************************************************************************************************
** ��������: pthread_cleanup_pop
** ��������: ��һ��ѹջ�������в��ͷ�
** �䡡��  : iNeedRun          �Ƿ�ִ��
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
void  pthread_cleanup_pop (int  iNeedRun)
{
    API_ThreadCleanupPop((BOOL)iNeedRun);
}
/*********************************************************************************************************
** ��������: pthread_cleanup_push
** ��������: ��һ���������ѹ�뺯����ջ
** �䡡��  : pfunc         ����ָ��
**           arg           ��������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
void  pthread_cleanup_push (void (*pfunc)(void *), void *arg)
{
    API_ThreadCleanupPush(pfunc, arg);
}
/*********************************************************************************************************
** ��������: pthread_getschedparam
** ��������: ��õ���������
** �䡡��  : thread        �߳̾��
**           piPolicy      ���Ȳ���(����)
**           pschedparam   ����������(����)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_getschedparam (pthread_t            thread, 
                            int                 *piPolicy, 
                            struct sched_param  *pschedparam)
{
    UINT8       ucPriority;
    UINT8       ucPolicy;
    
    PX_ID_VERIFY(thread, pthread_t);
    
    if (pschedparam) {
        if (API_ThreadGetPriority(thread, &ucPriority)) {
            errno = ESRCH;
            return  (ESRCH);
        }
        pschedparam->sched_priority = PX_PRIORITY_CONVERT(ucPriority);
    }
    
    if (piPolicy) {
        if (API_ThreadGetSchedParam(thread, &ucPolicy, LW_NULL)) {
            errno = ESRCH;
            return  (ESRCH);
        }
        *piPolicy = (int)ucPolicy;
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_getschedparam
** ��������: ���õ���������
** �䡡��  : thread        �߳̾��
**           iPolicy       ���Ȳ���
**           pschedparam   ����������
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_setschedparam (pthread_t                  thread, 
                            int                        iPolicy, 
                            const struct sched_param  *pschedparam)
{
    UINT8       ucPriority;
    UINT8       ucActivatedMode;

    if (pschedparam == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if ((iPolicy != LW_OPTION_SCHED_FIFO) &&
        (iPolicy != LW_OPTION_SCHED_RR)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if ((pschedparam->sched_priority < __PX_PRIORITY_MIN) ||
        (pschedparam->sched_priority > __PX_PRIORITY_MAX)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    ucPriority= (UINT8)PX_PRIORITY_CONVERT(pschedparam->sched_priority);
    
    PX_ID_VERIFY(thread, pid_t);
    
    if (API_ThreadGetSchedParam(thread,
                                LW_NULL,
                                &ucActivatedMode)) {
        errno = ESRCH;
        return  (ESRCH);
    }
    
    API_ThreadSetSchedParam(thread, (UINT8)iPolicy, ucActivatedMode);
    
    if (API_ThreadSetPriority(thread, ucPriority)) {
        errno = ESRCH;
        return  (ESRCH);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: pthread_onec
** ��������: �̰߳�ȫ�Ľ�ִ��һ��ָ������
** �䡡��  : thread        �߳̾��
**           once          onec_t����
**           pfunc         ����ָ��
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_once (pthread_once_t  *once, void (*pfunc)(void))
{
    int  error;
    
    error = API_ThreadOnce(once, pfunc);
    if (error < ERROR_NONE) {
        return  (errno);
    } else {
        return  (error);
    }
}
/*********************************************************************************************************
** ��������: pthread_setschedprio
** ��������: �����߳����ȼ�
** �䡡��  : thread        �߳̾��
**           prio          ���ȼ�
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_setschedprio (pthread_t  thread, int  prio)
{
    UINT8       ucPriority;

    if ((prio < __PX_PRIORITY_MIN) ||
        (prio > __PX_PRIORITY_MAX)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    PX_ID_VERIFY(thread, pthread_t);
    
    ucPriority= (UINT8)PX_PRIORITY_CONVERT(prio);
    
    if (API_ThreadSetPriority(thread, ucPriority)) {
        errno = ESRCH;
        return  (ESRCH);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_getschedprio
** ��������: ����߳����ȼ�
** �䡡��  : thread        �߳̾��
**           prio          ���ȼ�(����)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_getschedprio (pthread_t  thread, int  *prio)
{
    UINT8       ucPriority;

    PX_ID_VERIFY(thread, pthread_t);

    if (prio) {
        if (API_ThreadGetPriority(thread, &ucPriority)) {
            errno = ESRCH;
            return  (ESRCH);
        }
        *prio = PX_PRIORITY_CONVERT(ucPriority);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_setcancelstate
** ��������: ����ȡ���߳��Ƿ�ʹ��
** �䡡��  : newstate      �µ�״̬
**           poldstate     ��ǰ��״̬(����)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_setcancelstate (int  newstate, int  *poldstate)
{
    ULONG   ulError = API_ThreadSetCancelState(newstate, poldstate);
    
    if (ulError) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_setcanceltype
** ��������: ���õ�ǰ�̱߳���ȡ��ʱ�Ķ���
** �䡡��  : newtype      �µĶ���
**           poldtype     ��ǰ�Ķ���(����)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_setcanceltype (int  newtype, int  *poldtype)
{
    ULONG   ulError = API_ThreadSetCancelType(newtype, poldtype);
    
    if (ulError) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_setconcurrency
** ��������: �����߳��µĲ��м���
** �䡡��  : newlevel      �²��м���
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_setconcurrency (int  newlevel)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_getconcurrency
** ��������: ��õ�ǰ�̵߳Ĳ��м���
** �䡡��  : NONE
** �䡡��  : ��ǰ�̵߳Ĳ��м���
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_getconcurrency (void)
{
    return  (LW_CFG_MAX_THREADS);
}
/*********************************************************************************************************
** ��������: pthread_getcpuclockid
** ��������: ����߳� CPU ʱ�� clock id.
** �䡡��  : thread    �߳̾��
**           clock_id  ʱ�� id
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_getcpuclockid (pthread_t thread, clockid_t *clock_id)
{
    if (!clock_id) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    *clock_id = CLOCK_THREAD_CPUTIME_ID;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_setname_np
** ��������: �����߳�����
** �䡡��  : thread    �߳̾��
**           name      �߳�����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_POSIXEX_EN > 0

LW_API 
int  pthread_setname_np (pthread_t  thread, const char  *name)
{
    ULONG   ulError;
    
    PX_ID_VERIFY(thread, pthread_t);

    ulError = API_ThreadSetName(thread, name);
    if (ulError == ERROR_KERNEL_PNAME_TOO_LONG) {
        errno = ERANGE;
        return  (ERANGE);
    
    } else if (ulError) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_getname_np
** ��������: ����߳�����
** �䡡��  : thread    �߳̾��
**           name      �߳�����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_getname_np (pthread_t  thread, char  *name, size_t len)
{
    ULONG   ulError;
    CHAR    cNameBuffer[LW_CFG_OBJECT_NAME_SIZE];
    
    PX_ID_VERIFY(thread, pthread_t);
    
    ulError = API_ThreadGetName(thread, cNameBuffer);
    if (ulError) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if (lib_strlen(cNameBuffer) >= len) {
        errno = ERANGE;
        return  (ERANGE);
    }
    
    lib_strcpy(name, cNameBuffer);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_safe_np
** ��������: �߳̽��밲ȫģʽ, �κζԱ��̵߳�ɾ�����������Ƴٵ��߳��˳���ȫģʽʱ����.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
void  pthread_safe_np (void)
{
    API_ThreadSafe();
}
/*********************************************************************************************************
** ��������: pthread_unsafe_np
** ��������: �߳��˳���ȫģʽ.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
void  pthread_unsafe_np (void)
{
    API_ThreadUnsafe();
}
/*********************************************************************************************************
** ��������: pthread_int_lock_np
** ��������: �߳�������ǰ���ں��ж�, ������Ӧ�ж�.
** �䡡��  : irqctx        ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_int_lock_np (pthread_int_t *irqctx)
{
    if (!irqctx) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    *irqctx = KN_INT_DISABLE();
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_int_unlock_np
** ��������: �߳̽�����ǰ���ں��ж�, ��ʼ��Ӧ�ж�.
** �䡡��  : irqctx        ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_int_unlock_np (pthread_int_t irqctx)
{
    KN_INT_ENABLE(irqctx);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_setaffinity_np
** ��������: �����̵߳��ȵ� CPU ����
** �䡡��  : pid           ���� / �߳� ID
**           setsize       CPU ���ϴ�С
**           set           CPU ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : SylixOS Ŀǰ��֧�ֽ�����������һ�� CPU, ��������ֻ�ܽ��߳�������ָ������С CPU ����.
**
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_setaffinity_np (pthread_t  thread, size_t setsize, const cpu_set_t *set)
{
#if LW_CFG_SMP_EN > 0
    if (!setsize || !set) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    if (API_ThreadSetAffinity(thread, setsize, (PLW_CLASS_CPUSET)set)) {
        _ErrorHandle(ESRCH);
        return  (ESRCH);
    }
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_getaffinity_np
** ��������: ��ȡ�̵߳��ȵ� CPU ����
** �䡡��  : pid           ���� / �߳� ID
**           setsize       CPU ���ϴ�С
**           set           CPU ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_getaffinity_np (pthread_t  thread, size_t setsize, cpu_set_t *set)
{
#if LW_CFG_SMP_EN > 0
    if ((setsize < sizeof(cpu_set_t)) || !set) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    if (API_ThreadGetAffinity(thread, setsize, set)) {
        _ErrorHandle(ESRCH);
        return  (ESRCH);
    }
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_POSIXEX_EN > 0       */
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
