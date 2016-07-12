/*
*
*                                 Apache License
*                           Version 2.0, January 2004
*                        http://www.apache.org/licenses/
*/

#define MYSQL_SERVER 1

#include <config.h>
#include <my_global.h>
#include <sql_priv.h>
#include <stdlib.h>
#include <ctype.h>
#include <mysql_version.h>
#include <mysql/plugin.h>
#include <my_dir.h>
#include "my_pthread.h"                         // pthread_handler_t
#include "my_sys.h"                             // my_write, my_malloc
#include "m_string.h"                           // strlen
#include "sql_plugin.h"                         // st_plugin_int

#include "sql_show.h"
#include "sql_class.h"
#include "log.h"
#include "log_event.h"
#include "binlog.h"
#include "rpl_mi.h"
#include "rpl_rli.h"
#include "rpl_master.h"
#include "rpl_slave.h"

#include "osload.h"

#define UNIV_LINUX                       1
#define _GNU_SOURCE			 1
#define HAVE_IB_ATOMIC_PTHREAD_T_GCC     1
#define HAVE_IB_GCC_ATOMIC_BUILTINS      1
#define HAVE_IB_GCC_ATOMIC_BUILTINS_64   1
#define HAVE_IB_GCC_ATOMIC_BUILTINS_BYTE 1
#define HAVE_IB_GCC_SYNC_SYNCHRONISE     1
#define SIZEOF_PTHREAD_T                 8

#include "../../storage/innobase/include/univ.i"
#include "../../storage/innobase/include/srv0srv.h"
#include "../../storage/innobase/include/trx0sys.h"
#include "../../storage/innobase/include/trx0trx.h"
#include "../../storage/innobase/include/trx0rseg.h"
#include "../../storage/innobase/include/trx0purge.h"
#include "../../storage/innobase/include/log0log.h"
#include "../../storage/innobase/include/lock0lock.h"
#include "../../storage/innobase/include/ibuf0ibuf.h"

/*
  Disable __attribute__() on non-gcc compilers.
*/
#if !defined(__attribute__) && !defined(__GNUC__)
#define __attribute__(A)
#endif

static char*    oneagent_disk1 = NULL;
static char*	oneagent_disk2 = NULL;
static char*    oneagent_disk3 = NULL;
static char*    oneagent_monitordb = NULL;
static unsigned int     oneagent_interval = 1l;
static unsigned int     oneagent_maxsize  = 100l;
static unsigned int     oneagent_longtran = 3l;

double oneagent_load    = 0;
unsigned long oneagent_cpu_sys = 0;
unsigned long oneagent_cpu_wio = 0;
unsigned long oneagent_cpu_usr = 0;
unsigned long oneagent_cpu_irq = 0;
unsigned long oneagent_cpu_idl = 0;

unsigned long oneagent_run_queue = 0;
unsigned long oneagent_blk_queue = 0;
unsigned long oneagent_processes = 0;
unsigned long oneagent_context   = 0;
unsigned long oneagent_fork      = 0;
unsigned long oneagent_uptime    = 0;
unsigned long oneagent_pagein    = 0;
unsigned long oneagent_pageout   = 0;
unsigned long oneagent_swapin    = 0;
unsigned long oneagent_swapout   = 0;
unsigned long oneagent_free      = 0;
unsigned long oneagent_swap      = 0;
unsigned long oneagent_cache     = 0;
unsigned long oneagent_pagetable = 0;
unsigned long oneagent_filenr    = 0;
unsigned long oneagent_netin     = 0;
unsigned long oneagent_netout    = 0;
unsigned long oneagent_neterr    = 0;

char oneagent_disk1_name[128], oneagent_disk2_name[128], oneagent_disk3_name[128];
unsigned long oneagent_disk_readc1 = 0, oneagent_disk_readc2=0, oneagent_disk_readc3=0;
unsigned long oneagent_disk_rblks1 = 0, oneagent_disk_rblks2=0, oneagent_disk_rblks3=0;
unsigned long oneagent_disk_readt1 = 0, oneagent_disk_readt2=0, oneagent_disk_readt3=0;
unsigned long oneagent_disk_writc1 = 0, oneagent_disk_writc2=0, oneagent_disk_writc3=0;
unsigned long oneagent_disk_wblks1 = 0, oneagent_disk_wblks2=0, oneagent_disk_wblks3=0;
unsigned long oneagent_disk_writt1 = 0, oneagent_disk_writt2=0, oneagent_disk_writt3=0;
unsigned long oneagent_disk_iotim1 = 0, oneagent_disk_iotim2=0, oneagent_disk_iotim3=0;

double   oneagent_fs_root_used = 0.0, oneagent_fs_temp_used = 0.0, oneagent_fs_home_used = 0.0, oneagent_fs_data_used = 0.0, oneagent_fs_binlog_used = 0.0;
double   oneagent_fs_root_free = 0.0, oneagent_fs_temp_free = 0.0, oneagent_fs_home_free = 0.0, oneagent_fs_data_free = 0.0, oneagent_fs_binlog_free = 0.0;
unsigned long oneagent_fs_root_pct = 0, oneagent_fs_temp_pct = 0, oneagent_fs_home_pct=0, oneagent_fs_data_pct=0, oneagent_fs_binlog_pct=0;
unsigned long oneagent_fs_root_inode = 0, oneagent_fs_temp_inode = 0, oneagent_fs_home_inode=0, oneagent_fs_data_inode=0, oneagent_fs_binlog_inode=0;

/*
extern const char *mysql_home_ptr, *opt_mysql_tmpdir, *mysql_real_data_home_ptr, *log_bin_index;
extern unsigned long thread_id, aborted_threads, aborted_connects, locks_immediate,locks_waited,connection_count,num_thread_running;
*/

extern HASH  slave_list;
extern const char *log_bin_index, *glob_hostname_ptr;
STATUS_VAR com_stat_tmp;

unsigned long oneagent_tcp_use = 0, oneagent_tcp_alloc = 0, oneagent_tcp_wait = 0, oneagent_tcp_connect = 0, oneagent_tcp_accept = 0;

