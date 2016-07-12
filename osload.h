/*
*
*                                 Apache License
*                           Version 2.0, January 2004
*                        http://www.apache.org/licenses/
*/

#ifndef __OSLOAD_H__
#define __OSLOAD_H__

#define UNIV_LINUX                       1
#define HAVE_IB_ATOMIC_PTHREAD_T_GCC     1 
#define HAVE_IB_GCC_ATOMIC_BUILTINS      1 
#define HAVE_IB_GCC_ATOMIC_BUILTINS_64   1 
#define HAVE_IB_GCC_ATOMIC_BUILTINS_BYTE 1
#define HAVE_IB_GCC_SYNC_SYNCHRONISE     1 
#define SIZEOF_PTHREAD_T                 8

#include "mysql.h"

typedef struct _MYDATABASE
{
   MYSQL      *conn;
   int        errcode;
   char       errmsg[512];
   char       charset[256];
} MYDATABASE;

typedef struct _OSLoadData
{
   char prefix1[128];
   char prefix2[128];
   char prefix3[128];
 
   /* Load Average */
   double load1m;
   double load5m;
   double load15m;
   int    procrun;
   int    procnum;

   /* CPU */
   double   user;
   double   system;
   double   idle;
   double   iowait;
   double   irq;
   double   softirq;
   int      processes;
   int      context;
   int      runproc;
   int      blkproc;
   unsigned int uptime;
   double   irqcall;

   /* Swap */
   int   pgpgin;
   int   pgpgout;
   int   pswpin;
   int   pswpout;

   /* Memory */
   double memfree;
   double buffers;
   double cached;
   double swapcached;
   double swaptotal;
   double swapfree;
   double lowtotal;
   double lowfree;
   double hightotal;
   double highfree;
   double pagetables;

   /* NetWork */
   double   ibytes;
   double   ipackets;
   int   ierrs;
   double   obytes;
   double   opackets;
   int   oerrs;

   /* File NR */
   int   nropen;
   int   nrtotal;

   /* Socket Stats */
   int   tcpuse;
   int   tcpalloc;
   int   tcpwait;
   int   udpuse;
   
   /* snmp tcp protocol */
   double   activeopen;
   double   pasiveopen;
   double   afails;
   double   eresets;
   double   insegs;
   double   outsegs;
   double   resegs;
   double   inerrs;
   double   outrst;

   /* Disk Stats */
   /*
	Field 1 -- # of reads issued
	Field 2 -- # of reads merged
	Field 3 -- # of sectors read
	Field 4 -- # of milliseconds spent reading
	Field 5 -- # of writes completed
	Field 6 -- # of writes merged
	Field 7 -- # of sectors written
	Field 8 -- # of milliseconds spent writing
	Field 9 -- # of I/Os currently in progress
	Field 10 -- # of milliseconds spent doing I/Os
	Field 11 -- weighted # of milliseconds spent doing I/Os  
   */
   int   disks1;
   double   disk1[11];
   int   disks2;
   double   disk2[11];
   int   disks3;
   double   disk3[11]; 

   /* innodb stats */
   unsigned long long  innodb_data_read;
   unsigned long long  innodb_data_write;

   unsigned long long  innodb_log_waits;
   unsigned long long  innodb_log_writes;

   unsigned long long  innodb_page_free;
   unsigned long long  innodb_page_read;
   unsigned long long  innodb_page_create;
   unsigned long long  innodb_page_write;
   unsigned long long  innodb_page_flush;

   unsigned long long  innodb_rowlock_waits;
   unsigned long long  innodb_rowlock_time;

   unsigned long long  innodb_rows_read;
   unsigned long long  innodb_rows_inserted;
   unsigned long long  innodb_rows_updated;
   unsigned long long  innodb_rows_deleted;

   unsigned long long  innodb_max_trxid;
   unsigned long long  innodb_max_purge;
   unsigned long long  innodb_max_undo;
   unsigned long long  innodb_all_undo;
   unsigned long long  innodb_log_lsn;
   unsigned long long  innodb_log_flushed_lsn;
   unsigned long long  innodb_log_checkpoint_lsn;

   unsigned long long  innodb_ibuf_size;
   unsigned long long  innodb_ibuf_segsize;
   unsigned long long  innodb_ibuf_freelist;
   unsigned long long  innodb_ibuf_merges;
   unsigned long long  innodb_ibuf_mergeop;
   unsigned long long  innodb_ibuf_discardop;

   unsigned long long  mysql_conn_abort;
   unsigned long long  mysql_lock_immediate;
   unsigned long long  mysql_lock_waited;
   unsigned long long  mysql_tmpfile_created;
   unsigned long long  mysql_conn_created;
   unsigned long long  mysql_conn_count;
   unsigned long long  mysql_conn_running;
   unsigned long long  mysql_com_insert;
   unsigned long long  mysql_com_update;
   unsigned long long  mysql_com_delete;
   unsigned long long  mysql_com_select;
   unsigned long long  mysql_tcache_get;
   unsigned long long  mysql_tcache_miss;

   unsigned long long  mysql_long_query;
   unsigned long long  mysql_tmp_tables;
   unsigned long long  mysql_tmp_disktables;
   unsigned long long  mysql_sort_merge;
   unsigned long long  mysql_sort_rows;
   unsigned long long  mysql_bytes_received;
   unsigned long long  mysql_bytes_sent;

   unsigned long long  mysql_com_commit;
   unsigned long long  mysql_com_rollback;

   unsigned long       mysql_slave_count;
   unsigned long long  mysql_binlog_offset;
   unsigned long long  mysql_iolog_offset;
   unsigned long long  mysql_relaylog_offset;
} OSLoadData;

