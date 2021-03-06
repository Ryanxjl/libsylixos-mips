/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: vmm.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2008 年 12 月 09 日
**
** 描        述: 平台无关虚拟内存管理, 基础页操作库.

** BUG
2009.03.08  所有的内存释放都将页面设定为访问失效状态.
2009.04.15  加入页面直接映射逻辑的地址的能力, 可以使 BSP 程序方便处理高端异常表等特殊操作.
2009.06.18  加入通过虚拟地址查询物理地址的 API.
2009.06.23  加入 API_VmmDmaAllocAlign 功能. 已解决一些特殊的 DMA 对内存对齐的特殊要求.
2009.07.11  API_VmmLibInit() 仅可以调用一次.
2009.11.10  DMA 内存分配, 返回值为物理地址.
2009.11.13  解决 vmmMalloc 对物理内存碎片处理 BUG.
            加入获取 zone 与 virtual 信息的 API.
2010.08.13  整理注释.
2011.03.02  加入修改内存属性的 API 这样可以将有些内存设为只读, 为内核模块的代码区服务.
2011.05.16  将 vmmMalloc vmmFree 函数移至 vmmMalloc.c 文件中.
2011.08.02  去掉 page alloc 相关操作, 物理内存分配均由 dma alloc 完成.
2011.12.08  API_VmmLibInit() 首先初始化物理页面控制块池.
2011.12.10  在取消映射关系或者改变映射关系属性时, 需要回写并无效 cache.
2012.04.18  加入 posix getpagesize api.
2013.03.16  加入对物理内存的分配函数, 这些物理内存分配出后不能直接使用, 必须让有效的虚拟地址映射后使用.
            DMA 物理内存分配函数分配的物理内存可以直接访问, 因为 DMA 物理内存和虚拟空间绝不重复.
2013.07.20  SMP 情况下 MMU 初始化加入主从核分离的初始化函数.
2014.09.18  加入 API_VmmIoRemapEx() 可指定内存属性.
2015.05.14  API_VmmLibSecondaryInit() 不加 VMM 锁.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#define  __VMM_MAIN_FILE
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
   
   下面是带有硬件 MMU 单元的处理器系统结构图
   
                                                   physical address
                                                                         RAM
   +---------------+    write    +---------------+     write       +-------------+
   |               |  ---------> |               | --------------> |             |
   |   PROCESSOR   |    read     |      MMU      |     read        |             |
   |               |  <--------- |               | <-------------- |   physical  |
   +---------------+             +---------------+ <------\        |    memory   |
                                                          |        |             |
                   virtual address                        |        |             |
                                                          |        |             |
                                                          |        |-------------|
                                                          |        |             |
                                                          \------- | translation |
                                                                   |    table    |
                                                                   |             |
                                                                   +-------------+
  
  注意: 所有的释放操作都没有执行 cache 会写或者更新操作, 这个一定由调用者完成.
