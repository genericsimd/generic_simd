/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: selectserver.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 28-Jan 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>
#include <string.h>


#include "selectserver.h"
#define    MAX_CONNECT_NUM       1000   //  

#define    FALSE         0
#define    TRUE          1

char *gbindaddr    = NULL,              // local interface to bind to
     *gbindport    = "7777";            // local port to bind to


struct socket_obj *gsocketlist=NULL,       // Linked list of all sockets allocated
           *gsocketlistend=NULL;    // End of socket list


int         gsocketcount=0;         // Number of socket objects in list

//
// Statistics counters
//
volatile long gbytesread=0,
              gbytessent=0,
              gstarttime=0,
              gbytesreadlast=0,
              gbytessentlast=0,
              gstarttimelast=0,
              gCurrentConnections=0;


int gaddressfamily = AF_INET,         // default to IPV4
    gsockettype    = SOCK_DGRAM,       // default to UDP socket type
    gprotocol      = IPPROTO_UDP,       // default to UDP protocol
    gbuffersize    = DEFAULT_BUFFER_SIZE;

pcallreceived      greceivedfun = NULL;



struct buffer_obj              *sendqueue;    //for sendthread
struct buffer_obj              *sendqueuetail;

struct buffer_obj              *receivedqueue;  //for receivedthrea
struct buffer_obj              *receivedqueuetail;

pthread_mutex_t          thsendbuffmutex;
pthread_mutex_t          thsendnotifymutex;
pthread_cond_t           sendbuffnotify;

pthread_mutex_t          threceivedbuffmutex;
pthread_mutex_t          threceivednotifymutex;
pthread_cond_t           receivedbuffnotify;


int        bthreadexit = FALSE;


pcallreceivedbuff           gcallreceivedfuncbuff;
pcallreceived               gcallreceivedfunc;


int convert2binaryip(char *strip,int port,struct sockaddr_in *bip)
{
    if(strip == NULL || bip == NULL)
    {
        return -1;
    }
    bzero(bip, sizeof(*bip));
    
    bip->sin_port = htons(port);
    if(inet_pton(AF_INET, strip, &(bip->sin_addr)) <= 0)
    {
        printf("not a valid IPaddress\n");
        exit(1);
    }
 
    return 0;
}
int conver2stringip(struct sockaddr_in *bip,char *strip,int *port)
{
   if(bip == NULL|| strip ==NULL||port ==NULL)
   {
      return -1;
   }
   sprintf(strip, "%s",inet_ntoa(bip->sin_addr) );
   *port = ntohs(bip->sin_port);
 
   return 0;
}




int registercallreceivedfunction(pcallreceived  pfunc)
{
     gcallreceivedfunc = pfunc;
     return 0;
}


int registercallreceivedfunctionbuff(pcallreceivedbuff  pfunc)
{
    gcallreceivedfuncbuff = pfunc;
  //gcallreceivedfuncbuff = process_map_message;
     return 0;
}

//
// Description:
//    Allocate a socket object and initialize its members. A socket object is
//    allocated for each socket created (either by socket or accept). The
//    socket objects mantain a list of all buffers received that need to
//    be sent.
//
struct socket_obj *getsocketobj(SOCKET s, int listening)
{
    struct socket_obj  *sockobj=NULL;

    sockobj = (struct socket_obj *)malloc(sizeof(struct socket_obj));
    if (sockobj == NULL)
    {
        fprintf(stderr, "getsocketobj:malloc failed: %d\n", errno);
        return NULL;
    }

    // Initialize the members
    sockobj->s = s;
    sockobj->listening = listening;
    sockobj->addrlen = sizeof(sockobj->addr);

    //initial mutex
    pthread_mutexattr_t        mattr;
    pthread_mutexattr_init(&mattr);
    //pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_TIMED_NP);

    pthread_mutex_init(&sockobj->pthmutex,&mattr);
    pthread_mutexattr_destroy(&mattr);
    

    return sockobj;
}




//
// Description:
//    Frees a socket object along with any queued buffer objects.
//
void freesocketobj(struct socket_obj *obj)
{
  /*struct buffer_obj  *ptr=NULL,
                *tmp=NULL;

    ptr = obj->pending;
    while (ptr)
    {
        tmp = ptr;
        ptr = ptr->next;

        freebufferobj(tmp);
	}*/
    pthread_mutex_destroy(&obj->pthmutex);
    free(obj);
    
}

