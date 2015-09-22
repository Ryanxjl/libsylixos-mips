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
** ��   ��   ��: pciLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 09 �� 28 ��
**
** ��        ��: PCI ��������ģ��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_PCI_EN > 0)
/*********************************************************************************************************
  PCI ������
*********************************************************************************************************/
PCI_CONFIG                             *_G_p_pciConfig;
/*********************************************************************************************************
  PCI ���������õ�ַ
*********************************************************************************************************/
#define PCI_CONFIG_ADDR0()              _G_p_pciConfig->PCIC_ulConfigAddr
#define PCI_CONFIG_ADDR1()              _G_p_pciConfig->PCIC_ulConfigData
#define PCI_CONFIG_ADDR2()              _G_p_pciConfig->PCIC_ulConfigBase
/*********************************************************************************************************
  PCI ���������� IO ����
*********************************************************************************************************/
#define PCI_IN_BYTE(addr)               _G_p_pciConfig->PCIC_pDrvFuncs->ioInByte((addr))
#define PCI_IN_WORD(addr)               _G_p_pciConfig->PCIC_pDrvFuncs->ioInWord((addr))
#define PCI_IN_DWORD(addr)              _G_p_pciConfig->PCIC_pDrvFuncs->ioInDword((addr))
#define PCI_OUT_BYTE(addr, data)        _G_p_pciConfig->PCIC_pDrvFuncs->ioOutByte((addr),  (UINT8)(data))
#define PCI_OUT_WORD(addr, data)        _G_p_pciConfig->PCIC_pDrvFuncs->ioOutWord((addr),  (UINT16)(data))
#define PCI_OUT_DWORD(addr, data)       _G_p_pciConfig->PCIC_pDrvFuncs->ioOutDword((addr), (UINT32)(data))
/*********************************************************************************************************
** ��������: API_PciConfigInit
** ��������: ��װ pci ��������������
** �䡡��  : p_pcicfg  pci ��������������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigInit (PCI_CONFIG *p_pcicfg)
{
    if (_G_p_pciConfig == LW_NULL) {
        _G_p_pciConfig =  p_pcicfg;
    }
    
    if (_G_p_pciConfig->PCIC_ulLock == LW_OBJECT_HANDLE_INVALID) {
        _G_p_pciConfig->PCIC_ulLock =  API_SemaphoreMCreate("pci_lock", LW_PRIO_DEF_CEILING, 
                                                            LW_OPTION_WAIT_PRIORITY |
                                                            LW_OPTION_DELETE_SAFE |
                                                            LW_OPTION_INHERIT_PRIORITY |
                                                            LW_OPTION_OBJECT_GLOBAL,
                                                            LW_NULL);
    }
    
    return  (_G_p_pciConfig) ? (ERROR_NONE) : (PX_ERROR);
}
/*********************************************************************************************************
** ��������: API_PciConfigReset
** ��������: ֹͣ pci �����豸 (ֻ�������ϲ������豸)
** �䡡��  : iRebootType   ϵͳ��λ����
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
VOID  API_PciConfigReset (INT  iRebootType)
{
    (VOID)iRebootType;
    
    if (_G_p_pciConfig == LW_NULL) {
        return;
    }
    
    API_PciTraversalDev(0, LW_FALSE, (INT (*)())API_PciFuncDisable, LW_NULL);
}
/*********************************************************************************************************
** ��������: API_PciLock
** ��������: pci ����
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciLock (VOID)
{
    if (_G_p_pciConfig->PCIC_ulLock) {
        API_SemaphoreMPend(_G_p_pciConfig->PCIC_ulLock, LW_OPTION_WAIT_INFINITE);
        return  (ERROR_NONE);
        
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: API_PciUnlock
** ��������: pci ����
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciUnlock (VOID)
{
    if (_G_p_pciConfig->PCIC_ulLock) {
        API_SemaphoreMPost(_G_p_pciConfig->PCIC_ulLock);
        return  (ERROR_NONE);
        
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: API_PciConfigInByte
** ��������: �� pci ���ÿռ��ȡһ���ֽ�
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           pucValue  ��ȡ�Ľ��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigInByte (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT8 *pucValue)
{
    UINT8   ucRet = 0;
    
    switch (_G_p_pciConfig->PCIC_ucMechanism) {
    
    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | (iOft & 0xfc) | 0x80000000));
        ucRet = PCI_IN_BYTE(PCI_CONFIG_ADDR1() + (iOft & 0x3));
        break;
        
    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xf0 | (iFunc << 1));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), iBus);
        ucRet = PCI_IN_DWORD(PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        ucRet >>= (iOft & 0x03) * 8;
        break;
        
    default:
        return  (PX_ERROR);
    }

    if (pucValue) {
        *pucValue = ucRet;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigInWord
** ��������: �� pci ���ÿռ��ȡһ���� (16 bit)
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           pusValue  ��ȡ�Ľ��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigInWord (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT16 *pusValue)
{
    UINT16  usRet = 0;
    
    if (iOft & 0x01) {
        return  (PX_ERROR);
    }

    switch (_G_p_pciConfig->PCIC_ucMechanism) {

    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | (iOft & 0xfc) | 0x80000000));
        usRet = PCI_IN_WORD(PCI_CONFIG_ADDR1() + (iOft & 0x2));
        break;

    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xf0 | (iFunc << 1));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), iBus);
        usRet = PCI_IN_DWORD(PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        usRet >>= (iOft & 0x02) * 8;
        break;

    default:
        return  (PX_ERROR);
    }

    if (pusValue) {
        *pusValue = usRet;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigInDword
** ��������: �� pci ���ÿռ��ȡһ��˫�� (32 bit)
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           puiValue  ��ȡ�Ľ��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigInDword (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT32 *puiValue)
{
    UINT32  uiRet = 0;
    
    if (iOft & 0x03) {
        return  (PX_ERROR);
    }

    switch (_G_p_pciConfig->PCIC_ucMechanism) {

    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | (iOft & 0xfc) | 0x80000000));
        uiRet = PCI_IN_DWORD(PCI_CONFIG_ADDR1());
        break;

    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xf0 | (iFunc << 1));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), iBus);
        uiRet = PCI_IN_DWORD(PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        break;

    default:
        return  (PX_ERROR);
    }

    if (puiValue) {
        *puiValue = uiRet;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigOutByte
** ��������: �� pci ���ÿռ�д��һ���ֽ�
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           ucValue   д�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigOutByte (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT8 ucValue)
{
    UINT32  uiRet;
    UINT32  uiMask = 0x000000ff;

    switch (_G_p_pciConfig->PCIC_ucMechanism) {

    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | (iOft & 0xfc) | 0x80000000));
        PCI_OUT_BYTE((PCI_CONFIG_ADDR1() + (iOft & 0x3)), ucValue);
        break;

    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xf0 | (iFunc << 1));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), iBus);
        uiRet    = PCI_IN_DWORD(PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc));
        ucValue  = (ucValue & uiMask) << ((iOft & 0x03) * 8);
        uiMask <<= (iOft & 0x03) * 8;
        uiRet    = (uiRet & ~uiMask) | ucValue;
        PCI_OUT_DWORD((PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc)), uiRet);
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        break;

    default:
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigOutWord
** ��������: �� pci ���ÿռ�д��һ���� (16 bit)
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           usValue   д�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigOutWord (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT16 usValue)
{
    UINT32  uiRet;
    UINT32  uiMask = 0x0000ffff;
    
    if (iOft & 0x01) {
        return  (PX_ERROR);
    }

    switch (_G_p_pciConfig->PCIC_ucMechanism) {

    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | (iOft & 0xfc) | 0x80000000));
        PCI_OUT_WORD((PCI_CONFIG_ADDR1() + (iOft & 0x2)), usValue);
        break;

    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xf0 | (iFunc << 1));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), iBus);
        uiRet    = PCI_IN_DWORD(PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc));
        usValue  = (usValue & uiMask) << ((iOft & 0x02) * 8);
        uiMask <<= (iOft & 0x02) * 8;
        uiRet    = (uiRet & ~uiMask) | usValue;
        PCI_OUT_DWORD((PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc)), uiRet);
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        break;

    default:
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigOutDword
** ��������: �� pci ���ÿռ�д��һ��˫�� (32 bit)
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           uiValue   д�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigOutDword (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT32 uiValue)
{
    if (iOft & 0x03) {
        return  (PX_ERROR);
    }
    
    switch (_G_p_pciConfig->PCIC_ucMechanism) {

    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | (iOft & 0xfc) | 0x80000000));
        PCI_OUT_DWORD(PCI_CONFIG_ADDR1(), uiValue);
        break;

    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xf0 | (iFunc << 1));
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), iBus);
        PCI_OUT_DWORD((PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8) | (iOft & 0xfc)), uiValue);
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        break;

    default:
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigModifyByte
** ��������: �� pci ���ÿռ��������λ���޸�һ���ֽ�
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           ucMask    ����
**           ucValue   д�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigModifyByte (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT8 ucMask, UINT8 ucValue)
{
    INTREG  iregInterLevel;
    UINT8   ucTemp;
    INT     iRet = PX_ERROR;
    
    iregInterLevel = KN_INT_DISABLE();
    
    if (API_PciConfigInByte(iBus, iSlot, iFunc, iOft, &ucTemp) == ERROR_NONE) {
        ucTemp = (ucTemp & ~ucMask) | (ucValue & ucMask);
        iRet   = API_PciConfigOutByte(iBus, iSlot, iFunc, iOft, ucTemp);
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_PciConfigModifyWord
** ��������: �� pci ���ÿռ��������λ���޸�һ���� (16 bit)
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           usMask    ����
**           usValue   д�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigModifyWord (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT16 usMask, UINT16 usValue)
{
    INTREG  iregInterLevel;
    UINT16  usTemp;
    INT     iRet = PX_ERROR;
    
    if (iOft & 0x01) {
        return  (PX_ERROR);
    }
    
    iregInterLevel = KN_INT_DISABLE();
    
    if (API_PciConfigInWord(iBus, iSlot, iFunc, iOft, &usTemp) == ERROR_NONE) {
        usTemp = (usTemp & ~usMask) | (usValue & usMask);
        iRet   = API_PciConfigOutWord(iBus, iSlot, iFunc, iOft, usTemp);
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_PciConfigModifyDword
** ��������: �� pci ���ÿռ��������λ���޸�һ��˫�� (32 bit)
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           iOft      ���ÿռ��ַƫ��
**           uiMask    ����
**           uiValue   д�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigModifyDword (INT iBus, INT iSlot, INT iFunc, INT iOft, UINT32 uiMask, UINT32 uiValue)
{
    INTREG  iregInterLevel;
    UINT32  uiTemp;
    INT     iRet = PX_ERROR;
    
    if (iOft & 0x03) {
        return  (PX_ERROR);
    }
    
    iregInterLevel = KN_INT_DISABLE();
    
    if (API_PciConfigInDword(iBus, iSlot, iFunc, iOft, &uiTemp) == ERROR_NONE) {
        uiTemp = (uiTemp & ~uiMask) | (uiValue & uiMask);
        iRet   = API_PciConfigOutDword(iBus, iSlot, iFunc, iOft, uiTemp);
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_PciSpecialCycle
** ��������: �� pci �����Ϲ㲥һ����Ϣ
** �䡡��  : iBus      ���ߺ�
**           uiMsg     ��Ϣ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciSpecialCycle (INT iBus, UINT32 uiMsg)
{
    INTREG  iregInterLevel;
    INT     iRet  = ERROR_NONE;
    INT     iSlot = 0x1f;
    INT     iFunc = 0x07;
    
    iregInterLevel = KN_INT_DISABLE();
    
    switch (_G_p_pciConfig->PCIC_ucMechanism) {
    
    case PCI_MECHANISM_1:
        PCI_OUT_DWORD(PCI_CONFIG_ADDR0(), (PCI_PACKET(iBus, iSlot, iFunc) | 0x80000000));
        PCI_OUT_DWORD(PCI_CONFIG_ADDR1(), uiMsg);
        break;
    
    case PCI_MECHANISM_2:
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0xff);
        PCI_OUT_BYTE(PCI_CONFIG_ADDR1(), 0x00);
        PCI_OUT_DWORD((PCI_CONFIG_ADDR2() | ((iSlot & 0x000f) << 8)), uiMsg);
        PCI_OUT_BYTE(PCI_CONFIG_ADDR0(), 0);
        break;
    
    default:
        iRet = PX_ERROR;
        break;
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_PciFindDev
** ��������: ��ѯһ��ָ���� pci �豸
** �䡡��  : usVendorId    ��Ӧ�̺�
**           usDeviceId    �豸 ID
**           iInstance     �ڼ����豸ʵ��
**           piBus         ��ȡ�����ߺ�
**           piSlot        ��ȡ�Ĳ�ۺ�
**           piFunc        ��ȡ�Ĺ��ܺ�
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciFindDev (UINT16  usVendorId, 
                     UINT16  usDeviceId, 
                     INT     iInstance,
                     INT    *piBus, 
                     INT    *piSlot, 
                     INT    *piFunc)
{
    INT         iBus;
    INT         iSlot;
    INT         iFunc;
    
    INT         iRet = PX_ERROR;

    UINT8       ucHeader;
    UINT16      usVendorTemp;
    UINT16      usDeviceTemp;
    
    if (iInstance < 0) {
        return  (PX_ERROR);
    }
    
    for (iBus = 0; iBus < PCI_MAX_BUS; iBus++) {
        for (iSlot = 0; iSlot < (PCI_MAX_SLOTS - 1); iSlot++) {
            for (iFunc = 0; iFunc < (PCI_MAX_FUNCTIONS - 1); iFunc++) {
            
                API_PciConfigInWord(iBus, iSlot, iFunc, PCI_VENDOR_ID, &usVendorTemp);
                API_PciConfigInWord(iBus, iSlot, iFunc, PCI_DEVICE_ID, &usDeviceTemp);
                
                if (usVendorTemp == 0xffff) {                           /*  û���豸����                */
                    if (iFunc == 0) {
                        break;
                    }
                    continue;                                           /*  next function               */
                }
                
                if ((usVendorTemp == usVendorId) && 
                    (usDeviceTemp == usDeviceId)) {                     /*  ƥ���ѯ����                */
                    
                    if (iInstance == 0) {
                        if (piBus) {
                            *piBus = iBus;
                        }
                        if (piSlot) {
                            *piSlot = iSlot;
                        }
                        if (piFunc) {
                            *piFunc = iFunc;
                        }
                        iRet = ERROR_NONE;
                        goto    __out;
                    }
                    iInstance--;
                }
                
                if (iFunc == 0) {
                    API_PciConfigInByte(iBus, iSlot, iFunc, PCI_HEADER_TYPE, &ucHeader);
                    
                    if ((ucHeader & PCI_HEADER_MULTI_FUNC) != PCI_HEADER_MULTI_FUNC) {
                        break;
                    }
                }
            }
        }
    }
    
