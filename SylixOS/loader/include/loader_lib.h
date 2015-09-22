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
** ��   ��   ��: loader_lib.h
**
** ��   ��   ��: Jiang.Taijin (��̫��)
**
** �ļ���������: 2010 �� 04 �� 17 ��
**
** ��        ��: ����ͷ�ļ�

** BUG:
2011.02.20  Ϊ���� posix ��̬���ӿ��׼, ����Է���ȫ������ֲ��ԵĴ���.
2012.09.21  ���� BSD ����, �����˶� issetugid �Ĺ���.
2012.10.25  ������� I/O ����.
2012.12.09  �������������.
2012.12.18  �������˽�� FD ��.
2014.05.13  �������߳�����.
2014.09.30  ������̶�ʱ��.
2015.09.01  ����MIPS�ܹ�
*********************************************************************************************************/

#ifndef __LOADER_LIB_H
#define __LOADER_LIB_H

/*********************************************************************************************************
  ��������ļ������� 
  (��Ϊ����0 1 2��׼�ļ����ں�һ��ӳ�䷽ʽ����, ����ı�׼�ļ�Ϊ��ʵ�򿪵��ļ�, ����û�� STD_UNFIX ����.
   Ϊ�˼̳��ں��ļ�������, �������Ϊ LW_CFG_MAX_FILES + 3)
*********************************************************************************************************/

#define LW_VP_MAX_FILES     (LW_CFG_MAX_FILES + 3)

/*********************************************************************************************************
  ���̶�ʱ��
*********************************************************************************************************/

typedef struct lw_vproc_timer {
    ULONG                   VPT_ulCounter;                              /*  ��ʱ����ǰ��ʱʱ��          */
    ULONG                   VPT_ulInterval;                             /*  ��ʱ���Զ�װ��ֵ            */
} LW_LD_VPROC_T;

/*********************************************************************************************************
  ���̿��ƿ�
*********************************************************************************************************/

typedef struct lw_ld_vproc {
    LW_LIST_LINE            VP_lineManage;                              /*  ��������                    */
    LW_LIST_RING_HEADER     VP_ringModules;                             /*  ģ������                    */
    FUNCPTR                 VP_pfuncProcess;                            /*  ���������                  */
    
    LW_OBJECT_HANDLE        VP_ulModuleMutex;                           /*  ����ģ��������              */
    
    BOOL                    VP_bRunAtExit;                              /*  �Ƿ����� atexit ��װ�ĺ���  */
    BOOL                    VP_bForceTerm;                              /*  �Ƿ��Ǳ�ǿ���˳�            */
    
    pid_t                   VP_pid;                                     /*  ���̺�                      */
    BOOL                    VP_bIssetugid;                              /*  �Ƿ����õ� ugid             */
    PCHAR                   VP_pcName;                                  /*  ��������                    */
    LW_OBJECT_HANDLE        VP_ulMainThread;                            /*  ���߳̾��                  */
    PVOID                   VP_pvProcInfo;                              /*  proc �ļ�ϵͳ��Ϣ           */
    
    clock_t                 VP_clockUser;                               /*  times ��Ӧ�� utime          */
    clock_t                 VP_clockSystem;                             /*  times ��Ӧ�� stime          */
    clock_t                 VP_clockCUser;                              /*  times ��Ӧ�� cutime         */
    clock_t                 VP_clockCSystem;                            /*  times ��Ӧ�� cstime         */
    
    PLW_IO_ENV              VP_pioeIoEnv;                               /*  I/O ����                    */
    LW_OBJECT_HANDLE        VP_ulWaitForExit;                           /*  ���̵߳ȴ�����              */

#define __LW_VP_INIT        0
#define __LW_VP_RUN         1
#define __LW_VP_EXIT        2
#define __LW_VP_STOP        3
    INT                     VP_iStatus;                                 /*  ��ǰ����״̬                */
    INT                     VP_iExitCode;                               /*  ��������                    */
    INT                     VP_iSigCode;                                /*  iSigCode                    */
    
#define __LW_VP_FT_DAEMON   0x01                                        /*  daemon ����                 */
    ULONG                   VP_ulFeatrues;                              /*  ���� featrues               */
    
    struct lw_ld_vproc     *VP_pvprocFather;                            /*  ���� (NULL ��ʾ�¶�����)    */
    LW_LIST_LINE_HEADER     VP_plineChild;                              /*  ���ӽ�������ͷ              */
    LW_LIST_LINE            VP_lineBrother;                             /*  �ֵܽ���                    */
    pid_t                   VP_pidGroup;                                /*  �� id ��                    */
    LW_LIST_LINE_HEADER     VP_plineThread;                             /*  ���߳�����                  */
    
    LW_FD_DESC              VP_fddescTbl[LW_VP_MAX_FILES];              /*  ���� fd ��                  */
    
    INT                     VP_iExitMode;                               /*  �˳�ģʽ                    */
    LW_LD_VPROC_T           VP_vptimer[3];                              /*  REAL / VIRTUAL / PROF ��ʱ��*/
    
    LW_LIST_LINE_HEADER     VP_plineMap;                                /*  �����ڴ�ռ�                */
    ULONG                   VP_ulPad[7];                                /*  Ԥ��                        */
} LW_LD_VPROC;

