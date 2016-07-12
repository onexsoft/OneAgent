# OneAgent

A MySQL plugin to export linux performance metrics as status variables, including CPU/network/memory/disk space/disk IO, also some metrics of InnoDB internal data structure.

The plugin cannot be compiled as dynamic plugin libray, it can only be compiled as a static linked plugin. Please put the code into $MYSQL_SRC/plugin/oneagent , and then run cmake to compile the MySQL from the source code.

- Linux OS performance: http://www.onexsoft.com/zh/oneagent-linux-load-plugin.html
- MySQL Space Usage: http://www.onexsoft.com/zh/oneagent-server-space-usage.html
- InnoDB Perf Metrics: http://www.onexsoft.com/zh/oneagent-innodb-key-metrics.html

# View Linux OS performance metrics

    mysql> show status like 'oneagent_os%';
    +----------------------+----------+
    | Variable_name        | Value    |
    +----------------------+----------+
    | oneagent_os_blkq     | 2        |
    | oneagent_os_cache    | 884      |
    | oneagent_os_context  | 8749     |
    | oneagent_os_filenr   | 0        |
    | oneagent_os_fork     | 3        |
    | oneagent_os_free     | 128      |
    | oneagent_os_ibytes   | 556      |
    | oneagent_os_idl      | 54       |
    | oneagent_os_irq      | 2        |
    | oneagent_os_load     | 1.210000 |
    | oneagent_os_nerr     | 0        |
    | oneagent_os_obytes   | 342      |
    | oneagent_os_pgin     | 0        |
    | oneagent_os_pgout    | 1048     |
    | oneagent_os_proc     | 186      |
    | oneagent_os_ptable   | 4        |
    | oneagent_os_runq     | 2        |
    | oneagent_os_swap     | 0        |
    | oneagent_os_swpin    | 0        |
    | oneagent_os_swpout   | 0        |
    | oneagent_os_sys      | 7        |
    | oneagent_os_tcpacpt  | 2        |
    | oneagent_os_tcpalloc | 44       |
    | oneagent_os_tcpconn  | 2        |
    | oneagent_os_tcpuse   | 43       |
    | oneagent_os_tcpwait  | 12       |
    | oneagent_os_uptime   | 9019     |
    | oneagent_os_usr      | 16       |
    | oneagent_os_wio      | 21       |
    +----------------------+----------+
    29 rows in set (0.00 sec)

# View Disk IO Statistics

    mysql> show status like 'oneagent_d1%';
    +-----------------+-------+
    | Variable_name   | Value |
    +-----------------+-------+
    | oneagent_d1     | xvda  |
    | oneagent_d1busy | 58    |
    | oneagent_d1rblk | 0     |
    | oneagent_d1read | 0     |
    | oneagent_d1rtim | 0     |
    | oneagent_d1wblk | 568   |
    | oneagent_d1writ | 71    |
    | oneagent_d1wtim | 967   |
    +-----------------+-------+
    8 rows in set (0.00 sec)

# View MySQL File System Space Information

    mysql> show status like 'oneagent_fs%';
    +-----------------------+-----------+
    | Variable_name         | Value     |
    +-----------------------+-----------+
    | oneagent_fs_blogfree  | 0.000000  |
    | oneagent_fs_bloginode | 0         |
    | oneagent_fs_blogpct   | 0         |
    | oneagent_fs_blogused  | 0.000000  |
    | oneagent_fs_datafree  | 11.982000 |
    | oneagent_fs_datainode | 92        |
    | oneagent_fs_datapct   | 59        |
    | oneagent_fs_dataused  | 8.174000  |
    | oneagent_fs_homefree  | 11.982000 |
    | oneagent_fs_homeinode | 92        |
    | oneagent_fs_homepct   | 59        |
    | oneagent_fs_homeused  | 8.174000  |
    | oneagent_fs_rootfree  | 11.982000 |
    | oneagent_fs_rootinode | 92        |
    | oneagent_fs_rootpct   | 59        |
    | oneagent_fs_rootused  | 8.174000  |
    | oneagent_fs_tmpfree   | 11.982000 |
    | oneagent_fs_tmpinode  | 92        |
    | oneagent_fs_tmppct    | 59        |
    | oneagent_fs_tmpused   | 8.174000  |
    +-----------------------+-----------+
    20 rows in set (0.00 sec)