*********************************************************************************************************/
/*********************************************************************************************************
  加入裁剪支持
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
#include "phyPage.h"
#include "virPage.h"
/*********************************************************************************************************
  外部全局变量声明
*********************************************************************************************************/
extern LW_VMM_ZONE          _G_vmzonePhysical[LW_CFG_VMM_ZONE_NUM];     /*  物理区域                    */
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
LW_MMU_OP                   _G_mmuOpLib;                                /*  MMU 操作库                  */
/*********************************************************************************************************
  操作锁
*********************************************************************************************************/
LW_OBJECT_HANDLE            _G_ulVmmLock;
/*********************************************************************************************************
** 函数名称: API_VmmGetLibBlock
** 功能描述: 获得系统级 LW_MMU_OP 结构，(用于 BSP 程序初始化 MMU 系统)
** 输　入  : NONE
** 输　出  : &_G_mmuOpLib
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
LW_MMU_OP  *API_VmmGetLibBlock (VOID)
{
    return  (&_G_mmuOpLib);
}
/*********************************************************************************************************
** 函数名称: API_VmmLibPrimaryInit
** 功能描述: 初始化 vmm 库 (注意: LW_VMM_ZONE_DESC 结构表内的元素数一定要与 LW_CFG_VMM_ZONE_NUM 一致)
** 输　入  : vmzone        物理内存区
**           mmugdesc      全局映射关系描述表
**           pcMachineName 正在运行的机器名称
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : 多核模式为主核 MMU 初始化
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmLibPrimaryInit (LW_VMM_ZONE_DESC       vmzone[],
                              LW_MMU_GLOBAL_DESC     mmugdesc[],
                              CPCHAR                 pcMachineName)
{
    static BOOL                 bIsInit  = LW_FALSE;
           PLW_MMU_VIRTUAL_DESC pvirdesc = __vmmVirtualDesc();
           INT                  i;
           ULONG                ulError;
           ULONG                ulPageNum = 0;
    
    if (bIsInit) {
        return  (ERROR_NONE);
    }

    if ((vmzone == LW_NULL) || (mmugdesc == LW_NULL)) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    for (i = 0; i < LW_CFG_VMM_ZONE_NUM; i++) {
        ulPageNum += (ULONG)(vmzone[i].ZONED_stSize >> LW_CFG_VMM_PAGE_SHIFT);
    }
    
    ulError = __pageCbInit(ulPageNum);                                  /*  初始化物理页面控制块池      */
    if (ulError) {
        _ErrorHandle(ulError);
        return  (ulError);
    }
    
    for (i = 0; i < LW_CFG_VMM_ZONE_NUM; i++) {
        ulError = __vmmPhysicalCreate((ULONG)i, vmzone[i].ZONED_ulAddr, 
                                                vmzone[i].ZONED_stSize,
                                                vmzone[i].ZONED_uiAttr);/*  初始化物理 zone             */
        if (ulError) {
            _ErrorHandle(ulError);
            return  (ulError);
        }
        
        ulError = __areaPhysicalSpaceInit((ULONG)i, vmzone[i].ZONED_ulAddr, 
                                                    vmzone[i].ZONED_stSize);
        if (ulError) {
            _ErrorHandle(ulError);
            return  (ulError);
        }
    }
    
    ulError = __vmmVirtualCreate(pvirdesc->ulVirtualStart, 
                                 pvirdesc->stSize);                     /*  初始化逻辑空间              */
    if (ulError) {
        _ErrorHandle(ulError);
        return  (ulError);
    }
    
    ulError = __areaVirtualSpaceInit(pvirdesc->ulVirtualStart, 
                                     pvirdesc->stSize);
    if (ulError) {
        _ErrorHandle(ulError);
        return  (ulError);
    }
    
    _G_ulVmmLock = API_SemaphoreMCreate("vmm_lock", LW_PRIO_DEF_CEILING, 
                                        LW_OPTION_INHERIT_PRIORITY |
                                        LW_OPTION_WAIT_PRIORITY | 
                                        LW_OPTION_DELETE_SAFE |
                                        LW_OPTION_OBJECT_GLOBAL,        /*  基于优先级等待              */
                                        LW_NULL);
    if (!_G_ulVmmLock) {
        return  (API_GetLastError());
    }
    
    __vmmMapInit();                                                     /*  初始化映射管理库            */
    
    ulError = __vmmLibPrimaryInit(mmugdesc, pcMachineName);             /*  初始化底层 MMU              */
    if (ulError) {
        _ErrorHandle(ulError);
        return  (ulError);
    }
    
    bIsInit = LW_TRUE;
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "MMU initilaized.\r\n");

    return  (ERROR_NONE);                                               /*  初始化底层 MMU              */
}
/*********************************************************************************************************
** 函数名称: API_VmmLibSecondaryInit
** 功能描述: 初始化 vmm 库 (从核 MMU 初始化)
** 输　入  : pcMachineName     正在运行的机器名称
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

LW_API  
ULONG  API_VmmLibSecondaryInit (CPCHAR  pcMachineName)
{
    ULONG    ulError;
    
    ulError = __vmmLibSecondaryInit(pcMachineName);
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "secondary MMU initilaized.\r\n");

    return  (ulError);
}
                             
#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
** 函数名称: API_VmmMmuEnable
** 功能描述: 启动 MMU, MMU 启动前后虚拟地址不能有任何变化.
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
VOID  API_VmmMmuEnable (VOID)
{
    KN_SMP_MB();
    __VMM_MMU_ENABLE();                                                 /*  启动 MMU                    */
    KN_SMP_MB();
}
/*********************************************************************************************************
** 函数名称: API_VmmMmuDisable
** 功能描述: 停止 MMU, MMU 停止前后虚拟地址不能有任何变化.
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
VOID  API_VmmMmuDisable (VOID)
{
    KN_SMP_MB();
    __VMM_MMU_DISABLE();                                                /*  停止 MMU                    */
    KN_SMP_MB();
}
/*********************************************************************************************************
** 函数名称: API_VmmPhyAlloc
** 功能描述: 从物理内存区分配连续的物理分页
** 输　入  : stSize     需要分配的内存大小
** 输　出  : 连续分页首地址 (物理地址, 不能直接使用!)
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmPhyAlloc (size_t  stSize)
{
    return  (API_VmmPhyAllocAlign(stSize, LW_CFG_VMM_PAGE_SIZE, LW_ZONE_ATTR_NONE));
}
/*********************************************************************************************************
** 函数名称: API_VmmPhyAlloc
** 功能描述: 从物理内存区分配连续的物理分页, 扩展接口.
** 输　入  : stSize     需要分配的内存大小
**           uiAttr     需要满足的物理页面属性
** 输　出  : 连续分页首地址 (物理地址, 不能直接使用!)
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmPhyAllocEx (size_t  stSize, UINT  uiAttr)
{
    return  (API_VmmPhyAllocAlign(stSize, LW_CFG_VMM_PAGE_SIZE, uiAttr));
}
/*********************************************************************************************************
** 函数名称: API_VmmPhyAllocAlign
** 功能描述: 从物理内存区分配连续的物理分页, (满足对齐关系)
** 输　入  : stSize     需要分配的内存大小
**           stAlign    对齐关系
**           uiAttr     需要满足的物理页面属性
** 输　出  : 连续分页首地址 (物理地址, 不能直接使用!)
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmPhyAllocAlign (size_t  stSize, size_t  stAlign, UINT  uiAttr)
{
    REGISTER ULONG          ulPageNum = (ULONG) (stSize >> LW_CFG_VMM_PAGE_SHIFT);
    REGISTER size_t         stExcess  = (size_t)(stSize & ~LW_CFG_VMM_PAGE_MASK);
    
    REGISTER PLW_VMM_PAGE   pvmpage;
             ULONG          ulZoneIndex;

    if (stAlign & (stAlign - 1)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "iAlign invalidate.\r\n");
        _ErrorHandle(ERROR_VMM_ALIGN);
        return  (LW_NULL);
    }
    
    if (stAlign < LW_CFG_VMM_PAGE_SIZE) {
        stAlign = LW_CFG_VMM_PAGE_SIZE;
    }
    
    if (stExcess) {
        ulPageNum++;
    }
    
    if (ulPageNum < 1) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    
    __VMM_LOCK();
    pvmpage = __vmmPhysicalPageAllocAlign(ulPageNum, 
                                          stAlign, uiAttr,
                                          &ulZoneIndex);                /*  分配连续物理页面            */
    if (pvmpage  == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_LOW_PHYSICAL_PAGE);
        return  (LW_NULL);
    }
    
    pvmpage->PAGE_ulMapPageAddr = PAGE_MAP_ADDR_INV;
    pvmpage->PAGE_ulFlags = LW_VMM_FLAG_FAIL;                           /*  记录分页类型                */
    
    __areaPhysicalInsertPage(ulZoneIndex, 
                             pvmpage->PAGE_ulPageAddr, pvmpage);        /*  插入物理空间反查表          */
    __VMM_UNLOCK();
    
    MONITOR_EVT_LONG3(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_PHY_ALLOC,
                      pvmpage->PAGE_ulPageAddr, stSize, stAlign, LW_NULL);
    
    return  ((PVOID)pvmpage->PAGE_ulPageAddr);                          /*  直接返回物理内存地址        */
}
/*********************************************************************************************************
** 函数名称: API_VmmPhyFree
** 功能描述: 释放连续物理分页
** 输　入  : pvPhyMem     连续分页首地址 (物理地址)
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
VOID  API_VmmPhyFree (PVOID  pvPhyMem)
{
    REGISTER PLW_VMM_PAGE   pvmpage;
             addr_t         ulAddr = (addr_t)pvPhyMem;
             ULONG          ulZoneIndex;

    __VMM_LOCK();
    ulZoneIndex = __vmmPhysicalGetZone(ulAddr);
    if (ulZoneIndex >= LW_CFG_VMM_ZONE_NUM) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_PHYSICAL_PAGE);                          /*  无法寻找到指定的物理页面    */
        return;
    }
    
    pvmpage = __areaPhysicalSearchPage(ulZoneIndex, ulAddr);
    if (pvmpage  == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_PHYSICAL_PAGE);                          /*  无法反向查询物理页面控制块  */
        return;
    }
    
    __areaPhysicalUnlinkPage(ulZoneIndex, 
                             pvmpage->PAGE_ulPageAddr, 
                             pvmpage);                                  /*  物理地址反查表释放          */
    
    __vmmPhysicalPageFree(pvmpage);                                     /*  释放物理页面                */
    __VMM_UNLOCK();

    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_PHY_FREE,
                      pvPhyMem, LW_NULL);
}
/*********************************************************************************************************
** 函数名称: API_VmmDmaAlloc
** 功能描述: 从物理内存区分配连续的物理分页, 主要用于 DMA 连续物理内存操作.
** 输　入  : stSize     需要分配的内存大小
** 输　出  : 连续分页首地址 (物理地址, 此地址对应的物理地址存在于 LW_ZONE_ATTR_DMA 区域内)
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmDmaAlloc (size_t  stSize)
{
    REGISTER ULONG          ulPageNum = (ULONG) (stSize >> LW_CFG_VMM_PAGE_SHIFT);
    REGISTER size_t         stExcess  = (size_t)(stSize & ~LW_CFG_VMM_PAGE_MASK);

    REGISTER PLW_VMM_PAGE   pvmpage;
             ULONG          ulZoneIndex;
             ULONG          ulError;
    
    if (stExcess) {
        ulPageNum++;
    }
    
    if (ulPageNum < 1) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    
    __VMM_LOCK();
    pvmpage = __vmmPhysicalPageAlloc(ulPageNum, LW_ZONE_ATTR_DMA, 
                                     &ulZoneIndex);                     /*  分配连续物理页面            */
    if (pvmpage  == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_LOW_PHYSICAL_PAGE);
        return  (LW_NULL);
    }
    
    ulError = __vmmLibPageMap(pvmpage->PAGE_ulPageAddr,                 /*  不使用 CACHE                */
                              pvmpage->PAGE_ulPageAddr,
                              ulPageNum, 
                              LW_VMM_FLAG_DMA);                         /*  映射逻辑物理同一地址        */
    if (ulError) {                                                      /*  映射错误                    */
        __vmmPhysicalPageFree(pvmpage);                                 /*  释放物理页面                */
        __VMM_UNLOCK();
        _ErrorHandle(ulError);
        return  (LW_NULL);
    }
    
    pvmpage->PAGE_ulMapPageAddr = pvmpage->PAGE_ulPageAddr;             /*  记录映射关系                */
    pvmpage->PAGE_ulFlags = LW_VMM_FLAG_DMA;                            /*  记录分页类型                */
    
    __areaPhysicalInsertPage(ulZoneIndex, 
                             pvmpage->PAGE_ulPageAddr, pvmpage);        /*  插入物理空间反查表          */
    __VMM_UNLOCK();
    
    MONITOR_EVT_LONG3(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_DMA_ALLOC,
                      pvmpage->PAGE_ulPageAddr, stSize, LW_CFG_VMM_PAGE_SIZE, LW_NULL);
    
    return  ((PVOID)pvmpage->PAGE_ulPageAddr);                          /*  直接返回物理内存地址        */
}
/*********************************************************************************************************
** 函数名称: API_VmmDmaAllocAlign
** 功能描述: 从物理内存区分配连续的物理分页, 主要用于 DMA 连续物理内存操作.(可指定内存关系)
** 输　入  : stSize     需要分配的内存大小
**           stAlign    对齐关系
** 输　出  : 连续分页首地址 (物理地址, 此地址对应的物理地址存在于 LW_ZONE_ATTR_DMA 区域内)
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmDmaAllocAlign (size_t  stSize, size_t  stAlign)
{
    REGISTER ULONG          ulPageNum = (ULONG) (stSize >> LW_CFG_VMM_PAGE_SHIFT);
    REGISTER size_t         stExcess  = (size_t)(stSize & ~LW_CFG_VMM_PAGE_MASK);
    
    REGISTER PLW_VMM_PAGE   pvmpage;
             ULONG          ulZoneIndex;
             ULONG          ulError;

    if (stAlign & (stAlign - 1)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "iAlign invalidate.\r\n");
        _ErrorHandle(ERROR_VMM_ALIGN);
        return  (LW_NULL);
    }
    if (stAlign < LW_CFG_VMM_PAGE_SIZE) {
        stAlign = LW_CFG_VMM_PAGE_SIZE;
    }
    
    if (stExcess) {
        ulPageNum++;
    }
    
    if (ulPageNum < 1) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    
    __VMM_LOCK();
    pvmpage = __vmmPhysicalPageAllocAlign(ulPageNum, 
                                          stAlign, LW_ZONE_ATTR_DMA,
                                          &ulZoneIndex);                /*  分配连续物理页面            */
    if (pvmpage  == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_LOW_PHYSICAL_PAGE);
        return  (LW_NULL);
    }
    
    ulError = __vmmLibPageMap(pvmpage->PAGE_ulPageAddr,                 /*  不使用 CACHE                */
                              pvmpage->PAGE_ulPageAddr,
                              ulPageNum, 
                              LW_VMM_FLAG_DMA);                         /*  映射逻辑物理同一地址        */
    if (ulError) {                                                      /*  映射错误                    */
        __vmmPhysicalPageFree(pvmpage);                                 /*  释放物理页面                */
        __VMM_UNLOCK();
        _ErrorHandle(ulError);
        return  (LW_NULL);
    }
    
    pvmpage->PAGE_ulMapPageAddr = pvmpage->PAGE_ulPageAddr;
    pvmpage->PAGE_ulFlags = LW_VMM_FLAG_DMA;                            /*  记录分页类型                */
    
    __areaPhysicalInsertPage(ulZoneIndex, 
                             pvmpage->PAGE_ulPageAddr, pvmpage);        /*  插入物理空间反查表          */
    __VMM_UNLOCK();
    
    MONITOR_EVT_LONG3(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_DMA_ALLOC,
                      pvmpage->PAGE_ulPageAddr, stSize, stAlign, LW_NULL);
                      
    return  ((PVOID)pvmpage->PAGE_ulPageAddr);                          /*  直接返回物理内存地址        */
}
/*********************************************************************************************************
** 函数名称: API_VmmDmaFree
** 功能描述: 释放连续物理分页
** 输　入  : pvDmaMem     连续分页首地址 (物理地址)
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
VOID  API_VmmDmaFree (PVOID  pvDmaMem)
{
    REGISTER PLW_VMM_PAGE   pvmpage;
             addr_t         ulAddr = (addr_t)pvDmaMem;
             ULONG          ulZoneIndex;

    __VMM_LOCK();
    ulZoneIndex = __vmmPhysicalGetZone(ulAddr);
    if (ulZoneIndex >= LW_CFG_VMM_ZONE_NUM) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_PHYSICAL_PAGE);                          /*  无法寻找到指定的物理页面    */
        return;
    }
    
    pvmpage = __areaPhysicalSearchPage(ulZoneIndex, ulAddr);
    if (pvmpage  == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_PHYSICAL_PAGE);                          /*  无法反向查询物理页面控制块  */
        return;
    }