/*********************************************************************************************************
  ����װ�����ֹͣ���� (sigval.sival_int == 0 ����װ����ɲ�ֹͣ, < 0 ��ʾװ���쳣, ���������˳�)
*********************************************************************************************************/

typedef struct {
    INT                     VPS_iSigNo;                                 /*  װ����ɺ��͵��ź�        */
    LW_OBJECT_HANDLE        VPS_ulId;                                   /*  Ŀ���߳� (�����)           */
} LW_LD_VPROC_STOP;

/*********************************************************************************************************
  ģ������ʱռ�õ��ڴ�Σ�ж��ʱ���ͷ���Щ��
*********************************************************************************************************/

typedef struct {
    addr_t                  ESEG_ulAddr;                                /*  �ڴ�ε�ַ                  */
    size_t                  ESEG_stLen;                                 /*  �ڴ�γ���                  */
} LW_LD_EXEC_SEGMENT;

/*********************************************************************************************************
  ģ����ű���
*********************************************************************************************************/

typedef struct {
    LW_LIST_LINE            ESYM_lineManage;                            /*  ��������                    */
    size_t                  ESYM_stSize;                                /*  �ֶλ����С                */
    size_t                  ESYM_stUsed;                                /*  ���ֶ��Ѿ�ʹ�õ��ڴ�����    */
} LW_LD_EXEC_SYMBOL;

#define LW_LD_EXEC_SYMBOL_HDR_SIZE  ROUND_UP(sizeof(LW_LD_EXEC_SYMBOL), sizeof(LW_STACK))

/*********************************************************************************************************
  ģ�����ͣ�KO\SO
*********************************************************************************************************/

#define LW_LD_MOD_TYPE_KO           0                                   /*  �ں�ģ��                    */
#define LW_LD_MOD_TYPE_SO           1                                   /*  Ӧ�ó����̬���ӿ�        */

/*********************************************************************************************************
  ģ������״̬
*********************************************************************************************************/

#define LW_LD_STATUS_UNLOAD         0                                   /*  δ����                      */
#define LW_LD_STATUS_LOADED         1                                   /*  �Ѽ���δ��ʼ��              */
#define LW_LD_STATUS_INITED         2                                   /*  �ѳ�ʼ��                    */
#define LW_LD_STATUS_FINIED         3                                   /*  �ѳ�ʼ��                    */

/*********************************************************************************************************
  ģ��ṹ��������֯ģ�����Ϣ
*********************************************************************************************************/

#define __LW_LD_EXEC_MODULE_MAGIC   0x25ef68af

