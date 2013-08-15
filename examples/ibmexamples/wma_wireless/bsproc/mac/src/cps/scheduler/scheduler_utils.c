/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: scheduler_utils.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "scheduler_utils.h"

int estimate_sdu_PnF_overhead(int cid, sdu_cid_queue* sdu_cid_q, int data_bytes_needed)
{
 int pdu_overhead = 0;
 int is_encrypted=-1;
 is_encrypt_enabled(cid,(u_int8_t*)&is_encrypted);
 if (is_encrypted) pdu_overhead = PDU_OH_WITH_ENCRYPT;
 else pdu_overhead = PDU_MIN_OVERHEAD; 
  int total_bytes_needed = 0, num_sdu = 0, num_pdu = 0, arq_blk_size = 0;
  int num_blks_per_pdu = 0, max_blks_per_sdu = 0, max_sdu_size = 0, min_sdu_size = 0;
  int num_psh = 0, num_frag = 0, pdu_size = 0;

  u_int8_t is_frag;
  u_int8_t is_pack;
  u_int8_t is_arq;
  int psh_len = 0, fsh_len = 0;

  // This function peeks in the SDU CID queue and tells the number of SDUs
  // in data_bytes_needed. While traversing, it also calculates the min and
  // max SDU size in this set. These are used for overhead estimation
  peek_cid_queue(sdu_cid_q, cid, data_bytes_needed, &num_sdu, &min_sdu_size, &max_sdu_size);
  pdu_size = get_mac_pdu_size(cid);

  // get the basic attribute of this connection
  is_frag_enabled(cid, &is_frag);
  is_pack_enabled(cid, &is_pack);
  is_arq_enabled(cid, &is_arq);

  // initialize subheader lengths according to connection attributes
  init_subheader_len(cid, is_frag, is_arq, &psh_len, &fsh_len);

  // Can't predict overhead for Fragmentation & Packing without knowing 
  // exact SDU sizes. Assume worst case overheads for each of the scenarios
  if (is_arq)
    {
      arq_blk_size = get_blk_size(cid);
      if (is_frag)
	{
	  num_blks_per_pdu = floor((pdu_size - pdu_overhead)/(arq_blk_size+psh_len));
	  max_blks_per_sdu = ceil((float)max_sdu_size/arq_blk_size);
          if (is_pack)
	    {
	      // ARQ=TRUE, FRAG=TRUE, PACK=TRUE
	      num_pdu = ceil((float)num_sdu*max_blks_per_sdu/num_blks_per_pdu);
	      // MC CHANGED
	      //num_frag = ceil((float)max_blks_per_sdu/num_blks_per_pdu)*num_sdu; 
	      int num_sdu_frag_per_pdu= ceil((float)num_blks_per_pdu/max_blks_per_sdu);
	      int num_full_sdu_per_pdu= floor((float)num_blks_per_pdu/max_blks_per_sdu);
	      if(num_full_sdu_per_pdu >num_sdu) {
		num_frag=num_sdu;
	      }
	      else if(num_full_sdu_per_pdu==0) {
		num_frag=ceil((float)max_blks_per_sdu/num_blks_per_pdu)*num_sdu;
	      }
	      else {
		num_frag=(num_sdu/num_full_sdu_per_pdu)*num_sdu_frag_per_pdu + (num_sdu%num_full_sdu_per_pdu);
	      }
	      // MC CHANGED
	      total_bytes_needed = data_bytes_needed + (num_pdu * pdu_overhead) + (num_frag * psh_len);
	    }
	  else
	    {
	      // ARQ=TRUE, FRAG=TRUE, PACK=FALSE
	      total_bytes_needed = data_bytes_needed + (num_sdu*ceil((float)max_blks_per_sdu/num_blks_per_pdu)*(pdu_overhead + fsh_len));
	    }
	}
      else
	{
          if (is_pack)                                             
            {                                   
	      // ARQ=TRUE, FRAG=FALSE, PACK=TRUE
	      if (max_sdu_size + pdu_overhead > pdu_size)
		{
		  FLOG_FATAL("Error: SDU size exceeds PDU size on Fragmentation-disabled connection %d\n", cid);
		  // Can't serve this CID, go to next
		  return -1;
		}

	      // Worst case: Each SDU has its PSH. Assuming sending PDUs
	      // smaller than PDU size is ok and no space is wasted becaus
	      // of empty fragments. Not 100% accurate worst case. Might
	      // need >num_pdu PDUs because of sending smaller PDUs
	      num_pdu = ceil((float)(data_bytes_needed + psh_len*num_sdu)/(pdu_size - pdu_overhead));
	      total_bytes_needed = data_bytes_needed + (psh_len*num_sdu)  + (num_pdu * pdu_overhead);
            }
          else                                                                 
            {                                                                  
	      // ARQ=TRUE, FRAG=FALSE, PACK=FALSE. 100% accurate
	      // Each SDU goes in a separate PDU, and no subheaders
	      total_bytes_needed = data_bytes_needed + num_sdu * (pdu_overhead+fsh_len);
            }   
	}
    }
  else
    {
      if (is_frag)                                                 
	{ 
	  if (is_pack)                                             
	    {                                                                  
	      // ARQ=FALSE, FRAG=TRUE, PACK=TRUE
	      // Worst Case: When large packets necessitate > 1 PSH per SDU
	      // As P&F both enabled, each PDU can be exactly pdu_size (incl OH)
	      num_psh = ceil((float)max_sdu_size/(pdu_size-pdu_overhead))+1;
	      num_pdu = ceil((float)(data_bytes_needed + psh_len * num_sdu * num_psh)/(pdu_size - pdu_overhead));
	      total_bytes_needed = data_bytes_needed + (psh_len * num_sdu * num_psh) + (num_pdu * pdu_overhead);
	    }
	  else                                                                 
	    {                                                                  
	      // ARQ=FALSE, FRAG=TRUE, PACK=FALSE
	      // Worst case: each SDU will be split into num_frag fragments, 
	      // each going in a different PDU with all the overheads
	      num_frag = ceil((float)max_sdu_size/(pdu_size-pdu_overhead))+1;
	      total_bytes_needed = data_bytes_needed + (pdu_overhead + psh_len) * num_frag * num_sdu;
	    }                                                                  
	}
      else                                                                     
	{                                                                      
	  if (is_pack)
	    {
	      // ARQ=FALSE, FRAG=FALSE, PACK=TRUE
	      if (max_sdu_size + pdu_overhead > pdu_size)
		{
		  FLOG_FATAL("Error: SDU size exceeds PDU size on Fragmentation-disabled connection %d\n", cid);
		  // Can't serve this CID, go to next
		  return -1;
		}

	      // Worst case: Each SDU has its PSH. Assuming sending PDUs
	      // smaller than PDU size is ok and no space is wasted becaus
	      // of empty fragments. Not 100% accurate worst case. Might
	      // need >num_pdu PDUs because of sending smaller PDUs
	      num_pdu = ceil((float)(data_bytes_needed + psh_len*num_sdu)/(pdu_size - pdu_overhead));
	      total_bytes_needed = data_bytes_needed + (psh_len * num_sdu) + (num_pdu * pdu_overhead);
	    }
	  else
	    {
	      // ARQ=FALSE, FRAG=FALSE, PACK=FALSE. 100% accurate
	      total_bytes_needed = data_bytes_needed + num_sdu * (pdu_overhead);
	    }
	}
    }
  return total_bytes_needed;
}

