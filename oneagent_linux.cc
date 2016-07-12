/*
*
*                                 Apache License
*                           Version 2.0, January 2004
*                        http://www.apache.org/licenses/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/statvfs.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>
#else
#include <winsock2.h>
#include <io.h>
#define ioctl ioctlsocket
#endif

#include "osload.h"

#if defined(_WIN32)
#define STRNCASECMP memicmp
#else
#define STRNCASECMP strncasecmp
#endif

double my_app_atof(const char *ptr)
{
    int    len = strlen(ptr);
   
    len = len - 12; 
    if (len <= 0)
        return atof(ptr);
    else
        return atof(ptr + len);
}
int my_app_atoi(const char *ptr)
{
    int    len = strlen(ptr);

    len = len - 12;
    if (len <= 0)
        return atoi(ptr);
    else
        return atoi(ptr + len);
}

void getLoadAvg(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[128], *ptr=0, *endp=0;

  if ((fp=fopen("/proc/loadavg","r"))!=NULL)
  {
      memset(tmpbuf,0,128);
      fgets(tmpbuf,127,fp);
      ptr = tmpbuf;
      ptr = strtok_r(ptr," ", &endp);
      os->load1m = my_app_atof(ptr);
      ptr = strtok_r(NULL," ",&endp);
      os->load5m = my_app_atof(ptr);
      ptr = strtok_r(NULL," ",&endp);
      os->load15m= my_app_atof(ptr);
      ptr = strtok_r(NULL," ",&endp);
      os->procrun = my_app_atoi(ptr);
      while(*ptr != '/' && *ptr != '\0') ptr ++;
      if (*ptr == '/') ptr ++ ;
      os->procnum = my_app_atoi(ptr);
      fclose(fp);
  }
}

void getFileNR(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[128], *ptr=0, *endp=0;

  if ((fp=fopen("/proc/sys/fs/file-nr","r"))!=NULL)
  {
      memset(tmpbuf,0,128);
      fgets(tmpbuf,127,fp);
      ptr = tmpbuf;
      ptr = strtok_r(ptr,"\t", &endp);
      os->nropen = my_app_atoi(ptr);
      ptr = strtok_r(NULL,"\t",&endp);
      ptr = strtok_r(NULL,"\t",&endp);
      os->nrtotal= my_app_atoi(ptr);
      fclose(fp);
  }
}

void getCPUUsage(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;

  if ((fp=fopen("/proc/stat","r"))!=NULL)
  {
      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         if (STRNCASECMP(tmpbuf,"cpu ",4) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->user = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->system = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->idle = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->iowait = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->irq = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->softirq = my_app_atof(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "ctxt ",5) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->context = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "intr ",5) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->irqcall = my_app_atof(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "processes ",10) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->processes = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "procs_running ",14) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->runproc = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "procs_blocked ",14) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->blkproc = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "btime ",6) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->uptime = my_app_atoi(ptr);
         }
      }
      fclose(fp);
  }
}

void getSwapUsage(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;

  if ((fp=fopen("/proc/vmstat","r"))!=NULL)
  {
      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         if (STRNCASECMP(tmpbuf, "pgpgin ",7) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->pgpgin = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "pgpgout ",8) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->pgpgout = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "pswpin ",7) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->pswpin = my_app_atoi(ptr);
         }
         else if (STRNCASECMP(tmpbuf, "pswpout ",8) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->pswpout = my_app_atoi(ptr);
         }
      }
      fclose(fp);
  }
}

void getMemoryUsage(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;

  if ((fp=fopen("/proc/meminfo","r"))!=NULL)
  {
      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         if (STRNCASECMP(tmpbuf, "MemFree:",8) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->memfree = my_app_atof(ptr);
	     if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
	     {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
		      os->memfree = os->memfree * 1024;
		  else if (STRNCASECMP(ptr, "mB", 2) == 0)
		      os->memfree = os->memfree * 1024 * 1024;
	     }
         }
         else if (STRNCASECMP(tmpbuf, "Buffers:",8) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->buffers = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->buffers = os->buffers * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->buffers = os->buffers * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "Cached:",7) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->cached = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->cached = os->cached * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->cached = os->cached * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "SwapCached:",11) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->swapcached = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->swapcached = os->swapcached * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->swapcached = os->swapcached * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "SwapTotal:",10) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->swaptotal = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->swaptotal = os->swaptotal * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->swaptotal = os->swaptotal * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "SwapFree:",9) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->swapfree = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->swapfree = os->swapfree * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->swapfree = os->swapfree * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "LowTotal:",9) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->lowtotal = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->lowtotal = os->lowtotal * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->lowtotal = os->lowtotal * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "LowFree:",8) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->lowfree = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->lowfree = os->lowfree * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->lowfree = os->lowfree * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "HighTotal:",10) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->hightotal = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->hightotal = os->hightotal * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->hightotal = os->hightotal * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "HighFree:",9) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->highfree = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->highfree = os->highfree * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->highfree = os->highfree * 1024 * 1024;
             }
         }
         else if (STRNCASECMP(tmpbuf, "PageTables:",11) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->pagetables = my_app_atof(ptr);
             if ((ptr = strtok_r(NULL," ",&endp)) != NULL)
             {
                  if (STRNCASECMP(ptr, "kB", 2) == 0)
                      os->pagetables = os->pagetables * 1024;
                  else if (STRNCASECMP(ptr, "mB", 2) == 0)
                      os->pagetables = os->pagetables * 1024 * 1024;
             }
         }
      }
      fclose(fp);
  }
}

void getSocketUsage(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;

  if ((fp=fopen("/proc/net/sockstat","r"))!=NULL)
  {
      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         if (STRNCASECMP(tmpbuf, "TCP: ",5) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
	     while(ptr != NULL)
	     {
		if (STRNCASECMP(ptr, "inuse",5) == 0)
		{
		   ptr = strtok_r(NULL," ",&endp);
             	   os->tcpuse = my_app_atoi(ptr);
		}
		else if (STRNCASECMP(ptr, "alloc", 6) == 0)
		{
		   ptr = strtok_r(NULL," ",&endp);
		   os->tcpalloc = my_app_atoi(ptr);
		}
                else if (STRNCASECMP(ptr, "tw", 2) == 0)
                {
                   ptr = strtok_r(NULL," ",&endp);
                   os->tcpwait = my_app_atoi(ptr);
                }
		ptr = strtok_r(NULL," ",&endp);
	     }
         }
         else if (STRNCASECMP(tmpbuf, "UDP: ",5) == 0)
         {
             ptr=tmpbuf;
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             while(ptr != NULL)
             {
                if (STRNCASECMP(ptr, "inuse",5) == 0)
                {
                   ptr = strtok_r(NULL," ",&endp);
                   os->udpuse = my_app_atoi(ptr);
                }
                ptr = strtok_r(NULL," ",&endp);
             }
         }
      }
      fclose(fp);
  }
}

void getNetworkUsage(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;

  if ((fp=fopen("/proc/net/dev","r"))!=NULL)
  {
      os->ibytes=os->obytes=0;
      os->ipackets=os->opackets=0;
      os->ierrs = os->oerrs=0;

      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         ptr = tmpbuf;
         ptr = strtok_r(ptr,":",&endp);
         if (ptr == NULL) break;
         if (STRNCASECMP(ptr, "  eth",5) == 0 || STRNCASECMP(ptr, "   em",5) == 0)
         {
             ptr=endp;
             ptr = strtok_r(ptr," ",&endp);
             os->ibytes += my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->ipackets += my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->ierrs += my_app_atoi(ptr);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->obytes += my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->opackets += my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->oerrs += my_app_atoi(ptr);
         }
      }
      fclose(fp);
  }
}

void getSNMPTcpUsage(OSLoadData *os)
{
  int i=0;
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;

  if ((fp=fopen("/proc/net/snmp","r"))!=NULL)
  {
      os->activeopen=os->pasiveopen=0;

      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         if (STRNCASECMP(tmpbuf, "Tcp:",4) != 0 || i==0)
	 {
              if (STRNCASECMP(tmpbuf, "Tcp:",4) == 0) i++;
              continue;
         }
	 ptr = tmpbuf;
         if (STRNCASECMP(ptr, "Tcp:",4) == 0)
         {
             ptr = strtok_r(ptr," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->activeopen = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->pasiveopen = my_app_atof(ptr);

             ptr = strtok_r(NULL," ",&endp);
             os->afails = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->eresets = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             ptr = strtok_r(NULL," ",&endp);
             os->insegs = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->outsegs = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->resegs = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->inerrs = my_app_atof(ptr);
             ptr = strtok_r(NULL," ",&endp);
             os->outrst = my_app_atof(ptr);
	     break;
         }
      }
      fclose(fp);
  }
}

void getDiskList(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;
  int i,diskid = 0;
  
  if ((fp=fopen("/proc/diskstats","r"))!=NULL)
  {
      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         ptr = tmpbuf;
         ptr = strtok_r(ptr," ",&endp);
         if (ptr == NULL) break;
	 i = atoi(ptr);
	 if (i < 8 || i > 253) continue;
         ptr = strtok_r(NULL," ",&endp);
         if (ptr == NULL) break;
         i = atoi(ptr);
         if (i % 16 != 0) continue;
	 ptr = strtok_r(NULL," ",&endp);

	 if (diskid == 0)
		memcpy(os->prefix1, ptr, strlen(ptr));
	 else if (diskid == 1)
                memcpy(os->prefix2, ptr, strlen(ptr));
	 else if (diskid == 2)
                memcpy(os->prefix3, ptr, strlen(ptr));
	 diskid ++;
      }
      fclose(fp);
  }
}

void getDiskStats(OSLoadData *os)
{
  FILE *fp=NULL;
  char tmpbuf[512], *ptr, *endp;
  int  i;

  if ((fp=fopen("/proc/diskstats","r"))!=NULL)
  {
      os->disks1 = 0;
      os->disks2 = 0;
      os->disks3 = 0;
      for (i=0;i<11;i++)
      {
          os->disk1[i] = 0;
          os->disk2[i] = 0;
	  os->disk3[i] = 0;
      }
      while(!feof(fp))
      {
         memset(tmpbuf,0,512);
         fgets(tmpbuf,511,fp);
         if (tmpbuf == NULL) break;
         ptr = tmpbuf;
         ptr = strtok_r(ptr," ",&endp);
	 if (ptr == NULL) break;
	 ptr = strtok_r(NULL," ",&endp);
	 if (ptr == NULL) break;
	 i = my_app_atoi(ptr);
         if (i % 16 != 0) continue;
	 ptr = strtok_r(NULL," ",&endp);

	 if (strlen(os->prefix1) && STRNCASECMP(ptr, os->prefix1, strlen(os->prefix1)) == 0)
         {
            os->disks1 ++;
	    i = 0;
	    while(i < 11 && (ptr = strtok_r(NULL," ",&endp)) != NULL)
	    {
	        os->disk1[i] += my_app_atof(ptr);
	        i ++;
	    }
         }
         else if (strlen(os->prefix2) && STRNCASECMP(ptr, os->prefix2, strlen(os->prefix2)) == 0)
         {
            os->disks2 ++;
            i = 0;
            while(i < 11 && (ptr = strtok_r(NULL," ",&endp)) != NULL)
            {
                os->disk2[i] += my_app_atof(ptr);
                i ++;
            }
         }
         else if (strlen(os->prefix3) && STRNCASECMP(ptr, os->prefix3, strlen(os->prefix3)) == 0)
         {
            os->disks3 ++;
            i = 0;
            while(i < 11 && (ptr = strtok_r(NULL," ",&endp)) != NULL)
            {
                os->disk3[i] += my_app_atof(ptr);
                i ++;
            }
         }
      }
      fclose(fp);
  }
}

void getOSLoadData(OSLoadData *os)
{
  getLoadAvg(os);
  getCPUUsage(os);
  getSwapUsage(os);
  getNetworkUsage(os);
  getFileNR(os);
  getSocketUsage(os);
  getSNMPTcpUsage(os);
  getDiskStats(os); 
  getMemoryUsage(os);
}

int  intDouble(double a, double b)
{
   int rtnval = 0;
   if (a<=b)
   {
       rtnval = ((int)(b-a)) & 0x8fffffff;
   }
   else
   {
       rtnval = ((int)(b + 4294967295.0 - a)) & 0x8fffffff;
   }
   if (rtnval < 0) rtnval = 0;
   return rtnval;
}

double  doubleDouble(double a, double b)
{
   if (a<=b)
   {
       return (b-a);
   }
   return (b + 4294967295.0 - a);
}

void getOSLoadRecord(OSLoadData *os1, OSLoadData *os2, OSLoadRecord *rec)
{
     float duser, dsystem, diowait, didle, dtotal, dirq, dsoftirq;

     duser = os2->user - os1->user;
     dsystem = os2->system - os1->system;
     diowait = os2->iowait - os1->iowait;
     didle   = os2->idle - os1->idle;
     dirq     = os2->irq  - os1->irq;
     dsoftirq = os2->softirq - os1->softirq;
     dtotal  = duser + dsystem + diowait + didle + dirq + dsoftirq ;

     rec->load = os2->load1m;
     rec->run  = os2->procrun;
     rec->blk  = os2->blkproc;
     rec->num  = os2->procnum;
    
     rec->user = (100 * duser / dtotal);
     rec->system = (100 * dsystem / dtotal);
     rec->iowait = (100 * diowait / dtotal);
     rec->irq    = (100 * (dirq + dsoftirq) / dtotal);
     rec->softirq= (100 * dsoftirq / dtotal);

     rec->irqcall=intDouble(os1->irqcall , os2->irqcall);

     rec->context= os2->context - os1->context;
     rec->fork   = os2->processes - os1->processes;
     rec->uptime = (time(0) - os2->uptime)/3600;

     rec->pgin = doubleDouble(os1->pgpgin , os2->pgpgin);
     rec->pgout = doubleDouble(os1->pgpgout , os2->pgpgout);
     rec->swpin = doubleDouble(os1->pswpin , os2->pswpin);
     rec->swpout = doubleDouble(os1->pswpout , os2->pswpout);

     rec->free = os2->memfree/1048576;
     rec->swap = (os2->swaptotal - os2->swapfree)/1048576;
     rec->lfree = os2->lowfree/1048576;
     rec->cache = os2->cached/1048576;
     rec->hfree = os2->highfree/1048576;
     rec->pagetables = os2->pagetables/1048576;

     rec->ibytes = intDouble(os1->ibytes , os2->ibytes);
     rec->ipackets = intDouble(os1->ipackets , os2->ipackets);
     rec->ierrors = os2->ierrs - os1->ierrs;
     rec->obytes = intDouble(os1->obytes , os2->obytes);
     rec->opackets = intDouble(os1->opackets , os2->opackets);
     rec->oerrors = os2->oerrs - os1->oerrs;

     rec->nr = 100 * os2->nropen / os2->nrtotal;

     rec->tcpuse = os2->tcpuse;
     rec->tcpalloc = os2->tcpalloc;
     rec->tcpwait = os2->tcpwait;
     rec->aopen = intDouble(os1->activeopen, os2->activeopen);
     rec->popen = intDouble(os1->pasiveopen, os2->pasiveopen);

     rec->afail = intDouble(os1->afails, os2->afails);
     rec->reset = intDouble(os1->eresets, os2->eresets);
     rec->isegs = intDouble(os1->insegs, os2->insegs);
     rec->osegs = intDouble(os1->outsegs, os2->outsegs);
     rec->rsegs = intDouble(os1->resegs, os2->resegs);
     rec->inerr = intDouble(os1->inerrs, os2->inerrs);
     rec->orest = intDouble(os1->outrst, os2->outrst);

     rec->readc1 = intDouble(os1->disk1[0] , os2->disk1[0]);
     rec->rmerg1 = intDouble(os1->disk1[1] , os2->disk1[1]);
     rec->rblks1 = intDouble(os1->disk1[2] , os2->disk1[2]);
     rec->readt1 = intDouble(os1->disk1[3] , os2->disk1[3]);
     rec->writc1 = intDouble(os1->disk1[4] , os2->disk1[4]);
     rec->wmerg1 = intDouble(os1->disk1[5] , os2->disk1[5]);
     rec->wblks1 = intDouble(os1->disk1[6] , os2->disk1[6]);
     rec->writt1 = intDouble(os1->disk1[7] , os2->disk1[7]);
     rec->ioact1 = intDouble(0, os2->disk1[8]);
     rec->iotime1 = (int)(intDouble(os1->disk1[9] , os2->disk1[9])/(os2->disks1+0.1));

     rec->readc2 = intDouble(os1->disk2[0] , os2->disk2[0]);
     rec->rmerg2 = intDouble(os1->disk2[1] , os2->disk2[1]);
     rec->rblks2 = intDouble(os1->disk2[2] , os2->disk2[2]);
     rec->readt2 = intDouble(os1->disk2[3] , os2->disk2[3]);
     rec->writc2 = intDouble(os1->disk2[4] , os2->disk2[4]);
     rec->wmerg2 = intDouble(os1->disk2[5] , os2->disk2[5]);
     rec->wblks2 = intDouble(os1->disk2[6] , os2->disk2[6]);
     rec->writt2 = intDouble(os1->disk2[7] , os2->disk2[7]);
     rec->ioact2 = intDouble(0, os2->disk2[8]);
     rec->iotime2 = (int)(intDouble(os1->disk2[9] , os2->disk2[9])/(os2->disks2+0.1));

     rec->readc3 = intDouble(os1->disk3[0] , os2->disk3[0]);
     rec->rmerg3 = intDouble(os1->disk3[1] , os2->disk3[1]);
     rec->rblks3 = intDouble(os1->disk3[2] , os2->disk3[2]);
     rec->readt3 = intDouble(os1->disk3[3] , os2->disk3[3]);
     rec->writc3 = intDouble(os1->disk3[4] , os2->disk3[4]);
     rec->wmerg3 = intDouble(os1->disk3[5] , os2->disk3[5]);
     rec->wblks3 = intDouble(os1->disk3[6] , os2->disk3[6]);
     rec->writt3 = intDouble(os1->disk3[7] , os2->disk3[7]);
     rec->ioact3 = intDouble(0, os2->disk3[8]);
     rec->iotime3 = (int)(intDouble(os1->disk3[9] , os2->disk3[9])/(os2->disks3+0.1));

     rec->innodb_data_read      = os2->innodb_data_read      - os1->innodb_data_read;
     rec->innodb_data_write     = os2->innodb_data_write     - os1->innodb_data_write;
     rec->innodb_log_waits      = os2->innodb_log_waits      - os1->innodb_log_waits;
     rec->innodb_log_writes     = os2->innodb_log_writes     - os1->innodb_log_writes;
     rec->innodb_page_free      = os2->innodb_page_free      - os1->innodb_page_free;
     rec->innodb_page_read      = os2->innodb_page_read      - os1->innodb_page_read;
     rec->innodb_page_create    = os2->innodb_page_create    - os1->innodb_page_create;
     rec->innodb_page_write     = os2->innodb_page_write     - os1->innodb_page_write;
     rec->innodb_page_flush     = os2->innodb_page_flush     - os1->innodb_page_flush;
     rec->innodb_rowlock_waits  = os2->innodb_rowlock_waits  - os1->innodb_rowlock_waits;
     rec->innodb_rowlock_time   = os2->innodb_rowlock_time   - os1->innodb_rowlock_time;
     rec->innodb_rows_read      = os2->innodb_rows_read      - os1->innodb_rows_read;
     rec->innodb_rows_inserted  = os2->innodb_rows_inserted  - os1->innodb_rows_inserted;
     rec->innodb_rows_updated   = os2->innodb_rows_updated   - os1->innodb_rows_updated;
     rec->innodb_rows_deleted   = os2->innodb_rows_deleted   - os1->innodb_rows_deleted;
     rec->innodb_max_trxid      = os2->innodb_max_trxid      - os1->innodb_max_trxid;
     rec->innodb_log_lsn        = os2->innodb_log_lsn        - os1->innodb_log_lsn;
     if (os2->innodb_log_flushed_lsn)
         rec->innodb_log_flushed_lsn    = os2->innodb_log_lsn    - os2->innodb_log_flushed_lsn;
     if (os2->innodb_log_checkpoint_lsn)
         rec->innodb_log_checkpoint_lsn = os2->innodb_log_lsn    - os2->innodb_log_checkpoint_lsn;
     rec->innodb_max_purge          = os2->innodb_max_purge  - os1->innodb_max_purge;
     rec->innodb_max_undo           = os2->innodb_max_undo;
     rec->innodb_all_undo	    = os2->innodb_all_undo;

     rec->innodb_ibuf_size          = os2->innodb_ibuf_size;
     rec->innodb_ibuf_segsize       = os2->innodb_ibuf_segsize;
     rec->innodb_ibuf_freelist      = os2->innodb_ibuf_freelist;
     rec->innodb_ibuf_merges        = os2->innodb_ibuf_merges - os1->innodb_ibuf_merges;
     rec->innodb_ibuf_mergeop       = os2->innodb_ibuf_mergeop - os1->innodb_ibuf_mergeop;
     rec->innodb_ibuf_discardop     = os2->innodb_ibuf_discardop - os1->innodb_ibuf_discardop;

     rec->mysql_conn_abort          = os2->mysql_conn_abort  - os1->mysql_conn_abort;
     rec->mysql_lock_immediate      = os2->mysql_lock_immediate - os1->mysql_lock_immediate;
     rec->mysql_lock_waited         = os2->mysql_lock_waited    - os1->mysql_lock_waited;
     rec->mysql_tmpfile_created     = os2->mysql_tmpfile_created - os1->mysql_tmpfile_created;
     rec->mysql_conn_created        = os2->mysql_conn_created    - os1->mysql_conn_created;
     rec->mysql_conn_count          = os2->mysql_conn_count;
     rec->mysql_conn_running        = os2->mysql_conn_running;

     rec->mysql_com_insert          = os2->mysql_com_insert      - os1->mysql_com_insert;
     rec->mysql_com_update          = os2->mysql_com_update      - os1->mysql_com_update;
     rec->mysql_com_delete          = os2->mysql_com_delete      - os1->mysql_com_delete;
     rec->mysql_com_select          = os2->mysql_com_select      - os1->mysql_com_select;
     rec->mysql_com_commit          = os2->mysql_com_commit      - os1->mysql_com_commit;
     rec->mysql_com_rollback        = os2->mysql_com_rollback    - os1->mysql_com_rollback;

     rec->mysql_tcache_get          = os2->mysql_tcache_get      - os1->mysql_tcache_get;
     rec->mysql_tcache_miss         = os2->mysql_tcache_miss     - os1->mysql_tcache_miss;

     rec->mysql_long_query          = os2->mysql_long_query      - os1->mysql_long_query;
     rec->mysql_tmp_tables          = os2->mysql_tmp_tables      - os1->mysql_tmp_tables;
     rec->mysql_tmp_disktables      = os2->mysql_tmp_disktables  - os1->mysql_tmp_disktables;
     rec->mysql_sort_merge          = os2->mysql_sort_merge      - os1->mysql_sort_merge;
     rec->mysql_sort_rows           = os2->mysql_sort_rows       - os1->mysql_sort_rows;
     rec->mysql_bytes_received      = os2->mysql_bytes_received  - os1->mysql_bytes_received;
     rec->mysql_bytes_sent          = os2->mysql_bytes_sent      - os1->mysql_bytes_sent;

     rec->mysql_slave_count         = os2->mysql_slave_count;
     rec->mysql_binlog_offset       = os2->mysql_binlog_offset - os1->mysql_binlog_offset;
     rec->mysql_iolog_offset        = os2->mysql_iolog_offset  - os1->mysql_iolog_offset;
     rec->mysql_relaylog_offset     = os2->mysql_relaylog_offset - os1->mysql_relaylog_offset;
     rec->mysql_apply_delay         = (os2->mysql_iolog_offset - os2->mysql_relaylog_offset)/1024;
}

void formatInt(FILE *fp, int digit, int val)
{
     if (digit == 4)
     {
         if (val >= 1000000)
            fprintf(fp, " %3dM", val/1000/1000);
         else if (val >= 10000)
            fprintf(fp, " %3dK", val/1000);
         else if (val >= 0)
            fprintf(fp, " %4d", val);
         else
            fprintf(fp, " %4d", 0);
     }
     else
     {
         if (val >= 10000000)
            fprintf(fp, " %4dM", val/1000/1000);
         else if (val >= 100000)
            fprintf(fp, " %4dK", val/1000);
         else if (val >= 0)
            fprintf(fp, " %5d", val);
         else 
            fprintf(fp, " %5d", 0);
     }
}

void formatInt2(FILE *fp, int digit, int val)
{
     if (digit == 4)
     {
         if (val >= 1000000)
            fprintf(fp, " %3dM", val/1024/1024);
         else if (val >= 10000)
            fprintf(fp, " %3dK", val/1024);
         else if (val >= 0)
            fprintf(fp, " %4d", val);
         else
            fprintf(fp, " %4d", 0);
     }
     else
     {
         if (val >= 10000000)
            fprintf(fp, " %4dM", val/1024/1024);
         else if (val >= 100000)
            fprintf(fp, " %4dK", val/1024);
         else if (val >= 0)
            fprintf(fp, " %5d", val);
         else
            fprintf(fp, " %5d", 0);
     }
}

void formatFloat(FILE *fp, int digit, float val)
{
     if (digit == 4)
     {
         if (val > 1000000000)
            fprintf(fp, " %3.0fG", val/1000/1000/1000);
         else if (val >= 1000000)
            fprintf(fp, " %3.0fM", val/1000/1000);
         else if (val >= 10000)
            fprintf(fp, " %3.0fK", val/1000);
         else if (val >= 0)
            fprintf(fp, " %4.0f", val);
         else 
            fprintf(fp, " %4.0f", 0.0);
     }
     else
     {
         if (val > 10000000000)
            fprintf(fp, " %4.0fG", val/1000/1000/1000);
         else if (val >= 10000000)
            fprintf(fp, " %4.0fM", val/1000/1000);
         else if (val >= 100000)
            fprintf(fp, " %4.0fK", val/1000);
         else if (val >= 0)
            fprintf(fp, " %5.0f", val);
         else
            fprintf(fp, " %5.0f", 0.0);
     }
}

void formatFloat2(FILE *fp, int digit, float val)
{
     if (digit == 4)
     {
         if (val > 1000000000)
            fprintf(fp, " %3.0fG", val/1024/1024/1024);
         else if (val >= 1000000)
            fprintf(fp, " %3.0fM", val/1024/1024);
         else if (val >= 10000)
            fprintf(fp, " %3.0fK", val/1024);
         else if (val >= 0)
            fprintf(fp, " %4.0f", val);
         else
            fprintf(fp, " %4.0f", 0.0);
     }
     else
     {
         if (val > 10000000000)
            fprintf(fp, " %4.0fG", val/1024/1024/1024);
         else if (val >= 10000000)
            fprintf(fp, " %4.0fM", val/1024/1024);
         else if (val >= 10000)
            fprintf(fp, " %4.0fK", val/1024);
         else if (val >= 0)
            fprintf(fp, " %5.0f", val);
         else
            fprintf(fp, " %5.0f", 0.0);
     }
}

void printLoadRecord(FILE *fp, OSLoadRecord *rec)
{
    time_t now = time(0);
    struct tm *ptm = localtime(&now);
    int idlecpu = 0;

    idlecpu = 100 - rec->system - rec->iowait - rec->user - rec->irq;
    if (idlecpu == 100) idlecpu --;

    if (rec->row % 20 == 0)
    {
        if (rec->disk2)
        {
            fprintf(fp,"                ----------------------CPU---------------------- ---------Memory--------- --------------------NetWork------------------------ ------------Disk------------- ------------Disk2------------\n");
            fprintf(fp,"www.AnySQL.net  Load Run Blk Proc SY/WI/US/IR/ID  Irq Ctxt Fork PgIO SwIO Free Page Cach NetI NetO Err NR TcpU TcpA TcpW ActC PsvC Fail Rest Read RTim Writ WTim Busy Time Read RTim Writ WTim Busy Time\n");
        }
        else
        {
            fprintf(fp,"                ----------------------CPU---------------------- ---------Memory--------- --------------------NetWork------------------------ ------------Disk-------------\n");
            fprintf(fp,"www.AnySQL.net  Load Run Blk Proc SY/WI/US/IR/ID  Irq Ctxt Fork PgIO SwIO Free Page Cach NetI NetO Err NR TcpU TcpA TcpW ActC PsvC Fail Rest Read RTim Writ WTim Busy Time\n");
        }
        rec->row = 0;
    }
    fprintf(fp,"%02d/%02d-%02d:%02d:%02d", ptm->tm_mon + 1,
                ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    if (rec->load>99.99)
        fprintf(fp," %5.0f", rec->load);
    else if (rec->load>99.99)
        fprintf(fp," %5.1f", rec->load);
    else
        fprintf(fp," %5.2f", rec->load);
    fprintf(fp," %3d %3d %4d %2d/%2d/%2d/%2d/%2d", 
                rec->run, rec->blk, rec->num, rec->system, rec->iowait, rec->user, rec->irq, idlecpu);
    formatInt(fp,4,rec->irqcall);
    formatInt(fp,4,rec->context);
    formatInt(fp,4,rec->fork);
    formatInt(fp,4,rec->pgin + rec->pgout);
    formatInt(fp,4,rec->swpin + rec->swpout);
    formatFloat2(fp,4,rec->free);
    formatFloat2(fp,4,rec->pagetables);
    formatFloat2(fp,4,rec->cache);
    /* formatFloat2(fp,4,rec->swap); */
    formatInt2(fp,4,rec->ibytes);
    formatInt2(fp,4,rec->obytes);
    fprintf(fp, " %3d", rec->ierrors + rec->oerrors);
    fprintf(fp, " %2d", rec->nr);
    formatInt(fp,4,rec->tcpuse);
    formatInt(fp,4,rec->tcpalloc);
    formatInt(fp,4,rec->tcpwait);
    formatInt(fp,4,rec->aopen);
    formatInt(fp,4,rec->popen);
    formatInt(fp,4,rec->afail);
    formatInt(fp,4,rec->reset);
    formatInt(fp,4,rec->readc1);
    formatInt(fp,4,10 * rec->readt1/(rec->readc1+1));
    formatInt(fp,4,rec->writc1);
    formatInt(fp,4,10 * rec->writt1/(rec->writc1+1));
    formatInt(fp,4,rec->ioact1);
    formatInt(fp,4,rec->iotime1);
    if (rec->disk2)
    {
       formatInt(fp,4,rec->readc2);
       formatInt(fp,4,10 * rec->readt2/(rec->readc2+1));
       formatInt(fp,4,rec->writc2);
       formatInt(fp,4,10 * rec->writt2/(rec->writc2+1));
       formatInt(fp,4,rec->ioact2);
       formatInt(fp,4,rec->iotime2);
    }
    fprintf(fp,"\n");
    fflush(fp);
    rec->row++;
}

