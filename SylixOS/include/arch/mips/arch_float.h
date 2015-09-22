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
** ��   ��   ��: arch_float.h
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 09 �� 01 ��
**
** ��        ��: MIPS �������.
*********************************************************************************************************/

#ifndef __MIPS_ARCH_FLOAT_H
#define __MIPS_ARCH_FLOAT_H

/*********************************************************************************************************
  MIPS floating point coprocessor register definitions
*********************************************************************************************************/

#define fp0     $f0                                                     /* return reg 0                 */
#define fp1     $f1                                                     /* return reg 1                 */
#define fp2     $f2                                                     /* return reg 2                 */
#define fp3     $f3                                                     /* return reg 3                 */
#define fp4     $f4                                                     /* caller saved 0               */
#define fp5     $f5                                                     /* caller saved 1               */
#define fp6     $f6                                                     /* caller saved 2               */
#define fp7     $f7                                                     /* caller saved 3               */
#define fp8     $f8                                                     /* caller saved 4               */
#define fp9     $f9                                                     /* caller saved 5               */
#define fp10    $f10                                                    /* caller saved 6               */
#define fp11    $f11                                                    /* caller saved 7               */
#define fp12    $f12                                                    /* arg reg 0                    */
#define fp13    $f13                                                    /* arg reg 1                    */
#define fp14    $f14                                                    /* arg reg 2                    */
#define fp15    $f15                                                    /* arg reg 3                    */
#define fp16    $f16                                                    /* caller saved 8               */
#define fp17    $f17                                                    /* caller saved 9               */
#define fp18    $f18                                                    /* caller saved 10              */
#define fp19    $f19                                                    /* caller saved 11              */
#define fp20    $f20                                                    /* callee saved 0               */
#define fp21    $f21                                                    /* callee saved 1               */
#define fp22    $f22                                                    /* callee saved 2               */
#define fp23    $f23                                                    /* callee saved 3               */
#define fp24    $f24                                                    /* callee saved 4               */
#define fp25    $f25                                                    /* callee saved 5               */
#define fp26    $f26                                                    /* callee saved 6               */
#define fp27    $f27                                                    /* callee saved 7               */
#define fp28    $f28                                                    /* callee saved 8               */
#define fp29    $f29                                                    /* callee saved 9               */
#define fp30    $f30                                                    /* callee saved 10              */
#define fp31    $f31                                                    /* callee saved 11              */

#define CP1_IR       $0                                                 /* implementation/revision reg  */
#define CP1_STATUS   $31                                                /* control/status reg           */

/*********************************************************************************************************
  float ��ʽ (ʹ�� union ������Ϊ�м�ת��, ���� GCC 3.x.x strict aliasing warning)
*********************************************************************************************************/

#define __ARCH_FLOAT_EXP_NAN           255                              /*  NaN ���������� Exp ֵ     */

typedef struct __cpu_float_field {
    unsigned int        frac : 23;
    unsigned int        exp  :  8;
    unsigned int        sig  :  1;
} __CPU_FLOAT_FIELD;

typedef union __cpu_float {
    __CPU_FLOAT_FIELD   fltfield;                                       /*  float λ���ֶ�              */
    float               flt;                                            /*  float ռλ                  */
} __CPU_FLOAT;

static LW_INLINE INT  __ARCH_FLOAT_ISNAN (float  x)
{
    __CPU_FLOAT     cpuflt;

    cpuflt.flt = x;

    return  ((cpuflt.fltfield.exp == __ARCH_FLOAT_EXP_NAN) && (cpuflt.fltfield.frac != 0));
}

static LW_INLINE INT  __ARCH_FLOAT_ISINF (float  x)
{
    __CPU_FLOAT     cpuflt;

    cpuflt.flt = x;

    return  ((cpuflt.fltfield.exp == __ARCH_FLOAT_EXP_NAN) && (cpuflt.fltfield.frac == 0));
}

/*********************************************************************************************************
  double ��ʽ
*********************************************************************************************************/

#define __ARCH_DOUBLE_EXP_NAN           2047                            /*  NaN ���������� Exp ֵ     */
#define __ARCH_DOUBLE_INC_FLOAT_H          0                            /*  �Ƿ����ñ����� float.h �ļ� */

/*********************************************************************************************************
  arm-none-eabi-gcc ... GNU
*********************************************************************************************************/

#if LW_CFG_DOUBLE_MIX_ENDIAN > 0
typedef struct __cpu_double_field {                                     /*  old mixed-endian            */
    unsigned int        frach : 20;
    unsigned int        exp   : 11;
    unsigned int        sig   :  1;

    unsigned int        fracl : 32;                                     /*  �� 32 λ����ߵ�ַ          */
} __CPU_DOUBLE_FIELD;
#else
typedef struct __cpu_double_field {                                     /*  native-endian               */
    unsigned int        fracl : 32;                                     /*  �� 32 λ����͵�ַ          */

    unsigned int        frach : 20;
    unsigned int        exp   : 11;
    unsigned int        sig   :  1;
} __CPU_DOUBLE_FIELD;
#endif                                                                  /*  __ARCH_DOUBLE_MIX_ENDIAN    */

typedef union __cpu_double {
    __CPU_DOUBLE_FIELD  dblfield;                                       /*  float λ���ֶ�              */
    double              dbl;                                            /*  float ռλ                  */
} __CPU_DOUBLE;

static LW_INLINE INT  __ARCH_DOUBLE_ISNAN (double  x)
{
    __CPU_DOUBLE     dblflt;

    dblflt.dbl = x;

    return  ((dblflt.dblfield.exp == __ARCH_DOUBLE_EXP_NAN) &&
             ((dblflt.dblfield.fracl != 0) &&
              (dblflt.dblfield.frach != 0)));
}

static LW_INLINE INT  __ARCH_DOUBLE_ISINF (double  x)
{
    __CPU_DOUBLE     dblflt;

    dblflt.dbl = x;

    return  ((dblflt.dblfield.exp == __ARCH_DOUBLE_EXP_NAN) &&
             ((dblflt.dblfield.fracl == 0) ||
              (dblflt.dblfield.frach == 0)));
}

#endif                                                                  /*  __MIPS_ARCH_FLOAT_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