__out:
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_PciFindClass
** ��������: ��ѯָ���豸���͵� pci �豸
** �䡡��  : usClassCode   ���ͱ��� (PCI_CLASS_STORAGE_SCSI, PCI_CLASS_DISPLAY_XGA ...)
**           iInstance     �ڼ����豸ʵ��
**           piBus         ��ȡ�����ߺ�
**           piSlot        ��ȡ�Ĳ�ۺ�
**           piFunc        ��ȡ�Ĺ��ܺ�
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciFindClass (UINT16  usClassCode, 
                       INT     iInstance,
                       INT    *piBus, 
                       INT    *piSlot, 
                       INT    *piFunc)
{
    INT         iBus;
    INT         iSlot;
    INT         iFunc;
    
    INT         iRet = PX_ERROR;

    UINT8       ucHeader;
    UINT16      usVendorTemp;
    UINT16      usClassTemp;
    
    if (iInstance < 0) {
        return  (PX_ERROR);
    }
    
    for (iBus = 0; iBus < PCI_MAX_BUS; iBus++) {
        for (iSlot = 0; iSlot < (PCI_MAX_SLOTS - 1); iSlot++) {
            for (iFunc = 0; iFunc < (PCI_MAX_FUNCTIONS - 1); iFunc++) {
            
                API_PciConfigInWord(iBus, iSlot, iFunc, PCI_VENDOR_ID, &usVendorTemp);
                
                if (usVendorTemp == 0xffff) {                           /*  û���豸����                */
                    if (iFunc == 0) {
                        break;
                    }
                    continue;                                           /*  next function               */
                }
            
                API_PciConfigInWord(iBus, iSlot, iFunc, PCI_CLASS_DEVICE, &usClassTemp);
                
                if (usClassTemp == usClassCode) {
                
                    if (iInstance == 0) {
                        if (piBus) {
                            *piBus = iBus;
                        }
                        if (piSlot) {
                            *piSlot = iSlot;
                        }
                        if (piFunc) {
                            *piFunc = iFunc;
                        }
                        iRet = ERROR_NONE;
                        goto    __out;
                    }
                    iInstance--;
                }
                
                if (iFunc == 0) {
                    API_PciConfigInByte(iBus, iSlot, iFunc, PCI_HEADER_TYPE, &ucHeader);
                    
                    if ((ucHeader & PCI_HEADER_MULTI_FUNC) != PCI_HEADER_MULTI_FUNC) {
                        break;
                    }
                }
            }
        }
    }
    
__out:
    return  (iRet);
}
/*********************************************************************************************************
** ��������: API_PciTraversal
** ��������: pci ���߱���
** �䡡��  : pfuncCall     �����ص�����
**           pvArg         �ص���������
**           iMaxBusNum    ������ߺ�
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciTraversal (INT (*pfuncCall)(), PVOID pvArg, INT iMaxBusNum)
{
    INT         iBus;
    INT         iSlot;
    INT         iFunc;

    UINT8       ucHeader;
    UINT16      usVendorTemp;
    
    if (!pfuncCall || (iMaxBusNum < 0)) {
        return  (PX_ERROR);
    }
    
    iMaxBusNum = (iMaxBusNum > (PCI_MAX_BUS - 1)) ? (PCI_MAX_BUS - 1) : iMaxBusNum;
    
    for (iBus = 0; iBus <= iMaxBusNum; iBus++) {
        for (iSlot = 0; iSlot < (PCI_MAX_SLOTS - 1); iSlot++) {
            for (iFunc = 0; iFunc < (PCI_MAX_FUNCTIONS - 1); iFunc++) {
            
                API_PciConfigInWord(iBus, iSlot, iFunc, PCI_VENDOR_ID, &usVendorTemp);
                
                if (usVendorTemp == 0xffff) {                           /*  û���豸����                */
                    if (iFunc == 0) {
                        break;                                          /*  next slot                   */
                    }
                    continue;                                           /*  next function               */
                }
                
                if (pfuncCall(iBus, iSlot, iFunc, pvArg) != ERROR_NONE) {
                    goto    __out;
                }
                
                if (iFunc == 0) {
                    API_PciConfigInByte(iBus, iSlot, iFunc, PCI_HEADER_TYPE, &ucHeader);
                    
                    if ((ucHeader & PCI_HEADER_MULTI_FUNC) != PCI_HEADER_MULTI_FUNC) {
                        break;
                    }
                }
            }
        }
    }
    
