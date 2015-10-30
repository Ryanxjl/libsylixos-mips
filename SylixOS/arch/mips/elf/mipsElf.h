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
** ��   ��   ��: mipsElf.h
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2015 �� 09 �� 09 ��
**
** ��        ��: ʵ�� MIPS ��ϵ�ṹ�� ELF �ļ��ض�λ.
*********************************************************************************************************/
#ifndef __ELF_MIPS_H
#define __ELF_MIPS_H

#define ELF_CLASS       ELFCLASS32
#define ELF_ARCH        EM_MIPS

/*
 * MIPS relocs.
 */

#define R_MIPS_NONE         0   /* No reloc */
#define R_MIPS_16           1   /* Direct 16 bit */
#define R_MIPS_32           2   /* Direct 32 bit */
#define R_MIPS_REL32        3   /* PC relative 32 bit */
#define R_MIPS_26           4   /* Direct 26 bit shifted */
#define R_MIPS_HI16         5   /* High 16 bit */
#define R_MIPS_LO16         6   /* Low 16 bit */
#define R_MIPS_GPREL16      7   /* GP relative 16 bit */
#define R_MIPS_LITERAL      8   /* 16 bit literal entry */
#define R_MIPS_GOT16        9   /* 16 bit GOT entry */
#define R_MIPS_PC16         10  /* PC relative 16 bit */
#define R_MIPS_CALL16       11  /* 16 bit GOT entry for function */
#define R_MIPS_GPREL32      12  /* GP relative 32 bit */

#define R_MIPS_JUMP_SLOT    127
#endif                                                                  /*  __ELF_MIPS_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/