void getHostName(char *buf, int buflen)
{
    int i;
    FILE *fp=NULL;
    
    if ((fp = fopen("/proc/sys/kernel/hostname", "r+")) != NULL)
    {
        memset(buf,0,buflen);
        fgets(buf, buflen-1, fp);
        for(i=strlen(buf); i>0; i--)
        {
            if (*(buf+i-1) != '\n' && *(buf+i-1) != '\a') break;
	    *(buf+i-1) = '\0';
        } 
        fclose(fp);
    }
}

int getCreateTableSyntax(char *buf)
{
    size_t pos = 0;

    pos += sprintf(buf + pos, "create table linux_os_statistics (\n");
    pos += sprintf(buf + pos, " day datetime not null, host varchar(64) not null, load1m float, \n");
    pos += sprintf(buf + pos, " runque int, blkque int, proc int, uptime int, syscpu int, wiocpu int, \n");
    pos += sprintf(buf + pos, " usrcpu int, irqcpu int, softirq int, \n");

    pos += sprintf(buf + pos, " irqcall int, context int, fork int, pgin int, pgout int, swpin int, swpout int, \n");
    pos += sprintf(buf + pos, " free int, cache int, swap int, lfree int, hfree int, pgtables int, \n");

    pos += sprintf(buf + pos, " ibytes int, ipackets int, ierrors int, obytes int, opackets int, oerrors int, \n");
    pos += sprintf(buf + pos, " filenr int, tcpuse int, tcpalloc int, tcpwait int, actopen int, pasopen int, \n");

    pos += sprintf(buf + pos, " readc1 int, rmerg1 int, rblks1 int, readt1 int, \n");
    pos += sprintf(buf + pos, " writc1 int, wmerg1 int, wblks1 int, writt1 int, ioact1 int, iotime1 int, \n");

    pos += sprintf(buf + pos, " readc2 int, rmerg2 int, rblks2 int, readt2 int, \n");
    pos += sprintf(buf + pos, " writc2 int, wmerg2 int, wblks2 int, writt2 int, ioact2 int, iotime2 int, \n");

    pos += sprintf(buf + pos, " readc3 int, rmerg3 int, rblks3 int, readt3 int, \n");
    pos += sprintf(buf + pos, " writc3 int, wmerg3 int, wblks3 int, writt3 int, ioact3 int, iotime3 int, \n");

    pos += sprintf(buf + pos, " primary key (day, host), key ix2_linux_os_statistics (host, day))");

    buf[pos] = '\0';
    return pos;
}