# View MySQL Server Layer Perf Metrics

    mysql> show status like 'oneagent_srv%';
    +------------------------+---------+
    | Variable_name          | Value   |
    +------------------------+---------+
    | oneagent_srv_abort     | 0       |
    | oneagent_srv_active    | 1       |
    | oneagent_srv_binapply  | 0       |
    | oneagent_srv_bindelay  | 0       |
    | oneagent_srv_bindump   | 0       |
    | oneagent_srv_binfiles  | 0       |
    | oneagent_srv_binlog    | 0       |
    | oneagent_srv_binslave  | 0       |
    | oneagent_srv_commit    | 132     |
    | oneagent_srv_lockget   | 2507    |
    | oneagent_srv_lockmiss  | 0       |
    | oneagent_srv_login     | 0       |
    | oneagent_srv_longqry   | 0       |
    | oneagent_srv_netin     | 145329  |
    | oneagent_srv_netout    | 1945148 |
    | oneagent_srv_rollback  | 0       |
    | oneagent_srv_session   | 15      |
    | oneagent_srv_sortmerge | 0       |
    | oneagent_srv_sortrows  | 18827   |
    | oneagent_srv_sqldel    | 132     |
    | oneagent_srv_sqlins    | 133     |
    | oneagent_srv_sqlsel    | 1848    |
    | oneagent_srv_sqlupd    | 394     |
    | oneagent_srv_tcget     | 2507    |
    | oneagent_srv_tcmiss    | 0       |
    | oneagent_srv_tmpdisk   | 0       |
    | oneagent_srv_tmpfile   | 0       |
    | oneagent_srv_tmptab    | 134     |
    +------------------------+---------+
    28 rows in set (0.00 sec)

# View MySQL InnoDB Perf Metrics

    mysql> show status like 'oneagent_idb%';
    +-------------------------+---------+
    | Variable_name           | Value   |
    +-------------------------+---------+
    | oneagent_idb_dataread   | 0       |
    | oneagent_idb_datawrite  | 232960  |
    | oneagent_idb_ib         | 1       |
    | oneagent_idb_ibdisop    | 0       |
    | oneagent_idb_ibfree     | 0       |
    | oneagent_idb_ibmerg     | 0       |
    | oneagent_idb_ibmrgop    | 0       |
    | oneagent_idb_ibseg      | 2       |
    | oneagent_idb_lockelap   | 0       |
    | oneagent_idb_locksess   | 0       |
    | oneagent_idb_locktime   | 0       |
    | oneagent_idb_lockwait   | 0       |
    | oneagent_idb_log        | 2308    |
    | oneagent_idb_logckpt    | 5570395 |
    | oneagent_idb_logflush   | 0       |
    | oneagent_idb_logwait    | 0       |
    | oneagent_idb_logwrite   | 1       |
    | oneagent_idb_pagecreate | 0       |
    | oneagent_idb_pageflush  | 7       |
    | oneagent_idb_pageread   | 7       |
    | oneagent_idb_pagewait   | 0       |
    | oneagent_idb_pagewrite  | 7       |
    | oneagent_idb_rowdel     | 0       |
    | oneagent_idb_rowins     | 1       |
    | oneagent_idb_rowsel     | 0       |
    | oneagent_idb_rowupd     | 0       |
    | oneagent_idb_trx        | 1       |
    | oneagent_idb_trxlist    | 0       |
    | oneagent_idb_trxlong    | 0       |
    | oneagent_idb_trxpurge   | 0       |
    | oneagent_idb_trxrseg    | 1682    |
    | oneagent_idb_trxtime    | 0       |
    | oneagent_idb_trxview    | 0       |
    | oneagent_idb_undoall    | 377     |
    | oneagent_idb_undomax    | 3       |
    +-------------------------+---------+
    35 rows in set (0.00 sec)

# Contact

Please submit a issue or send mail to flou(@)onexsoft.com
