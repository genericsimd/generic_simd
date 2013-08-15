/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_dl_frag_pack.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include "mac_dl_frag_pack.h"
#include "memmgmt.h"
#include "mac_auth.h"
#include "dl_exp_params.h"


int release_sducontainer(logical_packet* sdulist, u_int8_t is_release_payload, logical_element** le_tobe_discard){
    logical_packet * sdu;
    logical_packet * pre_sdu;
    logical_element * sdu_le;
    logical_element * pre_sdu_le;

    sdu = sdulist;

    while (sdu)
    {
        pre_sdu = sdu;
        sdu = pre_sdu->next;
        
        sdu_le = pre_sdu->element_head;
        while(sdu_le)
        {
            pre_sdu_le = sdu_le;
            sdu_le = pre_sdu_le->next;
            if (is_release_payload)
            {
                // only ARQ may contain more than one le, but ARQ never run
                // into this block, so there is only one le and the le->next = NULL;
                if ((*le_tobe_discard)== NULL)
                {
                    (*le_tobe_discard) = pre_sdu_le;
                }
                else
                {
                    pre_sdu_le->next = (*le_tobe_discard);
                    (*le_tobe_discard)=pre_sdu_le;
                }
//	                mac_sdu_free((void*) pre_sdu_le->data, pre_sdu_le->length, pre_sdu_le->blk_type);
            }
            else
            {
                free(pre_sdu_le);
                pre_sdu_le = NULL;
            }
        }
        free(pre_sdu);
        pre_sdu = NULL;
    }

    return 0;
}

// need to be adjusted according to the new data structure

