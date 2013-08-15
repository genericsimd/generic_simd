/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_common_data.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_common_data.h"
#include "free_maps.h"

extern int param_DL_MCS;  // Modulation and Coding type for DL and UL
extern int param_UL_MCS;

int init_stored_ulmap_list()
{
  int ii;
  for(ii = 0; ii < NUM_ULMAP_STORED; ii++)
    {
      stored_ulmap_list.ulmap[ii] = NULL;
      stored_ulmap_list.frame_num[ii] = -1;
    }
	
  stored_ulmap_list.last_stored_index = -1;
  return 0;
}

int free_stored_ulmap_list()
{
  int ii;
  for(ii = 0; ii < NUM_ULMAP_STORED; ii++)
    {
      if(stored_ulmap_list.ulmap[ii] != NULL)
	{
	   // This frees the list of IE's inside the ULMAP
           free_ulmap(stored_ulmap_list.ulmap[ii]);
           // Free the ULMAP container structure
           mac_free(sizeof(ul_map_msg), stored_ulmap_list.ulmap[ii]);
	}
    }
	
  return 0;
}


int set_ul_map_msg(ul_map_msg* ulmap, long long int frame_num)
{
  stored_ulmap_list.last_stored_index = (stored_ulmap_list.last_stored_index + 1)%NUM_ULMAP_STORED;

  // If a previous ulmap is already stored in this location, free it
  if (stored_ulmap_list.ulmap[stored_ulmap_list.last_stored_index] != NULL)
    {
  //    printf("Freeing ULMAP for frame num: %ld", stored_ulmap_list.frame_num[stored_ulmap_list.last_stored_index]);
      // This frees the list of IE's inside the ULMAP
      free_ulmap(stored_ulmap_list.ulmap[stored_ulmap_list.last_stored_index]);
      // Free the ULMAP container structure
      mac_free(sizeof(ul_map_msg), stored_ulmap_list.ulmap[stored_ulmap_list.last_stored_index]);
    }

  // Now store the new ULMAP in this location
  stored_ulmap_list.ulmap[stored_ulmap_list.last_stored_index] = ulmap;
  stored_ulmap_list.frame_num[stored_ulmap_list.last_stored_index] = frame_num;

  return 0;
}

int get_ul_map_msg(ul_map_msg** ulmap, long long int frame_num)
{
  int ii;
  for (ii=0; ii < NUM_ULMAP_STORED; ii++)
    {
      if (stored_ulmap_list.frame_num[ii] == frame_num)
	{
	  *ulmap = stored_ulmap_list.ulmap[ii];
	  return 0;
	}
    }
  return -1;
}

int init_stored_dlmap_list()
{
  int ii;
  for(ii = 0; ii < NUM_DLMAP_STORED; ii++)
    {
      stored_dlmap_list.dlmap[ii] = NULL;
      stored_dlmap_list.frame_num[ii] = -1;
    }
	
  stored_dlmap_list.last_stored_index = -1;
  return 0;
}

int free_stored_dlmap_list()
{
  int ii;
  for(ii = 0; ii < NUM_DLMAP_STORED; ii++)
    {
      if(stored_dlmap_list.dlmap[ii] != NULL)
	{
	   // This frees the list of IE's inside the DLMAP
           free_dlmap(stored_dlmap_list.dlmap[ii]);
           // Free the DLMAP container structure
           mac_free(sizeof(dl_map_msg), stored_dlmap_list.dlmap[ii]);
	}
    }
  return 0;
}

int set_dl_map_msg(dl_map_msg* dlmap, long long int frame_num)
{
  stored_dlmap_list.last_stored_index = (stored_dlmap_list.last_stored_index + 1)%NUM_DLMAP_STORED;

  // If a previous ulmap is already stored in this location, free it
  if (stored_dlmap_list.dlmap[stored_dlmap_list.last_stored_index] != NULL)
    {
      // This frees the list of IE's inside the ULMAP
      free_dlmap(stored_dlmap_list.dlmap[stored_dlmap_list.last_stored_index]);
      // Free the ULMAP container structure
      mac_free(sizeof(dl_map_msg), stored_dlmap_list.dlmap[stored_dlmap_list.last_stored_index]);
    }

  // Now store the new ULMAP in this location
  stored_dlmap_list.dlmap[stored_dlmap_list.last_stored_index] = dlmap;
  stored_dlmap_list.frame_num[stored_dlmap_list.last_stored_index] = frame_num;
  return 0;
}

