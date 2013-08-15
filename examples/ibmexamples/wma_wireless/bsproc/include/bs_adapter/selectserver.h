/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: selectserver.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 28-Jan 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef NBSERVER_H
#define NBSERVER_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>



#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include <pthread.h>
#include <semaphore.h>

#include "resolve.h"

#include "asynqueue.h"


#include "types.h"




#define DEFAULT_BUFFER_SIZE     4096    // default buffer size
#define FD_SET_SIZE             1024

#define     BEGIN_MAP_PROCESS()               \
                                              int    process_map_message(int fdsocket,buffer_obj *buffobj,int *pflag){	\
                                              int ireturn = 0;
#define     PROCESS_HANDLE(ifd,fun)     \
                                              if(fdsocket == ifd )  \
                                                  return fun(buffobj,pflag);      
#define     END_MAP_PROCESS()                 \
                                              return ireturn;\
					      }


//the function is  callback func,must be register when your need get data from receiving;
int  received(int fdsocket, struct sockaddr_in *from,void *pdata,int isize,int *pflag);
typedef  int (*pcallreceived)(int fdsocket, struct sockaddr_in *from,void *pdata,int isize,int *pflag);
int registercallreceivedfunction(pcallreceived  buff);


//example fun for create
//int impfuncbuff(buffer_obj *buffobj,int *pflag); while for call in furture 
//like this :
//   BEGIN_MAP_PROCESS()
//   PROCESS_HANDLE()
//   END_MAP_PROCESS()
//   registercallreceivedfunctionbuff(process_map_message);
//
//
int receivedbuff(int fdsocket,struct buffer_obj *buffobj,int *pflag);
typedef int (*pcallreceivedbuff)(int fdsocket,struct buffer_obj *bufferobj,int *pflag);
int registercallreceivedfunctionbuff(pcallreceivedbuff pcallbuff);


//
// Allocated for each receiver posted
//

//the fllowing function is called for external
/****************************************************************************************************************************************************************************/


  int convert2binaryip(char *strip,int port,struct sockaddr_in *bip);
  int conver2stringip(struct sockaddr_in *bip,char *strip,int *port);



int initnetsocket(char *host,char *strip, char*  port,int isserver);
int sendnetdata(int fdsocket,struct sockaddr_in *addr,u_char *pbufferdata,int ibuffersize);
int getlasterror(int backcoder);  



//
//function initialall
//description:  this function for init  thread and thread_mutex.when u use the lib ,must call this func at first .
int   initialrun();

//function:  releaseallresource
//description:  release all resouce thread's resource mutex and other
int  releaseallresource();

/****************************************************************************************************************************************************************************/
void* workthread(void * para);
void* sendprocess_thread(void *para);
void* receiveprocess_thread(void *para);


#endif
