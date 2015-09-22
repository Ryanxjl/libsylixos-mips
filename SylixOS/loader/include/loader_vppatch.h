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
** ��   ��   ��: loader_vppatch.h
**
** ��   ��   ��: Han.hui (����)
**
** �ļ���������: 2010 �� 12 �� 08 ��
**
** ��        ��: ���̿�
*********************************************************************************************************/

#ifndef __LOADER_VPPATCH_H
#define __LOADER_VPPATCH_H

#include "SylixOS.h"
#include "loader_lib.h"

/*********************************************************************************************************
  vprocess ���ƿ����
*********************************************************************************************************/

#define __LW_VP_GET_TCB_PROC(ptcb)      ((LW_LD_VPROC *)(ptcb->TCB_pvVProcessContext))
#define __LW_VP_GET_CUR_PROC()          ((LW_LD_VPROC *)(API_ThreadTcbSelf()->TCB_pvVProcessContext))
#define __LW_VP_SET_CUR_PROC(pvproc)    (API_ThreadTcbSelf()->TCB_pvVProcessContext = (PVOID)(pvproc))

/*********************************************************************************************************
  vprocess pid
*********************************************************************************************************/

static LW_INLINE pid_t __lw_vp_get_tcb_pid (PLW_CLASS_TCB ptcb)
{
    LW_LD_VPROC  *pvproc = __LW_VP_GET_TCB_PROC(ptcb);
    
    if (pvproc) {
        return  (pvproc->VP_pid);
    } else {
        return  (0);
    }
}

/*********************************************************************************************************
  vprocess �ڲ�����
*********************************************************************************************************/

VOID                vprocThreadExitHook(PVOID  pvVProc, LW_OBJECT_HANDLE  ulId);
INT                 vprocSetGroup(pid_t  pid, pid_t  pgid);
pid_t               vprocGetGroup(pid_t  pid);
pid_t               vprocGetFather(pid_t  pid);
INT                 vprocDetach(pid_t  pid);
LW_LD_VPROC        *vprocCreate(CPCHAR  pcFile);
INT                 vprocDestroy(LW_LD_VPROC *pvproc);                  /*  û�е��� vprocRun �ſɵ���  */
LW_LD_VPROC        *vprocGet(pid_t  pid);
pid_t               vprocGetPidByTcb(PLW_CLASS_TCB ptcb);
pid_t               vprocGetPidByTcbdesc(PLW_CLASS_TCB_DESC  ptcbdesc);
LW_OBJECT_HANDLE    vprocMainThread(pid_t pid);
INT                 vprocNotifyParent(LW_LD_VPROC *pvproc, INT  iSigCode, BOOL  bUpDateStat);
VOID                vprocReclaim(LW_LD_VPROC *pvproc, BOOL  bFreeVproc);
INT                 vprocSetForceTerm(pid_t  pid);
VOID                vprocExit(LW_LD_VPROC *pvproc, LW_OBJECT_HANDLE  ulId, INT  iCode);
VOID                vprocExitNotDestroy(LW_LD_VPROC *pvproc);
INT                 vprocRun(LW_LD_VPROC      *pvproc, 
                             LW_LD_VPROC_STOP *pvpstop,
                             CPCHAR            pcFile, 
                             CPCHAR            pcEntry, 
                             INT              *piRet,
                             INT               iArgC, 
                             CPCHAR            ppcArgV[],
                             CPCHAR            ppcEnv[]);
VOID                vprocTickHook(PLW_CLASS_TCB  ptcb, PLW_CLASS_CPU  pcpu);
PLW_IO_ENV          vprocIoEnvGet(PLW_CLASS_TCB  ptcb);
FUNCPTR             vprocGetMain(VOID);
pid_t               vprocFindProc(PVOID  pvAddr);
INT                 vprocGetPath(pid_t  pid, PCHAR  pcPath, size_t stMaxLen);
VOID                vprocThreadAdd(PVOID   pvVProc, PLW_CLASS_TCB  ptcb);
VOID                vprocThreadDel(PVOID   pvVProc, PLW_CLASS_TCB  ptcb);
INT                 vprocThreadNum(pid_t pid, ULONG  *pulCnt);
VOID                vprocThreadTraversal(PVOID          pvVProc, 
                                         VOIDFUNCPTR    pfunc, 
                                         PVOID          pvArg0,
                                         PVOID          pvArg1,
                                         PVOID          pvArg2,
                                         PVOID          pvArg3,
                                         PVOID          pvArg4,
                                         PVOID          pvArg5);
