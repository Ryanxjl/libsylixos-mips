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
** 文   件   名: pciDev.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 09 月 28 日
**
** 描        述: PCI 总线驱动模型, 设备头定义.
*********************************************************************************************************/

#ifndef __PCI_DEV_H
#define __PCI_DEV_H

#include "pciBus.h"
#include "pciIds.h"
#include "pciDrv.h"
#include "pciPm.h"
#include "pciMsi.h"
#include "pciCap.h"
#include "pciError.h"
#include "pciCapExt.h"
#include "pciExpress.h"
#include "pciAutoCfg.h"

/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_PCI_EN > 0)

/*********************************************************************************************************
  PCI 配置
*********************************************************************************************************/

#if LW_CFG_PCI_64 > 0
typedef UINT64          pci_addr_t;
typedef UINT64          pci_size_t;
#else
typedef UINT32          pci_addr_t;
typedef UINT32          pci_size_t;
#endif                                                                  /*  LW_CFG_PCI_64 > 0           */

/*********************************************************************************************************
   PCI 硬件连接结构例图
   
                      +-------+
                      |       |
                      |  CPU  |
                      |       |
                      +-------+
                          |
       Host bus           |
    ----------------------+-------------------------- ...
                          |
                 +------------------+
                 |     Bridge 0     |
                 |  (host adapter)  |
                 +------------------+
                          |
       PCI bus segment 0  |  (primary bus segment)
    ----------------------+-------------------------- ...
             |            |    |                  |
           dev 0          |   dev 1              dev 2
                          |
                 +------------------+
                 |     Bridge 1     |
                 |   (PCI to PCI)   |
                 +------------------+
                          |
       PCI bus segment 1  |  (secondary bus segment)
    ----------------------+-------------------------- ...
             |            |    |                  |
           dev 0          |   dev 1              dev 2
                          |
                 +------------------+
                 |     Bridge 2     |
                 |   (PCI to PCI)   |
                 +------------------+
                          |
       PCI bus segment 2  |  (tertiary bus segment)
    ----------------------+-------------------------- ...
             |                 |                  |
           dev 0              dev 1              dev 2
*********************************************************************************************************/
/*********************************************************************************************************
  PCI 设备头
*********************************************************************************************************/

typedef struct {
    UINT16          PCID_usVendorId;                                    /* vendor ID                    */
    UINT16          PCID_usDeviceId;                                    /* device ID                    */
    UINT16          PCID_usCommand;                                     /* command register             */
    UINT16          PCID_usStatus;                                      /* status register              */
    
    UINT8           PCID_ucRevisionId;                                  /* revision ID                  */
    UINT8           PCID_ucProgIf;                                      /* programming interface        */
    UINT8           PCID_ucSubClass;                                    /* sub class code               */
    UINT8           PCID_ucClassCode;                                   /* class code                   */
    
    UINT8           PCID_ucCacheLine;                                   /* cache line                   */
    UINT8           PCID_ucLatency;                                     /* latency time                 */
    UINT8           PCID_ucHeaderType;                                  /* header type                  */
    UINT8           PCID_ucBist;                                        /* BIST                         */
    
    UINT32          PCID_uiBase0;                                       /* base address 0               */
    UINT32          PCID_uiBase1;                                       /* base address 1               */
    UINT32          PCID_uiBase2;                                       /* base address 2               */
    UINT32          PCID_uiBase3;                                       /* base address 3               */
    UINT32          PCID_uiBase4;                                       /* base address 4               */
    UINT32          PCID_uiBase5;                                       /* base address 5               */
    
    UINT32          PCID_uiCis;                                         /* cardBus CIS pointer          */
    UINT16          PCID_usSubVendorId;                                 /* sub system vendor ID         */
    UINT16          PCID_usSubSystemId;                                 /* sub system ID                */
    UINT32          PCID_uiRomBase;                                     /* expansion ROM base address   */

    UINT32          PCID_uiReserved0;                                   /* reserved                     */
    UINT32          PCID_uiReserved1;                                   /* reserved                     */

    UINT8           PCID_ucIntLine;                                     /* interrupt line               */
    UINT8           PCID_ucIntPin;                                      /* interrupt pin                */
    
    UINT8           PCID_ucMinGrant;                                    /* min Grant                    */
    UINT8           PCID_ucMaxLatency;                                  /* max Latency                  */
} PCI_DEV_HDR;

/*********************************************************************************************************
  PCI 桥设备头
*********************************************************************************************************/