int fragpack(sdu_queue* sduq, logical_burst_map* burst_map, logical_packet** pdulisthead, logical_element** le_tobe_discard, int* status){
    logical_pdu_map* pdu_map;
    logical_packet* sdulist;
    logical_packet* arq_blk_list;
    logical_packet* arqblk;
    logical_element* arq_blks;
    logical_element* start_arq_blk;
    logical_element* next_arq_blk;
    logical_packet* sdu;
    logical_packet* pdu;
    logical_packet* pre_pdu;
    u_int16_t cid;
    int my_basic_cid;
    int my_aksn;
    struct ss_security_info* my_security_info;
    int pdu_size;
    int sdu_size;
    u_int8_t is_frag;
    u_int8_t is_pack;
    u_int8_t is_arq;
    u_int8_t is_fixed_sdu;
    u_int8_t is_crc;
    u_int8_t is_encrypt;
    u_int8_t is_first;
    u_int8_t is_mgt;
    int modulo;
    int fsn_size;
    int fsh_len;
    int psh_len;
    int crc_len;
    int gmh_len;
    int sdu_len;
    int arq_block_size;
    u_int8_t is_extended_type;
    element_type frag_hdr_type;
    element_type pack_hdr_type;
    u_int8_t is_fsh_present;
    u_int8_t is_psh_present;
    u_int8_t is_arq_feedback_present;
    u_int8_t is_gmsh_present;
    u_int8_t is_ffsh_present;
    u_int8_t is_release_payload;
    int payloadlength;
    int sdu_num;
    int left_bw;
    int burst_bytes_num;
    logical_element* leh;
    logical_element* lep;
    logical_element* prev_le;
    logical_element* sdu_le;
    generic_mac_hdr* gmh;
    frag_sub_hdr* fsh;
    pack_sub_hdr* psh;
    int current_fsn;
    int max_arqblock_num;
    int i;
    u_int8_t is_first_arqblk;
    u_int8_t is_last_arqblk;
    u_int8_t is_unfrag_arqblk;
    pdu_map = burst_map->pdu_map_header;
    burst_bytes_num = burst_map->burst_bytes_num;
#ifdef TRANSITION_TEST
        unsigned long long int trans_time;
        unsigned long long int current_time;
#endif
    if (burst_bytes_num < 0)
    {
        FLOG_ERROR("frag_pack: not assign enough burst bytes. 1\n");
	*status=1;
        return 1;
    }
    
    gmh_len = GENERIC_MAC_HEADER_LEN;

    pre_pdu = (*pdulisthead);

    while (pdu_map)
    {
        sdulist = NULL;
        arq_blk_list = NULL;
        cid = pdu_map->cid;

        // get the basic attribute of this connection
        is_frag_enabled(cid, &is_frag);
        is_pack_enabled(cid, &is_pack);
        is_arq_enabled(cid, &is_arq);
	 is_fix_sdu_length(cid, &is_fixed_sdu);
        is_mgt_con(cid, &is_mgt);
        if (!is_arq)
        {
            // if this is not arq enabled, the memory should be released no matter its is management message or not
            is_release_payload = 1;
        }
        else 
        {
            is_release_payload = 0;
        }

        if (!is_arq)
        {
            get_current_seq_no(cid, &current_fsn);
        }

        get_fsn_size(cid, &fsn_size);
        get_pdu_size(cid, &pdu_size);

        is_crc_enabled(cid, &is_crc);

        is_encrypt_enabled(cid, &is_encrypt);

	if (is_encrypt == 1)
	{
		get_basic_cid(cid,&my_basic_cid);
		my_security_info = find_sa_from_said (my_basic_cid);
		if (my_security_info == NULL) {printf("Couldnt find secinfo in fragpack\n");continue;}
		pthread_mutex_lock(&(my_security_info->sa_lock));
#ifndef TRANSITION_TEST
		my_aksn = my_security_info->current_seq_no;
#else
	#ifdef SS_TX
		my_aksn = (my_security_info->current_seq_no - 1)%4;
	#else
		my_aksn = (my_security_info->current_seq_no + 1)%4;
	#endif
		trans_time = my_security_info->key_gen_time[my_aksn].tv_sec*1000000LL + my_security_info->key_gen_time[my_aksn].tv_usec;	
		current_time = readtsc();
		if (trans_time + param_KEY_LIFE_DURATION <= current_time) my_aksn = my_security_info->current_seq_no;

#endif	
		pthread_mutex_unlock(&(my_security_info->sa_lock));
	}

        get_modulo(cid, &modulo);
        // initialization
        if (fsn_size ==3)
        {
            if (is_frag || is_arq)
            {
                fsh_len = FRAG_SUBHEADER_LEN;
                frag_hdr_type = FRAG_SUBHEADER;
            }
            if (is_pack)
            {
                psh_len = PACK_SUBHEADER_LEN;
                pack_hdr_type = PACK_SUBHEADER;
            }
            is_extended_type = 0;
        }
        else
        {
            if (is_frag || is_arq)
            {
                fsh_len = EXTEND_FRAG_SUBHEADER_LEN;
                frag_hdr_type = EXTEND_FRAG_SUBHEADER;
            }
	     if (is_pack)
            {
                psh_len = EXTEND_PACK_SUBHEADER_LEN;
                pack_hdr_type = EXTEND_PACK_SUBHEADER;
            }
            is_extended_type = 1;
        }
        if (is_crc)
        {
            crc_len = MAC_CRC_LEN;
        }
        else 
        {
            crc_len = 0;
        }

        if (is_arq)
        {
            get_arq_block_size(cid, &arq_block_size);

            // first schedule the retransmitted arq blocks
            if (pdu_map->arq_sdu_map)
            {
                arq_blk_list = pdu_map->arq_sdu_map->arq_retransmit_block;
                arqblk = arq_blk_list;
                arq_blks = arq_blk_list->element_head;
                // suppose that the arq_sdu_map is organized as a set of arq block logical element, each logical element is one block
                if (is_pack)
                {

                    // each sdu can be divided into several fragments, each fragments' size is limited to the pdu size
                    while( arqblk){
                         // now try to generate the PDUs
                         // generate one pdu for each loop
                         pdu = (logical_packet*) malloc(sizeof(logical_packet));
                         memset(pdu, 0, sizeof(logical_packet));
                         if (pre_pdu== NULL)
                         {
                             pre_pdu = pdu;
                             (*pdulisthead) = pdu;
                         }
                         else
                         {
                             pre_pdu->next = pdu;
                         }

                         pdu->cid = cid;
                         // initialize the real pdu length as the crc lenght if crc is enabled
                        if (is_encrypt==1) pdu->length = crc_len + IVLEN+TAGLEN;
			else pdu->length = crc_len;
                         pdu->element_head = NULL;
                         pdu->next= NULL;

                         prev_le = pdu->element_head;

                         leh = (logical_element*) malloc(sizeof(logical_element));
                         memset(leh, 0, sizeof(logical_element));
                         leh->type = MAC_GENERIC_HEADER;
                         leh->length = gmh_len;
                         leh->next = NULL;

                         gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
                         memset(gmh, 0, sizeof(generic_mac_hdr));
                         gmh->ht = 0;
                         gmh->ec = is_encrypt;
                         gmh->ci = is_crc;
			 if (is_encrypt == 1)
			 {
				gmh->eks = (u_int8_t) my_aksn;
		 	 }
			 else
			 {
                         	gmh->eks = 0;
			 }
                         gmh->esf = 0;
                         gmh->rsv = 0;
                         gmh->cid = cid;

                         leh->data = (u_char *) gmh;
                         is_fsh_present = 0;
                         is_psh_present = 0;
                         is_arq_feedback_present = 0;
                         is_gmsh_present = 0;
                         is_ffsh_present = 0;

                         // connect the generic mac headerlogical element
                         if (prev_le == NULL){
                             // in case this is the first sdu
                             pdu->element_head = leh;
                         }
                         else
                         {
                             prev_le->next = leh;
                         }
                         pdu->length += leh->length;
                         prev_le = leh;

                          // here the payload denotest the shall be added payload, include subheader and sdu or sdu fragments

                         if (is_encrypt == 1) left_bw = pdu_size - PDU_OH_WITH_ENCRYPT ;
			 else left_bw = pdu_size - PDU_MIN_OVERHEAD;

                         while(left_bw>0)
                         {
                              // check whether there are still sdu or sdu fragment available
                             if (arq_blks== NULL)
                             {
                                 // the retransmitted arq blocks for this sdu packet has been totally processed, so move to the next sdu
                                 arqblk = arqblk->next;

                                 if (arqblk == NULL)
                                 {
                                     break;
                                 }
                                 else 
                                 {
                                     arq_blks = arqblk->element_head;
                                     current_fsn = arq_blks->start_bsn;
                                 }
                             }
                                
                             // the next sdu could not be contained in the residual pdu even though it is smaller than an arq block
                             if ((arq_blks->length < arq_block_size) && (left_bw <(arq_blks->length + psh_len)))
                             {
                                 // no further room to further include at least one arq block
                                 break;
                             }
                             // the room for the residual pdu could not contain the next arq block, the next arq block should have not consecutive bsn
                             if ((arq_blks->length >= arq_block_size) && (left_bw < (arq_block_size + psh_len)))
                             {
                                 break;
                             }
                        
                             // generate one logical element , subheader, 
                             leh = (logical_element*) malloc(sizeof(logical_element));
                             memset(leh, 0, sizeof(logical_element));
                             leh->type = pack_hdr_type;
                                
                             // generate the packing subheader
                             psh=(pack_sub_hdr*)malloc(sizeof(pack_sub_hdr));
                             memset(psh, 0, sizeof(pack_sub_hdr));
                             psh->fsn = arq_blks->start_bsn;
                             // psh->fc = CONTINUING_FRAGMENT;

                             start_arq_blk = arq_blks;
                             payloadlength = psh_len;
                             is_first_arqblk = 0;
                             is_last_arqblk = 0;
                             is_unfrag_arqblk = 0;
                             if (arq_blks->blk_type == NO_FRAGMENTATION)
                             {
                                 is_unfrag_arqblk = 1;
                             }
                             if (arq_blks->blk_type == FIRST_FRAGMENT)
                             {
                                 is_first_arqblk = 1;
                             }
                                
                             while ((left_bw - payloadlength)>= arq_blks->length)
                             {
                                 payloadlength += arq_blks->length;
                                 next_arq_blk = arq_blks->next;
                                 if (next_arq_blk == NULL)
                                 {
                                     // no more retransmitted arq blocks in this sdu packet
                                     // check whether it is the last frament
                                     if (arq_blks->blk_type == LAST_FRAGMENT)
                                     {
                                         is_last_arqblk = 1;
                                     }
                                     arq_blks = next_arq_blk;
                                     break;
                                 }
                                 else 
                                 {
                                       
                                     if (next_arq_blk->start_bsn != (arq_blks->start_bsn+1))
                                     {
                                         // the bsn is no longer consecutive, so it should be put into another packing subheader
                                         arq_blks = next_arq_blk;
                                         break;
                                     }
                                     else 
                                     {
                                         arq_blks = next_arq_blk;
                                     }
                                 }
                                    
                             }

                             if ((is_first_arqblk && is_last_arqblk) || is_unfrag_arqblk)
                             {
                                 psh->fc = NO_FRAGMENTATION;
                             }
                             else if (is_first_arqblk)
                             {
                                 psh->fc = FIRST_FRAGMENT;
                             }
                             else if (is_last_arqblk)
                             {
                                 psh->fc = LAST_FRAGMENT;
                             }
                             else
                             {
                                 psh->fc = CONTINUING_FRAGMENT;
                             }
                             // the fragmentation should according to the boundary of arq block
                             psh->length = payloadlength;

                             leh->data = (u_char *) psh;
                             leh->length = psh_len;
                             is_psh_present = 1;

                                
                             // connect the subheader and payload logical element
                             prev_le->next = leh;

                             pdu->length +=leh->length;
                             prev_le = leh;
                               
                             // now generate the retransmitted arq block logical element
                             lep = (logical_element*) malloc(sizeof(logical_element));
                             memset(lep, 0, sizeof(logical_element));
                             lep->data = start_arq_blk->data;
                             lep->type = ARQ_BLOCK;
                             lep->next = NULL;
                             lep->length = psh->length - psh_len;
                             // connect the retransmitted arq block logical element
                             prev_le->next = lep;
                             pdu->length += lep->length;
                             prev_le = lep;
                             // calculate the legacy bandwidth
                             left_bw -= lep->length;
                             left_bw -= leh->length;

                         }
                         // construct the generic mac header
//point 1: if encryption is by cid, input to fragpack cid's encryption info and update here. Right now, enabling encrypt for all non-mgmt conns                         
			gmh->len = pdu->length;
			gmh->type = ((is_arq_feedback_present & 0x01) << 4) | ((is_extended_type & 0x01) << 3) | ((is_fsh_present & 0x01) << 2) | ((is_psh_present & 0x01) <<1) | ((is_ffsh_present & 0x01) | (is_gmsh_present & 0x01));
                
                         // save the current pdu
                         pre_pdu = pdu;

                         // update the left allowed capacity 
                         burst_bytes_num -= pdu->length;
                         if (burst_bytes_num < 0)
                         {
                             // release the logical packet and logical element container
                             if (arq_blk_list)
                             {
                                 release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
                             }
                             FLOG_ERROR("frag_packe: not assign enough burst bytes. 2 \n");
			     *status=1;
                             return 1;
                         }
                    }
                    
                    
                }
                else 
                {
                    // packing is not supported
                    // each sdu can be divided into several fragments, each fragments' size is limited to the pdu size
                    arq_blks = arqblk->element_head;
                    while(arq_blks)
                    {
                         // now try to generate the PDUs
                         // generate one pdu for each loop
                            
                         pdu = (logical_packet*) malloc(sizeof(logical_packet));
                         memset(pdu, 0, sizeof(logical_packet));
                         if (pre_pdu== NULL)
                         {
                             pre_pdu = pdu;
                             (*pdulisthead) = pdu;
                         }
                         else
                         {
                             pre_pdu->next = pdu;
                         }

                         pdu->cid = cid;
                         // initialize the real pdu length as the crc lenght if crc is enabled
                         if (is_encrypt == 1) pdu->length = crc_len + IVLEN + TAGLEN;
			 else pdu->length = crc_len;
                         pdu->element_head = NULL;
                         pdu->next= NULL;

                         prev_le = pdu->element_head;

                         leh = (logical_element*) malloc(sizeof(logical_element));
                         memset(leh, 0, sizeof(logical_element));
                         leh->type = MAC_GENERIC_HEADER;
                         leh->length = gmh_len;
                         leh->next = NULL;

                         gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
                         memset(gmh, 0, sizeof(generic_mac_hdr));
                         gmh->ht = 0;
                         gmh->ec = is_encrypt;
                         gmh->ci = is_crc;
			 if (is_encrypt == 1)
			 {
				gmh->eks = (u_int8_t) my_aksn;
		 	 }
			 else
			 {
                         	gmh->eks = 0;
			 }
                         gmh->esf = 0;
                         gmh->rsv = 0;
                         gmh->cid = cid;

                         leh->data = (u_char *) gmh;
                         is_fsh_present = 0;
                         is_psh_present = 0;
                         is_arq_feedback_present = 0;
                         is_gmsh_present = 0;
                         is_ffsh_present = 0;

                         // connect the generic mac headerlogical element
                         if (prev_le == NULL)
                         {
                             // in case this is the first sdu
                             pdu->element_head = leh;
                         }
                         else
                         {
                             prev_le->next = leh;
                         }
                         pdu->length += leh->length;
                         prev_le = leh;

                          // here the payload denotest the shall be added payload, include subheader and sdu or sdu fragments
                         if (is_encrypt == 1) left_bw = pdu_size - PDU_OH_WITH_ENCRYPT;
			 else left_bw = pdu_size - PDU_MIN_OVERHEAD;
                             
                         // generate one logical element , subheader, 
                         leh = (logical_element*) malloc(sizeof(logical_element));
                         memset(leh, 0, sizeof(logical_element));
                         leh->type = frag_hdr_type;
                                
                         // generate the packing subheader
                         fsh=(frag_sub_hdr*)malloc(sizeof(frag_sub_hdr));
                         memset(fsh, 0, sizeof(frag_sub_hdr));
                         fsh->fsn = arq_blks->start_bsn;
                         fsh->rsv = 0;

                         start_arq_blk = arq_blks;
                         payloadlength = fsh_len;

                         is_first_arqblk = 0;
                         is_last_arqblk = 0;
                         is_unfrag_arqblk = 0;
                         if (arq_blks->blk_type == NO_FRAGMENTATION)
                         {
                             is_unfrag_arqblk = 1;
                         }
                         if (arq_blks->blk_type == FIRST_FRAGMENT)
                         {
                             is_first_arqblk = 1;
                         }
                         while ((left_bw - payloadlength) >= arq_blks->length)
                         {
                             payloadlength += arq_blks->length;
                             next_arq_blk = arq_blks->next;
                             if (next_arq_blk == NULL)
                             {
                                 // no more retransmitted arq blocks in this sdu packet
                                 if (arq_blks->blk_type == LAST_FRAGMENT)
                                 {
                                     is_last_arqblk = 1;
                                 }
                                 arq_blks = next_arq_blk;
                                 break;
                             }
                             else 
                             {
                                        
                                 if (next_arq_blk->start_bsn != (arq_blks->start_bsn+1))
                                 {
                                     // the bsn is no longer consecutive, so it should be put into another packing subheader
                                     arq_blks = next_arq_blk;
                                     break;
                                 }
                                 else 
                                 {
                                     arq_blks = next_arq_blk;
                                 }

                             }
                                    
                         }
                         if ((is_first_arqblk && is_last_arqblk) || is_unfrag_arqblk)
                         {
                             fsh->fc = NO_FRAGMENTATION;
                         }
                         else if (is_first_arqblk)
                         {
                             fsh->fc = FIRST_FRAGMENT;
                         }
                         else if (is_last_arqblk)
                         {
                             fsh->fc = LAST_FRAGMENT;
                         }
                         else
                         {
                             fsh->fc = CONTINUING_FRAGMENT;
                         }
                         // the fragmentation should according to the boundary of arq block

                         leh->data = (u_char *) fsh;
                         leh->length = fsh_len;
                         is_fsh_present = 1;
                              
                         // connect the subheader and payload logical element
                         prev_le->next = leh;
                         pdu->length += leh->length;
                         prev_le = leh;
                                
                         // now generate the retransmitted arq block logical element
                         lep = (logical_element*) malloc(sizeof(logical_element));
                         memset(lep, 0, sizeof(logical_element));
                         lep->data = start_arq_blk->data;
                         lep->type = ARQ_BLOCK;
                         lep->next = NULL;
                         lep->length = payloadlength - fsh_len;
                         // connect the retransmitted arq block logical element
                         prev_le->next = lep;
                         pdu->length += lep->length;
                         prev_le = lep;
                         // calculate the legacy bandwidth
                         left_bw -= lep->length;
                         left_bw -= leh->length;
                           
                         // construct the generic mac header
                        gmh->len = pdu->length;
                        gmh->type = ((is_arq_feedback_present & 0x01) << 4) | ((is_extended_type & 0x01) << 3) | ((is_fsh_present & 0x01) << 2) | ((is_psh_present & 0x01) <<1) | ((is_ffsh_present & 0x01) | (is_gmsh_present & 0x01));

                        // save the current pdu
                        pre_pdu = pdu;

                        // update the left allowed capacity 
                        burst_bytes_num -= pdu->length;
                        if (burst_bytes_num < 0)
                        {
                            // release the logical packet and logical element container
                            if (arq_blk_list)
                            {
                                release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
                            }
                            FLOG_ERROR("frag_packe: not assign enough burst bytes. 3 \n");
			    *status=1;
                            return 1;
                        }

                        // check whether there are still sdu or sdu fragment available
                        if (arq_blks== NULL)
                        {
                            arqblk = arqblk->next;
                            if (arqblk)
                            {
                                arq_blks = arqblk->element_head;
                            }
                            else 
                            {
                                break;
                            }
                        }
                    }
                  
                }
                
            }
        }
                
        if (pdu_map->mac_msg_map)
        {
            // process the downlink management message
            sdulist = pdu_map->mac_msg_map->mac_mgmt_msg_sdu;
        }
        if (pdu_map->transport_sdu_map)
        {
            // secondly schedule the newly sdu for transmission
            if (!is_arq)
            {
                is_release_payload = 1;
            }
            else
            {
                is_release_payload = 0;
            }
            is_fix_sdu_length(cid, &is_fixed_sdu);
            dequeue_sduq(sduq, cid, pdu_map->transport_sdu_map->num_bytes, &sdulist);
        }

        sdu = sdulist;
	
        if (sdu == NULL)
        {
            pdu_map = pdu_map->next;
            // release the logical packet and logical element container
            if (arq_blk_list)
            {
                release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
            }
            continue;
        }

        if (is_pack)
        {
        // if packing is enabled
           if (is_fixed_sdu)
           {
                // The MAC SDU is fixed in length, so the fragmentation as well as arq should be prohibited
                // get the fixed sdu size
                get_sdu_size(cid, &sdu_size);
               
                if (is_encrypt == 1) payloadlength = pdu_size - PDU_OH_WITH_ENCRYPT;
		 else payloadlength = pdu_size - PDU_MIN_OVERHEAD;
                sdu_num =(int)(payloadlength / sdu_size);
                // here the pdu size is the minimal pdu size after packing
                //pdu_size = sdu_size + gmh_len + crc_len;
             
                // now try to pack given number of sdu
                while(sdu)
                {
                     // now try to generate the PDUs
                     // generate one pdu for each loop
                     pdu = (logical_packet*) malloc(sizeof(logical_packet));
                     memset(pdu, 0, sizeof(logical_packet));
                     if (pre_pdu== NULL)
                     {
                         pre_pdu = pdu;
                         (*pdulisthead) = pdu;
                     }
                     else
                     {
                         pre_pdu->next = pdu;
                     }
                     pdu->cid = cid;
                     // initialize the real pdu length as the crc lenght if crc is enabled
                     if (is_encrypt == 1) pdu->length = crc_len+ IVLEN + TAGLEN;			
		     else pdu->length = crc_len;

                     pdu->element_head = NULL;
                     pdu->next= NULL;
                     leh = (logical_element*) malloc(sizeof(logical_element));
                     memset(leh, 0, sizeof(logical_element));
                     leh->type = MAC_GENERIC_HEADER;
                     leh->length = gmh_len;
                     leh->next = NULL;

                     gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
                     memset(gmh, 0, sizeof(generic_mac_hdr));
                     gmh->ht = 0;
                     gmh->ec = is_encrypt;
                     gmh->ci = is_crc;
		     if (is_encrypt == 1)
		     {
		     	gmh->eks = (u_int8_t) my_aksn;
		     }
		     else
		     {
                     	gmh->eks = 0;
		     }
                     gmh->esf = 0;
                     gmh->rsv = 0;
                     gmh->cid = cid;
                     
                     leh->data = (u_char *) gmh;

                     is_fsh_present = 0;
                     is_psh_present = 0;
                     is_arq_feedback_present = 0;
                     is_gmsh_present = 0;
                     is_ffsh_present = 0;

                     // connect the generic mac headerlogical element
                     if (pdu->element_head != NULL)
                     {
                         prev_le->next = leh;
                     }
                     else
                     {
                         // in case this is the first sdu
                         pdu->element_head = leh;
                     }
                     pdu->length += leh->length;
                     prev_le = leh;
                     for (i=0; i < sdu_num;i++)
                     {
                         if (NULL == sdu )
                         {
                             break;
                         }
                         else 
                         {
                              // get the sdu element from the sdu packet
                              sdu_le = sdu->element_head;
                              // directly use  this logical element
                              lep = (logical_element *)malloc(sizeof(logical_element));
                              memset(lep, 0, sizeof(logical_element));
                              lep->data = sdu_le->data;
			    
			      if (sdu->cid <= BASIC_CID_MAX_VALUE && sdu->cid >= BASIC_CID_MAX_VALUE)
				{	
					ARQ_feedback_message * temp1 = (ARQ_feedback_message*) sdu_le->data;
					ARQ_discard_message * temp2 = (ARQ_discard_message*) sdu_le->data;
					ARQ_reset_message * temp3 = (ARQ_reset_message*) sdu_le->data;
					if (temp1->mgmt_msg_type == ARQ_FEEDBACK_MSG || temp2->mgmt_msg_type == ARQ_DISCARD_MSG || temp3->mgmt_msg_type == ARQ_RESET_MSG) {is_arq_feedback_present = 1;/*printf("This pdu has arq mmm type %d %d %d\n",temp1->mgmt_msg_type,temp2->mgmt_msg_type,temp3->mgmt_msg_type);*/}
				}	 
 
                              lep->length = sdu_le->length;
                              lep->type = MAC_SDU;
                              lep->blk_type = NO_FRAGMENTATION;
                              lep->next = NULL;
                              // connect the subheader and payload logical element
                              prev_le->next = lep;

                              prev_le = lep;
                              pdu->length += lep->length;
                        }
                        sdu = sdu->next;
                    }

                    gmh->len = pdu->length;
                    gmh->type = ((is_arq_feedback_present & 0x01) << 4) | ((is_extended_type & 0x01) << 3) | ((is_fsh_present & 0x01) << 2) | ((is_psh_present & 0x01) <<1) | ((is_ffsh_present & 0x01) | (is_gmsh_present & 0x01));

                    // save the pointer to the current pdu
                    pre_pdu = pdu;
 
                    // update the left allowed capacity 
                    burst_bytes_num-= pdu->length;
                    if (burst_bytes_num < 0)
                    {
                        // release the logical packet and logical element container
                        if (arq_blk_list)
                        {
                            release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
                        }
                        if (sdulist)
                        {
                            release_sducontainer(sdulist, is_release_payload, le_tobe_discard);
                        }
                        FLOG_ERROR("frag_packe: not assign enough burst bytes. 4\n");
			*status=1;
                        return 1;
                    }
                }
            
            }
            else 
            {
                // now consider the case where the sdu is variable in length, packing is supported

                //consider the MAC SDU to be transmitted for the first time

                if (sdu && is_arq)
                {
                    current_fsn = sdu->element_head->start_bsn;
                }

                //  arq block wait to be retransmitted has been generated into MAC PDUs, 
                while(burst_bytes_num > 0 && sdu)
                {

                     // now try to generate the PDUs
                     // generate one pdu for each loop

                     pdu = (logical_packet *) malloc(sizeof(logical_packet));
                     if (!pdu)
                     {
                         FLOG_ERROR("mac_dl_frag_pack: could not allocate pdu");
                     }
                     memset(pdu, 0, sizeof(logical_packet));
                     if (pre_pdu== NULL)
                     {
                         pre_pdu = pdu;
                         (*pdulisthead) = pdu;
                     }
                     else
                     {
                         pre_pdu->next = pdu;
                     }
                     pdu->cid = cid;
                     // initialize the real pdu length as the crc lenght if crc is enabled
                     if (is_encrypt == 1 ) pdu->length = crc_len + IVLEN + TAGLEN;
		     else pdu->length = crc_len;
                     pdu->element_head = NULL;
                     pdu->next= NULL;

                     prev_le = pdu->element_head;

                     leh = (logical_element*) malloc(sizeof(logical_element));
                     memset(leh, 0, sizeof(logical_element));
                     leh->type = MAC_GENERIC_HEADER;
                     leh->length = gmh_len;
                     leh->next = NULL;

                     gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
                     memset(gmh, 0, sizeof(generic_mac_hdr));
                     gmh->ht = 0;
                     gmh->ec = is_encrypt;
                     gmh->ci = is_crc;
		     if (is_encrypt == 1)
		     {
		     	gmh->eks = (u_int8_t) my_aksn;
		     }
		     else
		     {
                        gmh->eks = 0;
		     }
                     gmh->esf = 0;
                     gmh->rsv = 0;
                     gmh->cid = cid;

                     leh->data = (u_char *) gmh;
                     is_fsh_present = 0;
                     is_psh_present = 0;
                     is_arq_feedback_present = 0;
                     is_gmsh_present = 0;
                     is_ffsh_present = 0;

                     // connect the generic mac headerlogical element
                     if (prev_le == NULL)
                     {
                         // in case this is the first sdu
                         pdu->element_head = leh;
                     }
                     else
                     {
                         prev_le->next = leh;
                     }
                     pdu->length += leh->length;
                     prev_le = leh;

                      // here the payload denotest the shall be added payload, include subheader and sdu or sdu fragments
                     if (is_encrypt == 1) left_bw = pdu_size - PDU_OH_WITH_ENCRYPT;
		     else left_bw = pdu_size - PDU_MIN_OVERHEAD;
        
                     while(left_bw > 0)
                     {
                        // check whether there are still sdu or sdu fragment available
                        if (sdu->element_head->length == 0)
                        {
                            // this sdu has been totally processed, so move to the next sdu
                            sdu = sdu->next;

                            if (sdu == NULL)
                            {
                                break;
                            }
                            else if (is_arq)
                            {
                                current_fsn = sdu->element_head->start_bsn;
                            }
                        }
                        sdu_le = sdu->element_head;
                        sdu_len = sdu_le->length;
                        if (is_arq)
                        {
                            // the next sdu could not be contained in the residual pdu even though it is smaller than an arq block
                            if ((sdu_le->length < arq_block_size) && (left_bw <(sdu_le->length + psh_len)))
                            {
                                // no further room to further include at least one arq block
                                break;
                            }
                            // the room for the residual pdu could not contain the next arq block
                            if ((sdu_le->length >=arq_block_size) && (left_bw <(arq_block_size+psh_len)))
                            {
                                break;
                            }
                        }

                        if (!is_frag)
                        {
                            if (left_bw < (sdu_le->length+psh_len))
                            {
                                break;
                            }
                        }
                        else
                        {
                            if (left_bw <= psh_len)
                            {
                                break;
                            }
                            if (sdu_le->type == MAC_SDU )
                            {
                                is_first=1;
                                
                            }
                            else if (sdu_le->type == MAC_SDU_FRAG)
                            {
                                if (sdu_le->blk_type == FIRST_FRAGMENT)
                                {
                                    is_first = 1;
                                    sdu_le->blk_type = CONTINUING_FRAGMENT;
                                }
                                else
                                {
                                    is_first=0;
                                }
                                
                            }
                        }
                        // generate one logical element , subheader, 
                        leh = (logical_element*) malloc(sizeof(logical_element));
                        memset(leh, 0, sizeof(logical_element));
                        if (is_frag)
                        {
                            // fragment is enabled, first generate the appropriate subheader
                            if (is_first)
                            {
                                leh->type = pack_hdr_type;
                                leh->length = psh_len;
                                // generate the packing subheader
                                psh=(pack_sub_hdr*)malloc(sizeof(pack_sub_hdr));
                                memset(psh, 0, sizeof(pack_sub_hdr));
                                if (!is_arq)
                                {
                                    current_fsn = (current_fsn + 1) % modulo;
                                }

                                psh->fsn = current_fsn;
                                // a new sdu, judge  the length of the sdu to see whether fragmentation is needed
                                if (sdu_len<=(left_bw-psh_len))
                                {
                                    // the whole sdu can be included in this pdu
                                    psh->length = sdu_len+psh_len;
                                    if (sdu_le->type == MAC_SDU_FRAG && sdu_le->blk_type == CONTINUING_FRAGMENT)
                                    {
                                        psh->fc = FIRST_FRAG;
                                    }
                                    else
                                    {
                                        psh->fc = UN_FRAGMENTED; // it is unfragmented
                                    }
                                }
                                else 
                                {
                                    // need fragmentation
                                    if (is_arq)
                                    {
                                        // the fragmentation should according to the boundary of arq block
                                        max_arqblock_num = (left_bw-psh_len) / arq_block_size;
                                        payloadlength = max_arqblock_num * arq_block_size;
                                        psh->length = payloadlength+psh_len;
                                        current_fsn += max_arqblock_num;
                                    }
                                    else 
                                    {
                                        psh->length = left_bw;
                                    }
                                    psh->fc = FIRST_FRAG; // the first fragments
                                }
                                leh->data = (u_char *) psh;
                                is_psh_present = 1;

                            }
                            else 
                            {
                                // the legacy fragment , check whether it is the last fragment, if yes, add psh subheader, else fsh subheader
                                if (sdu_len>=(left_bw-fsh_len))
                                {
                                    // it is the continuous or last fragment that just fit, so utilize the fragment subheader
                                    leh->type = frag_hdr_type;
                                    leh->length = fsh_len;
                                    // generate the fragment subheader
                                    fsh=(frag_sub_hdr*)malloc(sizeof(frag_sub_hdr));
                                    memset(fsh, 0, sizeof(frag_sub_hdr));
                                    if (!is_arq)
                                    {
                                        current_fsn = (current_fsn + 1) % modulo;
                                    }

                                    fsh->fsn = current_fsn;
 
                                    if (sdu_len == ((left_bw-fsh_len)))
                                    {
                                        // all the fragment is just fit into the left bandwidth
                                        if (sdu_le->blk_type == CONTINUING_FRAGMENT)
                                        {
                                            fsh->fc = CONTINUING_FRAG;
                                        }
                                        else
                                        {
                                            fsh->fc = LAST_FRAG;
                                        }

                                        fsh->rsv = 0;
                                        payloadlength = sdu_len;
                                    }
                                    else 
                                    {
                                        // needs further fragmentation
                                        if (is_arq)
                                        {
                                            max_arqblock_num = (left_bw-fsh_len) / arq_block_size;
                                            payloadlength = max_arqblock_num * arq_block_size;
                                            current_fsn += max_arqblock_num;
                                        }
                                        else 
                                        {
                                            payloadlength = left_bw-fsh_len;
                                        }
                                        fsh->fc = CONTINUING_FRAG; // the continuous fragments
                                        fsh->rsv = 0;
                                    }
                                    leh->data = (u_char *) fsh;
                                    is_fsh_present = 1;
                                }
                                else 
                                {
                                    // the last fragment and still has bw room for more packets, need psh subheader
                                    leh->type = pack_hdr_type;
                                    leh->length = psh_len;
                                    // generate the packing subheader
                                    psh=(pack_sub_hdr*)malloc(sizeof(pack_sub_hdr));
                                    memset(psh, 0, sizeof(pack_sub_hdr));
                                    if (sdu_le->blk_type == CONTINUING_FRAGMENT)
                                    {
                                        psh->fc = CONTINUING_FRAG;
                                    }
                                    else
                                    {
                                        psh->fc = LAST_FRAG;
                                    }

                                    if (!is_arq)
                                    {
                                        current_fsn = (current_fsn + 1) % modulo;
                                    }

                                    psh->fsn = current_fsn;
                                    
                                    psh->length = sdu_len+psh_len;
		                      leh->data = (u_char*) psh;
                                    is_psh_present = 1;
                                }
                            }

                            // connect the subheader and payload logical element
                            prev_le->next = leh;

                            pdu->length +=leh->length;
                            prev_le = leh;
                            // now generate the sdu or sdu fragment logical element
                            lep = (logical_element*) malloc(sizeof(logical_element));
                            memset(lep, 0, sizeof(logical_element));
                            lep->data = sdu_le->data;
			    if (sdu->cid <= BASIC_CID_MAX_VALUE && sdu->cid >=BASIC_CID_MIN_VALUE)
			    {
				ARQ_feedback_message *temp1 = (ARQ_feedback_message*) sdu_le->data;
					ARQ_discard_message * temp2 = (ARQ_discard_message*) sdu_le->data;
					ARQ_reset_message * temp3 = (ARQ_reset_message*) sdu_le->data;
				if (temp1->mgmt_msg_type == ARQ_FEEDBACK_MSG || temp2->mgmt_msg_type == ARQ_DISCARD_MSG || temp3->mgmt_msg_type == ARQ_RESET_MSG)
				{
					is_arq_feedback_present = 1;
					//printf("This pdu has arq mmm type %d %d %d\n",temp1->mgmt_msg_type,temp2->mgmt_msg_type,temp3->mgmt_msg_type);
				}
			    }
                            lep->next = NULL;
                            if (leh->type == PACK_SUBHEADER || leh->type == EXTEND_PACK_SUBHEADER)
                            {
                                lep->length = psh->length - psh_len;

                                if (psh->fc ==UN_FRAGMENTED)
                                {
                                    lep->type = MAC_SDU;
                                }
                                else 
                                {
                                    lep->type = MAC_SDU_FRAG;
                                }
                            }
                            else
                            {
                                // the fragmentation subheader is utilized
                                if (is_arq)
                                {
                                    lep->length = payloadlength;
                                }
                                else 
                                {
                                    lep->length = left_bw-leh->length;
                                }
                                lep->type = MAC_SDU_FRAG;
                            }
	                     // modify the left logical element
                            sdu_le->data += lep->length;
		              sdu_le->length -= lep->length;
                            sdu_le->type = MAC_SDU_FRAG;
                            //sdu_le->start_bsn = current_fsn+1;
                            //sdu->length -= lep->length;
                       
                        }
                        else
                        {
                            // fragment is not enabled, only packing is enabled, first generate the appropriate subheader

                            leh->type = pack_hdr_type;
                            leh->length = psh_len;
                            // generate the packing subheader
                            psh=(pack_sub_hdr*)malloc(sizeof(pack_sub_hdr));
                            memset(psh, 0, sizeof(pack_sub_hdr));
                            if (!is_arq)
                            {
                                current_fsn = (current_fsn + 1) % modulo;
                            }

                            psh->fsn = current_fsn;
                            // a new sdu, judge  the length of the sdu to see whether fragmentation is needed
                            if (sdu_len<=(left_bw-psh_len))
                            {
                                // the whole sdu can be included in this pdu
                                psh->length = sdu_len+psh_len;
                                psh->fc = UN_FRAGMENTED; // it is unfragmented
                            }
                            leh->data = (u_char *) psh;
                            is_psh_present = 1;

                            // connect the subheader and payload logical element
                            prev_le->next = leh;

                            pdu->length +=leh->length;
                            prev_le = leh;
                            // now generate the sdu or sdu fragment logical element
                            lep = (logical_element*) malloc(sizeof(logical_element));
                            memset(lep, 0, sizeof(logical_element));
                            lep->data = sdu_le->data;
			    if (sdu->cid <= BASIC_CID_MAX_VALUE && sdu->cid >=BASIC_CID_MIN_VALUE)
			    {
				ARQ_feedback_message *temp1 = (ARQ_feedback_message*) sdu_le->data;
					ARQ_discard_message * temp2 = (ARQ_discard_message*) sdu_le->data;
					ARQ_reset_message * temp3 = (ARQ_reset_message*) sdu_le->data;
				if (temp1->mgmt_msg_type == ARQ_FEEDBACK_MSG || temp2->mgmt_msg_type == ARQ_DISCARD_MSG || temp3->mgmt_msg_type == ARQ_RESET_MSG)
				{
					is_arq_feedback_present = 1;
					//printf("This pdu has arq mmm type %d %d %d\n",temp1->mgmt_msg_type,temp2->mgmt_msg_type,temp3->mgmt_msg_type);
				}
			    }
                            lep->next = NULL;

                            lep->length = psh->length - psh_len;
                            lep->type = MAC_SDU;

	                     // modify the left logical element
                            sdu_le->data += lep->length;
		            sdu_le->length -= lep->length;
                            sdu_le->type = MAC_SDU_FRAG;
                            //sdu->length -= lep->length;
                       
                        }
                        // connect the sdu or sdu fragment logical element
                        prev_le->next = lep;
                        pdu->length += lep->length;
                        prev_le = lep;
                        // calculate the legacy bandwidth
                        left_bw -= lep->length;
                        left_bw -= leh->length;

                    }

                    // construct the generic mac header
                    gmh->len = pdu->length;
                    gmh->type = ((is_arq_feedback_present & 0x01) << 4) | ((is_extended_type & 0x01) << 3) | ((is_fsh_present & 0x01) << 2) | ((is_psh_present & 0x01) <<1) | ((is_ffsh_present & 0x01) | (is_gmsh_present & 0x01));

                    // save the current pdu
                    pre_pdu = pdu;

                    // update the left allowed capacity 
                    burst_bytes_num -= pdu->length;
                    if (burst_bytes_num < 0)
                    {
                        // release the logical packet and logical element container
                        if (arq_blk_list)
                        {
                            release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
                        }
                        if (sdulist)
                        {
                            release_sducontainer(sdulist, is_release_payload, le_tobe_discard);
                        }
                        FLOG_ERROR("frag_packe: not assign enough burst bytes. 5\n");
			*status=1;
                        return 1;
                    }	
            }
        }

        }
        else
        {
		int tot_overhead=0;
		if (is_encrypt == 1) tot_overhead = gmh_len + crc_len + IVLEN + TAGLEN;
		else tot_overhead = gmh_len + crc_len;
 
            // packing is not enabled
            if (is_frag)
            {
                if (is_arq)
                {
                    // recalculate the maximum mac PDU size
                    // if arq is enabled, the real payloadlength should be the value that satisfy the follwing:
                    // [max num of arq block*arq_block_size]<specified payload length
                    max_arqblock_num = (pdu_size-tot_overhead-fsh_len) / arq_block_size;
                    payloadlength = max_arqblock_num * arq_block_size;
                    pdu_size = payloadlength + tot_overhead + fsh_len;
                    current_fsn = sdu->element_head->start_bsn;
                }
                else 
                {
                    // in case the fragmentation is enabled, bu the arq is not enabled
                    payloadlength = pdu_size - tot_overhead - fsh_len;
                }
            }
            else 
            {
                if (is_arq)
                {
                     // look at the first sdu to determine the current pdu size
                     pdu_size = sdu->element_head->length+tot_overhead+fsh_len;
                     current_fsn = sdu->element_head->start_bsn;
                }
                else
                {
                     pdu_size = sdu->element_head->length+tot_overhead;
                }
            }

 
            while(sdu)
            {
                 // check whether there are still sdu or sdu fragment available
                 if (sdu->element_head->length == 0)
                 {
                     // this sdu has been totally processed, so move to the next sdu
                     sdu = sdu->next;
                     
                     if (sdu == NULL)
                     {
                        break;
                     }
                 }
		else
		{
                     if (is_arq)
                     {
                        current_fsn = sdu->element_head->start_bsn;
                     }
		}
                // get the sdu element from the sdu packet
                sdu_le = sdu->element_head;

                sdu_len = sdu_le->length;
                if (sdu_le->type == MAC_SDU )
                {
                    is_first=1;
                                
                 }
                 else if (sdu_le->type == MAC_SDU_FRAG)
                 {
                     if (sdu_le->blk_type == FIRST_FRAGMENT)
                     {
                         is_first = 1;
                         sdu_le->blk_type = CONTINUING_FRAGMENT;
                     }
                     else
                     {
                         is_first=0;
                     }
                              
                 }
                
                // now try to generate the PDUs
                // generate one pdu for each loop
                pdu = (logical_packet*) malloc(sizeof(logical_packet));
                memset(pdu, 0, sizeof(logical_packet));
                pdu->cid = cid;
                if (is_encrypt ==1 ) pdu->length = crc_len+IVLEN+TAGLEN;
		else pdu->length = crc_len;
                pdu->element_head = NULL;
                pdu->next= NULL;
                prev_le = NULL;
                if (pre_pdu== NULL)
                {
                    pre_pdu = pdu;
                    (*pdulisthead) = pdu;
                }
                else
                {
                    pre_pdu->next = pdu;
                }

                leh = (logical_element*) malloc(sizeof(logical_element));
                memset(leh, 0, sizeof(logical_element));
                leh->type = MAC_GENERIC_HEADER;
                leh->length = gmh_len;
                leh->next = NULL;

                gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
                memset(gmh, 0, sizeof(generic_mac_hdr));
                gmh->ht = 0;
                gmh->ec = is_encrypt;
                gmh->ci = is_crc;
                if (is_encrypt == 1)
                {
                    gmh->eks = (u_int8_t) my_aksn;
                }
                else
                {
                    gmh->eks = 0;
                }
                gmh->esf = 0;
                gmh->rsv = 0;
                gmh->cid = cid;

                leh->data = (u_char *) gmh;
                is_fsh_present = 0;
                is_psh_present = 0;
                is_arq_feedback_present = 0;
                is_gmsh_present = 0;
                is_ffsh_present = 0;

                // connect the generic mac headerlogical element
                if (prev_le == NULL)
                {
                    // in case this is the first sdu
                    pdu->element_head = leh;
                }
                else
                {
                    prev_le->next = leh;
                }
                
                pdu->length += leh->length;
                prev_le = leh;

                // generate one logical element , subheader, 
                if (is_frag || is_arq)
                {
                    leh = (logical_element*) malloc(sizeof(logical_element));
                    memset(leh, 0, sizeof(logical_element));
                    leh->type = frag_hdr_type;
                    leh->length = fsh_len;
                    // generate the fragment subheader
                    fsh=(frag_sub_hdr*)malloc(sizeof(frag_sub_hdr));
                    memset(fsh, 0, sizeof(frag_sub_hdr));
                    if (!is_arq)
                    {
                        current_fsn = (current_fsn + 1) % modulo;
                    }

                    fsh->fsn = current_fsn;
                    leh->data = (u_char *) fsh;
		    // connect the logical element head to this frag_sub_hdr
		      prev_le->next = leh;
                    prev_le = leh;
                    pdu->length += leh->length;
                    is_fsh_present = 1;
                }

	         // generate the second logical element, payload
                lep = (logical_element*) malloc(sizeof(logical_element));
                memset(lep, 0, sizeof(logical_element));
                lep->data = sdu_le->data;
		lep->start_bsn = sdu_le->start_bsn;
            
			    if (sdu->cid <= BASIC_CID_MAX_VALUE && sdu->cid >=BASIC_CID_MIN_VALUE)
			    {
				ARQ_feedback_message *temp1 = (ARQ_feedback_message*) sdu_le->data;
					ARQ_discard_message * temp2 = (ARQ_discard_message*) sdu_le->data;
					ARQ_reset_message * temp3 = (ARQ_reset_message*) sdu_le->data;
				if (temp1->mgmt_msg_type == ARQ_FEEDBACK_MSG || temp2->mgmt_msg_type == ARQ_DISCARD_MSG || temp3->mgmt_msg_type == ARQ_RESET_MSG)
				{
					is_arq_feedback_present = 1;
					//printf("This pdu has arq mmm type %d %d %d\n",temp1->mgmt_msg_type,temp2->mgmt_msg_type,temp3->mgmt_msg_type);
				}
			    }
                lep->next = NULL;
                if (is_frag)
                {
                    if(sdu_le->length > payloadlength)
                    {  
                        // update several fields
                        if (is_first)
                        {
                            fsh->fc = FIRST_FRAGMENT; // the first fragments
                        }
                        else
                        {
                            fsh->fc = CONTINUING_FRAGMENT; // the middle fragments
                        }
                        fsh->rsv = 0;
                        // in this case, fragment must happen, since sdulength is larger than the payloadlength
                        lep->type = MAC_SDU_FRAG;
                        lep->length = payloadlength;

						// modify the left logical element
                        sdu_le->data += payloadlength;
						sdu_le->length -=payloadlength;
                        //sdu->length -=payloadlength;
                        sdu_le->type = MAC_SDU_FRAG;
                        if (is_arq)
                        {
                            //sdu->element_head->start_bsn = current_fsn + max_arqblock_num;
                            current_fsn += max_arqblock_num;
                        }
                    }
                    else
                    {
                        // the payload is less than one pdu, in this case, the fragmentation is not needed
                        if (is_first)
                        {
                            // a new sdu, so the fragmentation is not needed
                            if (sdu_le->type == MAC_SDU_FRAG && sdu_le->blk_type == CONTINUING_FRAGMENT)
                            {
                                fsh->fc = FIRST_FRAG;
                                lep->type = MAC_SDU_FRAG;
                            }
                            else
                            {
                                fsh->fc=UN_FRAGMENTED; // fragmentation is not needed
                                lep->type = MAC_SDU;
                            }

                        }
                        else
                        {
                            // a legacy fragment, since the left length is less than a pdu payload, so it is the last fragment
                            if (sdu_le->type == MAC_SDU_FRAG && sdu_le->blk_type == CONTINUING_FRAGMENT)
                            {
                                fsh->fc = CONTINUING_FRAG;
                                lep->type = MAC_SDU_FRAG;
                            }
                            else
                            {
                                fsh->fc=LAST_FRAG; // fragmentation is not needed
                                lep->type = MAC_SDU_FRAG;
                            }

                        }
                        fsh->rsv = 0;
                        // the pdu payload length is the sdu length or legacy fragment length
                        lep->length = sdu_le->length;

                    	   // modify the left logical element
                        //printf("####################the pointer to le is: %lx, end of the pointer is: %lx \n",sdu_le->data, sdu_le->data+sdu_le->length);
                        //sdu_le->data += sdu_le->length;
						//sdu_le->length = 0;
                        //sdu->length = 0;
                    }
                }
                else
                {
                    lep->type = MAC_SDU;
                    lep->length = sdu_le->length;
                    // modify the left logical element
                    //sdu_le->data += sdu_le->length;
					//sdu_le->length = 0;
                    //sdu->length = 0;
                }

                // connect the subheader and payload logical element
                leh->next = lep;
                pdu->length += lep->length;

		gmh->len=pdu->length;
                gmh->type = ((is_arq_feedback_present & 0x01) << 4) | ((is_extended_type & 0x01) << 3) | ((is_fsh_present & 0x01) << 2) | ((is_psh_present & 0x01) <<1) | ((is_ffsh_present & 0x01) | (is_gmsh_present & 0x01));

                // save the current pdu
                pre_pdu = pdu;

                // update the left allowed capacity 
                burst_bytes_num -= pdu->length;
                if (burst_bytes_num < 0)
                {
                    // release the logical packet and logical element container
                    if (arq_blk_list)
                    {
                        release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
                    }
                    if (sdulist)
                    {
                        release_sducontainer(sdulist, is_release_payload, le_tobe_discard);
                    }
                    FLOG_ERROR("frag_packe: not assign enough burst bytes. 6.BBNum %d \n", burst_bytes_num);
		    *status=1;
                    return 1;
                }
				sdu = sdu->next;
                
         }
    }
        if (! is_arq)
        {
            if (is_frag || is_pack)
            {
                set_current_seq_no(cid, current_fsn);
            }
        }
        pdu_map = pdu_map->next;
        // release the logical packet and logical element container
        if (arq_blk_list)
        {
            release_sducontainer(arq_blk_list, is_release_payload, le_tobe_discard);
        }
        if (sdulist)
        {
            release_sducontainer(sdulist, is_release_payload, le_tobe_discard);
        }


    }
    *status=0;
#ifdef ENCRYPT_TEST
			 FLOG_INFO("GMH Values CID %d ec %d length %d\n",cid,is_encrypt,gmh->len);
#endif


    return 0;
}