int estimate_arqReTx_PnF_overhead(int cid, ARQ_ReTX_Q_aggr_info arq_info)
{
  int pdu_size = 0, num_block_per_pdu = 0, num_pdu = 0, total_bytes_needed = 0;
  u_int8_t is_frag;
  u_int8_t is_arq;
  int psh_len = 0, fsh_len = 0;

 int pdu_overhead = 0;
 int is_encrypted=-1;
 is_encrypt_enabled(cid,(u_int8_t*)&is_encrypted);
 if (is_encrypted) pdu_overhead = PDU_OH_WITH_ENCRYPT;
 else pdu_overhead = PDU_MIN_OVERHEAD; 
  // get the basic attribute of this connection
  is_frag_enabled(cid, &is_frag);
  is_arq_enabled(cid, &is_arq);

  // initialize subheader lengths according to connection attributes
  init_subheader_len(cid, is_frag, is_arq, &psh_len, &fsh_len);

  pdu_size = get_mac_pdu_size(cid);
  num_block_per_pdu = floor((pdu_size - pdu_overhead)/(get_blk_size(cid) + psh_len));
  num_pdu = ceil((float)arq_info.num_blocks/num_block_per_pdu);
  // Calculate the number of bytes needed to serve all packets in this 
  // waiting for reTx queue. Conservative - all the blocks that are not 
  // contiguous will need a separate PSH. Even continuous blocks might 
  // be split across PDUs because of PDU size.
  total_bytes_needed = arq_info.num_bytes + \
    arq_info.num_blocks * psh_len + \
    num_pdu * pdu_overhead;
  //printf("Total bytes needed %d for cid %d in retx estimate\n",total_bytes_needed,cid);
  return total_bytes_needed;
}