int get_dl_map_msg(dl_map_msg** dlmap, long long int frame_num)
{
  int ii;
  for (ii=0; ii < NUM_DLMAP_STORED; ii++)
    {
      if (stored_dlmap_list.frame_num[ii] == frame_num)
	{
	  *dlmap = stored_dlmap_list.dlmap[ii];
	  return 0;
	}
    }
  return -1;
}

/*
  int initialize_system_mapq(system_map_queue** mapq)
  {
  (*mapq) = (system_map_queue*) malloc(sizeof(system_map_queue));
  (*mapq)->frame_num = 0;
  (*mapq)->maps_header = NULL;
  }

  int enqueue_system_map(system_map_queue* mapq, system_map_info* map_info)
  {
  system_map_info* map_head;
  system_map_info* pre_map;

  map_head = mapq->maps_header;
  pre_map = NULL;
  if (map_head == NULL)
  {
  mapq->maps_header = map_info;
  mapq->maps_tail = map_info;
  }
  else
  {
  if (!map_info->dcd)
  {
  map_info->dcd = (dcd_msg*) malloc(sizeof(dcd_msg));
  memcpy(map_info->dcd, mapq->tail->dcd, sizeof(dcd_msg));
  }
        
  if (!map_info->ucd)
  {
  map_info->ucd = (ucd_msg*) malloc(sizeof(ucd_msg));
  memcpy(map_info->ucd, mapq->tail->ucd, sizeof(ucd_msg));
  }

  mapq->tail->next = map_info;

  mapq->tail = map_info;
  map_info->next = NULL;
  }
  (*mapq)->frame_num++;
  return 0;
  }

  int dequeue_system_map(system_map_queue* mapq, int frame_no, system_map_info** map_info)
  {
  system_map_info* 
  return 0;
  }

  int release_system_mapq(system_map_queue* mapq)
  {
    
  return 0;
  }
*/
int initialize_common_data(){
  cur_frame_num = 0;
  dcd = NULL;
  ucd = NULL;
  dl_map = NULL;
  ul_map = NULL;
  return 0;
}

unsigned int get_current_frame_num(){
  cur_frame_num= (cur_frame_num +1) % FRAME_NUMBER_MOD;
  return (cur_frame_num);
}



int set_ucd_msg(ucd_msg* ucd)
{  
  ul_burst_profile* ulbp_temp;
  int ccc = ucd->configuration_change_count;
  if(stored_ucd[ccc] != NULL)
    {
    ulbp_temp = stored_ucd[ccc]->profile_header;
    while(ulbp_temp != NULL)
    {
      stored_ucd[ccc]->profile_header = ulbp_temp->next;
      free(ulbp_temp);
      ulbp_temp = stored_ucd[ccc]->profile_header;
    }
    free(stored_ucd[ccc]);
    }
  stored_ucd[ccc] = ucd;
  return 0;
}

int set_dcd_msg(dcd_msg* dcd)
{
  dl_burst_profile* dlbp_temp;
  int ccc = dcd->configuration_change_count;
  if(stored_dcd[ccc] != NULL)
    {
    dlbp_temp = stored_dcd[ccc]->profile_header;
    while(dlbp_temp != NULL)
    {
      stored_dcd[ccc]->profile_header = dlbp_temp->next;
      free(dlbp_temp);
      dlbp_temp = stored_dcd[ccc]->profile_header;
    }
    free(stored_dcd[ccc]);
    }
  stored_dcd[ccc] = dcd;
  return 0;
}

