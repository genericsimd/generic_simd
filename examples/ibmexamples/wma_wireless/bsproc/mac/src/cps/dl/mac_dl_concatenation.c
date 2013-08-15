/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_dl_concatenation.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_auth.h"

#include "mac_dl_concatenation.h"

int concatenation(logical_packet* pdu_list_head, u_char* burst, int length){

    if ( (pdu_list_head == NULL) && (length == 0) )
    {
        return 0;
    }

    int ret;
    logical_packet * pdu;
    logical_element * le;
    int offset, start, left_len, hdr_len;
    generic_mac_hdr * gmh;
    frag_sub_hdr * fsh;
    pack_sub_hdr * psh;
    u_int8_t is_crc;
    int suboffset;
    pdu = pdu_list_head;
    left_len = length;
    offset = 0;
    start = 0;

unsigned long int dummy;
unsigned char iv[IVLEN+1];
int is_encrypted=0;
    while (pdu)
    {
        le = pdu->element_head;

		// Start of this PDU (#bytes away from the start of this burst)
        start += offset;
		
		// Offset of different elements within the PDU
        offset = 0;
        is_crc = 0;
	int arq_block_count=0;int arq_blk_cumulative_length=0;logical_element *le_temp;int mmi;
        while (le)
        {
            switch(le->type)
            {
                case MAC_GENERIC_HEADER:
                    gmh = (generic_mac_hdr *) le->data;
                    is_crc = gmh->ci;
                    build_gmh(gmh, (burst+start+offset), &hdr_len);
                    suboffset = GENERIC_MAC_HEADER_LEN;
                    suboffset = suboffset -1;
                    hcs_calculation((burst+start+offset), suboffset, (burst+start+offset+suboffset));
                    offset += hdr_len;
                    break;
                case FRAG_SUBHEADER:
                    fsh = (frag_sub_hdr *) le->data;
                    build_fsh(fsh, burst+start+offset, 0, &hdr_len);
                    offset += hdr_len;
                    break;
                case EXTEND_FRAG_SUBHEADER:
                    fsh = (frag_sub_hdr *) le->data;
                    build_fsh(fsh, burst+start+offset, 1, &hdr_len);
                    offset += hdr_len;
                    break;
		case PACK_SUBHEADER:
		case EXTEND_PACK_SUBHEADER:
		case MAC_SDU_FRAG:
                case MAC_SDU:
                case ARQ_BLOCK:
			if (arq_block_count==0) le_temp = le;//Count the num of  blocks to be encrypted, then come back to this pointer and start encryption
			arq_block_count++;arq_blk_cumulative_length+=le->length;
			break;
		default: 
			return 1;
	   }

            le = le->next;
        }
	if (arq_blk_cumulative_length >0)
	{
		u_char *temp_data = (u_char*)malloc(sizeof(char)*arq_blk_cumulative_length);
		int parse_offset = 0; 
		while( le_temp!=NULL && (le_temp->type == PACK_SUBHEADER || le_temp->type== EXTEND_PACK_SUBHEADER || le_temp->type == ARQ_BLOCK || le_temp->type == MAC_SDU || le_temp->type == MAC_SDU_FRAG))
		{
			if (le_temp->type==PACK_SUBHEADER)
			{
                    		psh = (pack_sub_hdr *) le_temp->data;
                    		build_psh(psh, temp_data+parse_offset, 0, &hdr_len);
                    		parse_offset  += hdr_len;
				
			}
			else
			{
				if (le_temp->type ==EXTEND_PACK_SUBHEADER)
				{
                    			psh = (pack_sub_hdr *) le_temp->data;
                    			build_psh(psh, temp_data+parse_offset, 1, &hdr_len);
                    			parse_offset  += hdr_len;
				}	
				else
				{
					memcpy(temp_data + parse_offset,le_temp->data,le_temp->length);
					parse_offset += le_temp->length;
				}
			}
			le_temp=le_temp->next;
		}

		//printf("At DL : CID %d PDU Len %d\n",pdu->cid, arq_blk_cumulative_length);
	   	is_encrypt_enabled(pdu->cid,(u_int8_t*)&is_encrypted);
		if (is_encrypted==1)
		{
			u_char key[KEYLEN+1];
			ret = get_encryption_key(pdu->cid,key,KEYLEN);
			if (ret == -1) 
			{
				//SA is in PERM_AUTH_REJECT. Skip PDU.
				FLOG_WARNING("Could not read encryption key during transmission.\n");
				pdu = pdu->next;
				continue;
			}
			dummy=0;

			for (mmi=0;mmi<IVLEN;mmi++)
			{
				float tempno = rand();
				tempno=tempno/RAND_MAX;
				tempno=tempno*255;
				iv[mmi]=(unsigned char)tempno;
			}
			iv[IVLEN]='\0';

			memcpy (burst+start+offset,iv,IVLEN); //at the beginning of the ARQ blocks, insert IV

#ifdef ENCRYPT_TEST
			FLOG_INFO("\nDATA ENTERING ENCRYPT\n");
			for (mmi=0;mmi<arq_blk_cumulative_length;mmi++)
			{	
				FLOG_DEBUG("%d\t",temp_data[mmi]);
			}
			FLOG_DEBUG("\n");
		
#endif
	        	u_char *temp = (unsigned char*) malloc(sizeof(u_char)*(arq_blk_cumulative_length+TAGLEN+64));
			u_char *temp_plaintext = (u_char*)malloc(sizeof(u_char)*arq_blk_cumulative_length);

			memset(temp_plaintext,0,arq_blk_cumulative_length);
			memcpy(temp_plaintext,temp_data,arq_blk_cumulative_length);
	
	
			int retvar=0;
			retvar = mac_des_crypto(temp_plaintext,arq_blk_cumulative_length,iv,key,temp,1);//the 1 implies encrypt.
			dummy = arq_blk_cumulative_length;

			if (retvar == 0) {FLOG_FATAL("ENCRYPT FAILURE : Likely because one of IVLEN, KEYLEN, DATALEN are not within the right range stipulated by AES. Check AES documentation.\n");exit(-1);}

			memcpy (burst+start+offset+IVLEN, temp,dummy);
			

			free(temp_plaintext);
			free(temp);
 
			offset+=arq_blk_cumulative_length+IVLEN+TAGLEN;                 
		}
		
		else
		{
			memcpy(burst+start+offset, temp_data, arq_blk_cumulative_length);
                    	offset += arq_blk_cumulative_length;
		}	
		free(temp_data);   
	
	
	} //END OF IF(ARQ_BLK_CUMULATIVE_LENTH >0 CONDITION)
        
	// calculated crc
        if (is_crc)
        {
            suboffset = MAC_CRC_LEN;
            crc_calculation(burst+start, offset, burst+start + offset);
            offset += MAC_CRC_LEN;
        }
        
        left_len -= offset;


#ifdef ENCRYPT_TEST
 	if (left_len < 0 ) FLOG_ERROR("left len leq %d : THis is a major problem. Means allocations are not right \n",left_len);
        FLOG_DEBUG("num of arq blocks per pdu%d\n",arq_block_count);
#endif


	pdu = pdu->next;
    }
    int mac_hdr_len;
    mac_hdr_len = GENERIC_MAC_HEADER_LEN;
    while(left_len >= mac_hdr_len)
    {
        gmh = (generic_mac_hdr *)malloc(sizeof(generic_mac_hdr));
        memset(gmh, 0, sizeof(generic_mac_hdr));
        gmh->ht = 0;
        gmh->ec = 1;//is_encrypted;
        gmh->type = 0;
        gmh->esf = 0;
        gmh->ci = 0;
        gmh->eks = 0;
        gmh->rsv = 0;
        if (left_len > MAX_PDU_LEN)
		{
			gmh->len = MAX_PDU_LEN;
		}
		else
		{
        gmh->len = left_len;
		}
        gmh->cid = PADDING_CID;
        build_gmh(gmh, burst+length - left_len, &hdr_len);
        suboffset = GENERIC_MAC_HEADER_LEN;
        suboffset = suboffset -1;
        hcs_calculation(burst + length - left_len, suboffset, burst + length - left_len + suboffset);
        left_len = left_len - gmh->len;
	free(gmh);
        gmh = NULL;
        
    }
	// If any bytes (fewer than GMH_LEN) are remaining, pad with 0xFF
        while (left_len >0)
        {
            burst[length - left_len] = 0xff;
            left_len --;
    }

    return 0;

}

int release_logical_pdu_list(logical_packet* pdu_list_head){
    logical_packet *lp, *pre_lp;
    logical_element *le, *pre_le;
    lp = pdu_list_head;

    while (lp)
    {
        pre_lp = lp;
        lp = pre_lp->next;
        
        le = pre_lp->element_head;

        while (le)
        {
            pre_le = le;
            le = pre_le->next;
            if (pre_le->type == MAC_GENERIC_HEADER || pre_le->type == FRAG_SUBHEADER || pre_le->type == EXTEND_FRAG_SUBHEADER ||
                pre_le->type == PACK_SUBHEADER || pre_le->type == EXTEND_PACK_SUBHEADER ) 
            {
                free(pre_le->data);
                pre_le->data = NULL;
            }
            else 
            {
                // for sdu, arq block, sdu frag, the memory release is done by the IRL
                // here we release it temporarily
               // free(pre_le->data);
               //  pre_le->data = NULL;
            }

            free(pre_le);
            pre_le = NULL;
        }

        free(pre_lp);
        pre_lp = NULL;
        
    }
    free(lp);
    lp = NULL;
    pdu_list_head = NULL;

    return 0;

}