/* innodb stats */
unsigned long  innodb_data_read = 0;
unsigned long  innodb_data_write = 0;
unsigned long  innodb_log_waits = 0;
unsigned long  innodb_log_writes = 0;
unsigned long  innodb_page_free = 0;
unsigned long  innodb_page_read = 0;
unsigned long  innodb_page_create = 0;
unsigned long  innodb_page_write = 0;
unsigned long  innodb_page_flush = 0;
unsigned long  innodb_rowlock_waits = 0;
unsigned long  innodb_rowlock_time = 0;
unsigned long  innodb_rowlock_long = 0;
unsigned long  innodb_rowlock_sess = 0;
unsigned long  innodb_rows_read = 0;
unsigned long  innodb_rows_inserted = 0;
unsigned long  innodb_rows_updated = 0;
unsigned long  innodb_rows_deleted = 0;
unsigned long  innodb_max_trxid = 0;
unsigned long  innodb_log_lsn = 0;
unsigned long  innodb_log_flushed_lsn = 0;
unsigned long  innodb_log_checkpoint_lsn = 0;
unsigned long  innodb_active_trxid = 0;
unsigned long  innodb_rseg_history_len = 0;
unsigned long  innodb_active_view = 0;
unsigned long  innodb_max_trxtime = 0;
unsigned long  innodb_max_trxlong = 0;
unsigned long  innodb_max_purge = 0;
unsigned long  innodb_max_undo  = 0;
unsigned long  innodb_all_undo  = 0;

unsigned long  innodb_ibuf_size = 0;
unsigned long  innodb_ibuf_segsize = 0;
unsigned long  innodb_ibuf_freelist = 0;
unsigned long  innodb_ibuf_merges = 0;
unsigned long  innodb_ibuf_mergeop = 0;
unsigned long  innodb_ibuf_discardop = 0;

unsigned long  mysql_conn_abort = 0;
unsigned long  mysql_lock_immediate = 0;
unsigned long  mysql_lock_waited = 0;
unsigned long  mysql_tmpfile_created = 0;
unsigned long  mysql_conn_created = 0;
unsigned long  mysql_conn_count = 0;
unsigned long  mysql_conn_running = 0;

unsigned long  mysql_com_insert = 0;
unsigned long  mysql_com_update = 0;
unsigned long  mysql_com_delete = 0;
unsigned long  mysql_com_select = 0;
unsigned long  mysql_com_commit = 0;
unsigned long  mysql_com_rollback = 0;

unsigned long  mysql_tcache_get = 0;
unsigned long  mysql_tcache_miss = 0;

unsigned long  mysql_long_query = 0;
unsigned long  mysql_tmp_tables = 0;
unsigned long  mysql_tmp_disktables = 0;
unsigned long  mysql_sort_merge = 0;
unsigned long  mysql_sort_rows = 0;
unsigned long  mysql_bytes_received = 0;
unsigned long  mysql_bytes_sent = 0;

unsigned long  mysql_slave_count = 0;
unsigned long  mysql_binlog_offset = 0;
unsigned long  mysql_binlog_count  = 0;
unsigned long  mysql_iolog_offset  = 0;
unsigned long  mysql_relaylog_offset = 0;
unsigned long  mysql_apply_delay = 0;

extern "C" void tcc_clear_long_control();
void getJSONStatus(void *con);

struct oneagent_context
{
  pthread_t heartbeat_thread;
  pthread_t dataload_thread;
  OSLoadData      os1, os2;
  OSLoadRecord    rec;
  pthread_mutex_t   LOCK_concurrency;
  pthread_cond_t    COND_concurrency;
  FILE 	*logfile;
  int   exit_app;
  MYDATABASE monitordb;
};

inline uint get_binlog_index(const char *fname)
{
    uint lindex = 0;
    size_t len = strlen(fname);

    while(len && fname[len - 1] >= '0' && fname[len - 1] <= '9') len --;
    while(fname[len] == '0') len ++;

    lindex = atoi(fname + len - 1);

    return lindex;
}

void getInnoDBData(OSLoadData *os)
{
  int i;
  /* get innodb stats */
  srv_export_innodb_status();
  os->innodb_data_read     = export_vars.innodb_data_read;
  os->innodb_data_write    = export_vars.innodb_data_written;
  os->innodb_log_waits     = export_vars.innodb_log_waits;
  os->innodb_log_writes    = export_vars.innodb_log_writes;

  os->innodb_page_free     = export_vars.innodb_buffer_pool_wait_free;
  os->innodb_page_read     = export_vars.innodb_pages_read;
  os->innodb_page_create   = export_vars.innodb_pages_created;
  os->innodb_page_write    = export_vars.innodb_pages_written;
  os->innodb_page_flush    = export_vars.innodb_buffer_pool_pages_flushed;

  os->innodb_rowlock_waits = export_vars.innodb_row_lock_waits;
  os->innodb_rowlock_time  = export_vars.innodb_row_lock_time;
  os->innodb_rows_read     = export_vars.innodb_rows_read;
  os->innodb_rows_inserted = export_vars.innodb_rows_inserted;
  os->innodb_rows_updated  = export_vars.innodb_rows_updated;
  os->innodb_rows_deleted  = export_vars.innodb_rows_deleted;
  os->innodb_max_trxid     = trx_sys->max_trx_id;
  os->innodb_max_purge     = purge_sys->iter.trx_no;
  os->innodb_log_lsn       = log_sys->lsn;
  os->innodb_log_flushed_lsn    = log_sys->flushed_to_disk_lsn;
  os->innodb_log_checkpoint_lsn = log_sys->last_checkpoint_lsn;

  os->innodb_ibuf_size = ibuf->size;
  os->innodb_ibuf_segsize = ibuf->seg_size;
  os->innodb_ibuf_freelist = ibuf->free_list_len;
  os->innodb_ibuf_merges   = ibuf->n_merges;
  os->innodb_ibuf_mergeop = 0;
  os->innodb_ibuf_discardop = 0;
  for (i = 0; i < IBUF_OP_COUNT; i++)
  {
	os->innodb_ibuf_mergeop += ibuf->n_merged_ops[i];		
	os->innodb_ibuf_discardop += ibuf->n_discarded_ops[i];
  }
}