// This function performs the inverse function of estimate_sdu_PnF_overhead (which estimates total_bytes_needed from given data_bytes). 
// Here we estimate how many data bytes can be fit in given total_bytes_available, after accounting for the space needed for overheads
// This function is needed when the total available space for a CID falls short of that needed to serve its backlog and we serve partially, 
// e.g. in BE fair_share scheduling. 
int estimate_data_bytes_from_total(int cid, sdu_cid_queue* sdu_cid_q, int total_bytes_available)
{
  int data_bytes = 0, num_sdu = 0, num_full_pdu = 0; 
  int max_sdu_size = 0, min_sdu_size = 0;
  int num_frag_per_pdu = 0, num_sdu_per_pdu = 0, pdu_size = 0;
  int pdu_overhead=0;
  int is_encrypted=-1;
  is_encrypt_enabled(cid,(u_int8_t*)&is_encrypted);
 if (is_encrypted) pdu_overhead = PDU_OH_WITH_ENCRYPT;
 else pdu_overhead = PDU_MIN_OVERHEAD;
  u_int8_t is_frag;
  u_int8_t is_pack;
  u_int8_t is_arq;
  
  int psh_len = 0, fsh_len = 0;
  int arq_blk_size = get_blk_size(cid);

  // This function peeks in the SDU CID queue and tells the number of SDUs
  // in data_bytes_needed. While traversing, it also calculates the min and
  // max SDU size in this set. These are used for overhead estimation
  pdu_size = get_mac_pdu_size(cid);

  // get the basic attribute of this connection
  is_frag_enabled(cid, &is_frag);
  is_pack_enabled(cid, &is_pack);
  is_arq_enabled(cid, &is_arq);

  // initialize subheader lengths according to connection attributes
  init_subheader_len(cid, is_frag, is_arq, &psh_len, &fsh_len);

  if(total_bytes_available <= pdu_overhead + psh_len) 
    {
      data_bytes = 0;
      return data_bytes;
    }

  // If fragmentation is disabled, allocate only if the bytes available can accommodate at least
  // one SDU and its overhead
  if (is_frag == 0) 
    {
      peek_cid_queue(sdu_cid_q, cid, total_bytes_available, &num_sdu, &min_sdu_size, &max_sdu_size);
      if (total_bytes_available < max_sdu_size + pdu_overhead + EXTEND_PACK_SUBHEADER_LEN)
	{data_bytes = 0;}
      else
	{
	  if(is_pack)
	    {
	      num_sdu_per_pdu = floor((float)(pdu_size - pdu_overhead)/(max_sdu_size + psh_len));
	      num_full_pdu = floor((float)total_bytes_available/pdu_size);
	      data_bytes = num_full_pdu * num_sdu_per_pdu * min_sdu_size;
	    }
	  else
	    {
	      num_full_pdu = floor((float)total_bytes_available/(max_sdu_size + pdu_overhead));
	      data_bytes = num_full_pdu * min_sdu_size;
	    }
	}

    }
  else // if (is_frag)
    {
      if (is_arq &(total_bytes_available < arq_blk_size + pdu_overhead + psh_len))
	{
	  data_bytes = 0;
	  return data_bytes;
	}
      num_full_pdu = floor((float)total_bytes_available/pdu_size);
      if (num_full_pdu == 0)
	{
	  // If available bytes are less than preset PDU size, send a PDU of smaller size
	  num_full_pdu = 1;
	  pdu_size = total_bytes_available;
	  if (is_pack)
	    {peek_cid_queue(sdu_cid_q, cid, pdu_size - pdu_overhead, &num_sdu, &min_sdu_size, &max_sdu_size);}
	  else {num_sdu = 0;}
	  // +1 for the last fragment
	  data_bytes = pdu_size - pdu_overhead - (num_sdu + 1)* psh_len;
	  return data_bytes;
	}

      peek_cid_queue(sdu_cid_q, cid, total_bytes_available, &num_sdu, &min_sdu_size, &max_sdu_size);
      if (is_pack)
	{
	  // first compute for full PDU's (of pdu_size)
	  num_frag_per_pdu = floor((float)(pdu_size - pdu_overhead)/(max_sdu_size + psh_len)) + 1;
	  data_bytes = total_bytes_available - num_full_pdu*(pdu_overhead + num_frag_per_pdu * psh_len);
	  // now compute for the last PDU which might be smaller
	  pdu_size = total_bytes_available%pdu_size;
	  if (pdu_size > pdu_overhead + psh_len)
	    {
	      num_frag_per_pdu = floor((float)(pdu_size - pdu_overhead)/(max_sdu_size + psh_len)) + 1;
	      data_bytes = data_bytes - (pdu_overhead + num_frag_per_pdu*psh_len);
	    }
	  else
	    {data_bytes -= pdu_size;}
	}
      else
	{
	  num_full_pdu = floor((float)total_bytes_available/(min(max_sdu_size, pdu_size) + pdu_overhead + fsh_len));
	  data_bytes = total_bytes_available - num_full_pdu * (pdu_overhead + fsh_len);
	}
    }
  return data_bytes;
}

/*
int min(int a, int b)
{
  int min_val = a>b?b:a;
  return min_val;
}
int max(int a, int b)
{
  int max_val = a>b?a:b;
  return max_val;
}
*/
int init_subheader_len(short cid, u_int8_t is_frag, u_int8_t is_arq, int *psh_len, int *fsh_len)
{
  int fsn_size = 0;
  get_fsn_size(cid, &fsn_size);
  if (fsn_size == 3)
    {
      if (is_frag || is_arq)
	{
	  *fsh_len = FRAG_SUBHEADER_LEN;
	}
      *psh_len = PACK_SUBHEADER_LEN;
    }
  else
    {
      if (is_frag || is_arq)
	{
	  *fsh_len = EXTEND_FRAG_SUBHEADER_LEN;
	}
      *psh_len = EXTEND_PACK_SUBHEADER_LEN;
    }		  
  return 0;
}