int getInsertSQLSyntax(char *buf, const char *vallist)
{
    size_t pos = 0;

    pos += sprintf(buf + pos, "replace into linux_os_statistics (");
    pos += sprintf(buf + pos, " day, host, load1m, runque, blkque, proc, uptime, syscpu, wiocpu, usrcpu, irqcpu, softirq, ");
    pos += sprintf(buf + pos, " irqcall, context, fork, pgin, pgout, swpin, swpout, free, cache, swap, lfree, hfree, pgtables, ");
    pos += sprintf(buf + pos, " ibytes, ipackets, ierrors, obytes, opackets, oerrors, filenr, tcpuse, tcpalloc, tcpwait, actopen, pasopen, ");
    pos += sprintf(buf + pos, " readc1, rmerg1, rblks1, readt1, writc1, wmerg1, wblks1, writt1, ioact1, iotime1, ");
    pos += sprintf(buf + pos, " readc2, rmerg2, rblks2, readt2, writc2, wmerg2, wblks2, writt2, ioact2, iotime2, ");
    pos += sprintf(buf + pos, " readc3, rmerg3, rblks3, readt3, writc3, wmerg3, wblks3, writt3, ioact3, iotime3) values ");

    pos += sprintf(buf + pos, "(%s)", vallist);

    return pos;
}