//
// Function: InsertSocketObj
//
// Description:
//    Insert a socket object into the list of socket objects. Note that
//    no synchronization is performed because this app is single threaded!
//
void insertsocketobj(struct socket_obj *sock)
{
    sock->next = sock->prev = NULL;
    if (gsocketlist == NULL)
    {
        // List is empty
        gsocketlist = gsocketlistend = sock;
    }
    else
    {
        // Non-empty; insert at the end
        sock->prev = gsocketlistend;
        gsocketlistend->next = sock;
        gsocketlistend = sock;

    }
    gsocketcount++;
}

//
// Description:
//    Remove a socket object from the list of sockets. No synchronization is
//    is performed since this app is single threaded.
//
void removesocketobj(struct socket_obj *sock)
{
    if (sock->prev)
    {
        sock->prev->next = sock->next;
    }
    if (sock->next)
    {
        sock->next->prev = sock->prev;
    }

    if (gsocketlist == sock)
        gsocketlist = sock->next;
    if (gsocketlistend == sock)
        gsocketlistend = sock->prev;
    
    gsocketcount--;
}







// Description:
//    Receive data pending on the socket into a struct socket_obj buffer. Enqueue
//    the buffer into the socket object for sending later. This routine returns
//    -1 indicating that the socket is no longer valid and the calling function
//    should clean up (remove) the socket object. Zero is returned for success.
//
int receivependingdata(struct socket_obj *sockobj)
{

    //printf("start received pending data\n");
    struct buffer_obj *buffobj=NULL;
    int         rc,
                ret;

    // Get a buffer to receive the data
    buffobj = getbufferobj(gbuffersize);

    ret = 0;

    if (gprotocol == IPPROTO_TCP)
    {
        rc = recv(
                sockobj->s,
                buffobj->buf,
                buffobj->buflen,
                0
                );
    }
    else 
    {
        rc = recvfrom(
                sockobj->s,
                buffobj->buf,
                buffobj->buflen,
                0,
                (struct sockaddr *)&buffobj->addr,
                (socklen_t *)&buffobj->addrlen
                );
    }
    if (rc == SOCKET_ERROR)
    {
        if (errno != EAGAIN)
        {
            // Socket connection has failed, close the socket
            fprintf(stderr, "recv(from) failed: %d\n", errno);

            close(sockobj->s);

            ret = -1;
        }
        freebufferobj(buffobj);
    }
    else if (rc == 0)
    {
        printf("the result of rc is 0");
        // Graceful close
        if (gprotocol == IPPROTO_TCP)
        {
            freebufferobj(buffobj);
        }
        else
        {
            buffobj->buflen = 0;
            buffobj->fd = sockobj->s;;
            enqueuebufferobjwithmutex(&receivedqueue,&receivedqueuetail,buffobj,FALSE,&threceivedbuffmutex);
            pthread_cond_signal(&receivedbuffnotify);
	    // enqueuebufferobj(sockobj, buffobj, FALSE);
        }

        sockobj->closing = TRUE;

        /*if (sockobj->pending == NULL)
        {
            // If no sends are pending, close the socket for good
            close(sockobj->s);

            ret = -1;
	    }*/
    }
    else
    {
        // Read data, updated the counters and enqueue the buffer for client
        gbytesread += rc;
        gbytesreadlast += rc;

        buffobj->buflen = rc;
        buffobj->fd = sockobj->s;
        
        enqueuebufferobjwithmutex(&receivedqueue,&receivedqueuetail,buffobj,FALSE,&threceivedbuffmutex);
        pthread_cond_signal(&receivedbuffnotify);
        //enqueuebufferobj(sockobj, buffobj, FALSE);
    }
    //printf("end received pending data \n");
    return ret;
}


