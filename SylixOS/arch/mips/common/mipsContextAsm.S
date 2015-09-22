;/**********************************************************************************************************
;**
;**                                    �й�������Դ��֯
;**
;**                                   Ƕ��ʽʵʱ����ϵͳ
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------�ļ���Ϣ--------------------------------------------------------------------------------
;**
;** ��   ��   ��: mipsContextAsm.S
;**
;** ��   ��   ��: Ryan.Xin (����)
;**
;** �ļ���������: 2015 �� 09 �� 01 ��
;**
;** ��        ��: MIPS ��ϵ�����������л�.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>

FILE_BEGIN()

    IMPORT_LABEL(_SchedSwp)
#if LW_CFG_COROUTINE_EN > 0
    IMPORT_LABEL(_SchedCrSwp)
#endif

    EXPORT_LABEL(archTaskCtxStart)
    EXPORT_LABEL(archTaskCtxSwitch)
    EXPORT_LABEL(archCrtCtxSwitch)
    EXPORT_LABEL(archIntCtxLoad)
    EXPORT_LABEL(archSigCtxLoad)

/*********************************************************************************************************
  pop context (normal execution): at, v0-v1,a0-a3,t0-t9,s0-s7,gp,fp,ra, & pc
*********************************************************************************************************/
MACRO_DEF(RESTORE_REG_RET)
    .set noat
    .set noreorder
    lw ra,0(sp)
    lw fp,4(sp)
    lw gp,8(sp)
    lw t9,12(sp)
    lw t8,16(sp)
    lw s7,20(sp)
    lw s6,24(sp)
    lw s5,28(sp)
    lw s4,32(sp)
    lw s3,36(sp)
    lw s2,40(sp)
    lw s1,44(sp)
    lw s0,48(sp)
    lw t7,52(sp)
    lw t6,56(sp)
    lw t5,60(sp)
    lw t4,64(sp)
    lw t3,68(sp)
    lw t2,72(sp)
    lw t1,76(sp)
    lw t0,80(sp)
    lw a3,84(sp)
    lw a2,88(sp)
    lw a1,92(sp)
    lw a0,96(sp)
    lw v1,100(sp)
    lw v0,104(sp)
    lw AT,108(sp)
    lw k0,112(sp)
    MTC0 k0,CP0_STATUS
    lw k0,116(sp)
    addu sp,120
    jr k0
    nop
    .set at
MACRO_END()

/*********************************************************************************************************
  pop context: at, v0-v1,a0-a3,t0-t9,s0-s7,gp,fp,ra, & pc
*********************************************************************************************************/
MACRO_DEF(RESTORE_REG_ERET)
    .set noat
    .set noreorder
    lw ra,0(sp)
    lw fp,4(sp)
    lw gp,8(sp)
    lw t9,12(sp)
    lw t8,16(sp)
    lw s7,20(sp)
    lw s6,24(sp)
    lw s5,28(sp)
    lw s4,32(sp)
    lw s3,36(sp)
    lw s2,40(sp)
    lw s1,44(sp)
    lw s0,48(sp)
    lw t7,52(sp)
    lw t6,56(sp)
    lw t5,60(sp)
    lw t4,64(sp)
    lw t3,68(sp)
    lw t2,72(sp)
    lw t1,76(sp)
    lw t0,80(sp)
    lw a3,84(sp)
    lw a2,88(sp)
    lw a1,92(sp)
    lw a0,96(sp)
    lw v1,100(sp)
    lw v0,104(sp)
    lw AT,108(sp)
    lw k0,112(sp)
    MTC0 k0,CP0_STATUS
    lw k0,116(sp)
    MTC0 k0,CP0_EPC
    addu sp,120
    eret
    nop
    .set at
MACRO_END()

/*********************************************************************************************************
  push context: at, v0-v1,a0-a3,t0-t9,s0-s7,gp,fp,ra, & pc
*********************************************************************************************************/
MACRO_DEF(STORE_REG_RET Retaddr)
    .set noat
    .set noreorder
    subu sp,120
    sw ra,0(sp)
    sw fp,4(sp)
    sw gp,8(sp)
    sw t9,12(sp)
    sw t8,16(sp)
    sw s7,20(sp)
    sw s6,24(sp)
    sw s5,28(sp)
    sw s4,32(sp)
    sw s3,36(sp)
    sw s2,40(sp)
    sw s1,44(sp)
    sw s0,48(sp)
    sw t7,52(sp)
    sw t6,56(sp)
    sw t5,60(sp)
    sw t4,64(sp)
    sw t3,68(sp)
    sw t2,72(sp)
    sw t1,76(sp)
    sw t0,80(sp)
    sw a3,84(sp)
    sw a2,88(sp)
    sw a1,92(sp)
    sw a0,96(sp)
    sw v1,100(sp)
    sw v0,104(sp)
    sw AT,108(sp)
    MFC0 t0,CP0_STATUS
    sw t0,112(sp)
    sw \Retaddr,116(sp)
    .set at
MACRO_END()

;/*********************************************************************************************************
;  ��ǰ CPU ��ʼ���ж�����ִ��
;  ����Ϊ��ǰ CPU ���ƿ�, �� A0 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    lw     t0 , 0(a0)                                                   ;/*  ��ȡ�������ջջ����ַ      */
    lw     sp , 0(t0)                                                   ;/*  ��ջָ�븳ֵ                */
    RESTORE_REG_RET                                                     ;/*  ���мĴ�����ջ������
                                                                        ; *  CP0 Status�Ĵ���            */
FUNC_END(archTaskCtxStart)

;/*********************************************************************************************************
;  �߳��л�
;  ����Ϊ��ǰ CPU ���ƿ�, �� A0 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/
FUNC_DEF(archTaskCtxSwitch)
    STORE_REG_RET ra                                                    ;/* ���мĴ�����ջ������
                                                                        ; *CP0 Status�Ĵ���              */

    move   t0 , a0                                                      ;/*  stack current = SP          */
    lw     t1 , 0(a0)
    sw     sp , 0(t1)

    subu   sp , 4
    sw     a0, 0(sp)                                                    ;/*  ��ǰ CPU ��Ϣ��ջ           */

    la     t0 , _SchedSwp
    jalr   t0
    nop

    lw     a0 , 0(sp)
    addu   sp , 4

    move   t0 , a0
    lw     t1 , 0(a0)
    lw     sp , 0(t1)

    RESTORE_REG_ERET                                                    ;/* ���мĴ�����ջ������
                                                                        ; * CP0 Status�Ĵ���             */
FUNC_END(archTaskCtxSwitch)

;/*********************************************************************************************************
;  Э���л�
;  ����Ϊ��ǰ CPU ���ƿ�, �� A0 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    jr  ra
    nop
FUNC_END(archCrtCtxSwitch)
#endif

;/*********************************************************************************************************
;  �жϷ���ʱ, �߳�װ��
;  ����Ϊ��ǰ CPU ���ƿ�, �� A0 Ϊ��ǰ CPU ���ƿ�ָ��
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    jr  ra
    nop
FUNC_END(archIntCtxLoad)

;/*********************************************************************************************************
;  �ź������ķ���
;  ����Ϊ��Ҫ���صĶ�ջջ��λ��
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    jr  ra
    nop
FUNC_END(archSigCtxLoad)

FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/