void writeLoadRecordBuffer(char *buf, unsigned char *hostname, OSLoadRecord *rec)
{
    size_t pos = 0;
    time_t now = time(0);
    struct tm *ptm = localtime(&now);

    pos += sprintf(buf+pos,"'%04d-%02d-%02d %02d:%02d:%02d','%s',%.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	ptm->tm_year + 1900, ptm->tm_mon + 1,
        ptm->tm_mday, ptm->tm_hour,
        ptm->tm_min, ptm->tm_sec,
	hostname, rec->load, rec->run, rec->blk, rec->num,rec->uptime,
	rec->system, rec->iowait, rec->user,rec->irq,rec->softirq);

    pos += sprintf(buf+pos,",%d,%d,%d,%d,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
        rec->irqcall,rec->context,rec->fork, rec->pgin, rec->pgout, rec->swpin, rec->swpout,
	rec->free,rec->cache, rec->swap, rec->lfree, rec->hfree, rec->pagetables);

    pos += sprintf(buf+pos,",%d,%d,%d,%d,%d,%d",
	rec->ibytes, rec->ipackets, rec->ierrors,
	rec->obytes, rec->opackets, rec->oerrors);

    pos += sprintf(buf+pos,",%d,%d,%d,%d,%d,%d",
	rec->nr, rec->tcpuse, rec->tcpalloc, rec->tcpwait,rec->aopen,rec->popen);

    pos += sprintf(buf+pos,",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
        rec->readc1, rec->rmerg1, rec->rblks1, rec->readt1,
        rec->writc1, rec->wmerg1, rec->wblks1, rec->writt1,
	rec->ioact1, rec->iotime1);

    pos += sprintf(buf+pos,",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
        rec->readc2, rec->rmerg2, rec->rblks2, rec->readt2,
        rec->writc2, rec->wmerg2, rec->wblks2, rec->writt2,
        rec->ioact2, rec->iotime2);

    pos += sprintf(buf+pos,",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
        rec->readc3, rec->rmerg3, rec->rblks3, rec->readt3,
        rec->writc3, rec->wmerg3, rec->wblks3, rec->writt3,
        rec->ioact3, rec->iotime3);
   
    buf[pos] = '\0';
}