//
// Description:
//    Send any data pending on the socket. This routine goes through the 
//    queued buffer objects within the socket object and attempts to
//    send all of them. If the send fails with EWOULDBLOCK, put the
//    remaining buffer back in the queue (at the front) for sending
//    later when select indicates sends can be made. This routine returns
//    -1 to indicate that an error has occured on the socket and the
//    calling routine should remove the socket structure; otherwise, zero
//    is returned.
//
int sendpendingdata(struct socket_obj *sock)
{
    struct buffer_obj *bufobj=NULL;
    int         breakouter;
    int         nleft,
                idx,
                ret,
                rc;

    // Attempt to dequeue all the buffer objects on the socket
    ret = 0;
    bufobj = dequeuebufferobjwithmutex(&sock->pending,&sock->pendingtail,&sock->pthmutex);
    while (bufobj != NULL )
    {
        if (gprotocol == IPPROTO_TCP)
        {
            breakouter = FALSE;

            nleft = bufobj->buflen;
            idx = 0;

            // In the event not all the data was sent we need to increment
            // through the buffer. This only needs to be done for stream
            // sockets since UDP is datagram and its all or nothing for that.
            while (nleft)
            {
                rc = send(
                        sock->s,
                       &bufobj->buf[idx],
                        nleft,
                        0
                        );
                if (rc == SOCKET_ERROR)
                {
                    if (errno == EAGAIN)
                    {
                        struct buffer_obj *newbuf=NULL;

                        // Copy the unsent portion of the buffer and put it back
                        // at the head of the send queue
                        newbuf = getbufferobj(nleft);
                        memcpy(newbuf->buf, &bufobj->buf[idx], nleft);

                        enqueuebufferobjwithmutex(&sock->pending,&sock->pendingtail, newbuf, TRUE,&sock->pthmutex);
                        pthread_cond_signal(&sendbuffnotify);
                    }
                    else
                    {
                        // The connection was broken, indicate failure
                        ret = -1;
                    }
                    breakouter = TRUE;
                    break;
                }
                else
                {
                    // Update the stastics and increment the send counters
                    gbytessent += rc;
                    gbytessentlast += rc;

                    nleft -= rc;
                    idx += 0;
                }
            }
            freebufferobj(bufobj);

            if (breakouter)
                break;
        }
        else
        {
            rc = sendto(
                    sock->s,
                    bufobj->buf,
                    bufobj->buflen,
                    0,
                    (struct sockaddr *)&bufobj->addr,
                    bufobj->addrlen
                    );
            if (rc == SOCKET_ERROR)
            {
                if (errno == EAGAIN)
                {
                    // If the send couldn't be made, put the buffer
                    // back at the head of the queue
		    enqueuebufferobjwithmutex(&sock->pending,&sock->pendingtail, bufobj, TRUE,&sock->pthmutex);
                    pthread_cond_signal(&sendbuffnotify);
                    ret = 0;
                }
                else
                {
                    // Socket error occured so indicate the error to the caller
                    ret = -1;
                }
                break;
            }
            else
            {
                freebufferobj(bufobj);
            }
        }
        bufobj = dequeuebufferobjwithmutex(&sock->pending,&sock->pendingtail,&sock->pthmutex);
    }
    // If no more sends are pending and the socket was marked as closing (the
    // receiver got zero bytes) then close the socket and indicate to the caller
    // to remove the socket structure.
    if ((sock->pending == NULL) && (sock->closing))
    {
        close(sock->s);
        ret = -1;

        printf("Closing connection\n");
	}
    return ret;
}


int initialrun()
{

    pthread_mutexattr_t        mattr;
    pthread_mutexattr_init(&mattr);
    //pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_TIMED_NP);

    pthread_mutex_init(&thsendbuffmutex,&mattr);
    pthread_mutex_init(&threceivedbuffmutex,&mattr);
    
    pthread_mutex_init(&thsendnotifymutex,&mattr);
    pthread_mutex_init(&threceivednotifymutex,&mattr);
    pthread_mutexattr_destroy(&mattr);


    pthread_cond_init(&sendbuffnotify,NULL);
    pthread_cond_init(&receivedbuffnotify,NULL);


    //init work thread;
    pthread_t workthreadid;
    if(pthread_create(&workthreadid,NULL,workthread,NULL) != 0)
      {
	fprintf(stderr,"start workthread error\n");
	return -1;
      }

    //initial send thread;
    pthread_t sendthread;
    if(pthread_create(&sendthread,NULL,sendprocess_thread,NULL) != 0)
      {
        fprintf(stderr,"start workthread error\n");
	return -1;
      }

    //initial received thread;
    pthread_t receivedthread;
    if(pthread_create(&receivedthread,NULL,receiveprocess_thread,NULL) != 0)
      {
        fprintf(stderr,"start workthread error\n");
	return -1;
      }
    return 0;
}