void getMySQLData(OSLoadData *os)
{
  os->mysql_conn_abort = aborted_threads + aborted_connects;
  os->mysql_lock_immediate = locks_immediate;
  os->mysql_lock_waited    = locks_waited;
  os->mysql_tmpfile_created = my_tmp_file_created;
  os->mysql_conn_created    = thread_id;
  os->mysql_conn_count      = connection_count;
  os->mysql_conn_running    = num_thread_running;

  calc_sum_of_all_status(&com_stat_tmp);

  os->mysql_com_select = com_stat_tmp.com_stat[(uint) SQLCOM_SELECT];
  os->mysql_com_insert = com_stat_tmp.com_stat[(uint) SQLCOM_INSERT] + 
			 com_stat_tmp.com_stat[(uint) SQLCOM_INSERT_SELECT] + 
			 com_stat_tmp.com_stat[(uint) SQLCOM_REPLACE];
  os->mysql_com_update = com_stat_tmp.com_stat[(uint) SQLCOM_UPDATE] +
			 com_stat_tmp.com_stat[(uint) SQLCOM_UPDATE_MULTI];
  os->mysql_com_delete = com_stat_tmp.com_stat[(uint) SQLCOM_DELETE] + 
			 com_stat_tmp.com_stat[(uint) SQLCOM_DELETE_MULTI];
  os->mysql_com_commit = com_stat_tmp.com_stat[(uint) SQLCOM_COMMIT];
  os->mysql_com_rollback = com_stat_tmp.com_stat[(uint) SQLCOM_ROLLBACK];

  os->mysql_tcache_get = com_stat_tmp.table_open_cache_hits;
  os->mysql_tcache_miss = com_stat_tmp.table_open_cache_misses;

  os->mysql_long_query  = com_stat_tmp.long_query_count;
  os->mysql_tmp_tables  = com_stat_tmp.created_tmp_tables;
  os->mysql_tmp_disktables = com_stat_tmp.created_tmp_disk_tables;
  os->mysql_sort_merge     = com_stat_tmp.filesort_merge_passes;
  os->mysql_sort_rows      = com_stat_tmp.filesort_rows;
  os->mysql_bytes_received = com_stat_tmp.bytes_received;
  os->mysql_bytes_sent     = com_stat_tmp.bytes_sent;
}

void getBinlogData(OSLoadData *os)
{
  LOG_INFO li, li2;
  mysql_mutex_lock(&LOCK_slave_list);
  os->mysql_slave_count = slave_list.records;
  mysql_mutex_unlock(&LOCK_slave_list);

  if (mysql_bin_log.is_open())
  {
	mysql_bin_log.get_current_log(&li);
	os->mysql_binlog_offset = get_binlog_index(li.log_file_name);
	os->mysql_binlog_offset = os->mysql_binlog_offset * max_binlog_size;
	os->mysql_binlog_offset = os->mysql_binlog_offset + li.pos;
	mysql_bin_log.find_log_pos(&li2, NULL, true);
	mysql_binlog_count      = get_binlog_index(li.log_file_name) 
			- get_binlog_index(li2.log_file_name) + 1;
	
  }
  mysql_mutex_lock(&LOCK_active_mi);
  if (active_mi != NULL)
  {
	os->mysql_iolog_offset = get_binlog_index(active_mi->get_master_log_name());
        os->mysql_iolog_offset = os->mysql_iolog_offset * max_binlog_size;
        os->mysql_iolog_offset = os->mysql_iolog_offset + active_mi->get_master_log_pos();

        os->mysql_relaylog_offset = get_binlog_index(active_mi->rli->get_group_master_log_name());
        os->mysql_relaylog_offset = os->mysql_relaylog_offset * max_binlog_size;
        os->mysql_relaylog_offset = os->mysql_relaylog_offset + active_mi->rli->get_group_master_log_pos();
  }
  mysql_mutex_unlock(&LOCK_active_mi);
}