typedef struct {
    UINT16          PCIB_usVendorId;                                    /* vendor ID                    */
    UINT16          PCIB_usDeviceId;                                    /* device ID                    */
    UINT16          PCIB_usCommand;                                     /* command register             */
    UINT16          PCIB_usStatus;                                      /* status register              */

    UINT8           PCIB_ucRevisionId;                                  /* revision ID                  */
    UINT8           PCIB_ucProgIf;                                      /* programming interface        */
    UINT8           PCIB_ucSubClass;                                    /* sub class code               */
    UINT8           PCIB_ucClassCode;                                   /* class code                   */
    
    UINT8           PCIB_ucCacheLine;                                   /* cache line                   */
    UINT8           PCIB_ucLatency;                                     /* latency time                 */
    UINT8           PCIB_ucHeaderType;                                  /* header type                  */
    UINT8           PCIB_ucBist;                                        /* BIST                         */

    UINT32          PCIB_uiBase0;                                       /* base address 0               */
    UINT32          PCIB_uiBase1;                                       /* base address 1               */

    UINT8           PCIB_ucPriBus;                                      /* primary bus number           */
    UINT8           PCIB_ucSecBus;                                      /* secondary bus number         */
    UINT8           PCIB_ucSubBus;                                      /* subordinate bus number       */
    UINT8           PCIB_ucSecLatency;                                  /* secondary latency timer      */
    UINT8           PCIB_ucIoBase;                                      /* IO base                      */
    UINT8           PCIB_ucIoLimit;                                     /* IO limit                     */

    UINT16          PCIB_usSecStatus;                                   /* secondary status             */

    UINT16          PCIB_usMemBase;                                     /* memory base                  */
    UINT16          PCIB_usMemLimit;                                    /* memory limit                 */
    UINT16          PCIB_usPreBase;                                     /* prefetchable memory base     */
    UINT16          PCIB_usPreLimit;                                    /* prefetchable memory limit    */

    UINT32          PCIB_uiPreBaseUpper;                                /* prefetchable memory base     */
                                                                        /* upper 32 bits                */
    UINT32          PCIB_uiPreLimitUpper;                               /* prefetchable memory limit    */
                                                                        /* upper 32 bits                */

    UINT16          PCIB_usIoBaseUpper;                                 /* IO base upper 16 bits        */
    UINT16          PCIB_usIoLimitUpper;                                /* IO limit upper 16 bits       */

    UINT32          PCIB_uiReserved;                                    /* reserved                     */
    UINT32          PCIB_uiRomBase;                                     /* expansion ROM base address   */

    UINT8           PCIB_ucIntLine;                                     /* interrupt line               */
    UINT8           PCIB_ucIntPin;                                      /* interrupt pin                */

    UINT16          PCIB_usControl;                                     /* bridge control               */
} PCI_BRG_HDR;

/*********************************************************************************************************
  PCI 卡桥设备头
*********************************************************************************************************/

typedef struct {
    UINT16          PCICB_usVendorId;                                   /* vendor ID                    */
    UINT16          PCICB_usDeviceId;                                   /* device ID                    */
    UINT16          PCICB_usCommand;                                    /* command register             */
    UINT16          PCICB_usStatus;                                     /* status register              */

    UINT8           PCICB_ucRevisionId;                                 /* revision ID                  */
    UINT8           PCICB_ucProgIf;                                     /* programming interface        */
    UINT8           PCICB_ucSubClass;                                   /* sub class code               */
    UINT8           PCICB_ucClassCode;                                  /* class code                   */
    
    UINT8           PCICB_ucCacheLine;                                  /* cache line                   */
    UINT8           PCICB_ucLatency;                                    /* latency time                 */
    UINT8           PCICB_ucHeaderType;                                 /* header type                  */
    UINT8           PCICB_ucBist;                                       /* BIST                         */
    
    UINT32          PCICB_uiBase0;                                      /* base address 0               */

    UINT8           PCICB_ucCapPtr;                                     /* capabilities pointer         */
    UINT8           PCICB_ucReserved;                                   /* reserved                     */

    UINT16          PCICB_usSecStatus;                                  /* secondary status             */

    UINT8           PCICB_ucPriBus;                                     /* primary bus number           */
    UINT8           PCICB_ucSecBus;                                     /* secondary bus number         */
    UINT8           PCICB_ucSubBus;                                     /* subordinate bus number       */
    UINT8           PCICB_ucSecLatency;                                 /* secondary latency timer      */

    UINT32          PCICB_uiMemBase0;                                   /* memory base 0                */
    UINT32          PCICB_uiMemLimit0;                                  /* memory limit 0               */
    UINT32          PCICB_uiMemBase1;                                   /* memory base 1                */
    UINT32          PCICB_uiMemLimit1;                                  /* memory limit 1               */

    UINT32          PCICB_uiIoBase0;                                    /* IO base 0                    */
    UINT32          PCICB_uiIoLimit0;                                   /* IO limit 0                   */
    UINT32          PCICB_uiIoBase1;                                    /* IO base 1                    */
    UINT32          PCICB_uiIoLimit1;                                   /* IO limit 1                   */

    UINT8           PCICB_ucIntLine;                                    /* interrupt line               */
    UINT8           PCICB_ucIntPin;                                     /* interrupt pin                */

    UINT16          PCICB_usControl;                                    /* bridge control               */
    UINT16          PCICB_usSubVendorId;                                /* sub system vendor ID         */
    UINT16          PCICB_usSubSystemId;                                /* sub system ID                */

    UINT32          PCICB_uiLegacyBase;                                 /* pccard 16bit legacy mode base*/
} PCI_CBUS_HDR;