int  releaseallresource()
{
    bthreadexit = TRUE;
    return 0;
}
//
//Function initnetsocket
//
//initial socket, 
//
int initnetsocket(char *host,char *strip, char *port,int  isserver)
{

    struct addrinfo *res=NULL,
                    *ptr=NULL;

    //struct sockaddr_in servaddr;

    struct socket_obj      *socketobj;
    int             s = 0;            //for socket handle;
    int             rc = 0;
    if(host != NULL)
        printf("local host is %s",host);
    printf("Local address: %s:%s; Port: %s; Family: %d\n",host,
            strip, port, gaddressfamily);

    
    
    res = resolveaddress(strip, port, gaddressfamily, gsockettype, gprotocol);
    if (res == NULL)
    {
        fprintf(stderr, "ResolveAddress failed to return any addresses!\n");
        return -1;
    }

    // For each local address returned, create a listening/receiving socket
    ptr = res;
    while (ptr)
    {
       

        // create the socket
        s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (s == -1)
        {
            fprintf(stderr,"socket failed: %d\n", errno);
            return -1;
        }

        socketobj = getsocketobj(s, (gprotocol == IPPROTO_TCP) ? TRUE : FALSE);

        insertsocketobj(socketobj);

        if(isserver)
        {
        // bind the socket to a local address and port
            rc = bind(socketobj->s, ptr->ai_addr, ptr->ai_addrlen);
            if (rc == SOCKET_ERROR)
            {
                //const char *errorinfo = gai_strerror(rc);
                fprintf(stderr, "bind failed: %d\n",    errno);
                return -1;
            }

            if (gprotocol == IPPROTO_TCP)
            {
                rc = listen(socketobj->s, 200);
                if (rc == SOCKET_ERROR)
                {
                    fprintf(stderr, "listen failed: %d\n", errno);
                    return -1;
                }
            }
        }

        ptr = ptr->ai_next;
    }
    // free the addrinfo structure for the 'bind' address
    freeaddrinfo(res); 

    return s;
    


}

int sendnetdata(int fdsocket,struct sockaddr_in *addr,u_char *pbufferdata,int ibuffersize)
{
    int  breakouter = FALSE;
    int nleft = 0;
    int idx = 0;
    int rc = 0;
    int ret = 0;

    if(fdsocket == -1 || addr == NULL || pbufferdata == NULL)
      return -1;
    if(gprotocol == IPPROTO_TCP)
    {
         breakouter = FALSE;

         nleft = ibuffersize;
         idx = 0;

         // In the event not all the data was sent we need to increment
         // through the buffer. This only needs to be done for stream
         // sockets since UDP is datagram and its all or nothing for that.
         while (nleft)
         {
            rc = send(
                        fdsocket,
                        pbufferdata,
                        nleft,
                        0
                        );
            if (rc == SOCKET_ERROR)
            {
                if (errno == EAGAIN)
                {
                    struct buffer_obj *newbuf=NULL;
                    // Copy the unsent portion of the buffer and put it back
                    // at the head of the send queue
                    newbuf = getbufferobj(nleft);
                    memcpy(pbufferdata, &pbufferdata[idx], nleft);
                    newbuf->fd = fdsocket;
                    enqueuebufferobjwithmutex(&sendqueue,&sendqueuetail, newbuf, TRUE,&thsendbuffmutex);
                    pthread_cond_signal(&sendbuffnotify);
                }
                else
                {
                    // The connection was broken, indicate failure
                    ret = -1;
                }
                breakouter = TRUE;
                break;
            }
            else
            {
                // Update the stastics and increment the send counters
                gbytessent += rc;
                gbytessentlast += rc;

                nleft -= rc;
                idx += 0;
            }
	 }
    }
    else
    {
        rc = sendto(
                fdsocket,
                pbufferdata,
                ibuffersize,
                0,
                (struct sockaddr *)addr,
                sizeof(*addr)
                );
        if (rc == SOCKET_ERROR)
        {
            if (errno == EAGAIN)
            {
                // If the send couldn't be made, put the buffer
                // back at the head of the queue
	        struct buffer_obj  *buffobj;
                buffobj = getbufferobj(gbuffersize);
                buffobj->buf = malloc(ibuffersize);
                memcpy(buffobj->buf,pbufferdata,ibuffersize);
                buffobj->addr = *addr;
                buffobj->addrlen = sizeof(*addr);
                buffobj->fd = fdsocket;
                
                enqueuebufferobjwithmutex(&sendqueue,&sendqueuetail, buffobj, TRUE, &thsendbuffmutex);
                pthread_cond_signal(&sendbuffnotify);
                 ret = 0;
            }
            else
            {
                // Socket error occured so indicate the error to the caller
                ret = -1;
            }
            
        }
        
    }
    return ret;
}


