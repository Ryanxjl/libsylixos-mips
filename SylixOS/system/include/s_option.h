/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: s_option.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 02 �� 13 ��
**
** ��        ��: ����ϵͳѡ��궨�塣
**
** ע        ��: <FIOTRUNC, FIOSEEK, FIOWHERE>
                 ����������Ĳ�����λ off_t ָ��, ��ָ�� 64bit ��ָ��.
                 IO ϵͳ���֧�� 1TB ��С���ļ�, ���������� FAT ϵͳ (4GB FILE MAX)
                 <FIONREAD>
                 ��Ȼ���� LONG ��, Ҳ����˵, ���� 2GB ���ļ� FIONREAD ���ܲ�׼ȷ.
                 ͬʱ stdio ��� fseek �ڶ�������Ϊ long, ���֧�� 2GB �ļ�. (δ���Ľ�).
                 
** BUG:
2012.11.21  ������Ҫ O_TEMP ѡ��, ��Ϊ SylixOS �Ѿ�֧�� unlink ���򿪵��ļ�. 
2013.01.05  �����ļ���¼������.
2014.07.21  �����Դ������.
*********************************************************************************************************/

#ifndef  __S_OPTION_H
#define  __S_OPTION_H

/*********************************************************************************************************
  ioctl cmd
*********************************************************************************************************/

#define  FIONREAD                           1                   /* get num chars available to read      */
#define  FIOFLUSH                           2                   /* flush any chars in buffers           */
#define  FIOOPTIONS                         3                   /* set options (FIOSETOPTIONS)          */
#define  FIOBAUDRATE                        4                   /* set serial baud rate                 */
#define  FIODISKFORMAT                      5                   /* format disk                          */
#define  FIODISKINIT                        6                   /* initialize disk directory            */
#define  FIOSEEK                            7                   /* set current file char position       */
#define  FIOWHERE                           8                   /* get current file char position       */
#define  FIODIRENTRY                        9                   /* return a directory entry (obsolete)  */
#define  FIORENAME                          10                  /* rename a directory entry             */
#define  FIOMOVE                            FIORENAME
#define  FIOREADYCHANGE                     11                  /* return TRUE if there has been a      */

/*********************************************************************************************************
  media change on the device
*********************************************************************************************************/

#define  FIONWRITE                          12                  /* get num chars still to be written    */
#define  FIODISKCHANGE                      13                  /* set a media change on the device     */
#define  FIOCANCEL                          14                  /* cancel read or write on the device   */
#define  FIOSQUEEZE                         15                  /* squeeze out empty holes in rt-11     */

/*********************************************************************************************************
  file system
*********************************************************************************************************/

#define  FIONBIO                            16                  /* set non-blocking I/O; SOCKETS ONLY!  */
#define  FIONMSGS                           17                  /* return num msgs in pipe              */
#define  FIOGETNAME                         18                  /* return file name in arg              */
#define  FIOGETOPTIONS                      19                  /* get options                          */
#define  FIOSETOPTIONS                      FIOOPTIONS          /* set options                          */
#define  FIOISATTY                          20                  /* is a tty                             */
#define  FIOSYNC                            21                  /* sync to disk                         */
#define  FIOPROTOHOOK                       22                  /* specify protocol hook routine        */
#define  FIOPROTOARG                        23                  /* specify protocol argument            */
#define  FIORBUFSET                         24                  /* alter the size of read buffer        */
#define  FIOWBUFSET                         25                  /* alter the size of write buffer       */
#define  FIORFLUSH                          26                  /* flush any chars in read buffers      */
#define  FIOWFLUSH                          27                  /* flush any chars in write buffers     */
#define  FIOSELECT                          28                  /* wake up process in select on I/O     */
#define  FIOUNSELECT                        29                  /* wake up process in select on I/O     */
#define  FIONFREE                           30                  /* get free byte count on device        */
#define  FIOTRIM                            31                  /* ATA TRIM command                     */
#define  FIOLABELGET                        33                  /* get volume label                     */
#define  FIOLABELSET                        34                  /* set volume label                     */
#define  FIOATTRIBSET                       35                  /* set file attribute                   */
#define  FIOCONTIG                          36                  /* allocate contiguous space            */
#define  FIOREADDIR                         37                  /* read a directory entry (POSIX)       */
#define  FIOFSTATGET                        38                  /* get file status info                 */
#define  FIOUNMOUNT                         39                  /* unmount disk volume                  */
#define  FIOSCSICOMMAND                     40                  /* issue a SCSI command                 */
#define  FIONCONTIG                         41                  /* get size of max contig area on dev   */
#define  FIOTRUNC                           42                  /* truncate file to specified length    */
#define  FIOGETFL                           43                  /* get file mode, like fcntl(F_GETFL)   */
#define  FIOTIMESET                         44                  /* change times on a file for utime()   */
#define  FIOINODETONAME                     45                  /* given inode number, return filename  */
#define  FIOFSTATFSGET                      46                  /* get file status info 64bit           */
#define  FIOFSTATGET64                      47                  /* move file, ala mv, (mv not rename)   */
#define  FIODATASYNC                        48                  /* sync data to disk                    */
#define  FIOSETFL                           49                  /* set file mode, like fcntl(F_SETFL)   */

#define  FIOGETCC                           50                  /* get tty ctl char table               */
#define  FIOSETCC                           51                  /* set tty ctl char table               */