typedef  struct _OSLoadRecord 
{
   int   disk2;
   int   row;

   double load;
   int   run;
   int   blk;
   int   num;

   int   user;
   int   system;
   int   iowait;
   int   fork;
   int   irq;
   int   softirq;
   int   context;
   int   uptime;
   int   irqcall;
   
   int   pgin;
   int   pgout;
   int   swpin;
   int   swpout;

   double   free;
   double   swap;
   double   cache;
   double   lfree;
   double   hfree;
   double   pagetables;

   int   ibytes;
   int   ipackets;
   int   ierrors;
   int   obytes;
   int   opackets;
   int   oerrors;

   int   nr; 
   
   int   tcpuse;
   int   tcpalloc;
   int   tcpwait; 
   int   aopen;
   int   popen;
   int   afail;
   int   reset;
   int   isegs;
   int   osegs;
   int   rsegs;
   int   inerr;
   int   orest;

   int   disks1;
   int   readc1;
   int   rmerg1;
   int   rblks1;
   int   readt1;
   int   writc1;
   int   wmerg1;
   int   wblks1;
   int   writt1;
   int   ioact1;
   int   iotime1;

   int   disks2;
   int   readc2;
   int   rmerg2;
   int   rblks2;
   int   readt2;
   int   writc2;
   int   wmerg2;
   int   wblks2;
   int   writt2;
   int   ioact2;
   int   iotime2;

   int   disks3;
   int   readc3;
   int   rmerg3;
   int   rblks3;
   int   readt3;
   int   writc3;
   int   wmerg3;
   int   wblks3;
   int   writt3;
   int   ioact3;
   int   iotime3;

   /* innodb stats */
   unsigned long  innodb_data_read;
   unsigned long  innodb_data_write;

   unsigned long  innodb_log_waits;
   unsigned long  innodb_log_writes;

   unsigned long  innodb_page_free;
   unsigned long  innodb_page_read;
   unsigned long  innodb_page_create;
   unsigned long  innodb_page_write;
   unsigned long  innodb_page_flush;

   unsigned long  innodb_rowlock_waits;
   unsigned long  innodb_rowlock_time;

   unsigned long  innodb_rows_read;
   unsigned long  innodb_rows_inserted;
   unsigned long  innodb_rows_updated;
   unsigned long  innodb_rows_deleted;

   unsigned long  innodb_max_trxid;
   unsigned long  innodb_max_purge;
   unsigned long  innodb_max_undo;
   unsigned long  innodb_all_undo;
   unsigned long  innodb_active_trxid;
   unsigned long  innodb_active_view;
   unsigned long  innodb_rseg_history_len;

   unsigned long  innodb_log_lsn;
   unsigned long  innodb_log_flushed_lsn;
   unsigned long  innodb_log_checkpoint_lsn;
   unsigned long  innodb_log_gap;

   unsigned long  innodb_ibuf_size;
   unsigned long  innodb_ibuf_segsize;
   unsigned long  innodb_ibuf_freelist;
   unsigned long  innodb_ibuf_merges;
   unsigned long  innodb_ibuf_mergeop;
   unsigned long  innodb_ibuf_discardop;

   unsigned long  mysql_conn_abort;
   unsigned long  mysql_lock_immediate;
   unsigned long  mysql_lock_waited;
   unsigned long  mysql_tmpfile_created;
   unsigned long  mysql_conn_created;
   unsigned long  mysql_conn_count;
   unsigned long  mysql_conn_running;
   unsigned long  mysql_com_insert;
   unsigned long  mysql_com_update;
   unsigned long  mysql_com_delete;
   unsigned long  mysql_com_select;
   unsigned long  mysql_tcache_get;
   unsigned long  mysql_tcache_miss;
   unsigned long  mysql_long_query;
   unsigned long  mysql_tmp_tables;
   unsigned long  mysql_tmp_disktables;
   unsigned long  mysql_sort_merge;
   unsigned long  mysql_sort_rows;
   unsigned long  mysql_bytes_received;
   unsigned long  mysql_bytes_sent;
   unsigned long  mysql_com_commit;
   unsigned long  mysql_com_rollback;
   unsigned long  mysql_slave_count;
   unsigned long  mysql_binlog_offset;
   unsigned long  mysql_iolog_offset;
   unsigned long  mysql_relaylog_offset;
   unsigned long  mysql_apply_delay;
} OSLoadRecord;

void getOSLoadData(OSLoadData *os);
void getOSLoadRecord(OSLoadData *os1, OSLoadData *os2, OSLoadRecord *rec); 
void printLoadRecord(FILE *fp, OSLoadRecord *rec);
void writeLoadRecordBuffer(char *buf, unsigned char *hostname, OSLoadRecord *rec);
void writeLoadRecord(FILE *fp, unsigned char *hostname, OSLoadRecord *rec);
void getHostName(unsigned char *buf, int buflen);
void getMountSpaceInfo(const char *path, double *usedgb, double *freegb, unsigned long *freepct, unsigned long *inodepct);
void getDiskList(OSLoadData *os);
void switchFile(char *fname);
int  getHostMacAddress(unsigned char *hostmac);

void  initMYDB(MYDATABASE *db);
int myExecute(MYDATABASE *db, const char *sqlstring, int len);
int disconnectMYDB(MYDATABASE *db);
int LogonMYDB(MYDATABASE *db, const char *o_connstr);

#endif