void* receiveprocess_thread(void *para)
{
    struct buffer_obj  *buffobj;
    int  iflag;
    waitreceivesignal:
    pthread_mutex_lock(&threceivednotifymutex);
    pthread_cond_wait(&receivedbuffnotify,&threceivednotifymutex);
    //printf("have get a cond\n");
    buffobj = dequeuebufferobjwithmutex(&receivedqueue,&receivedqueuetail,&threceivedbuffmutex);
    while(buffobj != NULL)
    {
        if(buffobj == NULL)
            break;
        int iresult = gcallreceivedfuncbuff(buffobj->fd,buffobj,&iflag);
        if(iresult)
        {//
            
        }
	freebufferobj(buffobj);
        buffobj = NULL;
        buffobj = dequeuebufferobjwithmutex(&receivedqueue,&receivedqueuetail,&threceivedbuffmutex);
    }
    //printf("have finish a loop\n");
    pthread_mutex_unlock(&threceivednotifymutex);
    if(bthreadexit)
        goto endreceiveprocess;
    else
        goto waitreceivesignal;
    para = NULL;
    

endreceiveprocess:
      ;
    pthread_exit(0);
    return NULL;
}

 
//
//function:sendprocess
//the function is thread that for send data, loop sendbuffer queue ,get data
//and send to 
void* sendprocess_thread(void *para)
{//
    int ret = 0;
    int idx = 0; 
    int breakouter = TRUE;
    int nleft = 0;
    int rc = 0;

    struct buffer_obj *bufobj = NULL;
waitsendsignal:
    pthread_mutex_lock(&thsendnotifymutex);
    pthread_cond_wait(&sendbuffnotify,&thsendnotifymutex);
    bufobj = dequeuebufferobjwithmutex(&sendqueue,&sendqueuetail,&thsendbuffmutex);
    while (bufobj != NULL)
    {
        
        if (gprotocol == IPPROTO_TCP)
        {
            breakouter = FALSE;

            nleft = bufobj->buflen;
            idx = 0;

            // In the event not all the data was sent we need to increment
            // through the buffer. This only needs to be done for stream
            // sockets since UDP is datagram and its all or nothing for that.
            while (nleft)
            {
                rc = send(
                        bufobj->fd,
                       &bufobj->buf[idx],
                        nleft,
                        0
                        );
                if (rc == SOCKET_ERROR)
                {
                    if (errno == EAGAIN)
                    {
                        

		      continue;

		      //enqueuebufferobjwithmutex(*sendqueue,*sendqueuetail, newbuf, TRUE,&thsendbuffmutex);
                    }
                    else
                    {
                        // The connection was broken, indicate failure
                        ret = -1;
                    }
                    breakouter = TRUE;

                    break;
                }
                else
                {
                    // Update the stastics and increment the send counters
                    gbytessent += rc;
                    gbytessentlast += rc;

                    nleft -= rc;
                    idx += 0;
                }
            }
            freebufferobj(bufobj);

            if (breakouter)
                break;
        }
        else
        {
            rc = sendto(
                    bufobj->fd,
                    bufobj->buf,
                    bufobj->buflen,
                    0,
                    (struct sockaddr *)&bufobj->addr,
                    bufobj->addrlen
                    );
            if (rc == SOCKET_ERROR)
            {
                if (errno == EAGAIN)
                {
                    // If the send couldn't be made, put the buffer
                    // back at the head of the queue
                    continue;
                    //enqueuebufferobjwithmutex(*sendqueue,*sendqueuetail, newbuf, TRUE,&thsendbuffmutex);

                    ret = 0;
                }
                else
                {
                    // Socket error occured so indicate the error to the caller
                    ret = -1;
                }
                break;
            }
            else
            {
                freebufferobj(bufobj);
            }
        }
        bufobj = dequeuebufferobjwithmutex(&sendqueue,&sendqueuetail,&thsendbuffmutex);
    }

    pthread_mutex_unlock(&thsendnotifymutex);

    if(bthreadexit)
      goto endsendprocess;
    else
      goto waitsendsignal;

endsendprocess:
	;
    pthread_exit(0); 
    para = NULL;
    return NULL;
}