typedef struct {
    ULONG                   EMOD_ulMagic;                               /*  ����ʶ�𱾽ṹ��            */
    ULONG                   EMOD_ulModType;                             /*  ģ�����ͣ�KO����SO�ļ�      */
    ULONG                   EMOD_ulStatus;                              /*  ģ�鵱ǰ����״̬            */
    ULONG                   EMOD_ulRefs;                                /*  ģ�����ü���                */
    PVOID                   EMOD_pvUsedArr;                             /*  ������ģ������              */
    ULONG                   EMOD_ulUsedCnt;                             /*  ����ģ�������С            */

    PCHAR                   EMOD_pcSymSection;                          /*  ������ָ�� section ���ű�   */

    VOIDFUNCPTR            *EMOD_ppfuncInitArray;                       /*  ��ʼ����������              */
    ULONG                   EMOD_ulInitArrCnt;                          /*  ��ʼ����������              */
    VOIDFUNCPTR            *EMOD_ppfuncFiniArray;                       /*  ������������                */
    ULONG                   EMOD_ulFiniArrCnt;                          /*  ������������                */

    PCHAR                   EMOD_pcInit;                                /*  ��ʼ����������              */
    FUNCPTR                 EMOD_pfuncInit;                             /*  ��ʼ������ָ��              */

    PCHAR                   EMOD_pcExit;                                /*  ������������                */
    FUNCPTR                 EMOD_pfuncExit;                             /*  ��������ָ��                */

    PCHAR                   EMOD_pcEntry;                               /*  ��ں�������                */
    FUNCPTR                 EMOD_pfuncEntry;                            /*  main����ָ��                */
    FUNCPTR                 EMOD_pfuncDestroy;                          /*  ����ģ�麯��ָ��            */
    BOOL                    EMOD_bIsSymbolEntry;                        /*  �Ƿ�Ϊ�������              */

    size_t                  EMOD_stLen;                                 /*  Ϊģ�������ڴ泤��        */
    PVOID                   EMOD_pvBaseAddr;                            /*  Ϊģ�������ڴ��ַ        */

    BOOL                    EMOD_bIsGlobal;                             /*  �Ƿ�Ϊȫ�ַ���              */
    ULONG                   EMOD_ulSymCount;                            /*  ����������Ŀ                */
    ULONG                   EMOD_ulSymHashSize;                         /*  ���� hash ���С            */
    LW_LIST_LINE_HEADER    *EMOD_psymbolHash;                           /*  ���������� hash ��          */
    LW_LIST_LINE_HEADER     EMOD_plineSymbolBuffer;                     /*  �������ű���              */

    ULONG                   EMOD_ulSegCount;                            /*  �ڴ����Ŀ                  */
    LW_LD_EXEC_SEGMENT     *EMOD_psegmentArry;                          /*  �ڴ���б�                  */
    BOOL                    EMOD_bExportSym;                            /*  �Ƿ񵼳�����                */
    PCHAR                   EMOD_pcModulePath;                          /*  ģ���ļ�·��                */

    LW_LD_VPROC            *EMOD_pvproc;                                /*  ��������                    */
    LW_LIST_RING            EMOD_ringModules;                           /*  ���������еĿ�����          */
    PVOID                   EMOD_pvFormatInfo;                          /*  �ض�λ�����Ϣ              */

#ifdef LW_CFG_CPU_ARCH_ARM
    size_t                  EMOD_stARMExidxCount;                       /*  ARM.exidx �γ���            */
    PVOID                   EMOD_pvARMExidx;                            /*  ARM.exidx ���ڴ��ַ        */
#endif                                                                  /*  LW_CFG_CPU_ARCH_ARM         */

} LW_LD_EXEC_MODULE;

/*********************************************************************************************************
  �ں˽��̿��ƿ�
*********************************************************************************************************/

extern LW_LD_VPROC          _G_vprocKernel;

/*********************************************************************************************************
  module ������
*********************************************************************************************************/

extern LW_OBJECT_HANDLE     _G_ulVProcMutex;

#define LW_LD_LOCK()        API_SemaphoreMPend(_G_ulVProcMutex, LW_OPTION_WAIT_INFINITE)
#define LW_LD_UNLOCK()      API_SemaphoreMPost(_G_ulVProcMutex)

#define LW_VP_LOCK(a)       API_SemaphoreMPend(a->VP_ulModuleMutex, LW_OPTION_WAIT_INFINITE)
#define LW_VP_UNLOCK(a)     API_SemaphoreMPost(a->VP_ulModuleMutex)

/*********************************************************************************************************
  ������Ϣ��ӡ
*********************************************************************************************************/

#ifdef  __SYLIXOS_DEBUG
#define LD_DEBUG_MSG(msg)   printf msg
#else
#define LD_DEBUG_MSG(msg)
#endif                                                                  /*  __LOAD_DEBUG                */

/*********************************************************************************************************
  ����Ĭ�������Ӧ�����
*********************************************************************************************************/

#define LW_LD_DEFAULT_ENTRY         "_start"                            /*  ���̳�ʼ����ڷ���          */
#define LW_LD_PROCESS_ENTRY         "main"                              /*  ��������ڷ���              */

/*********************************************************************************************************
  С�����ڴ����
*********************************************************************************************************/

#define LW_LD_SAFEMALLOC(size)      __SHEAP_ALLOC((size_t)size)
#define LW_LD_SAFEFREE(a)           { if (a) { __SHEAP_FREE((PVOID)a); a = 0; } }
                                                                        /*  ��ȫ�ͷ�                    */

/*********************************************************************************************************
  �ں�ģ��������ڴ����
*********************************************************************************************************/

PVOID __ldMalloc(size_t  stLen);                                        /*  �����ڴ�                    */
PVOID __ldMallocAlign(size_t  stLen, size_t  stAlign);
VOID  __ldFree(PVOID  pvAddr);                                          /*  �ͷ��ڴ�                    */

