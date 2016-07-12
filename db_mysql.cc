/*
*
*                                 Apache License
*                           Version 2.0, January 2004
*                        http://www.apache.org/licenses/
*/

#ifdef RCSID
static char *RCSid =
   "$Header: copydata.c 2005.05.19 Lou Fangxin, http://www.anysql.net $ ";
#endif /* RCSID */

/*
   NAME
     copydata.c - Using OCI to rewrite the unload script.

   MODIFIED   (MM/DD/YY)
    Lou Fangxin    2005.05.19 -  Initial write.
    Lou Fangxin    2005.05.22 -  Add File Option to command
    Lou Fangxin    2005.05.25 -  Enable login as sysdba
*/

#if defined(_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "osload.h"

#define OCI_SUCCESS 0
#define OCI_ERROR   1

typedef          char      sb1;
typedef unsigned char      ub1;
typedef unsigned short     ub2;
typedef          short     sb2;
typedef unsigned int       ub4;
typedef          int       sb4;
typedef          int       sword;
typedef unsigned long long ub8;
typedef          long long sb8;
typedef unsigned char      text;

/* Constants used in this program. */
#define MAX_SELECT_LIST_SIZE    1023
#define MAX_ITEM_BUFFER_SIZE      33
#define PARSE_NO_DEFER             0
#define PARSE_V7_LNG               2
#define MAX_BINDS                 12
#define MAX_SQL_IDENTIFIER        31
#define ROW_BATCH_SIZE       1000000
#define UNLOAD_BUFFER_SIZE   4194304
#define IO_BUFFER_BITS            22
#define LOB_FILENAME_LENGTH       30
#define MAX_BIND_ARRAY          4096

#if defined(_WIN32)
#define STRNCASECMP memicmp
#else
#define STRNCASECMP strncasecmp
#endif

#if !defined(MIN)
#define  MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#if !defined(MAX)
#define  MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

#undef MYSQL_SERVER

void  initMYDB(MYDATABASE *db)
{
  db->conn=NULL;
  db->errcode = 0;
  memset(db->charset,0,256);
  memset(db->errmsg,0,512);
}

int  myError(MYDATABASE *db)
{
   const char *myerr=NULL;
   db->errcode=0;
   memset((void *) (db->errmsg), (int)'\0', (size_t)512);
   db->errcode = mysql_errno(db->conn);
   if (db->errcode)
   {
       myerr = mysql_error(db->conn);
       strncat(db->errmsg, myerr, MIN(strlen(myerr),511));
   }
   return db->errcode;
}

int myExecute(MYDATABASE *db, const char *sqlstring, int len)
{
   int ret = 0;
   MYSQL_RES *myresult = NULL;

   db->errcode = 0;
   if(db->conn != NULL)
   {
       if ((ret = mysql_real_query(db->conn, sqlstring, len)) == 0)
       {
           myresult = mysql_store_result(db->conn);
           if (myresult != NULL)
           {
               while (mysql_fetch_row(myresult));
               mysql_free_result(myresult);
           }
       }
       else
       {
           myError(db);
       }
   }
   else
   {
       db->errcode = 1063;
   }
   return db->errcode;
}

int disconnectMYDB(MYDATABASE *db)
{
    if(db->conn != NULL) mysql_close(db->conn);
    db->conn = NULL;
    return OCI_SUCCESS;
}

int LogonMYDB(MYDATABASE *db, const char *o_connstr)
{
    unsigned int  i=0;
    unsigned int  pos=0;
    unsigned int  inquote=0;
    char o_user[132];
    char o_pass[132];
    char o_host[132];
    char o_port[132];
    char o_dbid[132];
    char o_socket[132];
    char tempbuf[256];
    my_bool autoreconnect = 1;

    memset(o_user,0,132);
    memset(o_pass,0,132);
    memset(o_host,0,132);
    memset(o_port,0,132);
    memset(o_dbid,0,132);
    memset(o_socket,0,132);
    memset(tempbuf,0,256);
   
    if (i < strlen(o_connstr))
    {
       while(i<strlen(o_connstr) && o_connstr[i] != '/' && (o_connstr[i] != '@' || inquote == 1))
       {
	   if (o_connstr[i] == '"')
           {
              if (inquote) inquote = 0  ;
              else inquote = 1;
           }
           i++; 
       }
       if (o_connstr[i]=='/')
       {
          memcpy(o_user, o_connstr+pos, i-pos);
          pos = i + 1;
          i++;
          while(i<strlen(o_connstr) && (o_connstr[i] != '@' || inquote == 1))
          {
	     if (o_connstr[i] == '"')
             {
                if (inquote) inquote = 0  ;
                else inquote = 1;
             }
             i++; 
          }
          if (o_connstr[i]=='@')
          {
             if (i>pos) memcpy(o_pass, o_connstr+pos, i-pos);
             pos = i + 1;
             i++;
             strcat(tempbuf, o_connstr + pos);
          }
          else  
          {
              strcat(o_pass, o_connstr + pos);
          }
       }
       else
       {
      	  if (o_connstr[i]=='@')
      	  {
              strcat(o_user, o_connstr + pos);
              pos = i+ 1;
              strcat(tempbuf, o_connstr + pos);     	  	
      	  }
       }
    }
    i=0; 
    pos=0;
    while(tempbuf[i] && tempbuf[i] != ':') 
    {
       o_host[pos++] = tempbuf[i++];
    }
    if (tempbuf[i]) i++;
    pos=0;
    while(tempbuf[i] && tempbuf[i] != ':') 
    {
       o_port[pos++] = tempbuf[i++];
    }
    if (tempbuf[i]) i++;
    pos=0;
    while(tempbuf[i] && tempbuf[i] != ':') 
    {
       o_dbid[pos++] = tempbuf[i++];
    }
    if (tempbuf[i]) i++;
    pos=0;
    while(tempbuf[i] && tempbuf[i] != ':') 
    {
       o_socket[pos++] = tempbuf[i++];
    }

    db->conn = mysql_init(NULL);
    if (db->conn == NULL) return OCI_ERROR;

    if (db->charset[0])
    {
	mysql_options(db->conn, MYSQL_SET_CHARSET_NAME, db->charset);
    }

    mysql_options(db->conn, MYSQL_OPT_RECONNECT, &autoreconnect);

    if (!mysql_real_connect(db->conn, o_host,
        o_user, o_pass, o_dbid, atoi(o_port), NULL, CLIENT_REMEMBER_OPTIONS)) 
    {
	myError(db); 
	fflush(stderr);
        mysql_close(db->conn);
	db->conn = NULL;
        return OCI_ERROR;
    }

    if (db->charset[0])
    {
       memset(tempbuf,0,256);
       sprintf(tempbuf, "set names %s", db->charset);
       mysql_real_query(db->conn, tempbuf, strlen(tempbuf));
    }

    memset(tempbuf,0,256);
    sprintf(tempbuf, "set autocommit=1");
    mysql_real_query(db->conn, tempbuf, strlen(tempbuf));

    return OCI_SUCCESS;
}