#if LW_CFG_CACHE_EN > 0
    API_CacheClearPage(DATA_CACHE, 
                       (PVOID)pvmpage->PAGE_ulPageAddr,
                       (PVOID)pvmpage->PAGE_ulPageAddr,
                       (size_t)(pvmpage->PAGE_ulCount << LW_CFG_VMM_PAGE_SHIFT));
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
    
    __vmmLibPageMap(pvmpage->PAGE_ulPageAddr,
                    pvmpage->PAGE_ulPageAddr,
                    pvmpage->PAGE_ulCount, 
                    LW_VMM_FLAG_FAIL);                                  /*  不允许访问                  */
    
    __areaPhysicalUnlinkPage(ulZoneIndex, 
                             pvmpage->PAGE_ulPageAddr, 
                             pvmpage);                                  /*  物理地址反查表释放          */
    
    __vmmPhysicalPageFree(pvmpage);                                     /*  释放物理页面                */
    __VMM_UNLOCK();

    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_DMA_FREE,
                      pvDmaMem, LW_NULL);
}
/*********************************************************************************************************
** 函数名称: API_VmmIoRemapEx
** 功能描述: 将物理 IO 空间指定内存映射到逻辑空间. (用户可指定 CACHE 与否)
** 输　入  : pvPhysicalAddr     物理内存地址
**           stSize             需要映射的内存大小
**           ulFlags            内存属性
** 输　出  : 映射到的逻辑内存地址
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmIoRemapEx (PVOID  pvPhysicalAddr, 
                         size_t stSize,
                         ULONG  ulFlags)
{
    REGISTER ULONG          ulPageNum = (ULONG) (stSize >> LW_CFG_VMM_PAGE_SHIFT);
    REGISTER size_t         stExcess  = (size_t)(stSize & ~LW_CFG_VMM_PAGE_MASK);

    REGISTER PLW_VMM_PAGE   pvmpageVirtual;
             ULONG          ulError;
    
    if (stExcess) {
        ulPageNum++;                                                    /*  确定分页数量                */
    }

    if (ulPageNum < 1) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    
    __VMM_LOCK();
    pvmpageVirtual = __vmmVirtualPageAlloc(ulPageNum);                  /*  分配连续虚拟页面            */
    if (pvmpageVirtual == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_VIRTUAL_PAGE);
        return  (LW_NULL);
    }
    
    ulError = __vmmLibPageMap((addr_t)pvPhysicalAddr,                   /*  不使用 CACHE                */
                              pvmpageVirtual->PAGE_ulPageAddr,
                              ulPageNum, 
                              ulFlags);                                 /*  映射为连续虚拟地址          */
    if (ulError) {                                                      /*  映射错误                    */
        __vmmVirtualPageFree(pvmpageVirtual);                           /*  释放虚拟地址空间            */
        __VMM_UNLOCK();
        _ErrorHandle(ulError);
        return  (LW_NULL);
    }
    
    pvmpageVirtual->PAGE_ulFlags = ulFlags;
    
    __areaVirtualInsertPage(pvmpageVirtual->PAGE_ulPageAddr, 
                            pvmpageVirtual);                            /*  插入逻辑空间反查表          */
    __VMM_UNLOCK();
    
    MONITOR_EVT_LONG3(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_IOREMAP,
                      pvmpageVirtual->PAGE_ulPageAddr, pvPhysicalAddr, stSize, LW_NULL);
    
    return  ((PVOID)pvmpageVirtual->PAGE_ulPageAddr);
}
/*********************************************************************************************************
** 函数名称: API_VmmIoRemap
** 功能描述: 将物理 IO 空间指定内存映射到逻辑空间. (非 CACHE)
** 输　入  : pvPhysicalAddr     物理内存地址
**           stSize             需要映射的内存大小
** 输　出  : 映射到的逻辑内存地址
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmIoRemap (PVOID  pvPhysicalAddr, 
                       size_t stSize)
{
    return  (API_VmmIoRemapEx(pvPhysicalAddr, stSize, LW_VMM_FLAG_DMA));
}
/*********************************************************************************************************
** 函数名称: API_VmmIoUnmap
** 功能描述: 释放 ioremap 占用的逻辑空间
** 输　入  : pvVirtualMem    虚拟地址
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
VOID  API_VmmIoUnmap (PVOID  pvVirtualAddr)
{
    REGISTER PLW_VMM_PAGE   pvmpageVirtual;
             addr_t         ulAddr = (addr_t)pvVirtualAddr;
    
    __VMM_LOCK();
    pvmpageVirtual = __areaVirtualSearchPage(ulAddr);
    if (pvmpageVirtual == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_VIRTUAL_PAGE);                           /*  无法反向查询虚拟页面控制块  */
        return;
    }
    
