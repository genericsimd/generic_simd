/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ulmap_queue.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 18-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_bs_ulmap_queue.h"
#include "mac_assistfunc.h"


struct frame_ul_map_msg       *g_ulmap_start,*g_ulmap_end;

/*
 *function : find_ul_map_msg
*find data node by iframenum,when have any data node equeue the iframenum ,then return 0, else return -1;
*/
int find_ul_map_msg(int iframenum,struct frame_ul_map_msg **pp_getulmap)
{
    if(g_ulmap_start == NULL && g_ulmap_end == NULL)
        return -1;

    struct frame_ul_map_msg    *p_findframe = g_ulmap_start;

    while(p_findframe != NULL)
      {
	if(p_findframe->iframenum == iframenum)
	{
            *pp_getulmap = p_findframe;
	    return 0;
	}
	if(p_findframe == g_ulmap_end)
	  return -1;

      }
    //didn't find;
    return -1;
}

int enqueue_ul_map_msg_by_framenum(int iframenum,ul_map_msg **p_ul_map_msg)
{

    
    struct frame_ul_map_msg    *p_newframe = NULL;
 
    ERROR_RETURN(find_ul_map_msg(iframenum,&p_newframe),0,"have one frame num in queue please check the framenum or ul_map_msg information\n");


    p_newframe = (struct frame_ul_map_msg *) malloc(sizeof(struct frame_ul_map_msg));
    ERROR_RETURN(p_newframe,NULL,"allocate memory failed in enqueue_ul_map_msg_by_framenum\n");
    p_newframe->iframenum = iframenum;
    p_newframe->p_map_msg = *p_ul_map_msg;
 

    if(g_ulmap_start == NULL && g_ulmap_end == NULL)
    {
        g_ulmap_start =  g_ulmap_end = p_newframe;
         return 0;
    }

    g_ulmap_end->next = p_newframe;
    g_ulmap_end = p_newframe;
 
    return 0;
}

int dequeue_ul_map_msg_by_framenum(int iframenum,ul_map_msg **pp_ul_map_msg)
{

    struct frame_ul_map_msg  *p_find_ul_map_frame = NULL;
    ERROR_RETURN(find_ul_map_msg(iframenum,&p_find_ul_map_frame),-1,"couldn't find frame node in queue\n");

    
    *pp_ul_map_msg = p_find_ul_map_frame->p_map_msg;

    return 0;
    
}

int remove_ul_map_msg_by_framenum(int iframenum,ul_map_msg **pp_ul_map_msg)
{
    struct frame_ul_map_msg  *p_find_ul_map_frame = NULL;
    ERROR_RETURN(find_ul_map_msg(iframenum,&p_find_ul_map_frame),-1,"couldn't find frame node in queue\n");

    struct frame_ul_map_msg    *p_recordframe = g_ulmap_start,*p_preframe = NULL;
    while(p_recordframe != NULL && p_recordframe != g_ulmap_end)
    {
      if(p_recordframe->iframenum == iframenum)
      {
          p_preframe->next = p_recordframe->next;
          *pp_ul_map_msg = p_recordframe->p_map_msg;
          return 0;
      }
      p_preframe = p_recordframe;
      p_recordframe = p_recordframe->next;
    }
    return -1;
}