#define LW_LD_VMSAFEMALLOC(size)    \
        __ldMalloc(size)
        
#define LW_LD_VMSAFEMALLOC_ALIGN(size, align)   \
        __ldMallocAlign(size, align)
        
#define LW_LD_VMSAFEFREE(a) \
        { if (a) { __ldFree((PVOID)a); a = 0; } }

/*********************************************************************************************************
  �����������ڴ����
*********************************************************************************************************/

extern LW_OBJECT_HANDLE             _G_ulExecShareLock;                 /*  ������ڴ����              */

PVOID  __ldMallocArea(size_t  stLen);
PVOID  __ldMallocAreaAlign(size_t  stLen, size_t  stAlign);
VOID   __ldFreeArea(PVOID  pvAddr);

#define LW_LD_VMSAFEMALLOC_AREA(size)   \
        __ldMallocArea(size)
        
#define LW_LD_VMSAFEMALLOC_AREA_ALIGN(size, align)  \
        __ldMallocAreaAlign(size, align)
        
#define LW_LD_VMSAFEFREE_AREA(a)    \
        { if (a) { __ldFreeArea((PVOID)a); a = 0; } }
        
INT     __ldMmap(PVOID  pvBase, size_t  stAddrOft, INT  iFd, struct stat64 *pstat64,
                 off_t  oftOffset, size_t  stLen,  BOOL  bCanShare, BOOL  bCanExec);
VOID    __ldShare(PVOID  pvBase, size_t  stLen, dev_t  dev, ino64_t ino64);
VOID    __ldShareAbort(dev_t  dev, ino64_t  ino64);
INT     __ldShareConfig(BOOL  bShareEn, BOOL  *pbPrev);

#define LW_LD_VMSAFEMAP_AREA(base, addr_offset, fd, pstat64, file_offset, len, can_share, can_exec) \
        __ldMmap(base, addr_offset, fd, pstat64, file_offset, len, can_share, can_exec)
        
#define LW_LD_VMSAFE_SHARE(base, len, dev, ino64) \
        __ldShare(base, len, dev, ino64)
        
#define LW_LD_VMSAFE_SHARE_ABORT(dev, ino64)    \
        __ldShareAbort(dev, ino64)
        
#define LW_LD_VMSAFE_SHARE_CONFIG(can_share, prev)  \
        __ldShareConfig(can_share, prev)

/*********************************************************************************************************
  ��ַת��
*********************************************************************************************************/

#define LW_LD_V2PADDR(vBase, pBase, vAddr)      \
        ((size_t)pBase + (size_t)vAddr - (size_t)vBase)                 /*  ���������ַ                */

#define LW_LD_P2VADDR(vBase, pBase, pAddr)      \
        ((size_t)vBase + (size_t)pAddr - (size_t)pBase)                 /*  ���������ַ                */
        
/*********************************************************************************************************
  vp ��������
*********************************************************************************************************/
#define LW_LD_VMEM_MAX      64

PCHAR __moduleVpPatchVersion(LW_LD_EXEC_MODULE *pmodule);               /*  ��ò����汾                */

#if LW_CFG_VMM_EN == 0
PVOID __moduleVpPatchHeap(LW_LD_EXEC_MODULE *pmodule);                  /*  ��ò��������ڴ��          */
#endif                                                                  /*  LW_CFG_VMM_EN == 0          */

INT   __moduleVpPatchVmem(LW_LD_EXEC_MODULE *pmodule, PVOID  ppvArea[], INT  iSize);
                                                                        /*  ��ý��������ڴ�ռ�        */
VOID  __moduleVpPatchInit(LW_LD_EXEC_MODULE *pmodule);                  /*  ���̲�������������          */
VOID  __moduleVpPatchFini(LW_LD_EXEC_MODULE *pmodule);
/*********************************************************************************************************
  ��ϵ�ṹ���к���
*********************************************************************************************************/

#ifdef LW_CFG_CPU_ARCH_ARM
typedef long unsigned int   *_Unwind_Ptr;
_Unwind_Ptr dl_unwind_find_exidx(_Unwind_Ptr pc, int *pcount, PVOID *pvVProc);
#endif                                                                  /*  LW_CFG_CPU_ARCH_ARM         */

#ifdef LW_CFG_CPU_ARCH_MIPS
typedef long unsigned int   *_Unwind_Ptr;
_Unwind_Ptr dl_unwind_find_exidx(_Unwind_Ptr pc, int *pcount, PVOID *pvVProc);
#endif                                                                  /*  LW_CFG_CPU_ARCH_ARM         */

#endif                                                                  /*  __LOADER_LIB_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
