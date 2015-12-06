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
** ��   ��   ��: mipsContextAsm.h
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 09 �� 01 ��
**
** ��        ��: MIPS ��ϵ���������Ĵ���.
*********************************************************************************************************/

#ifndef __MIPSCONTEXTASM_H
#define __MIPSCONTEXTASM_H

/*********************************************************************************************************
  Operate the general registers: at, v0-v1, a0-a3, t0-t9, s0-s7, gp, fp, ra
*********************************************************************************************************/

#define OPERATE_REG(op)                 \
    op      RA , STK_OFFSET_RA(SP);     \
    op      FP , STK_OFFSET_FP(SP);     \
    op      GP , STK_OFFSET_GP(SP);     \
    op      S7 , STK_OFFSET_S7(SP);     \
    op      S6 , STK_OFFSET_S6(SP);     \
    op      S5 , STK_OFFSET_S5(SP);     \
    op      S4 , STK_OFFSET_S4(SP);     \
    op      S3 , STK_OFFSET_S3(SP);     \
    op      S2 , STK_OFFSET_S2(SP);     \
    op      S1 , STK_OFFSET_S1(SP);     \
    op      S0 , STK_OFFSET_S0(SP);     \
    op      T9 , STK_OFFSET_T9(SP);     \
    op      T8 , STK_OFFSET_T8(SP);     \
    op      T7 , STK_OFFSET_T7(SP);     \
    op      T6 , STK_OFFSET_T6(SP);     \
    op      T5 , STK_OFFSET_T5(SP);     \
    op      T4 , STK_OFFSET_T4(SP);     \
    op      T3 , STK_OFFSET_T3(SP);     \
    op      T2 , STK_OFFSET_T2(SP);     \
    op      T1 , STK_OFFSET_T1(SP);     \
    op      T0 , STK_OFFSET_T0(SP);     \
    op      A3 , STK_OFFSET_A3(SP);     \
    op      A2 , STK_OFFSET_A2(SP);     \
    op      A1 , STK_OFFSET_A1(SP);     \
    op      A0 , STK_OFFSET_A0(SP);     \
    op      V1 , STK_OFFSET_V1(SP);     \
    op      V0 , STK_OFFSET_V0(SP);     \
    op      AT , STK_OFFSET_AT(SP);

/*********************************************************************************************************
  Operate the general registers: t0-t1
*********************************************************************************************************/

#define LW_T0_T1_REG()                  \
    LW      T0 , STK_OFFSET_T0(SP);     \
    LW      T1 , STK_OFFSET_T1(SP);     \

/*********************************************************************************************************
  Pop the context: at, v0-v1, a0-a3, t0-t9, s0-s7, gp, fp, ra, pc, sr, lo, hi
*********************************************************************************************************/

#define RESTORE_REGS()                  \
    .set    push;                       \
    .set    noat;                       \
    .set    noreorder;                  \
                                        \
    OPERATE_REG(LW);                    \
                                        \
    LW      T0 , STK_OFFSET_LO(SP);     \
    MTLO    T0;                         \
                                        \
    LW      T0 , STK_OFFSET_HI(SP);     \
    MTHI    T0;                         \
                                        \
    LW      T0 , STK_OFFSET_EPC(SP);    \
    LW      T1 , STK_OFFSET_SR(SP);     \
    MOV     K0 , T0;                    \
    MOV     K1 , T1;                    \
                                        \
    LW_T0_T1_REG();                     \
                                        \
    ADDU    SP , STK_CTX_SIZE;          \
                                        \
    MTC0(K0, CP0_EPC);                  \
                                        \
    ORI     K1 , K1 , M_StatusEXL;      \
    MTC0(K1, CP0_STATUS);               \
                                        \
    ERET;                               \
                                        \
    .set    pop

/*********************************************************************************************************
  Push the context: at, v0-v1, a0-a3, t0-t9, s0-s7, gp, fp, ra, pc, sr, lo, hi, cause
*********************************************************************************************************/

#define SAVE_REGS()                     \
    .set    push;                       \
    .set    noat;                       \
    .set    noreorder;                  \
                                        \
    SUBU    SP , STK_CTX_SIZE;          \
                                        \
    OPERATE_REG(SW);                    \
                                        \
    SW      RA , STK_OFFSET_EPC(SP);    \
                                        \
    MFC0(T1, CP0_STATUS);               \
    SW      T1 , STK_OFFSET_SR(SP);     \
                                        \
    MFC0(T1, CP0_CAUSE);                \
    SW      T1 , STK_OFFSET_CAUSE(SP);  \
                                        \
    MFLO    T1;                         \
    SW      T1 , STK_OFFSET_LO(SP);     \
                                        \
    MFHI    T1;                         \
    SW      T1 , STK_OFFSET_HI(SP);     \
                                        \
    .set    pop

#endif                                                                  /*  __MIPSCONTEXTASM_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