//
// Function: workthread
//
// Description:
//      This is the main program. It parses the command line and creates
//      the main socket. For UDP this socket is used to receive datagrams.
//      For TCP the socket is used to accept incoming client connections.
//      Each client TCP connection is handed off to a worker thread which
//      will receive any data on that connection until the connection is
//      closed.
//
void*  workthread(void *para)
{
    
    int              s;
    struct socket_obj       *sockobj=NULL,
                     *sptr=NULL,
                     *tmp=NULL;
    int              rc;
    fd_set           fdread,
                     fdwrite,
                     fdexcept;
    struct timeval   timeout;

    para = NULL;
  

    while (1)
    {
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcept);

        sptr = gsocketlist;

        // Set each socket in the FD_SET structures
        while (sptr)
        {
            FD_SET(sptr->s, &fdread);
            FD_SET(sptr->s, &fdwrite);
            FD_SET(sptr->s, &fdexcept);

            sptr = sptr->next;
        }

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        //fprintf(stderr,"enter select function\n");
        rc = select(MAX_CONNECT_NUM, &fdread,NULL, &fdexcept, &timeout);
        //fprintf(stderr,"the result of rc is %d\n",rc);
        if (rc == SOCKET_ERROR)
        {
            fprintf(stderr, "select failed: %d\n", errno);
            return NULL;
        }
        else if (rc == 0)
        {
            // timeout
            //intStatistics();
        }
        else
        {
            // Go through all the socket and see if they're present in the
            // fd_set structures.
            sptr = gsocketlist;
            while (sptr)
            {
                if (FD_ISSET(sptr->s, &fdread))
                {
                    if (sptr->listening)
                    {
                        // Read is indicated on a listening socket, accept the connection
                        sockobj = getsocketobj(INVALID_SOCKET, FALSE);

                        s = accept(sptr->s, (struct sockaddr *)&sockobj->addr, (socklen_t *)&sockobj->addrlen);
                        if (s == INVALID_SOCKET)
                        {
                            fprintf(stderr, "accept failed: %d\n", errno);
                            return NULL;
                        }

                        //InterlockedIncrement(&gCurrentConnections);

                        sockobj->s = s;
                        
                        

                        insertsocketobj(sockobj);
                    }
                    else
                    {
                        // Read is indicated on a client socket, receive data
                        if (receivependingdata(sptr) != 0)
                        {
                            printf("receivependingdata indicated to remove obj\n");
                            tmp = sptr;
                            sptr = sptr->next;

                            removesocketobj(tmp);
                            freesocketobj(tmp);

                            // At the end of the list
                            if (sptr == NULL)
                                continue;
                        }

                        // Attempt to send pending data
                        if (sendpendingdata(sptr) != 0)
                        {
                            tmp = sptr;
                            sptr = sptr->next;

                            removesocketobj(tmp);
                            freesocketobj(tmp);

                            // At the end of the list
                            if (sptr == NULL)
                                continue;
                        }
                    }
                }
                if (FD_ISSET(sptr->s, &fdwrite))
                {
                    // Write is indicated so attempt to send the pending data
                    if (sendpendingdata(sptr) != 0)
                    {
                        tmp = sptr;
                        sptr = sptr->next;

                        removesocketobj(tmp);
                        freesocketobj(tmp);

                        // At the end of the list
                        if (sptr == NULL)
                            continue;
                    }
                }
                if (FD_ISSET(sptr->s, &fdexcept))
                {
                    // Not handling OOB data so just close the connection
                    tmp = sptr;
                    sptr = sptr->next;

                    removesocketobj(tmp);
                    freesocketobj(tmp);

                    // At the end of the list
                    if (sptr == NULL)
                        continue;
                }

                sptr = sptr->next;
            }
        }
        
    }
    pthread_exit(0);

    return NULL;
}