/*********************************************************************************************************
  PCI 标准设备头
*********************************************************************************************************/

typedef struct {
    UINT8               PCIH_ucType;                                    /*  PCI 类型                    */
    /*
     * PCI_HEADER_TYPE_NORMAL
     * PCI_HEADER_TYPE_BRIDGE
     * PCI_HEADER_TYPE_CARDBUS
     */
    union {
        PCI_DEV_HDR     PCIHH_pcidHdr;
        PCI_BRG_HDR     PCIHH_pcibHdr;
        PCI_CBUS_HDR    PCIHH_pcicbHdr;
    } hdr;
#define PCIH_pcidHdr    hdr.PCIHH_pcidHdr
#define PCIH_pcibHdr    hdr.PCIHH_pcibHdr
#define PCIH_pcicbHdr   hdr.PCIHH_pcicbHdr
} PCI_HDR;

/*********************************************************************************************************
  PCI I/O Drv
*********************************************************************************************************/

typedef struct pci_drv_funcs0 {
    INT     (*cfgRead)(INT iBus, INT iSlot, INT iFunc, INT iOft, INT iLen, PVOID pvRet);
    INT     (*cfgWrite)(INT iBus, INT iSlot, INT iFunc, INT iOft, INT iLen, UINT32 uiData);
    INT     (*vpdRead)(INT iBus, INT iSlot, INT iFunc, INT iPos, UINT8 *pucBuf, INT iLen);
    INT     (*irqGet)(INT iBus, INT iSlot, INT iFunc, INT iMsiEn, INT iLine, INT iPin, ULONG *pulVector);
    INT     (*cfgSpcl)(INT iBus, UINT32 uiMsg);
} PCI_DRV_FUNCS0;                                                       /*  PCI_MECHANISM_0             */

typedef struct pci_drv_funcs12 {
    UINT8   (*ioInByte)(addr_t ulAddr);
    UINT16  (*ioInWord)(addr_t ulAddr);
    UINT32  (*ioInDword)(addr_t ulAddr);
    VOID    (*ioOutByte)(UINT8 ucValue, addr_t ulAddr);
    VOID    (*ioOutWord)(UINT16 usValue, addr_t ulAddr);
    VOID    (*ioOutDword)(UINT32 uiValue, addr_t ulAddr);
} PCI_DRV_FUNCS12;                                                      /*  PCI_MECHANISM_1 , 2         */

/*********************************************************************************************************
  The PCI interface treats multi-function devices as independent
  devices. The slot/function address of each device is encoded
  in a single byte as follows:
  7:3 = slot
  2:0 = function
*********************************************************************************************************/

#define PCI_SLOTFN(slot, func)  ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_DEVFN(slot, func)   PCI_SLOTFN(slot, func)
#define PCI_SLOT(slotfn)        (((slotfn) >> 3) & 0x1f)
#define PCI_FUNC(slotfn)        ((slotfn) & 0x07)

/*********************************************************************************************************
  pack parameters for the PCI Configuration Address Register
*********************************************************************************************************/
#define PCI_PACKET(bus, slot, func) ((((bus) << 16) & 0x00ff0000) | \
                                    (((slot) << 11) & 0x0000f800) | \
                                    (((func) << 8)  & 0x00000700))

/*********************************************************************************************************
  T. Straumann, 7/31/2001: increased to 32 - PMC slots are not
  scanned on mvme2306 otherwise
*********************************************************************************************************/

#define PCI_MAX_BUS             256
#define PCI_MAX_SLOTS           32
#define PCI_MAX_FUNCTIONS       8

/*********************************************************************************************************
  Configuration I/O addresses for mechanism 1
*********************************************************************************************************/

#define PCI_CONFIG_ADDR         0x0cf8                                  /* write 32 bits to set address */
#define PCI_CONFIG_DATA         0x0cfc                                  /* 8, 16, or 32 bit accesses    */