pthread_handler_t oneagent_heartbeat(void *p)
{
  int i;
  DBUG_ENTER("mysql_heartbeat");
  struct oneagent_context *con= (struct oneagent_context *)p;
  unsigned long loop_count = 0;
  time_t nowtime = my_time(0);
  const trx_t*    trx;
  trx_list_t*     trx_list = NULL;
  time_t	  tmptime;

  /* get innodb stats */
  getInnoDBData(&con->os2);
  getMySQLData(&con->os2); 
  getBinlogData(&con->os2);

  while(con->exit_app == 0 && shutdown_in_progress == 0)
  {
    sleep(oneagent_interval);
    if (mysqld_server_started == 0) continue;

    while((nowtime = my_time(0)) < 0);

    if (oneagent_disk1)
    {
	 memset(con->os2.prefix1,0, 128);
	 snprintf(con->os2.prefix1,127, "%s", oneagent_disk1);
	 memcpy(oneagent_disk1_name, con->os2.prefix1, 128);
    }
    if (oneagent_disk2)
    {
	 memset(con->os2.prefix2,0, 128);
	 snprintf(con->os2.prefix2,127, "%s", oneagent_disk2);
	 memcpy(oneagent_disk2_name, con->os2.prefix2, 128);
    }
    if (oneagent_disk3)
    {
	 memset(con->os2.prefix3,0, 128);
	 snprintf(con->os2.prefix3,127, "%s", oneagent_disk3);
	 memcpy(oneagent_disk3_name, con->os2.prefix3, 128);
    }

    memcpy(&con->os1, &con->os2, sizeof(OSLoadData));
    getOSLoadData(&con->os2);

    /* get innodb stats */
    getInnoDBData(&con->os2);
    getMySQLData(&con->os2); 
    getBinlogData(&con->os2);
    con->rec.innodb_active_trxid       = UT_LIST_GET_LEN(trx_sys->rw_trx_list);
    con->rec.innodb_active_view        = UT_LIST_GET_LEN(trx_sys->view_list);
    con->rec.innodb_rseg_history_len   = trx_sys->rseg_history_len;

    innodb_max_undo    = 0;
    innodb_all_undo    = 0;
    innodb_max_trxtime = 0;
    innodb_max_trxlong = 0;
    innodb_rowlock_long= 0;
    innodb_rowlock_sess= 0;

    mutex_enter(&trx_sys->mutex);
    trx_list = &trx_sys->rw_trx_list;
    for (trx = UT_LIST_GET_FIRST(trx_sys->mysql_trx_list);
             trx != NULL;
             trx = UT_LIST_GET_NEXT(mysql_trx_list, trx)) {
                if (trx_state_eq(trx, TRX_STATE_ACTIVE)) {
			tmptime = trx->start_time;
			if (tmptime > 0)
			{
				if (nowtime - tmptime > innodb_max_trxtime)
					innodb_max_trxtime = nowtime - tmptime;
				if (nowtime - tmptime >= oneagent_longtran)
					innodb_max_trxlong ++;
			}

			tmptime = trx->lock.wait_started;
			if (tmptime > 0 && trx->lock.que_state == TRX_QUE_LOCK_WAIT)
			{
				innodb_rowlock_sess ++;
				if (nowtime - tmptime > innodb_rowlock_long)
					innodb_rowlock_long = nowtime - tmptime;
			}
                }
    }
    for(i=0;i<TRX_SYS_N_RSEGS;i++)
    {
	if (trx_sys->rseg_array[i] != NULL)
        {
		if (trx_sys->rseg_array[i]->curr_size > innodb_max_undo)
			innodb_max_undo = trx_sys->rseg_array[i]->curr_size;
		innodb_all_undo += trx_sys->rseg_array[i]->curr_size;
        }
    }
    mutex_exit(&trx_sys->mutex);

    tcc_clear_long_control();

    getOSLoadRecord(&con->os1, &con->os2, &con->rec);

    if ((loop_count & 0x7) == 0)
    {
	    getMountSpaceInfo("/", &oneagent_fs_root_used, &oneagent_fs_root_free, &oneagent_fs_root_pct, &oneagent_fs_root_inode);
	    getMountSpaceInfo(opt_mysql_tmpdir, &oneagent_fs_temp_used, &oneagent_fs_temp_free, &oneagent_fs_temp_pct, &oneagent_fs_temp_inode);
	    getMountSpaceInfo(mysql_home_ptr, &oneagent_fs_home_used, &oneagent_fs_home_free, &oneagent_fs_home_pct, &oneagent_fs_home_inode);
	    getMountSpaceInfo(mysql_real_data_home_ptr, &oneagent_fs_data_used, &oneagent_fs_data_free, &oneagent_fs_data_pct, &oneagent_fs_data_inode);
	    getMountSpaceInfo(log_bin_index,&oneagent_fs_binlog_used, &oneagent_fs_binlog_free, &oneagent_fs_binlog_pct, &oneagent_fs_binlog_inode);
    }

    /* export value to status variables */
    oneagent_cpu_sys = con->rec.system;
    oneagent_cpu_usr = con->rec.user;
    oneagent_cpu_wio = con->rec.iowait;
    oneagent_cpu_irq = con->rec.irq;
    if (oneagent_cpu_sys + oneagent_cpu_usr + oneagent_cpu_wio + oneagent_cpu_irq < 100)
	 oneagent_cpu_idl = 100 - (oneagent_cpu_sys + oneagent_cpu_usr + oneagent_cpu_wio + oneagent_cpu_irq);
    else
	 oneagent_cpu_idl = 0;

    oneagent_load = con->rec.load;
    oneagent_run_queue = con->rec.run;
    oneagent_blk_queue = con->rec.blk;
    oneagent_processes = con->rec.num;

    oneagent_context   = con->rec.context;
    oneagent_fork      = con->rec.fork;
    oneagent_uptime    = con->rec.uptime;
    oneagent_pagein    = con->rec.pgin;
    oneagent_pageout   = con->rec.pgout;
    oneagent_swapin    = con->rec.swpin;
    oneagent_swapout   = con->rec.swpout;

    oneagent_free      = con->rec.free;
    oneagent_swap      = con->rec.swap;
    oneagent_cache     = con->rec.cache;
    oneagent_pagetable = con->rec.pagetables;

    oneagent_filenr    = con->rec.nr;
    oneagent_netin     = con->rec.ibytes;
    oneagent_netout    = con->rec.obytes;
    oneagent_neterr    = con->rec.ierrors + con->rec.oerrors;

    oneagent_disk_readc1 = con->rec.readc1;
    oneagent_disk_rblks1 = con->rec.rblks1;
    oneagent_disk_readt1 = con->rec.readt1;
    oneagent_disk_writc1 = con->rec.writc1;
    oneagent_disk_wblks1 = con->rec.wblks1;
    oneagent_disk_writt1 = con->rec.writt1;
    oneagent_disk_iotim1 = con->rec.iotime1;

    oneagent_disk_readc2 = con->rec.readc2;
    oneagent_disk_rblks2 = con->rec.rblks2;
    oneagent_disk_readt2 = con->rec.readt2;
    oneagent_disk_writc2 = con->rec.writc2;
    oneagent_disk_wblks2 = con->rec.wblks2;
    oneagent_disk_writt2 = con->rec.writt2;
    oneagent_disk_iotim2 = con->rec.iotime2;

    oneagent_disk_readc3 = con->rec.readc3;
    oneagent_disk_rblks3 = con->rec.rblks3;
    oneagent_disk_readt3 = con->rec.readt3;
    oneagent_disk_writc3 = con->rec.writc3;
    oneagent_disk_wblks3 = con->rec.wblks3;
    oneagent_disk_writt3 = con->rec.writt3;
    oneagent_disk_iotim3 = con->rec.iotime3;

    oneagent_tcp_use     = con->rec.tcpuse;
    oneagent_tcp_alloc   = con->rec.tcpalloc;
    oneagent_tcp_wait    = con->rec.tcpwait;
    oneagent_tcp_connect = con->rec.aopen;
    oneagent_tcp_accept  = con->rec.popen;

    innodb_data_read     = con->rec.innodb_data_read;
    innodb_data_write    = con->rec.innodb_data_write;
    innodb_log_waits     = con->rec.innodb_log_waits;
    innodb_log_writes    = con->rec.innodb_log_writes;
    innodb_page_free     = con->rec.innodb_page_free;
    innodb_page_read     = con->rec.innodb_page_write;
    innodb_page_create   = con->rec.innodb_page_create;
    innodb_page_write    = con->rec.innodb_page_write;
    innodb_page_flush    = con->rec.innodb_page_flush;
    innodb_rowlock_waits = con->rec.innodb_rowlock_waits;
    innodb_rowlock_time  = con->rec.innodb_rowlock_time;
    innodb_rows_read     = con->rec.innodb_rows_read;
    innodb_rows_inserted = con->rec.innodb_rows_inserted;
    innodb_rows_updated  = con->rec.innodb_rows_updated;
    innodb_rows_deleted  = con->rec.innodb_rows_deleted;
    innodb_max_trxid     = con->rec.innodb_max_trxid;
    innodb_max_purge     = con->rec.innodb_max_purge;
    innodb_log_lsn       = con->rec.innodb_log_lsn;
    innodb_log_flushed_lsn    = con->rec.innodb_log_flushed_lsn;
    innodb_log_checkpoint_lsn = con->rec.innodb_log_checkpoint_lsn;
    innodb_active_trxid       = con->rec.innodb_active_trxid;
    innodb_rseg_history_len   = con->rec.innodb_rseg_history_len;
    innodb_active_view        = con->rec.innodb_active_view;

    innodb_ibuf_size          = con->rec.innodb_ibuf_size;
    innodb_ibuf_segsize       = con->rec.innodb_ibuf_segsize;
    innodb_ibuf_freelist      = con->rec.innodb_ibuf_freelist;
    innodb_ibuf_merges        = con->rec.innodb_ibuf_merges;
    innodb_ibuf_mergeop       = con->rec.innodb_ibuf_mergeop;
    innodb_ibuf_discardop     = con->rec.innodb_ibuf_discardop;
 
    mysql_conn_abort          = con->rec.mysql_conn_abort;
    mysql_lock_immediate      = con->rec.mysql_lock_immediate;
    mysql_lock_waited         = con->rec.mysql_lock_waited;
    mysql_tmpfile_created     = con->rec.mysql_tmpfile_created;
    mysql_conn_created        = con->rec.mysql_conn_created;
    mysql_conn_count          = con->rec.mysql_conn_count;
    mysql_conn_running        = con->rec.mysql_conn_running;
    mysql_com_insert          = con->rec.mysql_com_insert;
    mysql_com_update          = con->rec.mysql_com_update;
    mysql_com_delete          = con->rec.mysql_com_delete;
    mysql_com_select          = con->rec.mysql_com_select;
    mysql_com_commit          = con->rec.mysql_com_commit;
    mysql_com_rollback        = con->rec.mysql_com_rollback;

    mysql_tcache_get	      = con->rec.mysql_tcache_get;
    mysql_tcache_miss         = con->rec.mysql_tcache_miss;

    mysql_long_query = con->rec.mysql_long_query;
    mysql_tmp_tables = con->rec.mysql_tmp_tables;
    mysql_tmp_disktables = con->rec.mysql_tmp_disktables;
    mysql_sort_merge = con->rec.mysql_sort_merge;
    mysql_sort_rows = con->rec.mysql_sort_rows;
    mysql_bytes_received = con->rec.mysql_bytes_received;
    mysql_bytes_sent = con->rec.mysql_bytes_sent;

    mysql_slave_count = con->rec.mysql_slave_count;
    mysql_binlog_offset = con->rec.mysql_binlog_offset;
    mysql_iolog_offset = con->rec.mysql_iolog_offset;
    mysql_relaylog_offset = con->rec.mysql_relaylog_offset;
    mysql_apply_delay     = con->rec.mysql_apply_delay;

    pthread_mutex_lock(&con->LOCK_concurrency);
    pthread_cond_signal(&con->COND_concurrency);
    pthread_mutex_unlock(&con->LOCK_concurrency);

    loop_count ++;
  }

  pthread_exit(0);
  DBUG_RETURN(0);
}