int free_rcvd_ucd_dcd_arr()
{
  int ii;
  ul_burst_profile* ulbp_temp;
  dl_burst_profile* dlbp_temp;
  for(ii = 0; ii < CONFIG_COUNT; ii++)
  {
    if(stored_dcd[ii] != NULL)
    {
      dlbp_temp = stored_dcd[ii]->profile_header;
      while(dlbp_temp != NULL)
      {
        stored_dcd[ii]->profile_header = dlbp_temp->next;
        free(dlbp_temp);
		dlbp_temp = stored_dcd[ii]->profile_header;
      }
      free(stored_dcd[ii]);
    }
    if(stored_ucd[ii] != NULL)
    {
      ulbp_temp = stored_ucd[ii]->profile_header;
      while(ulbp_temp != NULL)
      {
        stored_ucd[ii]->profile_header = ulbp_temp->next;
        free(ulbp_temp);
		ulbp_temp = stored_ucd[ii]->profile_header;
      }
      free(stored_ucd[ii]);
    }
  }
  return 0;
}

int init_rcvd_ucd_dcd_arr()
{
  int ii;
  // Initialize the first UCD and DCD structure - needed for the 1st frame at the SS
  ucd_msg *ucd_ptr = (ucd_msg*)mac_malloc(sizeof(ucd_msg));
  ucd_ptr->management_message_type = (u_int8_t)0;
  ucd_ptr->configuration_change_count = (u_int8_t)0; 

  ul_burst_profile *ulbp1 = (ul_burst_profile*)mac_malloc(sizeof(ul_burst_profile));
  ucd_ptr->profile_header = ulbp1; 

  ulbp1->type = (u_int8_t)1;                                          
  // TODO: In future have automatic calculation of Length according to the value field including all embedded TLVs
  ulbp1->length = (u_int8_t)UCD_BURST_PROFILE_LENGTH;                 
  ulbp1->uiuc = (u_int8_t)DEFAULT_UIUC;
    
  ulbp1->fec_code_modulation.type = (u_int8_t)150;                    
  ulbp1->fec_code_modulation.length = (u_int8_t)1;                    
  ulbp1->fec_code_modulation.value = (u_int8_t)param_UL_MCS;  

  stored_ucd[0] = ucd_ptr;
  
  dcd_msg *dcd_ptr = (dcd_msg*)mac_malloc(sizeof(dcd_msg));
  dcd_ptr->management_message_type = (u_int8_t)1;
  dcd_ptr->configuration_change_count = (u_int8_t)0; 

  dl_burst_profile *dlbp1 = (dl_burst_profile*)mac_malloc(sizeof(dl_burst_profile));
  dcd_ptr->profile_header = dlbp1; 

  dlbp1->type = (u_int8_t)1;                                          
  // TODO: In future have automatic calculation of Length according to the value field including all embedded TLVs
  dlbp1->length = (u_int8_t)DCD_BURST_PROFILE_LENGTH;                 
  dlbp1->diuc = (u_int8_t)DEFAULT_DIUC;
    
  dlbp1->fec_code_modulation.type = (u_int8_t)150;                    
  dlbp1->fec_code_modulation.length = (u_int8_t)1;                    
  dlbp1->fec_code_modulation.value = (u_int8_t)param_DL_MCS;  

  stored_dcd[0] = dcd_ptr;
  
  // Initialize the rest of the UCD/DCD to NULL
  for(ii = 1; ii < CONFIG_COUNT; ii++)
    {
      stored_dcd[ii] = NULL;
      stored_ucd[ii] = NULL;
    }

  return 0;
}

ucd_msg* get_ucd_msg(int query_ucd_ccc)
{
  if((query_ucd_ccc >= 0) && (query_ucd_ccc <= 255))
  {
    return(stored_ucd[query_ucd_ccc]);
  }
  else
  {
    FLOG_WARNING("WARNING: illegal UCD query. \n");
    return(NULL);
  }
}

dcd_msg* get_dcd_msg(int query_dcd_ccc)
{
  if((query_dcd_ccc >= 0) && (query_dcd_ccc <= 255))
  {
    return(stored_dcd[query_dcd_ccc]);
  }
  else
  {
    FLOG_WARNING("WARNING: illegal UCD query. \n");
    return(NULL);
  }
}

int release_common_data()
{
  return 0;
}