INT                 vprocThreadTraversal2(pid_t          pid, 
                                          VOIDFUNCPTR    pfunc, 
                                          PVOID          pvArg0,
                                          PVOID          pvArg1,
                                          PVOID          pvArg2,
                                          PVOID          pvArg3,
                                          PVOID          pvArg4,
                                          PVOID          pvArg5);

#if LW_CFG_GDB_EN > 0
ssize_t             vprocGetModsInfo(pid_t  pid, PCHAR  pcBuff, size_t stMaxLen);
#endif                                                                  /*  LW_CFG_GDB_EN > 0           */

/*********************************************************************************************************
  ���� CPU �׺Ͷ�
*********************************************************************************************************/

#if LW_CFG_SMP_EN > 0
INT                 vprocSetAffinity(pid_t  pid, size_t  stSize, const PLW_CLASS_CPUSET  pcpuset);
INT                 vprocGetAffinity(pid_t  pid, size_t  stSize, PLW_CLASS_CPUSET  pcpuset);
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

/*********************************************************************************************************
  vprocess �ļ�����������
*********************************************************************************************************/

VOID                vprocIoFileInit(LW_LD_VPROC *pvproc);               /*  vprocCreate  �ڱ�����       */
VOID                vprocIoFileDeinit(LW_LD_VPROC *pvproc);             /*  vprocDestroy �ڱ�����       */
PLW_FD_ENTRY        vprocIoFileGet(INT  iFd, BOOL  bIsIgnAbn);
PLW_FD_ENTRY        vprocIoFileGetEx(LW_LD_VPROC *pvproc, INT  iFd, BOOL  bIsIgnAbn);
PLW_FD_DESC         vprocIoFileDescGet(INT  iFd, BOOL  bIsIgnAbn);
INT                 vprocIoFileDup(PLW_FD_ENTRY pfdentry, INT  iMinFd);
INT                 vprocIoFileDup2(PLW_FD_ENTRY pfdentry, INT  iNewFd);
INT                 vprocIoFileRefInc(INT  iFd);
INT                 vprocIoFileRefDec(INT  iFd);
INT                 vprocIoFileRefGet(INT  iFd);

/*********************************************************************************************************
  �ļ����������ݲ���
*********************************************************************************************************/

INT                 vprocIoFileDupFrom(pid_t  pidSrc, INT  iFd);
INT                 vprocIoFileRefIncByPid(pid_t  pid, INT  iFd);
INT                 vprocIoFileRefDecByPid(pid_t  pid, INT  iFd);

/*********************************************************************************************************
  ��Դ�������������º���
*********************************************************************************************************/

VOID                vprocIoReclaim(pid_t  pid, BOOL  bIsExec);

/*********************************************************************************************************
  ���̶�ʱ��
*********************************************************************************************************/
#if LW_CFG_PTIMER_EN > 0

VOID                vprocItimerHook(PLW_CLASS_TCB  ptcb, PLW_CLASS_CPU  pcpu);

INT                 vprocSetitimer(INT        iWhich, 
                                   ULONG      ulCounter,
                                   ULONG      ulInterval,
                                   ULONG     *pulCounter,
                                   ULONG     *pulInterval);
INT                 vprocGetitimer(INT        iWhich, 
                                   ULONG     *pulCounter,
                                   ULONG     *pulInterval);

#endif                                                                  /*  LW_CFG_PTIMER_EN > 0        */
#endif                                                                  /*  __LOADER_SYMBOL_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