/*********************************************************************************************************
  Configuration I/O addresses for mechanism 2
*********************************************************************************************************/

#define PCI_CONFIG_CSE          0x0cf8                                  /* CSE register                 */
#define PCI_CONFIG_FORWARD      0x0cfa                                  /* forward register             */
#define PCI_CONFIG_BASE         0xc000                                  /* base register                */

/*********************************************************************************************************
  配置参数
*********************************************************************************************************/
#define PCI_DEBUG_EN            0                                       /* 是否使能调试信息             */

#define PCI_BARS_MAX            6                                       /* BAR 最大数量                 */
#define PCI_CONFIG_LEN_MAX      256                                     /* 配置空间大小                 */

#define PCI_DEV_NAME_MAX        (32 + 1)                                /* 设备名称最大值               */
#define PCI_DEV_IRQ_NAME_MAX    (32 + 1)                                /* 设备中断名称最大值           */

/*********************************************************************************************************
  ID 配置
*********************************************************************************************************/
#define PCI_ANY_ID              (~0)                                    /* 任何 ID                      */

#define PCI_DEVICE(vend,dev)                            (vend),                 \
                                                        (dev),                  \
                                                        PCI_ANY_ID,             \
                                                        PCI_ANY_ID

#define PCI_DEVICE_SUB(vend, dev, subvend, subdev)      (vend),                 \
                                                        (dev),                  \
                                                        (subvend),              \
                                                        (subdev)

#define PCI_DEVICE_CLASS(dev_class,dev_class_mask)      PCI_ANY_ID,             \
                                                        PCI_ANY_ID,             \
                                                        PCI_ANY_ID,             \
                                                        PCI_ANY_ID,             \
                                                        (dev_class),            \
                                                        (dev_class_mask)        \

#define PCI_VDEVICE(vend, dev)                          PCI_VENDOR_ID_##vend,   \
                                                        (dev),                  \
                                                        PCI_ANY_ID,             \
                                                        PCI_ANY_ID,             \
                                                        0,                      \
                                                        0

/*********************************************************************************************************
  调试模式配置
*********************************************************************************************************/
#if PCI_DEBUG_EN > 0                                                    /* 是否使能 PCI 调试模式        */
#define PCI_DEBUG_MSG                       _DebugHandle
#define PCI_DEBUG_MSG_FMT                   _DebugFormat
#else                                                                   /* PCI_DEBUG_EN                 */
#define PCI_DEBUG_MSG(level, msg)
#define PCI_DEBUG_MSG_FMT(level, fmt, ...)
#endif                                                                  /* PCI_DEBUG_EN                 */

/*********************************************************************************************************
  PCI access config
*********************************************************************************************************/

typedef struct {
    union {
        PCI_DRV_FUNCS0         *PCICF_pDrvFuncs0;
        PCI_DRV_FUNCS12        *PCICF_pDrvFuncs12;
    } f;
#define PCIC_pDrvFuncs0         f.PCICF_pDrvFuncs0
#define PCIC_pDrvFuncs12        f.PCICF_pDrvFuncs12
    
#define PCI_MECHANISM_0         0
#define PCI_MECHANISM_1         1
#define PCI_MECHANISM_2         2
    UINT8                       PCIC_ucMechanism;
    
    /*
     * 下面地址仅适用于 PCI_MECHANISM_1 与 PCI_MECHANISM_2
     */
    addr_t                      PCIC_ulConfigAddr;
    addr_t                      PCIC_ulConfigData;
    addr_t                      PCIC_ulConfigBase;                      /* only for PCI_MECHANISM_2     */
    
    LW_OBJECT_HANDLE            PCIC_ulLock;

    INT                         PCIC_iIndex;
    INT                         PCIC_iBusMax;
} PCI_CONFIG;

