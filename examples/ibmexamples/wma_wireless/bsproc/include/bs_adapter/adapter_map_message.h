#ifndef     _ADAPTER_MAP_MESSAGE_H_
#define     _ADAPTER_MAP_MESSAGE_H_

#define     BEGIN_MAP_PROCESS()               \
                                      int    process_map_message(int  fd,int port){\
                                              int ireturn = 0;
#define     PROCESS_HANDLE(ifd,iport,fun)     \
                                              if(fd = ifd )  \
                                                  return fun(iport);      
#define     END_MAP_PROCESS()                 \
                                              return ireturn;\
					      }
#endif