void writeLoadRecord(FILE *fp, unsigned char *hostname, OSLoadRecord *rec)
{
    char buf[2048];

    memset(buf,0,2048);

    writeLoadRecordBuffer(buf, hostname, rec);

    fprintf(fp, "v01,%s\n", buf);
    fflush(fp);
}

void getSwitchFile(char *fname, char *newname, int idx)
{
   char cidx[]="0123456789";
   char *p, *ptr;
   p = fname;
   ptr = newname;
   while(*p && *p != '.')
   {
        *ptr = *p;
        p ++;
        ptr ++;
   }
   *ptr++='.';
   *ptr++=cidx[idx%10];
   while(*p)
   {
        *ptr = *p;
        p ++;
        ptr ++;
   }
}

void switchFile(char *fname)
{
   char tempbuf1[512];
   char tempbuf2[512];
   char tempbuf3[512];

   memset(tempbuf1,0,512);
   memset(tempbuf2,0,512);
   memset(tempbuf3,0,512);

   getSwitchFile(fname, tempbuf1, 1);
   getSwitchFile(fname, tempbuf2, 2);
   getSwitchFile(fname, tempbuf3, 3);

   unlink(tempbuf3);
   rename(tempbuf2,tempbuf3);
   rename(tempbuf1,tempbuf2);
   rename(fname, tempbuf1);
}

