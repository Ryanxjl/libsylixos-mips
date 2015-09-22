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
** ��   ��   ��: inlSchedSeek.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: ����ϵͳ��������ѯ�о����̵߳�������ȼ���������
*********************************************************************************************************/

#ifndef  __INLSCHEDSEEK_H
#define  __INLSCHEDSEEK_H

/*********************************************************************************************************
  ��������ѯ�о����̵߳�������ȼ���������
*********************************************************************************************************/

static LW_INLINE PLW_CLASS_PCBBMAP  _SchedSeekPriority (PLW_CLASS_CPU  pcpu, UINT8 *ucPriority)
{
#if LW_CFG_SMP_EN > 0
    REGISTER PLW_CLASS_PCBBMAP  ppcbbmap;
    REGISTER UINT8              ucLocal, ucGlobal;
    
    if (_BitmapIsEmpty(LW_CPU_RDY_BMAP(pcpu))) {                        /*  CPU ���ؾ������            */
        if (_BitmapIsEmpty(LW_GLOBAL_RDY_BMAP())) {
            return  (LW_NULL);                                          /*  ��������������              */
        }
        *ucPriority = _BitmapHigh(LW_GLOBAL_RDY_BMAP());
        return  (LW_GLOBAL_RDY_PCBBMAP());
    
    } else {                                                            /*  CPU ���ؾ�����ǿ�          */
        if (_BitmapIsEmpty(LW_GLOBAL_RDY_BMAP())) {
            *ucPriority = _BitmapHigh(LW_CPU_RDY_BMAP(pcpu));
            return  (LW_CPU_RDY_PCBBMAP(pcpu));
        }

        ucLocal  = _BitmapHigh(LW_CPU_RDY_BMAP(pcpu));
        ucGlobal = _BitmapHigh(LW_GLOBAL_RDY_BMAP());
        
        if (LW_PRIO_IS_HIGH_OR_EQU(ucLocal, ucGlobal)) {                /*  ͬ���ȼ�, ����ִ�� local    */
            ppcbbmap    = LW_CPU_RDY_PCBBMAP(pcpu);
            *ucPriority = ucLocal;
        
        } else {
            ppcbbmap    = LW_GLOBAL_RDY_PCBBMAP();
            *ucPriority = ucGlobal;
        }
        
        return  (ppcbbmap);
    }
#else
    if (_BitmapIsEmpty(LW_GLOBAL_RDY_BMAP())) {                         /*  ��������������              */
        return  (LW_NULL);
    
    } else {
        *ucPriority = _BitmapHigh(LW_GLOBAL_RDY_BMAP());
        return  (LW_GLOBAL_RDY_PCBBMAP());
    }
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
}

#endif                                                                  /*  __INLSCHEDSEEK_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