#define  FIOGETLK                           52                  /* get a lockf                          */
#define  FIOSETLK                           53                  /* set a lockf                          */
#define  FIOSETLKW                          54                  /* set a lockf (with blocking)          */

/*********************************************************************************************************
  SylixOS extern
*********************************************************************************************************/

#define  FIORTIMEOUT                        60                  /* ���ö���ʱʱ��                       */
#define  FIOWTIMEOUT                        61                  /* ����д��ʱʱ��                       */
#define  FIOWAITABORT                       62                  /* ��ǰ�����߳�ֹͣ�ȴ� read()/write()  */
#define  FIOABORTFUNC                       63                  /* tty ���յ� control-C ʱ�Ķ�������    */
#define  FIOABORTARG                        64                  /* tty ���յ� control-C ʱ�Ĳ�������    */

#define  FIOCHMOD                           70                  /* chmod                                */
#define  FIOCHOWN                           71                  /* chown                                */
#define  FIOFSTYPE                          80                  /* get file system type sring           */

/*********************************************************************************************************
  hardware rtc driver
*********************************************************************************************************/

#define  FIOGETTIME                         90                  /*  get hardware RTC time (arg:time_t *)*/
#define  FIOSETTIME                         91                  /*  set hardware RTC time (arg:time_t *)*/

/*********************************************************************************************************
  force delete
*********************************************************************************************************/

#define  FIOGETFORCEDEL                     92                  /*  get force delete flag               */
#define  FIOSETFORCEDEL                     93                  /*  set force delete flag               */

/*********************************************************************************************************
  power manager
*********************************************************************************************************/

#define  FIOSETWATCHDOG                     94                  /*  set power manager watchdog          */
#define  FIOGETWATCHDOG                     95                  /*  get power manager watchdog          */
#define  FIOWATCHDOGOFF                     96                  /*  turn-off power manager watchdog     */

/*********************************************************************************************************
  64 bit extern
*********************************************************************************************************/

#define  FIONFREE64                         (FIONFREE + 100)
#define  FIONREAD64                         (FIONREAD + 100)

/*********************************************************************************************************
  �Զ��幦��
*********************************************************************************************************/

#define  FIOUSRFUNC                         0x2000

/*********************************************************************************************************
  ioctl option values
*********************************************************************************************************/

#define  OPT_ECHO                           0x01                /* echo input                           */
#define  OPT_CRMOD                          0x02                /* lf -> crlf                           */
#define  OPT_TANDEM                         0x04                /* ^S/^Q flow control protocol          */
#define  OPT_7_BIT                          0x08                /* strip parity bit from 8 bit input    */
#define  OPT_MON_TRAP                       0x10                /* enable trap to monitor               */
#define  OPT_ABORT                          0x20                /* enable shell restart                 */
#define  OPT_LINE                           0x40                /* enable basic line protocol           */

#define  OPT_RAW                            0                   /* raw mode                             */

#if LW_CFG_SIO_TERMINAL_NOT_7_BIT > 0
#define  OPT_TERMINAL                       (OPT_ECHO     | OPT_CRMOD | OPT_TANDEM | \
			                                 OPT_MON_TRAP | OPT_ABORT | OPT_LINE)
#else
#define  OPT_TERMINAL                       (OPT_ECHO     | OPT_CRMOD | OPT_TANDEM | \
			                                 OPT_MON_TRAP | OPT_7_BIT | OPT_ABORT  | OPT_LINE)
#endif                                                          /*  LW_CFG_SIO_TERMINAL_NOT_7_BIT > 0   */

#define  CONTIG_MAX                         -1                  /* "count" for FIOCONTIG if requesting  */
                                                                /*  maximum contiguous space on dev     */
/*********************************************************************************************************
  abort flag
*********************************************************************************************************/

#define  OPT_RABORT                         0x01                /*  read() �����˳�                     */
#define  OPT_WABORT                         0x02                /*  write() �����˳�                    */

/*********************************************************************************************************
  miscellaneous
*********************************************************************************************************/

#define  FOLLOW_LINK_FILE                   -2                  /* this file is a symbol link file      */
#define  FOLLOW_LINK_TAIL                   -3                  /* file in symbol link file (have tail) */

#define  DEFAULT_FILE_PERM                  0000644             /* default file permissions             */
                                                                /* unix style rw-r--r--                 */
#define  DEFAULT_DIR_PERM                   0000754             /* default directory permissions        */
                                                                /* unix style rwxr-xr--                 */

/*********************************************************************************************************
  file types -- NOTE:  these values are specified in the NFS protocol spec 
*********************************************************************************************************/

#define  FSTAT_DIR                          0040000             /* directory                            */
#define  FSTAT_CHR                          0020000             /* character special file               */
#define  FSTAT_BLK                          0060000             /* block special file                   */
#define  FSTAT_REG                          0100000             /* regular file                         */
#define  FSTAT_LNK                          0120000             /* symbolic link file                   */
#define  FSTAT_NON                          0140000             /* named socket                         */

/*********************************************************************************************************
  lseek offset option
*********************************************************************************************************/

#ifndef  SEEK_SET
#define  SEEK_SET	                        0                   /* set file offset to offset            */
#endif

#ifndef  SEEK_CUR
#define  SEEK_CUR                           1                   /* set file offset to current plus oft  */
#endif

#ifndef  SEEK_END
#define  SEEK_END	                        2                   /* set file offset to EOF plus offset   */
#endif

#endif                                                          /*  __S_OPTION_H                        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