void getMountSpaceInfo(const char *path, double *usedgb, double *freegb, unsigned long *freepct, unsigned long *inodepct)
{
   struct statvfs fsinfo;
   unsigned long long ll_val;
   double d_val;

   if (0 == statvfs(path, &fsinfo))
   {
	ll_val = fsinfo.f_blocks - fsinfo.f_bavail;
        ll_val = ll_val * fsinfo.f_bsize;
        ll_val = ll_val / 1048576;
        d_val = (1.0 * ll_val)/1000;
	*usedgb = d_val;

	ll_val = fsinfo.f_bavail;
	ll_val = ll_val * fsinfo.f_bsize;
	ll_val = ll_val / 1048576;
	d_val = (1.0 * ll_val)/1000;
	*freegb = d_val;

	ll_val = 100 * fsinfo.f_bavail;
	ll_val = ll_val / fsinfo.f_blocks;
	*freepct = ll_val;

        ll_val = 100 * fsinfo.f_favail;
        ll_val = ll_val / fsinfo.f_files;
        *inodepct = ll_val;
   }
}

int getNICMacAddress(char *eth, unsigned char *mac)
{
  struct ifreq ifr;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) return -1;
  strncpy(ifr.ifr_name, eth, sizeof(ifr.ifr_name));
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) return -1;
  memcpy(mac, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
  return 0;
}

int getHostMacAddress(unsigned char *hostmac)
{
     struct ifaddrs *ifaddr, *ifa=NULL;
     int family, rtn=-1;
     unsigned char mac[IFHWADDRLEN];

     if (getifaddrs(&ifaddr) == -1) {
         return -1;
     }

     for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
         /* get name of interface and its family */
         family = ifa->ifa_addr->sa_family;
         if (strlen(ifa->ifa_name) <= 2) continue;
         if ((family == AF_INET) || (family == AF_INET6)) break;
     }

     if (ifa)
     {
         /* get MAC address */
         if (getNICMacAddress(ifa->ifa_name, mac) == 0)
         {
                memcpy(hostmac, mac, IFHWADDRLEN);
                rtn = 0;
         }
      }
      freeifaddrs(ifaddr);
      return (rtn);
}