#if LW_CFG_CACHE_EN > 0
    API_CacheClear(DATA_CACHE, (PVOID)pvmpageVirtual->PAGE_ulPageAddr,
                   (size_t)(pvmpageVirtual->PAGE_ulCount << LW_CFG_VMM_PAGE_SHIFT));
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
    
    __vmmLibPageMap(pvmpageVirtual->PAGE_ulPageAddr,
                    pvmpageVirtual->PAGE_ulPageAddr,
                    pvmpageVirtual->PAGE_ulCount, 
                    LW_VMM_FLAG_FAIL);                                  /*  不允许访问                  */
    
    __areaVirtualUnlinkPage(pvmpageVirtual->PAGE_ulPageAddr,
                            pvmpageVirtual);
    
    __vmmVirtualPageFree(pvmpageVirtual);                               /*  删除虚拟页面                */
    __VMM_UNLOCK();
    
    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_IOUNMAP,
                      pvVirtualAddr, LW_NULL);
}
/*********************************************************************************************************
** 函数名称: API_VmmIoRemapNocache
** 功能描述: 将物理 IO 空间指定内存映射到逻辑空间. (非 CACHE)
** 输　入  : pvPhysicalAddr     物理内存地址
**           ulSize             需要映射的内存大小
** 输　出  : 映射到的逻辑内存地址
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
PVOID  API_VmmIoRemapNocache (PVOID  pvPhysicalAddr, 
                              size_t stSize)
{
    return  (API_VmmIoRemapEx(pvPhysicalAddr, stSize, LW_VMM_FLAG_DMA));
}
/*********************************************************************************************************
** 函数名称: API_VmmMap
** 功能描述: 将指定物理空间映射到指定的逻辑空间
** 输　入  : pvVirtualAddr      需要映射的虚拟地址
**           pvPhysicalAddr     物理内存地址
**           stSize             需要映射的内存大小
**           ulFlag             内存区域类型: LW_VMM_FLAG_UNWRITABLE, LW_VMM_FLAG_CACHEABLE ...
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : pvVirtualAddr 不能出现在 BSP 配置的虚拟地址空间中, 这样会影响内核 VMM 其他管理组件.
             请慎用此函数.
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmMap (PVOID  pvVirtualAddr,
                   PVOID  pvPhysicalAddr,
                   size_t stSize, 
                   ULONG  ulFlag)
{
    REGISTER addr_t ulVirtualAddr  = (addr_t)pvVirtualAddr;
    REGISTER addr_t ulPhysicalAddr = (addr_t)pvPhysicalAddr;
    
    REGISTER ULONG  ulPageNum = (ULONG) (stSize >> LW_CFG_VMM_PAGE_SHIFT);
    REGISTER size_t stExcess  = (size_t)(stSize & ~LW_CFG_VMM_PAGE_MASK);
             
    REGISTER ULONG  ulError;
    
    if (stExcess) {
        ulPageNum++;                                                    /*  确定分页数量                */
    }

    if (ulPageNum < 1) {
        _ErrorHandle(EINVAL);
        return  (ERROR_VMM_VIRTUAL_ADDR);
    }
    
    if (!ALIGNED(pvVirtualAddr,  LW_CFG_VMM_PAGE_SIZE) ||
        !ALIGNED(pvPhysicalAddr, LW_CFG_VMM_PAGE_SIZE)) {
        _ErrorHandle(EINVAL);
        return  (ERROR_VMM_ALIGN);
    }
    
    __VMM_LOCK();
    ulError = __vmmLibPageMap(ulPhysicalAddr, ulVirtualAddr, ulPageNum, ulFlag);
    __VMM_UNLOCK();
    
    return  (ulError);
}
/*********************************************************************************************************
** 函数名称: API_VmmSetFlag
** 功能描述: 设置指定逻辑地址空间的内存访问属性. 必须为虚拟空间分配出来的地址.
** 输　入  : pvVirtualAddr      虚拟地址
**           ulFlag             内存区域类型: LW_VMM_FLAG_UNWRITABLE, LW_VMM_FLAG_CACHEABLE ...
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmSetFlag (PVOID  pvVirtualAddr, ULONG  ulFlag)
{
    REGISTER PLW_VMM_PAGE   pvmpageVirtual;
             addr_t         ulVirtualAddr = (addr_t)pvVirtualAddr;
    
    __VMM_LOCK();
    pvmpageVirtual = __areaVirtualSearchPage(ulVirtualAddr);
    if (pvmpageVirtual == LW_NULL) {
        __VMM_UNLOCK();
        _ErrorHandle(ERROR_VMM_VIRTUAL_PAGE);                           /*  无法反向查询虚拟页面控制块  */
        return  (ERROR_VMM_VIRTUAL_PAGE);
    }
    