__out:
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciTraversalDev
** ��������: pci ���߱��������е��豸
** �䡡��  : iBusStart     ��ʼ���ߺ�
**           bSubBus       �Ƿ�����Ž�����
**           pfuncCall     �����ص�����
**           pvArg         �ص���������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciTraversalDev (INT   iBusStart, 
                          BOOL  bSubBus,
                          INT (*pfuncCall)(), 
                          PVOID pvArg)
{
    INT         iBus;
    INT         iSlot;
    INT         iFunc;
    
    UINT16      usVendorId;
    UINT16      usSubClass;
    
    UINT8       ucHeader;
    UINT8       ucSecBus;
    
    INT         iRet;
    
    if (!pfuncCall) {
        return  (PX_ERROR);
    }
    
    iBus = iBusStart;
    
    for (iSlot = 0; iSlot < (PCI_MAX_SLOTS - 1); iSlot++) {
        for (iFunc = 0; iFunc < (PCI_MAX_FUNCTIONS - 1); iFunc++) {
            
            API_PciConfigInWord(iBus, iSlot, iFunc, PCI_VENDOR_ID, &usVendorId);
            
            if (usVendorId == 0xffff) {
                if (iFunc == 0) {
                    break;                                              /*  next slot                   */
                }
                continue;                                               /*  next function               */
            }
            
            API_PciConfigInWord(iBus, iSlot, iFunc, PCI_CLASS_DEVICE, &usSubClass);
            
            if ((usSubClass & 0xff00) != (PCI_BASE_CLASS_BRIDGE << 8)) {
                iRet = pfuncCall(iBus, iSlot, iFunc, pvArg);
                if (iRet != ERROR_NONE) {
                    return  (iRet);
                }
            } 
            
            if (bSubBus) {
                if ((usSubClass == PCI_CLASS_BRIDGE_PCI) ||
                    (usSubClass == PCI_CLASS_BRIDGE_CARDBUS)) {         /*  PCI to PCI or PCI to cardbus*/
                    
                    API_PciConfigInByte(iBus, iSlot, iFunc, PCI_SECONDARY_BUS, &ucSecBus);
                    
                    if (ucSecBus > 0) {
                        iRet = API_PciTraversalDev(ucSecBus, bSubBus,
                                                   pfuncCall, pvArg);
                    
                    } else {
                        iRet = ERROR_NONE;
                    }
                    
                    if (iRet != ERROR_NONE) {
                        return  (iRet);
                    }
                }
            }
            
            if (iFunc == 0) {
                API_PciConfigInByte(iBus, iSlot, iFunc, PCI_HEADER_TYPE, &ucHeader);
                
                if ((ucHeader & PCI_HEADER_MULTI_FUNC) != PCI_HEADER_MULTI_FUNC) {
                    break;
                }
            }
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciConfigDev
** ��������: ���� pci �����ϵ�һ���豸
**           ���Ƚ�����豸����, Ȼ������ I/O ���� �ڴ��ַ, Ȼ�����ø��ٻ��������ӳټĴ���,
**           ����µ�ָ��д��ָ��Ĵ���.
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           uiIoBase  IO ����ַ
**           uiMemBase �ڴ����ַ
**           ucLatency �ӳ�ʱ�� (PCI clocks max 255) 
**           uiCommand ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciConfigDev (INT      iBus, 
                       INT      iSlot, 
                       INT      iFunc, 
                       UINT32   uiIoBase, 
                       UINT32   uiMemBase, 
                       UINT8    ucLatency,
                       UINT32   uiCommand)
{
    INT     i;
    UINT32  uiTemp;
    UINT8   ucCacheLine;

    API_PciConfigOutDword(iBus, iSlot, iFunc, PCI_COMMAND, 0x0);        /*  ����豸��ǰ����            */
    
    for (i = PCI_BASE_ADDRESS_0; i <= PCI_BASE_ADDRESS_5; i += 4) {
        
        API_PciConfigOutDword(iBus, iSlot, iFunc, i, 0xffffffff);
        API_PciConfigInDword(iBus, iSlot, iFunc, i, &uiTemp);
        
        if (uiTemp == 0) {
            break;
        }
        
        if (uiTemp & 0x1) {
            API_PciConfigOutDword(iBus, iSlot, iFunc, i, uiIoBase | 0x1);
        
        } else {
            API_PciConfigOutDword(iBus, iSlot, iFunc, i, uiMemBase & ~0x1);
        }
    }
    
#if LW_CFG_CACHE_EN > 0
    ucCacheLine = (API_CacheLine() >> 2);
#else
    ucCacheLine = (32 >> 2);
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
    
    API_PciConfigOutByte(iBus, iSlot, iFunc, PCI_CACHE_LINE_SIZE, ucCacheLine);
    
    API_PciConfigOutByte(iBus, iSlot, iFunc, PCI_LATENCY_TIMER, ucLatency);
    
    API_PciConfigModifyDword(iBus, iSlot, iFunc, PCI_COMMAND, (0xffff0000 | uiCommand), uiCommand);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PciFuncDisable
** ��������: ֹͣ pci �豸����
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciFuncDisable (INT iBus, INT iSlot, INT iFunc)
{
    return  (API_PciConfigModifyDword(iBus, iSlot, iFunc, PCI_COMMAND, 
                                      PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER, 0));
}
/*********************************************************************************************************
** ��������: API_PciInterConnect
** ��������: ���� pci �ж�����
** �䡡��  : ulVector  CPU �ж����� (pci ���忨���ܻṲ�� CPU ĳһ�ж�����)
**           pfuncIsr  �жϷ�����
**           pvArg     �жϷ���������
**           pcName    �жϷ�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciInterConnect (ULONG ulVector, PINT_SVR_ROUTINE pfuncIsr, PVOID pvArg, CPCHAR pcName)
{
    ULONG   ulFlag = 0ul;

    API_InterVectorGetFlag(ulVector, &ulFlag);
    if (!(ulFlag & LW_IRQ_FLAG_QUEUE)) {
        ulFlag |= LW_IRQ_FLAG_QUEUE;
        API_InterVectorSetFlag(ulVector, ulFlag);                       /*  �������༶�ж�              */
    }
    
    if (API_InterVectorConnect(ulVector, pfuncIsr, pvArg, pcName) == ERROR_NONE) {
        return  (ERROR_NONE);
    
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: API_PciInterDisonnect
** ��������: ���� pci ����ж�����
** �䡡��  : ulVector  CPU �ж����� (pci ���忨���ܻṲ�� CPU ĳһ�ж�����)
**           pfuncIsr  �жϷ�����
**           pvArg     �жϷ���������
**           pcName    �жϷ�������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciInterDisconnect (ULONG ulVector, PINT_SVR_ROUTINE pfuncIsr, PVOID pvArg)
{
    if (API_InterVectorDisconnect(ulVector, pfuncIsr, pvArg) == ERROR_NONE) {
        return  (ERROR_NONE);
    
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: API_PciGetHader
** ��������: ���ָ���豸�� pci ͷ��Ϣ
** �䡡��  : iBus      ���ߺ�
**           iSlot     ���
**           iFunc     ����
**           p_pcihdr  ͷ��Ϣ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
**                                            API ����
*********************************************************************************************************/
LW_API 
INT  API_PciGetHeader (INT iBus, INT iSlot, INT iFunc, PCI_HDR *p_pcihdr)
{
#define PCI_D  p_pcihdr->PCIH_pcidHdr
#define PCI_B  p_pcihdr->PCIH_pcibHdr
#define PCI_CB p_pcihdr->PCIH_pcicbHdr

    if (p_pcihdr == LW_NULL) {
        return  (PX_ERROR);
    }

    API_PciConfigInByte(iBus, iSlot, iFunc, PCI_HEADER_TYPE, &p_pcihdr->PCIH_ucType);
    p_pcihdr->PCIH_ucType &= PCI_HEADER_TYPE_MASK;

    if (p_pcihdr->PCIH_ucType == PCI_HEADER_TYPE_NORMAL) {              /* PCI iSlot                   */
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_VENDOR_ID, &PCI_D.PCID_usVendorId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_DEVICE_ID, &PCI_D.PCID_usDeviceId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_COMMAND,   &PCI_D.PCID_usCommand);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_STATUS,    &PCI_D.PCID_usStatus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_REVISION, &PCI_D.PCID_ucRevisionId);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_PROG,     &PCI_D.PCID_ucProgIf);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_DEVICE,   &PCI_D.PCID_ucSubClass);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS,          &PCI_D.PCID_ucClassCode);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CACHE_LINE_SIZE,&PCI_D.PCID_ucCacheLine);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_LATENCY_TIMER,  &PCI_D.PCID_ucLatency);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_HEADER_TYPE,    &PCI_D.PCID_ucHeaderType);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_BIST,           &PCI_D.PCID_ucBist);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_0, &PCI_D.PCID_uiBase0);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_1, &PCI_D.PCID_uiBase1);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_2, &PCI_D.PCID_uiBase2);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_3, &PCI_D.PCID_uiBase3);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_4, &PCI_D.PCID_uiBase4);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_5, &PCI_D.PCID_uiBase5);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CARDBUS_CIS,    &PCI_D.PCID_uiCis);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_SUBSYSTEM_VENDOR_ID, &PCI_D.PCID_usSubVendorId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_SUBSYSTEM_ID,        &PCI_D.PCID_usSubSystemId);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_ROM_ADDRESS,   &PCI_D.PCID_uiRomBase);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_INTERRUPT_LINE,&PCI_D.PCID_ucIntLine);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_INTERRUPT_PIN, &PCI_D.PCID_ucIntPin);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_MIN_GNT,       &PCI_D.PCID_ucMinGrant);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_MAX_LAT,       &PCI_D.PCID_ucMaxLatency);

    } else if (p_pcihdr->PCIH_ucType == PCI_HEADER_TYPE_BRIDGE) {       /* PCI to PCI bridge            */
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_VENDOR_ID, &PCI_B.PCIB_usVendorId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_DEVICE_ID, &PCI_B.PCIB_usDeviceId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_COMMAND,   &PCI_B.PCIB_usCommand);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_STATUS,    &PCI_B.PCIB_usStatus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_REVISION, &PCI_B.PCIB_ucRevisionId);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_PROG,     &PCI_B.PCIB_ucProgIf);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_DEVICE,   &PCI_B.PCIB_ucSubClass);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS,          &PCI_B.PCIB_ucClassCode);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CACHE_LINE_SIZE,&PCI_B.PCIB_ucCacheLine);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_LATENCY_TIMER,  &PCI_B.PCIB_ucLatency);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_HEADER_TYPE,    &PCI_B.PCIB_ucHeaderType);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_BIST,           &PCI_B.PCIB_ucBist);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_0, &PCI_B.PCIB_uiBase0);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_1, &PCI_B.PCIB_uiBase1);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_PRIMARY_BUS,    &PCI_B.PCIB_ucPriBus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_SECONDARY_BUS,  &PCI_B.PCIB_ucSecBus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_SUBORDINATE_BUS,&PCI_B.PCIB_ucSubBus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_SEC_LATENCY_TIMER, &PCI_B.PCIB_ucSecLatency);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_IO_BASE,           &PCI_B.PCIB_ucIoBase);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_IO_LIMIT,          &PCI_B.PCIB_ucIoLimit);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_SEC_STATUS,        &PCI_B.PCIB_usSecStatus);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_MEMORY_BASE,       &PCI_B.PCIB_usMemBase);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_MEMORY_LIMIT,      &PCI_B.PCIB_usMemLimit);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_PREF_MEMORY_BASE,  &PCI_B.PCIB_usPreBase);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_PREF_MEMORY_LIMIT, &PCI_B.PCIB_usPreLimit);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_PREF_BASE_UPPER32, &PCI_B.PCIB_uiPreBaseUpper);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_PREF_LIMIT_UPPER32, &PCI_B.PCIB_uiPreLimitUpper);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_IO_BASE_UPPER16,    &PCI_B.PCIB_usIoBaseUpper);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_IO_LIMIT_UPPER16, &PCI_B.PCIB_usIoLimitUpper);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_ROM_ADDRESS1,     &PCI_B.PCIB_uiRomBase);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_INTERRUPT_LINE,   &PCI_B.PCIB_ucIntLine);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_INTERRUPT_PIN,    &PCI_B.PCIB_ucIntPin);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_BRIDGE_CONTROL,   &PCI_B.PCIB_usControl);

    } else if (p_pcihdr->PCIH_ucType == PCI_HEADER_TYPE_CARDBUS) {      /* PCI card iBus bridge          */
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_VENDOR_ID, &PCI_CB.PCICB_usVendorId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_DEVICE_ID, &PCI_CB.PCICB_usDeviceId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_COMMAND,   &PCI_CB.PCICB_usCommand);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_STATUS,    &PCI_CB.PCICB_usStatus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_REVISION, &PCI_CB.PCICB_ucRevisionId);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_PROG,     &PCI_CB.PCICB_ucProgIf);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS_DEVICE,   &PCI_CB.PCICB_ucSubClass);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CLASS,          &PCI_CB.PCICB_ucClassCode);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CACHE_LINE_SIZE, &PCI_CB.PCICB_ucCacheLine);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_LATENCY_TIMER,   &PCI_CB.PCICB_ucLatency);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_HEADER_TYPE,     &PCI_CB.PCICB_ucHeaderType);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_BIST,            &PCI_CB.PCICB_ucBist);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_BASE_ADDRESS_0,  &PCI_CB.PCICB_uiBase0);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CB_CAP_PTR,      &PCI_CB.PCICB_ucCapPtr);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_CB_SEC_STATUS,   &PCI_CB.PCICB_usSecStatus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CB_PRIMARY_BUS,  &PCI_CB.PCICB_ucPriBus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CB_CARD_BUS,     &PCI_CB.PCICB_ucSecBus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CB_SUBORDINATE_BUS, &PCI_CB.PCICB_ucSubBus);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_CB_LATENCY_TIMER,   &PCI_CB.PCICB_ucSecLatency);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_MEMORY_BASE_0,   &PCI_CB.PCICB_uiMemBase0);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_MEMORY_LIMIT_0,  &PCI_CB.PCICB_uiMemLimit0);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_MEMORY_BASE_1,   &PCI_CB.PCICB_uiMemBase1);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_MEMORY_LIMIT_1,  &PCI_CB.PCICB_uiMemLimit1);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_IO_BASE_0,       &PCI_CB.PCICB_uiIoBase0);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_IO_LIMIT_0,      &PCI_CB.PCICB_uiIoLimit0);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_IO_BASE_1,       &PCI_CB.PCICB_uiIoBase1);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_IO_LIMIT_1,      &PCI_CB.PCICB_uiIoLimit1);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_INTERRUPT_LINE,     &PCI_CB.PCICB_ucIntLine);
        API_PciConfigInByte( iBus, iSlot, iFunc, PCI_INTERRUPT_PIN,      &PCI_CB.PCICB_ucIntPin);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_BRIDGE_CONTROL,     &PCI_CB.PCICB_usControl);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_CB_SUBSYSTEM_VENDOR_ID, &PCI_CB.PCICB_usSubVendorId);
        API_PciConfigInWord( iBus, iSlot, iFunc, PCI_CB_SUBSYSTEM_ID,        &PCI_CB.PCICB_usSubSystemId);
        API_PciConfigInDword(iBus, iSlot, iFunc, PCI_CB_LEGACY_MODE_BASE,    &PCI_CB.PCICB_uiLegacyBase);
    
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_PCI_EN > 0)         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