/*********************************************************************************************************
  PCI 设备控制块 (只管理 PCI_HEADER_TYPE_NORMAL 类型设备)
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE        PDT_lineDevNode;                                /* 设备管理节点                 */

    UINT32              PDT_uiDevVersion;                               /* 设备版本                     */
    UINT32              PDT_uiUnitNumber;                               /* 设备编号                     */
    CHAR                PDT_cDevName[PCI_DEV_NAME_MAX];                 /* 设备名称                     */
    INT                 PDT_iDevBus;                                    /* 总线号                       */
    INT                 PDT_iDevDevice;                                 /* 设备号                       */
    INT                 PDT_iDevFunction;                               /* 功能号                       */
    PCI_HDR             PDT_phDevHdr;                                   /* 设备头                       */

    PVOID               PDT_pvDevBarBase[PCI_BARS_MAX];                 /* 各 BAR 基地址                */
    PVOID               PDT_pvDevBarBasePhys[PCI_BARS_MAX];             /* 各 BAR 物理地址              */
    size_t              PDT_stDevBarBaseSize[PCI_BARS_MAX];             /* 各 BAR 大小                  */
    UINT32              PDT_uiDevBarBaseFlags[PCI_BARS_MAX];            /* 各 BAR 类型                  */

    PVOID               PDT_pvDevDriver;                                /* 驱动句柄                     */

    INT                 PDT_iDevIrqMsiEn;                               /* 是否使能 MSI                 */
    ULONG               PDT_ulDevIrqVector;                             /* 中断向量                     */
    CHAR                PDT_cDevIrqName[PCI_DEV_IRQ_NAME_MAX];          /* 中断名称                     */
    PINT_SVR_ROUTINE    PDT_pfuncDevIrqHandle;                          /* 中断服务句柄                 */
    PVOID               PDT_pvDevIrqArg;                                /* 中断服务参数                 */

    LW_OBJECT_HANDLE    PDT_hDevLock;                                   /* 设备自身操作锁               */
} PCI_DEV_TCB;
typedef PCI_DEV_TCB    *PCI_DEV_HANDLE;

/*********************************************************************************************************
  驱动支持设备列表控制块
*********************************************************************************************************/
typedef struct {
    UINT32      PDIT_uiVendor;                                          /* 厂商 ID                      */
    UINT32      PDIT_uiDevice;                                          /* 设备 ID                      */

    UINT32      PDIT_uiSubVendor;                                       /* 子厂商 ID                    */
    UINT32      PDIT_uiSubDevice;                                       /* 子设备 ID                    */

    UINT32      PDIT_uiClass;                                           /* 设备类                       */
    UINT32      PDIT_uiClassMask;                                       /* 设备子类                     */

    ULONG       PDIT_ulData;                                            /* 设备私有数据                 */
} PCI_DEVICE_ID_TCB;
typedef PCI_DEVICE_ID_TCB      *PCI_DEVICE_ID_HANDLE;

/*********************************************************************************************************
  驱动注册控制块
*********************************************************************************************************/
typedef struct {
    CPCHAR                  PDRT_pcName;                                /* 驱动名称                     */
    PVOID                   PDRT_pvHandle;                              /* 特定句柄                     */
    PCI_DEVICE_ID_HANDLE    PDRT_hIdTable;                              /* 设备支持列表                 */
    UINT32                  PDRT_uiIdTableSize;                         /* 设备支持列表大小             */

    /*
     *  驱动常用函数, PDRT_pfuncProbe 与 PDRT_pfuncRemove 不能为 LW_NULL, 其它可选
     */
    INT   (*PDRT_pfuncProbe)(PCI_DEV_HANDLE hHandle, const PCI_DEVICE_ID_HANDLE hIdEntry);
    VOID  (*PDRT_pfuncRemove)(PCI_DEV_HANDLE hHandle);
    INT   (*PDRT_pfuncSuspend)(PCI_DEV_HANDLE hHandle, PCI_PM_MESSAGE_HANDLE hPmMsg);
    INT   (*PDRT_pfuncSuspendLate)(PCI_DEV_HANDLE hHandle, PCI_PM_MESSAGE_HANDLE hPmMsg);
    INT   (*PDRT_pfuncResumeEarly)(PCI_DEV_HANDLE hHandle);
    INT   (*PDRT_pfuncResume)(PCI_DEV_HANDLE hHandle);
    VOID  (*PDRT_pfuncShutdown)(PCI_DEV_HANDLE hHandle);

    PCI_ERROR_HANDLE        PDRT_hErrorHandler;                         /* 错误处理句柄                 */

} PCI_DRV_REGISTER_TCB;
typedef PCI_DRV_REGISTER_TCB       *PCI_DRV_REGISTER_HANDLE;

/*********************************************************************************************************
  驱动管理设备控制块
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            PDDT_lineDrvDevNode;                        /* 设备节点管理                 */

    PCI_DEV_HANDLE          PDDT_hDrvDevHandle;                         /* 设备句柄                     */
} PCI_DRV_DEV_TCB;
typedef PCI_DRV_DEV_TCB     *PCI_DRV_DEV_HANDLE;

