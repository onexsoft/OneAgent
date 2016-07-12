# OneAgent

A MySQL plugin to export linux performance metrics as status variables, including CPU/network/memory/disk space/disk IO, also some metrics of InnoDB internal data structure.

The plugin cannot be compiled as dynamic plugin libray, it can only be compiled as a static linked plugin. Please put the code into $MYSQL_SRC/plugin/oneagent , and then run cmake to compile the MySQL from the source code.

- Linux OS performance: http://www.onexsoft.com/zh/oneagent-linux-load-plugin.html
- MySQL Space Usage: http://www.onexsoft.com/zh/oneagent-server-space-usage.html
- InnoDB Perf Metrics: http://www.onexsoft.com/zh/oneagent-innodb-key-metrics.html

# Download OneSQL (prebuilt with OneAgent plugin)

Visit http://www.onexsoft.com/?page_id=3391 to get the prebuilt RPM package with OneAgent plugin.

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

# Contact

Please submit a issue or send mail to flou(@)onexsoft.com