pthread_handler_t oneagent_dataload(void *p)
{
  struct oneagent_context *con= (struct oneagent_context *)p;

  if (oneagent_monitordb != NULL)
  {
	disconnectMYDB(&con->monitordb);
        LogonMYDB(&con->monitordb,oneagent_monitordb);
  }

  while(con->exit_app == 0 && shutdown_in_progress == 0)
  {
      pthread_mutex_lock(&con->LOCK_concurrency);
      pthread_cond_wait(&con->COND_concurrency, &con->LOCK_concurrency);
      pthread_mutex_unlock(&con->LOCK_concurrency);

      if (mysqld_server_started)
      {
          getJSONStatus(con);
      }
  }

  disconnectMYDB(&con->monitordb);
  pthread_exit(0);
  DBUG_RETURN(0);
}

/*
  Initialize the daemon example at server start or plugin installation.

  SYNOPSIS
    oneagent_plugin_init()

  DESCRIPTION
    Starts up heartbeatbeat thread

  RETURN VALUE
    0                    success
    1                    failure (cannot happen)
*/

static int oneagent_plugin_init(void *p)
{

  DBUG_ENTER("oneagent_plugin_init");
  struct oneagent_context *con;
  pthread_attr_t attr;          /* Thread attributes */

  struct st_plugin_int *plugin= (struct st_plugin_int *)p;

  con= (struct oneagent_context *) my_malloc(sizeof(struct oneagent_context), MYF(0)); 
  con->exit_app = 0;
  memset(&con->os2,0,sizeof(con->os2));
  memset(&con->monitordb, 0, sizeof(MYDATABASE));

  memset(oneagent_disk1_name, 0, 128);
  memset(oneagent_disk2_name, 0, 128);
  memset(oneagent_disk3_name, 0, 128);

  getDiskList(&con->os2);
  snprintf(oneagent_disk1_name, 127, "%s", con->os2.prefix1);
  snprintf(oneagent_disk2_name, 127, "%s", con->os2.prefix2);
  snprintf(oneagent_disk3_name, 127, "%s", con->os2.prefix3);

  getOSLoadData(&con->os2);

  if (oneagent_disk1) snprintf(con->os2.prefix1, 127, "%s", oneagent_disk1);
  if (oneagent_disk2) snprintf(con->os2.prefix2, 127, "%s", oneagent_disk2);
  if (oneagent_disk3) snprintf(con->os2.prefix3, 127, "%s", oneagent_disk3);

  pthread_mutex_init(&con->LOCK_concurrency, NULL); 
  pthread_cond_init(&con->COND_concurrency, NULL);

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* now create the thread */
  if (pthread_create(&con->heartbeat_thread, &attr, oneagent_heartbeat, (void *)con) != 0)
  {
    fprintf(stderr,"Could not create oneagent thread!\n");
    exit(0);
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (pthread_create(&con->dataload_thread, &attr, oneagent_dataload, (void *)con) != 0)
  {
    fprintf(stderr,"Could not create oneagent thread!\n");
    exit(0);
  }

  con->logfile = fopen("ONEAGENT_JSON.log", "a+");
  plugin->data= (void *)con;

  DBUG_RETURN(0);
}


/*
  Terminate the daemon example at server shutdown or plugin deinstallation.

  SYNOPSIS
    oneagent_plugin_deinit()
    Does nothing.

  RETURN VALUE
    0                    success
    1                    failure (cannot happen)

*/

static int oneagent_plugin_deinit(void *p)
{
  DBUG_ENTER("oneagent_plugin_deinit");
  struct st_plugin_int *plugin= (struct st_plugin_int *)p;
  struct oneagent_context *con=
    (struct oneagent_context *)plugin->data;
  void *dummy_retval;

  con->exit_app = 1;
  /*
    Need to wait for the hearbeat thread to terminate before closing
    the file it writes to and freeing the memory it uses
  */
  pthread_join(con->heartbeat_thread, &dummy_retval);

  con->exit_app = 1;
  pthread_mutex_lock(&con->LOCK_concurrency);
  pthread_cond_signal(&con->COND_concurrency);
  pthread_mutex_unlock(&con->LOCK_concurrency);
  pthread_join(con->dataload_thread, &dummy_retval);

  pthread_mutex_destroy(&con->LOCK_concurrency);
  pthread_cond_destroy(&con->COND_concurrency);
  if (con->logfile) fclose(con->logfile);

  if (con->monitordb.conn)
	disconnectMYDB(&con->monitordb);

  my_free(con);

  DBUG_RETURN(0);
}


/*
  Plugin system variables.
*/
static MYSQL_SYSVAR_STR(disk1, oneagent_disk1,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "The prefix for the first disk group.",
  NULL, NULL, NULL);

static MYSQL_SYSVAR_STR(disk2, oneagent_disk2,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "The prefix for the first disk group.",
  NULL, NULL, NULL);

static MYSQL_SYSVAR_STR(disk3, oneagent_disk3,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "The prefix for the first disk group.",
  NULL, NULL, NULL);

static MYSQL_SYSVAR_STR(monitordb, oneagent_monitordb,
  PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
  "The monitor db for the perf data (user/passwd@host:port:sid).",
  NULL, NULL, NULL);

static MYSQL_SYSVAR_UINT(interval, oneagent_interval,
  PLUGIN_VAR_RQCMDARG,
  "The time interval for load data capture",
  NULL, NULL, 1L, 1L, 10L, 0);

static MYSQL_SYSVAR_UINT(longtran, oneagent_longtran,
  PLUGIN_VAR_RQCMDARG,
  "The time period for long transction",
  NULL, NULL, 3L, 1L, 1800L, 0);

static MYSQL_SYSVAR_UINT(maxsize, oneagent_maxsize,
  PLUGIN_VAR_RQCMDARG,
  "The maximum log file size (MB) for hitory data",
  NULL, NULL, 100L, 20L, 2000L, 0);

static struct st_mysql_sys_var* oneagent_system_variables[]= {
  MYSQL_SYSVAR(disk1),
  MYSQL_SYSVAR(disk2),
  MYSQL_SYSVAR(disk3),
  MYSQL_SYSVAR(monitordb),
  MYSQL_SYSVAR(interval),
  MYSQL_SYSVAR(longtran),
  MYSQL_SYSVAR(maxsize),
  NULL
};

/* plugin status variables */
static SHOW_VAR oneagent_status_vars[]= {
  {"oneagent_os_sys",    (char*) &oneagent_cpu_sys,    SHOW_LONG},
  {"oneagent_os_usr",    (char*) &oneagent_cpu_usr,    SHOW_LONG},
  {"oneagent_os_wio",    (char*) &oneagent_cpu_wio,    SHOW_LONG},
  {"oneagent_os_irq",    (char*) &oneagent_cpu_irq,    SHOW_LONG},
  {"oneagent_os_idl",    (char*) &oneagent_cpu_idl,    SHOW_LONG},
  {"oneagent_os_load",   (char*) &oneagent_load,    SHOW_DOUBLE},
  {"oneagent_os_runq",   (char*) &oneagent_run_queue,    SHOW_LONG},
  {"oneagent_os_blkq",   (char*) &oneagent_blk_queue,    SHOW_LONG},
  {"oneagent_os_proc",   (char*) &oneagent_processes,    SHOW_LONG},
  {"oneagent_os_context",(char*) &oneagent_context,    SHOW_LONG},
  {"oneagent_os_fork",   (char*) &oneagent_fork,    SHOW_LONG},
  {"oneagent_os_uptime", (char*) &oneagent_uptime,    SHOW_LONG},
  {"oneagent_os_pgin",   (char*) &oneagent_pagein,    SHOW_LONG},
  {"oneagent_os_pgout",  (char*) &oneagent_pageout,    SHOW_LONG},
  {"oneagent_os_swpin",  (char*) &oneagent_swapin,    SHOW_LONG},
  {"oneagent_os_swpout", (char*) &oneagent_swapout,    SHOW_LONG},
  {"oneagent_os_free",   (char*) &oneagent_free,    SHOW_LONG},
  {"oneagent_os_swap",   (char*) &oneagent_swap,    SHOW_LONG},
  {"oneagent_os_cache",  (char*) &oneagent_cache,    SHOW_LONG},
  {"oneagent_os_ptable", (char*) &oneagent_pagetable,    SHOW_LONG},
  {"oneagent_os_filenr", (char*) &oneagent_filenr,    SHOW_LONG},
  {"oneagent_os_ibytes", (char*) &oneagent_netin,    SHOW_LONG},
  {"oneagent_os_obytes", (char*) &oneagent_netout,    SHOW_LONG},
  {"oneagent_os_nerr",   (char*) &oneagent_neterr,    SHOW_LONG},

  {"oneagent_d1",     (char*) &oneagent_disk1_name, SHOW_CHAR},
  {"oneagent_d1read", (char*) &oneagent_disk_readc1, SHOW_LONG},
  {"oneagent_d1rblk", (char*) &oneagent_disk_rblks1, SHOW_LONG},
  {"oneagent_d1rtim", (char*) &oneagent_disk_readt1, SHOW_LONG},
  {"oneagent_d1writ", (char*) &oneagent_disk_writc1, SHOW_LONG},
  {"oneagent_d1wblk", (char*) &oneagent_disk_wblks1, SHOW_LONG},
  {"oneagent_d1wtim", (char*) &oneagent_disk_writt1, SHOW_LONG},
  {"oneagent_d1busy", (char*) &oneagent_disk_iotim1, SHOW_LONG},

  {"oneagent_d2",     (char*) &oneagent_disk2_name, SHOW_CHAR},
  {"oneagent_d2read", (char*) &oneagent_disk_readc2, SHOW_LONG},
  {"oneagent_d2rblk", (char*) &oneagent_disk_rblks2, SHOW_LONG},
  {"oneagent_d2rtim", (char*) &oneagent_disk_readt2, SHOW_LONG},
  {"oneagent_d2writ", (char*) &oneagent_disk_writc2, SHOW_LONG},
  {"oneagent_d2wblk", (char*) &oneagent_disk_wblks2, SHOW_LONG},
  {"oneagent_d2wtim", (char*) &oneagent_disk_writt2, SHOW_LONG},
  {"oneagent_d2busy", (char*) &oneagent_disk_iotim2, SHOW_LONG},

  {"oneagent_d3",     (char*) &oneagent_disk3_name, SHOW_CHAR},
  {"oneagent_d3read", (char*) &oneagent_disk_readc3, SHOW_LONG},
  {"oneagent_d3rblk", (char*) &oneagent_disk_rblks3, SHOW_LONG},
  {"oneagent_d3rtim", (char*) &oneagent_disk_readt3, SHOW_LONG},
  {"oneagent_d3writ", (char*) &oneagent_disk_writc3, SHOW_LONG},
  {"oneagent_d3wblk", (char*) &oneagent_disk_wblks3, SHOW_LONG},
  {"oneagent_d3wtim", (char*) &oneagent_disk_writt3, SHOW_LONG},
  {"oneagent_d3busy", (char*) &oneagent_disk_iotim3, SHOW_LONG},

  {"oneagent_fs_rootused",     (char*) &oneagent_fs_root_used, SHOW_DOUBLE},
  {"oneagent_fs_rootfree",     (char*) &oneagent_fs_root_free, SHOW_DOUBLE},
  {"oneagent_fs_rootpct",      (char*) &oneagent_fs_root_pct, SHOW_LONG},
  {"oneagent_fs_rootinode",    (char*) &oneagent_fs_root_inode, SHOW_LONG},

  {"oneagent_fs_tmpused",      (char*) &oneagent_fs_temp_used, SHOW_DOUBLE},
  {"oneagent_fs_tmpfree",      (char*) &oneagent_fs_temp_free, SHOW_DOUBLE},
  {"oneagent_fs_tmppct",       (char*) &oneagent_fs_temp_pct, SHOW_LONG},
  {"oneagent_fs_tmpinode",     (char*) &oneagent_fs_temp_inode, SHOW_LONG},

  {"oneagent_fs_homeused",     (char*) &oneagent_fs_home_used, SHOW_DOUBLE},
  {"oneagent_fs_homefree",     (char*) &oneagent_fs_home_free, SHOW_DOUBLE},
  {"oneagent_fs_homepct",      (char*) &oneagent_fs_home_pct, SHOW_LONG},
  {"oneagent_fs_homeinode",    (char*) &oneagent_fs_home_inode, SHOW_LONG},

  {"oneagent_fs_dataused",     (char*) &oneagent_fs_data_used, SHOW_DOUBLE},
  {"oneagent_fs_datafree",     (char*) &oneagent_fs_data_free, SHOW_DOUBLE},
  {"oneagent_fs_datapct",      (char*) &oneagent_fs_data_pct, SHOW_LONG},
  {"oneagent_fs_datainode",    (char*) &oneagent_fs_data_inode, SHOW_LONG},

  {"oneagent_fs_blogused",     (char*) &oneagent_fs_binlog_used, SHOW_DOUBLE},
  {"oneagent_fs_blogfree",     (char*) &oneagent_fs_binlog_free, SHOW_DOUBLE},
  {"oneagent_fs_blogpct",      (char*) &oneagent_fs_binlog_pct, SHOW_LONG},
  {"oneagent_fs_bloginode",    (char*) &oneagent_fs_binlog_inode, SHOW_LONG},

  {"oneagent_os_tcpuse",       (char*) &oneagent_tcp_use, SHOW_LONG},
  {"oneagent_os_tcpalloc",     (char*) &oneagent_tcp_alloc, SHOW_LONG},
  {"oneagent_os_tcpwait",      (char*) &oneagent_tcp_wait, SHOW_LONG},
  {"oneagent_os_tcpconn",      (char*) &oneagent_tcp_connect, SHOW_LONG},
  {"oneagent_os_tcpacpt",      (char*) &oneagent_tcp_accept, SHOW_LONG},

  {"oneagent_idb_dataread",      (char*) &innodb_data_read,     SHOW_LONG},
  {"oneagent_idb_datawrite",     (char*) &innodb_data_write,    SHOW_LONG},
  {"oneagent_idb_pagewait",      (char*) &innodb_page_free,     SHOW_LONG},
  {"oneagent_idb_pageflush",     (char*) &innodb_page_flush,    SHOW_LONG},
  {"oneagent_idb_pageread",      (char*) &innodb_page_read,     SHOW_LONG},
  {"oneagent_idb_pagecreate",    (char*) &innodb_page_create,   SHOW_LONG},
  {"oneagent_idb_pagewrite",     (char*) &innodb_page_write,    SHOW_LONG},
  {"oneagent_idb_lockwait",      (char*) &innodb_rowlock_waits, SHOW_LONG},
  {"oneagent_idb_lockelap",      (char*) &innodb_rowlock_time,  SHOW_LONG},
  {"oneagent_idb_locksess",      (char*) &innodb_rowlock_sess,  SHOW_LONG},
  {"oneagent_idb_locktime",      (char*) &innodb_rowlock_long,  SHOW_LONG},
  {"oneagent_idb_rowsel",        (char*) &innodb_rows_read,     SHOW_LONG},
  {"oneagent_idb_rowins",        (char*) &innodb_rows_inserted, SHOW_LONG},
  {"oneagent_idb_rowupd",        (char*) &innodb_rows_updated,  SHOW_LONG},
  {"oneagent_idb_rowdel",        (char*) &innodb_rows_deleted,  SHOW_LONG},
  {"oneagent_idb_trx",           (char*) &innodb_max_trxid,     SHOW_LONG},
  {"oneagent_idb_trxpurge",      (char*) &innodb_max_purge,     SHOW_LONG},
  {"oneagent_idb_undomax",       (char*) &innodb_max_undo,     SHOW_LONG},
  {"oneagent_idb_undoall",       (char*) &innodb_all_undo,     SHOW_LONG},
  {"oneagent_idb_trxlist",       (char*) &innodb_active_trxid,  SHOW_LONG},
  {"oneagent_idb_trxview",       (char*) &innodb_active_view,   SHOW_LONG},
  {"oneagent_idb_trxrseg",       (char*) &innodb_rseg_history_len,  SHOW_LONG},
  {"oneagent_idb_trxtime",       (char*) &innodb_max_trxtime,  SHOW_LONG},
  {"oneagent_idb_trxlong",       (char*) &innodb_max_trxlong,  SHOW_LONG},
  {"oneagent_idb_log",           (char*) &innodb_log_lsn,       SHOW_LONG},
  {"oneagent_idb_logflush",      (char*) &innodb_log_flushed_lsn, SHOW_LONG},
  {"oneagent_idb_logckpt",       (char*) &innodb_log_checkpoint_lsn, SHOW_LONG},  
  {"oneagent_idb_logwait",       (char*) &innodb_log_waits,     SHOW_LONG},
  {"oneagent_idb_logwrite",      (char*) &innodb_log_writes,    SHOW_LONG},


  {"oneagent_idb_ib",            (char*) &innodb_ibuf_size,    SHOW_LONG},
  {"oneagent_idb_ibseg",         (char*) &innodb_ibuf_segsize,    SHOW_LONG},
  {"oneagent_idb_ibfree",        (char*) &innodb_ibuf_freelist,    SHOW_LONG},
  {"oneagent_idb_ibmerg",        (char*) &innodb_ibuf_merges,    SHOW_LONG},
  {"oneagent_idb_ibmrgop",       (char*) &innodb_ibuf_mergeop,    SHOW_LONG},
  {"oneagent_idb_ibdisop",       (char*) &innodb_ibuf_discardop,    SHOW_LONG},

  {"oneagent_srv_abort",         (char*) &mysql_conn_abort,        SHOW_LONG},
  {"oneagent_srv_lockget",       (char*) &mysql_lock_immediate,    SHOW_LONG},
  {"oneagent_srv_lockmiss",      (char*) &mysql_lock_waited,       SHOW_LONG},
  {"oneagent_srv_tmpfile",       (char*) &mysql_tmpfile_created,   SHOW_LONG},
  {"oneagent_srv_login",         (char*) &mysql_conn_created,      SHOW_LONG},
  {"oneagent_srv_session",       (char*) &mysql_conn_count,        SHOW_LONG},
  {"oneagent_srv_active",        (char*) &mysql_conn_running,      SHOW_LONG},
  {"oneagent_srv_sqlins",        (char*) &mysql_com_insert,      SHOW_LONG},
  {"oneagent_srv_sqlupd",        (char*) &mysql_com_update,      SHOW_LONG},
  {"oneagent_srv_sqldel",        (char*) &mysql_com_delete,      SHOW_LONG},
  {"oneagent_srv_sqlsel",        (char*) &mysql_com_select,      SHOW_LONG},
  {"oneagent_srv_commit",        (char*) &mysql_com_commit,      SHOW_LONG},
  {"oneagent_srv_rollback",      (char*) &mysql_com_rollback,    SHOW_LONG},
  {"oneagent_srv_tcget",         (char*) &mysql_tcache_get,      SHOW_LONG},
  {"oneagent_srv_tcmiss",        (char*) &mysql_tcache_miss,     SHOW_LONG},
  {"oneagent_srv_longqry",       (char*) &mysql_long_query,     SHOW_LONG},
  {"oneagent_srv_tmptab",        (char*) &mysql_tmp_tables,     SHOW_LONG},
  {"oneagent_srv_tmpdisk",       (char*) &mysql_tmp_disktables,     SHOW_LONG},
  {"oneagent_srv_sortmerge",     (char*) &mysql_sort_merge,     SHOW_LONG},
  {"oneagent_srv_sortrows",      (char*) &mysql_sort_rows,     SHOW_LONG},
  {"oneagent_srv_netin",         (char*) &mysql_bytes_received,     SHOW_LONG},
  {"oneagent_srv_netout",        (char*) &mysql_bytes_sent,     SHOW_LONG},
  {"oneagent_srv_binslave",      (char*) &mysql_slave_count,     SHOW_LONG},
  {"oneagent_srv_binlog",        (char*) &mysql_binlog_offset,    SHOW_LONG},
  {"oneagent_srv_binfiles",      (char*) &mysql_binlog_count,     SHOW_LONG},
  {"oneagent_srv_bindump",       (char*) &mysql_iolog_offset,    SHOW_LONG},
  {"oneagent_srv_binapply",      (char*) &mysql_relaylog_offset,    SHOW_LONG},
  {"oneagent_srv_bindelay",      (char*) &mysql_apply_delay,    SHOW_LONG},

  {NULL, NULL, SHOW_BOOL},
};

void getJSONStatus(void *p)
{
   unsigned int i, pos = 0;
   SHOW_VAR *var;
   char jsonbuf[8192];
   time_t now = my_time(0);
   struct tm *ptm = localtime(&now);
   struct oneagent_context *con= (struct oneagent_context *)p;

   pos = pos + sprintf(jsonbuf + pos, "REPLACE INTO oneagent_perfdata (host, port, uuid, day, data) VALUES (");
   pos = pos + sprintf(jsonbuf + pos, "'%s',%d,'%s','%04d-%02d-%02d %02d:%02d:%02d','", 
		glob_hostname, mysqld_port, server_uuid_ptr, 
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
   pos = pos + sprintf(jsonbuf + pos, "{");
   for(i = 0; i < sizeof(oneagent_status_vars) / sizeof(oneagent_status_vars[0]); i++)
   {
	var = &oneagent_status_vars[i];
        if (var->name != NULL)
        {
		if (i) pos = pos + sprintf(jsonbuf + pos, ",");
		pos = pos + sprintf(jsonbuf + pos, "\"%s\":", var->name + 9); 
		if (var->type == SHOW_LONG)
			pos = pos + sprintf(jsonbuf + pos,"%lu", *((ulong *)(var->value)));
		else if (var->type == SHOW_CHAR)
			pos = pos + sprintf(jsonbuf + pos,"\"%s\"", (char *)(var->value));
		else if (var->type == SHOW_DOUBLE)
			pos = pos + sprintf(jsonbuf + pos,"%.2f", *((double *)(var->value)));
	}
   }
   pos = pos + sprintf(jsonbuf + pos, "}')");
   jsonbuf[pos++] = ';';
   jsonbuf[pos++] = '\n';
   jsonbuf[pos] = 0;

   if (con->logfile)
   {
	fwrite(jsonbuf, 1, pos, con->logfile);
	fflush(con->logfile);
	if (ftell(con->logfile) > oneagent_maxsize * 1048576)
	{
		fclose(con->logfile);
		con->logfile = NULL;
		switchFile((char *)("ONEAGENT_JSON.log"));
		con->logfile = fopen("ONEAGENT_JSON.log", "a+");
	}
   }

   if (oneagent_monitordb != NULL)
   {
	if(myExecute(&con->monitordb, jsonbuf, pos))
	{
		disconnectMYDB(&con->monitordb);
		LogonMYDB(&con->monitordb, oneagent_monitordb);
		myExecute(&con->monitordb, jsonbuf, pos);
	}
   }
}

struct st_mysql_daemon oneagent_plugin=
{ MYSQL_DAEMON_INTERFACE_VERSION  };

/*
  Plugin library descriptor
*/

mysql_declare_plugin(oneagent)
{
  MYSQL_DAEMON_PLUGIN,
  &oneagent_plugin,
  "OneAgent",
  "Fangxin Lou (http://www.onexsoft.com)",
  "OneAgent, mapping OS load information to status varirables",
  PLUGIN_LICENSE_GPL,
  oneagent_plugin_init, /* Plugin Init */
  oneagent_plugin_deinit, /* Plugin Deinit */
  0x0100 /* 1.0 */,
  oneagent_status_vars,        		/* status variables                */
  oneagent_system_variables,           /* system variables                */
  NULL,                       		/* config options                  */
  0,                          		/* flags                           */
}
mysql_declare_plugin_end;