/*********************************************************************************************************
  驱动控制块
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            PDT_lineDrvNode;                            /* 驱动节点管理                 */
    CHAR                    PDT_cDrvName[PCI_DRV_NAME_MAX];             /* 驱动名称                     */
    PCI_DEVICE_ID_HANDLE    PDT_hDrvIdTable;                            /* 设备支持列表                 */
    UINT32                  PDT_uiDrvIdTableSize;                       /* 设备支持列表大小             */

    /*
     *  驱动常用函数, PDRT_pfuncProbe 与 PDRT_pfuncRemove 不能为 LW_NULL, 其它可选
     */
    INT   (*PDT_pfuncDrvProbe)(PCI_DEV_HANDLE hHandle, const PCI_DEVICE_ID_HANDLE hIdEntry);
    VOID  (*PDT_pfuncDrvRemove)(PCI_DEV_HANDLE hHandle);
    INT   (*PDT_pfuncDrvSuspend)(PCI_DEV_HANDLE hHandle, PCI_PM_MESSAGE_HANDLE hPmMsg);
    INT   (*PDT_pfuncDrvSuspendLate)(PCI_DEV_HANDLE hHandle, PCI_PM_MESSAGE_HANDLE hPmMsg);
    INT   (*PDT_pfuncDrvResumeEarly)(PCI_DEV_HANDLE hHandle);
    INT   (*PDT_pfuncDrvResume)(PCI_DEV_HANDLE hHandle);
    VOID  (*PDT_pfuncDrvShutdown)(PCI_DEV_HANDLE hHandle);

    PCI_ERROR_HANDLE        PDT_hDrvErrHandler;                         /* 错误处理句柄                 */

    INT                     PDT_iDrvFlag;                               /* 驱动标志                     */
    UINT32                  PDT_uiDrvDevNum;                            /* 关联设备数                   */
    LW_LIST_LINE_HEADER     PDT_plineDrvDevHeader;                      /* 设备管理链表头               */
} PCI_DRV_TCB;
typedef PCI_DRV_TCB    *PCI_DRV_HANDLE;

/*********************************************************************************************************
  API
  API_PciConfigInit() 必须在 BSP 初始化总线系统时被调用, 而且必须保证是第一个被正确调用的 PCI 系统函数.
*********************************************************************************************************/

LW_API INT          API_PciConfigInit(PCI_CONFIG *ppcicfg);
LW_API VOID         API_PciConfigReset(INT  iRebootType);

LW_API INT          API_PciLock(VOID);
LW_API INT          API_PciUnlock(VOID);

LW_API INT          API_PciConfigInByte(INT iBus, INT iSlot, INT iFunc, INT iOft, UINT8 *pucValue);
LW_API INT          API_PciConfigInWord(INT iBus, INT iSlot, INT iFunc, INT iOft, UINT16 *pusValue);
LW_API INT          API_PciConfigInDword(INT iBus, INT iSlot, INT iFunc, INT iOft, UINT32 *puiValue);

LW_API INT          API_PciConfigOutByte(INT iBus, INT iSlot, INT iFunc, INT iOft, UINT8 ucValue);
LW_API INT          API_PciConfigOutWord(INT iBus, INT iSlot, INT iFunc, INT iOft, UINT16 usValue);
LW_API INT          API_PciConfigOutDword(INT iBus, INT iSlot, INT iFunc, INT iOft, UINT32 uiValue);

LW_API INT          API_PciConfigModifyByte(INT iBus, INT iSlot, INT iFunc, INT iOft,
                                            UINT8 ucMask, UINT8 ucValue);
LW_API INT          API_PciConfigModifyWord(INT iBus, INT iSlot, INT iFunc, INT iOft,
                                            UINT16 usMask, UINT16 usValue);
LW_API INT          API_PciConfigModifyDword(INT iBus, INT iSlot, INT iFunc, INT iOft,
                                             UINT32 uiMask, UINT32 uiValue);
                                         
LW_API INT          API_PciSpecialCycle(INT iBus, UINT32 uiMsg);

LW_API INT          API_PciFindDev(UINT16 usVendorId, UINT16 usDeviceId, INT  iInstance,
                                   INT *piBus, INT *piSlot, INT *piFunc);
LW_API INT          API_PciFindClass(UINT16  usClassCode, INT  iInstance,
                                     INT *piBus, INT *piSlot, INT *piFunc);
                                 
LW_API INT          API_PciTraversal(INT (*pfuncCall)(), PVOID pvArg, INT iMaxBusNum);
LW_API INT          API_PciTraversalDev(INT iBusStart, BOOL bSubBus, INT (*pfuncCall)(), PVOID pvArg);
                                 
LW_API INT          API_PciConfigDev(INT iBus, INT iSlot, INT iFunc,
                                     ULONG ulIoBase, pci_addr_t ulMemBase,
                                     UINT8 ucLatency, UINT32 uiCommand);

LW_API INT          API_PciFuncDisable(INT iBus, INT iSlot, INT iFunc);

