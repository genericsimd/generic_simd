/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: memmgmt.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdlib.h>
#include <syscall.h>
//     #include <sys/syscall.h>
#include <unistd.h>
#include <assert.h>
#include "memmgmt.h"
#include "debug.h"
#include "log.h"


int app_malloc(void** ptr, size_t bytes) {

//#if(LOG_LEVEL == LOG_LEVEL_DEBUG)
//  pid_t tid = syscall(SYS_gettid);
//#endif
  
  *ptr = malloc (bytes);
  if(*ptr!=NULL) {
    return 0;
  }
  else {
    return -1;
  }
}

int app_free(void *ptr, size_t len) {
  assert (ptr != NULL);
//#if(LOG_LEVEL == LOG_LEVEL_DEBUG)
//  pid_t tid = syscall(SYS_gettid);
//#endif
  
  free (ptr);
  return 0;
}

int app_realloc(void** ptr, size_t bytes) {

//#if(LOG_LEVEL == LOG_LEVEL_DEBUG)
//  pid_t tid = syscall(SYS_gettid);
//#endif
  
  *ptr = realloc (*ptr, bytes);
  if(*ptr!=NULL) {
    return 0;
  }
  else {
    return -1;
  }
}

void* mac_malloc(size_t num_bytes) {
  FLOG_DEBUG("Calling mac_malloc ...\n");
  return malloc(num_bytes);
}

void mac_free(size_t num_bytes, void* mem) {
  FLOG_DEBUG("Calling mac_free()...\n");
  free(mem);
}

void *WiMAX_mac_malloc (size_t bytes)
{
	void *rgn;

//#if(LOG_LEVEL == LOG_LEVEL_DEBUG)
	pid_t tid = syscall(SYS_gettid);
//#endif

	FLOG_DEBUG("About to ALLOCATE %d bytes to thread %d\n", bytes, tid);
	rgn = malloc (bytes);
	assert (rgn != NULL);
	FLOG_DEBUG("ALLOCED %d bytes at %x to thread %d\n", bytes, rgn, tid);
	return rgn;
}

void *WiMAX_mac_calloc (int n, size_t bytes)
{
	void *rgn;
//#if(LOG_LEVEL == LOG_LEVEL_DEBUG)
	pid_t tid = syscall(SYS_gettid);
//#endif

	FLOG_DEBUG("About to ALLOCATE %d objects %d bytes each to thread %d\n", n, bytes, tid);
	rgn = calloc (n, bytes);
	assert (rgn != NULL);
	FLOG_DEBUG("ALLOCED %d objects %d bytes each at %x to thread %d\n", n, bytes, rgn, tid);
	return rgn;
}

void WiMAX_mac_free (void *ptr)
{
	assert (ptr != NULL);
//#if(LOG_LEVEL == LOG_LEVEL_DEBUG)
	pid_t tid = syscall(SYS_gettid);
//#endif

	FLOG_DEBUG("About to FREE: %x of thread %d\n", ptr, tid);
	free (ptr);
	FLOG_DEBUG("FREED: %x of thread %d\n", ptr, tid);
}

//This method is used to allocate memory for transport sdus
//class_type is currently unused
// class type in the future can an enum specifying
// the service classes of the transport or other app characteristics
// that can be exploited during mem mgmt.
char tempmemval='\0';
void* mac_sdu_malloc(size_t num_bytes, short class_type) {
  TRACE(10,"Calling mac_sdu_malloc() ...");
#ifdef LOG_METRICS
  void* ptr=malloc(num_bytes+sizeof(sdu_mem_tag)); 
  if(ptr!=NULL) {
    //update the end of the allocated memory with value of ptr
    sdu_mem_tag* bt=(sdu_mem_tag*)(ptr+num_bytes);
    bt->sdu_start_location=ptr;
    bt->class_type=class_type;
    FLOG_DEBUG("In mac_sdu_malloc: %p %p\n", ptr, bt);
    TRACE4(6,"sdu_start_location is:%p:%p class_type is:%d:%d\n", ptr, bt->sdu_start_location, class_type, bt->class_type);
    LOG_EVENT_sdu_allocated(readtsc(), ptr, class_type);
  }
  return ptr;
#else
  void* ptr=malloc(num_bytes+4+sizeof(ptr)); 
  if(ptr!=NULL) {
    //update the end of the allocated memory with value of ptr
    void* boundary=(void*)ptr+num_bytes;
	FLOG_DEBUG("In mac_sdu_malloc: %x %x\n", ptr, boundary);
	*(char*)boundary=num_bytes%4;
	if (num_bytes%4==0) boundary=(char*)boundary+4;
	if (num_bytes%4==1) boundary=(char*)boundary+3;
	if (num_bytes%4==2) boundary=(char*)boundary+2;	
	if (num_bytes%4==3) boundary=(char*)boundary+1;	
/*	if (num_bytes%4==1) {*(char*)boundary=tempmemval;boundary=(char*)boundary+1;*(char*)boundary=tempmemval;boundary=(char*)boundary+1;*(char*)boundary=tempmemval;boundary=(char*)boundary+1;}	
	if (num_bytes%4==2) {*(char*)boundary=tempmemval;boundary=(char*)boundary+1;*(char*)boundary=tempmemval;boundary=(char*)boundary+1;}	
	if (num_bytes%4==3) {*(char*)boundary=tempmemval;boundary=(char*)boundary+1;}	
*/
		 (*(char**)boundary)=(char*)ptr;
	//printf("size of this pointer is %d bytes\n",sizeof(ptr));	
	//printf("ptr value %p boundary value %p num_bytes %d\n",ptr,boundary,num_bytes);
  }
  return ptr;
#endif
}

//This function is used to free transport sdu memory
int mac_sdu_free(void* mem, size_t num_bytes, fragment_type ftype) {
  TRACE(10,"Calling mac_sdu_free() ...");
#ifdef LOG_METRICS
  if((ftype==LAST_FRAGMENT) || (ftype==NO_FRAGMENTATION)) {
    sdu_mem_tag* bt=(sdu_mem_tag*)(mem+num_bytes);
    void* ptr=bt->sdu_start_location;
    short class_type=bt->class_type;
    FLOG_DEBUG("FREE: sdu_start_location is: %p:%p class_type is:%d:%d \n", bt->sdu_start_location, ptr, bt->class_type, class_type);
    FLOG_DEBUG("Freeing %p within mac_sdu_free ...\n", ptr);
    LOG_EVENT_sdu_freed(readtsc(), ptr, class_type);
    free(ptr);
  }
  else {
    //Do nothing
  }
#else
  if((ftype==LAST_FRAGMENT) || (ftype==NO_FRAGMENTATION)) {
	char *ptr1=mem+num_bytes;
	switch(*ptr1)
	{
	case 0:ptr1=ptr1+4;break;
 	case 1:ptr1=ptr1+3;break;
	case 2:ptr1=ptr1+2;break; 
	case 3:ptr1=ptr1+1;break; 
	default: break;
	}	
//	while (*ptr1==tempmemval) ptr1=ptr1+1;	
	//if (*ptr1==tempmemval) {printf("%p\n",*ptr1);ptr1=ptr1+1;}
	void* ptr=(void*)(*(char**)ptr1);	
	//void* ptr=(void*)(*((char**)(mem+num_bytes)));
	FLOG_DEBUG("Freeing %0x within mac_sdu_free ...\n", ptr);
    free(ptr);
  }
  else {
    //Do nothing
  }
#endif
  return 0;
}

