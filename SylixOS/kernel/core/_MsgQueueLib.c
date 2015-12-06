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
** ��   ��   ��: _MsgQueueLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 01 �� 07 ��
**
** ��        ��: ��Ϣ�����ڲ������⡣

** BUG:
2009.06.19  �޸�ע��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _MsgQueueGetMsg
** ��������: ����Ϣ�����л��һ����Ϣ
** �䡡��  : pmsgqueue     ��Ϣ���п��ƿ�
**         : pvMsgBuffer   ���ջ�����
**         : stMaxByteSize ��������С
**         : pstMsgLen     �����Ϣ�ĳ���
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if (LW_CFG_MSGQUEUE_EN > 0) && (LW_CFG_MAX_MSGQUEUES > 0)

VOID  _MsgQueueGetMsg (PLW_CLASS_MSGQUEUE    pmsgqueue,                 /*  ��Ϣ����                    */
                       PVOID                 pvMsgBuffer,               /*  ������                      */
                       size_t                stMaxByteSize,             /*  ��������С                  */
                       size_t               *pstMsgLen)                 /*  ʵ�ʿ����ֽ���              */
{
    REGISTER size_t   stMsgLen;
    
    stMsgLen = *((size_t *)pmsgqueue->MSGQUEUE_pucOutputPtr);           /*  ��ñ���Ϣ��С              */
    pmsgqueue->MSGQUEUE_pucOutputPtr += sizeof(size_t);                 /*  ��ָ���ƶ�����Ϣ��          */
    
    *pstMsgLen = (stMaxByteSize < stMsgLen) ? 
                 (stMaxByteSize) : (stMsgLen);                          /*  ȷ��������Ϣ����            */
    
    lib_memcpy(pvMsgBuffer,                                             /*  ������Ϣ                    */
               pmsgqueue->MSGQUEUE_pucOutputPtr,
               *pstMsgLen);
    
    pmsgqueue->MSGQUEUE_pucOutputPtr += pmsgqueue->MSGQUEUE_stEachMsgByteSize;
    
    if (pmsgqueue->MSGQUEUE_pucOutputPtr >= 
        pmsgqueue->MSGQUEUE_pucBufferHighAddr) {                        /*  ���� OUT ָ��               */
        pmsgqueue->MSGQUEUE_pucOutputPtr = pmsgqueue->MSGQUEUE_pucBufferLowAddr;
    }
}
/*********************************************************************************************************
** ��������: _MsgQueueSendMsg
** ��������: ����Ϣ������д��һ����Ϣ
** �䡡��  : pmsgqueue     ��Ϣ���п��ƿ� 
**           pvMsgBuffer   ��Ϣ������
**         : stMsgLen      ��Ϣ����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _MsgQueueSendMsg (PLW_CLASS_MSGQUEUE    pmsgqueue,
                        PVOID                 pvMsgBuffer,
                        size_t                stMsgLen)
{
    REGISTER size_t   *pstMsgLen;
    
    pstMsgLen = (size_t *)pmsgqueue->MSGQUEUE_pucInputPtr;              /*  �����Ϣ����ָ��            */
    pmsgqueue->MSGQUEUE_pucInputPtr += sizeof(size_t);                  /*  ��ָ���ƶ�����Ϣ��          */
    
    lib_memcpy((PVOID)pmsgqueue->MSGQUEUE_pucInputPtr,                  /*  ������Ϣ                    */
               pvMsgBuffer,                                             
               stMsgLen);
    
    *pstMsgLen = stMsgLen;                                              /*  ��¼��Ϣ����                */
    
    pmsgqueue->MSGQUEUE_pucInputPtr += pmsgqueue->MSGQUEUE_stEachMsgByteSize;
    
    if (pmsgqueue->MSGQUEUE_pucInputPtr >= 
        pmsgqueue->MSGQUEUE_pucBufferHighAddr) {                        /*  ���� IN ָ��                */
        pmsgqueue->MSGQUEUE_pucInputPtr = pmsgqueue->MSGQUEUE_pucBufferLowAddr;
    }
}
/*********************************************************************************************************
** ��������: _MsgQueueSendMsgUrgent
** ��������: ����Ϣ������д��һ��������Ϣ
** �䡡��  : pmsgqueue     ��Ϣ���п��ƿ�
**         : pvMsgBuffer   ��Ϣ������
**         : stMsgLen      ��Ϣ����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _MsgQueueSendMsgUrgent (PLW_CLASS_MSGQUEUE    pmsgqueue,
                              PVOID                 pvMsgBuffer,
                              size_t                stMsgLen)
{
    REGISTER size_t   *pstMsgLen;
    REGISTER PBYTE     pucMsgQueueBuffer;
    
    if (pmsgqueue->MSGQUEUE_pucOutputPtr == 
        pmsgqueue->MSGQUEUE_pucBufferLowAddr) {                         /*  OUT ����                    */
        pmsgqueue->MSGQUEUE_pucOutputPtr = pmsgqueue->MSGQUEUE_pucBufferHighAddr;
    }
    
    pmsgqueue->MSGQUEUE_pucOutputPtr -=
        (pmsgqueue->MSGQUEUE_stEachMsgByteSize + sizeof(size_t));
    
    pstMsgLen = (size_t *)pmsgqueue->MSGQUEUE_pucOutputPtr;             /*  �����Ϣ����ָ��            */
    pucMsgQueueBuffer = pmsgqueue->MSGQUEUE_pucOutputPtr + sizeof(size_t);
    
    lib_memcpy((PVOID)pucMsgQueueBuffer,                                /*  ������Ϣ                    */
               pvMsgBuffer,                                             
               stMsgLen);
               
    *pstMsgLen = stMsgLen;                                              /*  ��¼��Ϣ����                */
}
/*********************************************************************************************************
** ��������: _MsgQueueGetMsgLen
** ��������: ����Ϣ�����л��һ����Ϣ�ĳ���
** �䡡��  : pmsgqueue     ��Ϣ���п��ƿ�
**           pstMsgLen     ��Ϣ����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _MsgQueueGetMsgLen (PLW_CLASS_MSGQUEUE  pmsgqueue, size_t  *pstMsgLen)
{
    *pstMsgLen = *((size_t *)pmsgqueue->MSGQUEUE_pucOutputPtr);
}

#endif                                                                  /*  (LW_CFG_MSGQUEUE_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_MSGQUEUES > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