LW_API INT          API_PciInterConnect(ULONG ulVector, PINT_SVR_ROUTINE pfuncIsr,
                                        PVOID pvArg, CPCHAR pcName);
LW_API INT          API_PciInterDisconnect(ULONG ulVector, PINT_SVR_ROUTINE pfuncIsr,
                                           PVOID pvArg);

LW_API INT          API_PciGetHeader(INT iBus, INT iSlot, INT iFunc, PCI_HDR *p_pcihdr);
LW_API INT          API_PciHeaderTypeGet(INT iBus, INT iSlot, INT iFunc, UINT8 *ucType);

LW_API INT          API_PciVpdRead(INT iBus, INT iSlot, INT iFunc, INT iPos, UINT8 *pucBuf, INT iLen);
LW_API INT          API_PciIrqGet(INT iBus, INT iSlot, INT iFunc,
                                  INT iMsiEn, INT iLine, INT iPin, ULONG *pulVector);

LW_API INT          API_PciConfigFetch(INT iBus, INT iSlot, INT iFunc, UINT uiPos, UINT uiLen);

LW_API PCI_CONFIG  *API_PciConfigHandleGet(INT iIndex);
LW_API INT          API_PciConfigIndexGet(PCI_CONFIG *ppcHandle);
LW_API INT          API_PciConfigBusMaxSet(INT iIndex, UINT32 uiBusMax);
LW_API INT          API_PciConfigBusMaxGet(INT iIndex);

LW_API INT          API_PciIntxEnableSet(INT iBus, INT iSlot, INT iFunc, INT iEnable);
LW_API INT          API_PciIntxMaskSupported(INT iBus, INT iSlot, INT iFunc, INT *piSupported);

LW_API VOID                 API_PciDrvBindEachDev(PCI_DRV_HANDLE hDrvHandle);
LW_API INT                  API_PciDrvLoad(PCI_DRV_HANDLE       hDrvHandle,
                                           PCI_DEV_HANDLE       hDevHandle,
                                           PCI_DEVICE_ID_HANDLE hIdEntry);
LW_API PCI_DRV_DEV_HANDLE   API_PciDrvDevFind(PCI_DRV_HANDLE hDrvHandle, PCI_DEV_HANDLE hDevHandle);
LW_API INT                  API_PciDrvDevDel(PCI_DRV_HANDLE hDrvHandle, PCI_DEV_HANDLE hDevHandle);
LW_API INT                  API_PciDrvDevAdd(PCI_DRV_HANDLE hDrvHandle, PCI_DEV_HANDLE hDevHandle);
LW_API PCI_DRV_HANDLE       API_PciDrvHandleGet(CPCHAR pcName);
LW_API INT                  API_PciDrvDelete(PCI_DRV_HANDLE  hDrvHandle);
LW_API INT                  API_PciDrvRegister(PCI_DRV_REGISTER_HANDLE hHandle);
LW_API INT                  API_PciDrvInit(VOID);

LW_API PCI_DEVICE_ID_HANDLE API_PciDevMatchDrv(PCI_DEV_HANDLE hDevHandle, PCI_DRV_HANDLE hDrvHandle);
LW_API VOID                 API_PciDevBindEachDrv(PCI_DEV_HANDLE hDevHandle);
LW_API INT                  API_PciDevInterDisable(PCI_DEV_HANDLE   hHandle,
                                                   ULONG            ulVector,
                                                   PINT_SVR_ROUTINE pfuncIsr,
                                                   PVOID            pvArg);
LW_API INT                  API_PciDevInterEnable(PCI_DEV_HANDLE   hHandle,
                                                  ULONG            ulVector,
                                                  PINT_SVR_ROUTINE pfuncIsr,
                                                  PVOID            pvArg);
LW_API INT                  API_PciDevInterDisonnect(PCI_DEV_HANDLE   hHandle,
                                                     ULONG            ulVector,
                                                     PINT_SVR_ROUTINE pfuncIsr,
                                                     PVOID            pvArg);
LW_API INT                  API_PciDevInterConnect(PCI_DEV_HANDLE   hHandle,
                                                   ULONG            ulVector,
                                                   PINT_SVR_ROUTINE pfuncIsr,
                                                   PVOID            pvArg,
                                                   CPCHAR           pcName);
LW_API INT                  API_PciDevInterVectorGet(PCI_DEV_HANDLE  hHandle, ULONG *pulVector);

LW_API INT                  API_PciDevConfigRead(PCI_DEV_HANDLE hHandle,
                                                 INT iPos, UINT8 *pucBuf, INT iLen);