#if LW_CFG_CACHE_EN > 0
    __vmmPhysicalPageClearAll(pvmpageVirtual);
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
    
    __vmmLibSetFlag(ulVirtualAddr, 
                    pvmpageVirtual->PAGE_ulCount, ulFlag);              /*  重新映射这些页面            */
    
    pvmpageVirtual->PAGE_ulFlags = ulFlag;                              /*  记录新权限信息              */
    
    __vmmPhysicalPageSetFlagAll(pvmpageVirtual, ulFlag);                /*  设置所有物理页面的 flag     */
    
    __VMM_UNLOCK();
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_SETFLAG,
                      pvVirtualAddr, ulFlag, LW_NULL);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_VmmGetFlag
** 功能描述: 获取指定逻辑地址的内存访问属性.
** 输　入  : pvVirtualAddr      虚拟地址
**           pulFlag            获取的内存区域类型: LW_VMM_FLAG_UNWRITABLE, LW_VMM_FLAG_CACHEABLE ...
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmGetFlag (PVOID  pvVirtualAddr, 
                       ULONG *pulFlag)
{
    REGISTER ULONG  ulError;

    if (pulFlag == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    __VMM_LOCK();
    ulError = __vmmLibGetFlag((addr_t)pvVirtualAddr, pulFlag);
    __VMM_UNLOCK();
    
    return  (ulError);
}
/*********************************************************************************************************
** 函数名称: API_VmmVirtualToPhysical
** 功能描述: 通过虚拟地址查询对应的物理地址 (vmm dma alloc 后需要通过此函数来获取真实的物理地址)
** 输　入  : ulVirtualAddr      虚拟地址
**           pulPhysicalAddr    返回的物理地址
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmVirtualToPhysical (addr_t  ulVirtualAddr, addr_t  *pulPhysicalAddr)
{
    REGISTER ULONG  ulError;

    __VMM_LOCK();
    ulError = __vmmLibVirtualToPhysical(ulVirtualAddr, pulPhysicalAddr);
    __VMM_UNLOCK();
    
    return  (ulError);
}
/*********************************************************************************************************
** 函数名称: API_VmmPhysicalToVirtual
** 功能描述: 通过物理地址查询虚拟地址
** 输　入  : ulPhysicalAddr      物理地址
**           pulVirtualAddr      返回的虚拟地址
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmPhysicalToVirtual (addr_t  ulPhysicalAddr, addr_t  *pulVirtualAddr)
{
    _ErrorHandle(ENOSYS);
    return  (ENOSYS);
}
/*********************************************************************************************************
** 函数名称: API_VmmVirtualIsInside
** 功能描述: 查询指定的地址是否在虚拟空间中
** 输　入  : ulAddr             地址
** 输　出  : 是否在虚拟地址空间中
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
BOOL  API_VmmVirtualIsInside (addr_t  ulAddr)
{
    return  (__vmmVirtualIsInside(ulAddr));                             /*  不需要使用 VMM LOCK         */
}
/*********************************************************************************************************
** 函数名称: API_VmmZoneStatus
** 功能描述: 获得 zone 的情况
** 输　入  : ulZoneIndex        物理区域索引
**           pulPhysicalAddr    物理地址
**           pstSize            大小
**           pulPgd             PDG 表项入口
**           pulFreePage        空闲页面的个数
**           puiAttr            区域属性   例如: LW_ZONE_ATTR_DMA
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmZoneStatus (ULONG     ulZoneIndex,
                          addr_t   *pulPhysicalAddr,
                          size_t   *pstSize,
                          addr_t   *pulPgd,
                          ULONG    *pulFreePage,
                          BOOL     *puiAttr)
{
    PLW_MMU_CONTEXT    pmmuctx = __vmmGetCurCtx();
    
    if (ulZoneIndex >= LW_CFG_VMM_ZONE_NUM) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    __VMM_LOCK();
    if (pulPhysicalAddr) {
        *pulPhysicalAddr = _G_vmzonePhysical[ulZoneIndex].ZONE_ulAddr;
    }
    if (pstSize) {
        *pstSize = _G_vmzonePhysical[ulZoneIndex].ZONE_stSize;
    }
    if (pulPgd) {
        *pulPgd = (addr_t)pmmuctx->MMUCTX_pgdEntry;
    }
    if (pulFreePage) {
        *pulFreePage = _G_vmzonePhysical[ulZoneIndex].ZONE_ulFreePage;
    }
    if (puiAttr) {
        *puiAttr = _G_vmzonePhysical[ulZoneIndex].ZONE_uiAttr;
    }
    __VMM_UNLOCK();
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_VmmVirtualStatus
** 功能描述: 获得 virtual 的情况
** 输　入  : pulVirtualAddr     虚拟地址
**           pstSize            大小
**           pulFreePage        页面的个数
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_VmmVirtualStatus (addr_t   *pulVirtualAddr,
                             size_t   *pstSize,
                             ULONG    *pulFreePage)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();
    
    __VMM_LOCK();
    if (pulVirtualAddr) {
        *pulVirtualAddr = pmmuctx->MMUCTX_vmzoneVirtual.ZONE_ulAddr;
    }
    if (pstSize) {
        *pstSize = pmmuctx->MMUCTX_vmzoneVirtual.ZONE_stSize;
    }
    if (pulFreePage) {
        *pulFreePage = pmmuctx->MMUCTX_vmzoneVirtual.ZONE_ulFreePage;
    }
    __VMM_UNLOCK();
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: getpagesize
** 功能描述: 获得 pagesize 
** 输　入  : NONE
** 输　出  : pagesize
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
int getpagesize (void)
{
    return  (LW_CFG_VMM_PAGE_SIZE);
}
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
