#ifndef           _TYPES_H_
#define           _TYPES_H_

#define  _LINUX_

//#define           FALSE     1
//#define           TRUE      0


#ifdef     _LINUX_
#define    SOCKET        int
#endif


#ifdef     _LINUX_
#define    INVALID_SOCKET         -1
#endif




#define SOCKET_ERROR      -1

typedef unsigned char u_char;

struct buffer_obj
{
    unsigned char    *buf;          // Data buffer for data
    int              buflen;        // Length of buffer or number of bytes contained in buffer

    struct sockaddr_in      addr;          // Address data was received from 
    int              addrlen;       // Length of address

    int              fd;            //handle for process handle;

    struct buffer_obj      *next;   // Used to maintain a linked list of buffers
};

//
// Allocated for each socket handle
//
struct socket_obj
{
    SOCKET             s;              // Socket handle
    int                listening;      // Socket is a listening socket (TCP)
    int                closing;        // Indicates whether the connection is closing

    struct sockaddr_in addr;      // Used for client's remote address
    int                addrlen;   // Length of the address

    pthread_mutex_t    pthmutex; 


    struct buffer_obj         *pending;
    struct buffer_obj         *pendingtail;


    struct socket_obj         *next,   // Used to link socket objects together
                              *prev;

} ;




#endif