LW_API INT                  API_PciDevConfigWrite(PCI_DEV_HANDLE hHandle,
                                                  INT iPos, UINT8 *pucBuf, INT iLen);
LW_API PCI_DEV_HANDLE       API_PciDevHandleGet(INT iBus, INT iDevice, INT iFunction);
LW_API PCI_DEV_HANDLE       API_PciDevAdd(INT iBus, INT iDevice, INT iFunction);
LW_API INT                  API_PciDevDelete(PCI_DEV_HANDLE hHandle);
LW_API INT                  API_PciDevDrvDel(PCI_DEV_HANDLE  hDevHandle, PCI_DRV_HANDLE  hDrvHandle);
LW_API INT                  API_PciDevDrvUpdate(PCI_DEV_HANDLE  hDevHandle, PCI_DRV_HANDLE  hDrvHandle);
LW_API INT                  API_PciDevListCreate(VOID);
LW_API INT                  API_PciDevInit(VOID);

LW_API INT                  API_PciDevMsiEnableGet(PCI_DEV_HANDLE  hHandle, INT *piEnable);

#define pciConfigInit           API_PciConfigInit
#define pciConfigReset          API_PciConfigReset

#define pciLock                 API_PciLock
#define pciUnlock               API_PciUnlock

#define pciConfigInByte         API_PciConfigInByte
#define pciConfigInWord         API_PciConfigInWord
#define pciConfigInDword        API_PciConfigInDword

#define pciConfigOutByte        API_PciConfigOutByte
#define pciConfigOutWord        API_PciConfigOutWord
#define pciConfigOutDword       API_PciConfigOutDword

#define pciConfigModifyByte     API_PciConfigModifyByte
#define pciConfigModifyWord     API_PciConfigModifyWord
#define pciConfigModifyDword    API_PciConfigModifyDword

#define pciSpecialCycle         API_PciSpecialCycle

#define pciFindDev              API_PciFindDev
#define pciFindClass            API_PciFindClass

#define pciTraversal            API_PciTraversal
#define pciTraversalDev         API_PciTraversalDev

#define pciConfigDev            API_PciConfigDev
#define pciFuncDisable          API_PciFuncDisable

#define pciInterDisable         API_PciInterDisable
#define pciInterEnable          API_PciInterEnable
#define pciInterConnect         API_PciInterConnect
#define pciInterDisconnect      API_PciInterDisconnect

#define pciGetHeader            API_PciGetHeader
#define pciHeaderTypeGet        API_PciHeaderTypeGet

#define pciVpdRead              API_PciVpdRead
#define pciIrqGet               API_PciIrqGet

#define pciConfigFetch          API_PciConfigFetch

#define pciConfigHandleGet      API_PciConfigHandleGet
#define pciConfigIndexGet       API_PciConfigIndexGet
#define pciConfigBusMaxSet      API_PciConfigBusMaxSet
#define pciConfigBusMaxGet      API_PciConfigBusMaxGet

#define pciIntxEnableSet        API_PciIntxEnableSet
#define pciIntxMaskSupported    API_PciIntxMaskSupported

#define pciDrvLoad              API_PciDrvLoad
#define pciDrvDevFind           API_PciDrvDevFind
#define pciDrvDevDel            API_PciDrvDevDel
#define pciDrvDevAdd            API_PciDrvDevAdd
#define pciDrvHandleGet         API_PciDrvHandleGet
#define pciDrvDelete            API_PciDrvDelete
#define pciDrvRegister          API_PciDrvRegister
#define pciDrvInit              API_PciDrvInit

#define pciDevMatchDrv          API_PciDevMatchDrv
#define pciDevBindEachDrv       API_PciDevBindEachDrv
#define pciDevInterDisable      API_PciDevInterDisable
#define pciDevInterEnable       API_PciDevInterEnable
#define pciDevInterDisonnect    API_PciDevInterDisonnect
#define pciDevInterConnect      API_PciDevInterConnect
#define pciDevInterVectorGet    API_PciDevInterVectorGet
#define pciDevConfigRead        API_PciDevConfigRead
#define pciDevConfigWrite       API_PciDevConfigWrite
#define pciHandleGet            API_PciDevHandleGet
#define pciDevAdd               API_PciDevAdd
#define pciDevDelete            API_PciDevDelete
#define pciDevDrvDel            API_PciDevDrvDel
#define pciDevDrvUpdate         API_PciDevDrvUpdate
#define pciDevListCreate        API_PciDevListCreate
#define pciDevInit              API_PciDevInit

#define pciDevMsiEnableGet      API_PciDevMsiEnableGet

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_PCI_EN > 0)         */
#endif                                                                  /*  __PCI_DEV_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
