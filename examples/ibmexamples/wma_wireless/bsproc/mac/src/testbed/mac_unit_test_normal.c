/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_unit_test_normal.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_unit_test_normal.h"

int status=0;

#if 0

/* int main(int argc,char *argv[])  */
/* {  */
/*     int total_test_case_num = 0; */
/*     int failed_test_case_num = 0; */
/*     // Testing of generic header builder and parser */
/*     total_test_case_num++; */
/*     failed_test_case_num += testgmh(); */

/*     // Testing of fragmentation subheader builder and parser */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfsh(); */

/*     // Testing of packing subheader builder and parser */
/*     total_test_case_num++; */
/*     failed_test_case_num += testpsh(); */

/*     // Testing of frame prefix builder and parser */
/*     total_test_case_num++; */
/*     failed_test_case_num += testframeprefix(); */

/*     // Testing of pdu crc calculation and verification */
/*     total_test_case_num++; */
/*     failed_test_case_num += testhcs(); */

/*     // Testing of pdu hcs calculation and verification */
/*     total_test_case_num++; */
/*     failed_test_case_num += testcrc(); */

/*     // Testing of pdu concatenation and parsing */
/*     total_test_case_num++; */
/*     failed_test_case_num += testconcat_parse(); */

/*     // Testing of fixed sdu packing */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_fixsdu(); */

/*     // Testing of fragmentation and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_fragonly(); */

/*     // Testing of packing and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_packonly(); */

/*     // Testing of both fragmentation and packing and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_fragpack(); */

/*     // Testing of the arq block and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_arqonly(); */

/*     // Testing of the arq block with fragmentation and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_arqfrag(); */

/*     // Testing of the arq block with packing and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_arqpack(); */

/*     // Testing of the arq block with packing and reassembly */
/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_assembe_arqfragpack(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testcps_framework(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_fragments_fragonly(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_fragments_fragpack(); */

/*     //total_test_case_num++; */
/*     //failed_test_case_num += testfrag_pack_fragments_arqfragpack(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_arqretrans_arqfragpack(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_arqretrans_arqpack(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_arqretrans_arqfrag(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testfrag_pack_arqretrans_arqonly(); */

/*     //total_test_case_num++; */
/*     //failed_test_case_num += testreassembly_arqretrans_arqfragpack(); */

/*     total_test_case_num++; */
/*     failed_test_case_num += testbrqueue(); */

/*     printf("total test case num: %d, failed test case num: %d \n", total_test_case_num, failed_test_case_num); */
 
/*    return 0; */
/* } */

int testgmh(){

    // first generate the generic mac header
    generic_mac_hdr* gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
    generic_mac_hdr * post_gmh = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
    int length, post_length;
    u_int8_t is_test_failed = 0;

    /*
      data format that store in memory
      00011100  01000011
      11101000  00000000
      10000001  00111111
      integer format for these 6 bytes
      28 67 232 0 129 63
    */    

    gmh->ht = 0;
    gmh->ec = 0;
    gmh->type = 28;
    gmh->esf = 0;
    gmh->ci = 1;
    gmh->eks = 0;
    gmh->rsv = 0;
    gmh->len = 1000;
    gmh->cid = 129;
    gmh->hcs = 63;

    u_char phy_gmh[6];


    build_gmh(gmh, phy_gmh, &length);

    if (phy_gmh[0] != 28 ||
        phy_gmh[1] != 67 ||
        phy_gmh[2] != 232 ||
        phy_gmh[3] != 0 ||
        phy_gmh[4] != 129 ||
        phy_gmh[5] != 63 )
    {
        ERROR_TRACE("testgmh: builder is not correct. \n");
        is_test_failed = 1;
    }
    

    parse_gmh(phy_gmh, post_gmh, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testgmh: length is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->ht != post_gmh->ht)
    {
        ERROR_TRACE("testgmh: ht field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->ec != post_gmh->ec)
    {
        ERROR_TRACE("testgmh: ec field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->type != post_gmh->type)
    {
        ERROR_TRACE("testgmh: type field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->esf != post_gmh->esf)
    {
        ERROR_TRACE("testgmh: esf field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->ci != post_gmh->ci)
    {
        ERROR_TRACE("testgmh: ci field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->eks != post_gmh->eks)
    {
        ERROR_TRACE("testgmh: eks field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->rsv != post_gmh->rsv)
    {
        ERROR_TRACE("testgmh: rsv field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->len != post_gmh->len)
    {
        ERROR_TRACE("testgmh: ec field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->cid != post_gmh->cid)
    {
        ERROR_TRACE("testgmh: cid field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (gmh->hcs != post_gmh->hcs)
    {
        ERROR_TRACE("testgmh: hcs field is not in accordance. \n");
        is_test_failed = 1;
    }
    
    free(gmh);
    free(post_gmh);
    if (is_test_failed)
    {
        printf("testgmh: test failed! \n");   
        return 1;
    }else{
        printf("testgmh: test success! \n");
        return 0;
    }

}

int testfsh(){

    // first generate the fragmentation sub header
    frag_sub_hdr* fsh = (frag_sub_hdr*) malloc(sizeof(frag_sub_hdr));
    frag_sub_hdr * post_fsh = (frag_sub_hdr*) malloc(sizeof(frag_sub_hdr));
    int length, post_length;
    u_int8_t is_test_failed = 0;
    u_char phy_fsh[2];

    u_int8_t is_extend = 0;
   
    /*
      data format in memory
      10110000
      integer format is 176
    */ 
    fsh->fc = 2;
    fsh->fsn = 6;
    fsh->rsv = 0;


    build_fsh(fsh, phy_fsh, is_extend, &length);

    if (phy_fsh[0] != 176)
    {
        ERROR_TRACE("testfsh: non-extended builder is not correct. \n");
        is_test_failed = 1;
    }


    parse_fsh(phy_fsh, post_fsh, is_extend, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testfsh(non-extended): length is not in accordance. \n");
        is_test_failed = 1;
    }
    
    if (fsh->fc!= post_fsh->fc)
    {
        ERROR_TRACE("testfsh(non-extended): ht field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (fsh->fsn != post_fsh->fsn)
    {
        ERROR_TRACE("testfsh(non-extended): fsh field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (fsh->rsv != post_fsh->rsv)
    {
        ERROR_TRACE("testfsh(non-extended): rsv field is not in accordance. \n");
        is_test_failed = 1;
    }

    is_extend = 1;
    
    /*
      data format in memory
      01011111  01000000
      integer format: 95 64
    */

    fsh->fc = 1;
    fsh->fsn = 1000;
    fsh->rsv = 0;

    build_fsh(fsh, phy_fsh, is_extend, &length);

    if (phy_fsh[0] != 95 || phy_fsh[1] != 64)
    {
        ERROR_TRACE("testfsh: extended builder is not correct. \n");
        is_test_failed = 1;
    }

    parse_fsh(phy_fsh, post_fsh, is_extend, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testfsh(extended): length is not in accordance. \n");
        is_test_failed = 1;
    }
    
    if (fsh->fc!= post_fsh->fc)
    {
        ERROR_TRACE("testfsh(extended): ht field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (fsh->fsn != post_fsh->fsn)
    {
        ERROR_TRACE("testfsh(extended): fsh field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (fsh->rsv != post_fsh->rsv)
    {
        ERROR_TRACE("testfsh(extended): rsv field is not in accordance. \n");
        is_test_failed = 1;
    }

    free(fsh);
    free(post_fsh);

    if (is_test_failed)
    {
        printf("testfsh: test failed! \n");
        return 1;
    } else {   
        printf("testfsh: test success! \n");
        return 0;
    }
}

int testpsh(){

    // first generate the packing sub header
    pack_sub_hdr* psh = (pack_sub_hdr*) malloc(sizeof(pack_sub_hdr));
    pack_sub_hdr* post_psh = (pack_sub_hdr*) malloc(sizeof(pack_sub_hdr));
    int length, post_length;
    u_int8_t is_test_failed = 0;
    u_char phy_psh[3];

    u_int8_t is_extend = 0;
    
    /*
      data format store in memory
      10110011  11101000
      integer format is: 179 232

    */    
 
    psh->fc = 2;
    psh->fsn = 6;
    psh->length = 1000;


    build_psh(psh, phy_psh, is_extend, &length);

    if (phy_psh[0] != 179 || phy_psh[1] != 232)
    {
        ERROR_TRACE("testpsh: non-extended builder is not correct. \n");
        is_test_failed = 1;
    }

    parse_psh(phy_psh, post_psh, is_extend, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testpsh(non-extended): length is not in accordance. \n");
        is_test_failed = 1;
    }
    
    if (psh->fc!= post_psh->fc)
    {
        ERROR_TRACE("testpsh(non-extended): fc field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (psh->fsn != post_psh->fsn)
    {
        ERROR_TRACE("testpsh(non-extended): fsh field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (psh->length != post_psh->length)
    {
        ERROR_TRACE("testpsh(non-extended): len field is not in accordance. \n");
        is_test_failed = 1;
    }

    is_extend = 1;
   
    /*
      data format store in memory
      01011111 01000011
      11101000
      integer format is 95 67 232
    */
 
    psh->fc = 1;
    psh->fsn = 1000;
    psh->length = 1000;


    build_psh(psh, phy_psh, is_extend, &length);

    if (phy_psh[0] != 95 || phy_psh[1] != 67 || phy_psh[2] !=232)
    {
        ERROR_TRACE("testpsh: extended builder is not correct. \n");
        is_test_failed = 1;
    }

    parse_psh(phy_psh, post_psh, is_extend, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testpsh(extended): length is not in accordance. \n");
        is_test_failed = 1;
    }
    
    if (psh->fc!= post_psh->fc)
    {
        ERROR_TRACE("testpsh(extended): fc field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (psh->fsn != post_psh->fsn)
    {
        ERROR_TRACE("testpsh(extended): fsh field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (psh->length != post_psh->length)
    {
        ERROR_TRACE("testpsh(extended): len field is not in accordance. \n");
        is_test_failed = 1;
    }

    free(psh);
    free(post_psh);
    
    if ( is_test_failed )
    { 
        printf("testpsh: test failed! \n");
        return 1;
    } else {
        printf("testpsh: test success! \n");
        return 0;
    }
}

int testframeprefix(){

    // first generate the dl subframe prefix
    dl_subframe_prefix* prefix = (dl_subframe_prefix*) malloc(sizeof(dl_subframe_prefix));
    dl_subframe_prefix* post_prefix = (dl_subframe_prefix*) malloc(sizeof(dl_subframe_prefix));
    int length, post_length;
    u_int8_t is_test_failed = 0;
    u_char phy_prefix[3];

    u_int8_t is_128fft = 0;
    
    /*
      data format store in memory
      10110001  11011001
      10100000
      integer format is: 177 217 160

    */    
 
    prefix->used_subchannel_bitmap =  44;
    prefix->rsv1 = 0;
    prefix->repetition_coding_indication = 3;
    prefix->coding_indication = 5;
    prefix->dl_map_length = 154;


    build_dlframeprefix(prefix, phy_prefix, is_128fft, &length);

    if (phy_prefix[0] != 177 || phy_prefix[1] != 217 || phy_prefix[2] != 160)
    {
        ERROR_TRACE("testframeprefix(non-128fft):  builder is not correct. \n");
        is_test_failed = 1;
    }

    parse_dlframeprefix(phy_prefix, post_prefix, is_128fft, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testframeprefix(non-128fft): length is not in accordance. \n");
        is_test_failed = 1;
    }
    
    if (prefix->used_subchannel_bitmap!= post_prefix->used_subchannel_bitmap)
    {
        ERROR_TRACE("testframeprefix(non-128fft): used_subchannel_bitmap field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (prefix->repetition_coding_indication!= post_prefix->repetition_coding_indication)
    {
        ERROR_TRACE("testframeprefix(non-128fft): repetition_coding_indication field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (prefix->coding_indication!= post_prefix->coding_indication)
    {
        ERROR_TRACE("testframeprefix(non-128fft): coding_indication field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (prefix->dl_map_length != post_prefix->dl_map_length)
    {
        ERROR_TRACE("testframeprefix(non-128fft): dl_map_length field is not in accordance. \n");
        is_test_failed = 1;
    }

    is_128fft = 1;
    phy_prefix[0] = 0;
    phy_prefix[1] = 0;
    phy_prefix[2] = 0;
    /*
      data format store in memory
      10010111 01010000
      integer format is 151 80
    */
 
    prefix->used_subchannel_bitmap =  1;
    prefix->rsv1 = 0;
    prefix->repetition_coding_indication = 1;
    prefix->coding_indication = 3;
    prefix->dl_map_length = 21;


    build_dlframeprefix(prefix, phy_prefix, is_128fft, &length);

    if (phy_prefix[0] != 151 || phy_prefix[1] != 80 )
    {
        ERROR_TRACE("testframeprefix(128fft):  builder is not correct. \n");
        is_test_failed = 1;
    }

    parse_dlframeprefix(phy_prefix, post_prefix, is_128fft, &post_length);

    if (length != post_length)
    {
        ERROR_TRACE("testframeprefix(128fft): length is not in accordance. \n");
        is_test_failed = 1;
    }
    
    if (prefix->used_subchannel_bitmap!= post_prefix->used_subchannel_bitmap)
    {
        ERROR_TRACE("testframeprefix(128fft): used_subchannel_bitmap field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (prefix->repetition_coding_indication!= post_prefix->repetition_coding_indication)
    {
        ERROR_TRACE("testframeprefix(128fft): repetition_coding_indication field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (prefix->coding_indication!= post_prefix->coding_indication)
    {
        ERROR_TRACE("testframeprefix(128fft): coding_indication field is not in accordance. \n");
        is_test_failed = 1;
    }

    if (prefix->dl_map_length != post_prefix->dl_map_length)
    {
        ERROR_TRACE("testframeprefix(128fft): dl_map_length field is not in accordance. \n");
        is_test_failed = 1;
    }

    free(prefix);
    free(post_prefix);
    
    if ( is_test_failed )
    { 
        printf("testframeprefix: test failed! \n");
        return 1;
    } else {
        printf("testframeprefix: test success! \n");
        return 0;
    }
}

int testconcat_parse(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);

    logical_packet * lp_header;
    logical_packet * lp = (logical_packet *) malloc(sizeof(logical_packet));


    lp_header = lp;

   // the first pdu
    logical_element * le_header = (logical_element *) malloc(sizeof(logical_element)); 

    lp->element_head = le_header;

    generic_mac_hdr * gmh1 = (generic_mac_hdr *) malloc(sizeof(generic_mac_hdr));
    gmh1->ht = 0;
    gmh1->ec = 0;
    gmh1->type = 12;
    gmh1->esf = 0;
    gmh1->ci = 1;
    gmh1->eks = 0;
    gmh1->rsv = 0;
    gmh1->len = 100;
    gmh1->cid = 129;
    gmh1->hcs = 0;

    le_header->length = GENERIC_MAC_HEADER_LEN;
    le_header->data = (u_char *)gmh1;
    le_header->type = MAC_GENERIC_HEADER;

    frag_sub_hdr * fsh = (frag_sub_hdr *) malloc(sizeof(frag_sub_hdr));
    fsh->fc = 2;
    fsh->fsn = 1000;
    fsh->rsv = 0;

    logical_element* le_fsh = (logical_element *) malloc(sizeof(logical_element));
    le_fsh->length = 2;
    le_fsh->data = (u_char *)fsh;
    le_fsh->type = EXTEND_FRAG_SUBHEADER;

    le_header->next = le_fsh;

    logical_element* le_frag1 = (logical_element *) malloc(sizeof(logical_element));
    
    // substract the generic mac header, the crc and the fragmentation subheader
    le_frag1->length =100-6 - 4 - 2;

    le_frag1->data = (u_char *) malloc(le_frag1->length);

    le_frag1->type = MAC_SDU_FRAG;

    le_fsh->next = le_frag1;

    le_frag1->next = NULL;

    lp->length = 100;
    lp->cid = 129;

    // the second pdu

    logical_packet* lp2 = (logical_packet *)malloc(sizeof(logical_packet));

    lp->next = lp2;
    
    le_header = (logical_element *) malloc(sizeof(logical_element)); 

    lp2->element_head = le_header;

    generic_mac_hdr * gmh2 = (generic_mac_hdr *) malloc(sizeof(generic_mac_hdr));
    gmh2->ht = 0;
    gmh2->ec = 0;
    gmh2->type = 10;
    gmh2->esf = 0;
    gmh2->ci = 1;
    gmh2->eks = 0;
    gmh2->rsv = 0;
    gmh2->len = 100;
    gmh2->cid = 129;
    gmh2->hcs = 0;

    le_header->length = GENERIC_MAC_HEADER_LEN;
    le_header->data = (u_char *)gmh2;
    le_header->type = MAC_GENERIC_HEADER;

    pack_sub_hdr * psh = (pack_sub_hdr *) malloc(sizeof(pack_sub_hdr));
    psh->fc = 1;
    psh->fsn = 1001;
    psh->length = 50;

    le_fsh = (logical_element *) malloc(sizeof(logical_element));
    le_fsh->length = 3;
    le_fsh->data = (u_char *)psh;
    le_fsh->type = EXTEND_PACK_SUBHEADER;

    le_header->next = le_fsh;

    logical_element* le_frag2 = (logical_element *) malloc(sizeof(logical_element));

    le_frag2->length =50-3;

    le_frag2->data = (u_char *) malloc(le_frag2->length);

    le_frag2->type = MAC_SDU_FRAG;

    le_fsh->next = le_frag2;

    psh = (pack_sub_hdr *) malloc(sizeof(pack_sub_hdr));
    psh->fc = 0;
    psh->fsn = 1002;
    // substract the previous frag, the generic mac header, the crc
    psh->length = 100-50-6-4;

    le_fsh = (logical_element *) malloc(sizeof(logical_element));
    le_fsh->length = 3;
    le_fsh->data = (u_char *)psh;
    le_fsh->type = EXTEND_PACK_SUBHEADER;

    le_frag2->next = le_fsh;

    logical_element* le_frag3 = (logical_element *) malloc(sizeof(logical_element));
    
    // substract the previous frag, the generic mac header, the crc, the packing subheader 
    le_frag3->length =100-50-6-4-3;

    le_frag3->data = (u_char *) malloc(le_frag3->length);

    le_frag3->type = MAC_SDU;

    le_fsh->next = le_frag3;

    le_frag3->next = NULL;

    lp2->length = 100;
    lp2->cid = 129;

    lp->next = lp2;


    logical_packet * lp3 = (logical_packet *) malloc(sizeof(logical_packet));

    le_header = (logical_element *) malloc(sizeof(logical_element)); 

    lp3->element_head = le_header;

    generic_mac_hdr * gmh3 = (generic_mac_hdr *) malloc(sizeof(generic_mac_hdr));
    gmh3->ht = 0;
    gmh3->ec = 0;
    gmh3->type = 0;
    gmh3->esf = 0;
    gmh3->ci = 0;
    gmh3->eks = 0;
    gmh3->rsv = 0;
    gmh3->len = 100;
    gmh3->cid = 130;
    gmh3->hcs = 0;

    le_header->length = GENERIC_MAC_HEADER_LEN;
    le_header->data = (u_char *)gmh3;
    le_header->type = MAC_GENERIC_HEADER;

    logical_element* le_frag4 = (logical_element *) malloc(sizeof(logical_element));
    // substract the mac header, and the crc 
    le_frag4->length =100-6;

    le_frag4->data = (u_char *) malloc(le_frag4->length);

    le_frag4->type = MAC_SDU;

    le_header->next = le_frag4;

    le_frag4->next = NULL;

    lp->length = 100;
    lp->cid = 130;

    lp2->next = lp3;

    lp3->next = NULL;


    // now begin testing

    u_int8_t is_test_failed = 0;
     
    crc_init( POLY ); 
    
    u_char * burst = (u_char *) malloc(307);

    // begin concatenation
    concatenation(lp_header, burst, 307, &schedule);

    // generate the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;

    initialize_pduq(&(ul_pduq_header));

    initialize_pduframeq(&pdu_frameq, 78);

    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);
    
    // parsing the pdu from the burst
    parse_burst_pdu(pdu_frameq->frame_no, burst, 307, pdu_frameq, NULL, NULL);

    // check the pdu queue
    pdu_cid_queue * pducidq;
    get_pducidq(pdu_frameq, 129, &(pducidq));
    // now check the content of the pdu
    if (pducidq->pdu_num != 2)
    {
        ERROR_TRACE("testconcat_parse: parsed pdu number  is not in accordance. \n");
        is_test_failed = 1;
    }
    // test the first pdu
    logical_element* pdu = pducidq->head;
    generic_mac_hdr* post_gmh1 = (generic_mac_hdr *) malloc(sizeof(generic_mac_hdr));
    int length;
    parse_gmh(pdu->data, post_gmh1, &length);

    if (gmh1->ci != post_gmh1->ci || gmh1->cid != post_gmh1->cid || gmh1->ec != post_gmh1->ec ||
        gmh1->eks != post_gmh1->eks || gmh1->esf != post_gmh1->esf ||
        gmh1->ht != post_gmh1->ht || gmh1->len != post_gmh1->len || gmh1->rsv != post_gmh1->rsv ||
        gmh1->type != post_gmh1->type)
    {
        ERROR_TRACE("testconcat_parse: the first pdu mac header is not in accordance. \n");
        is_test_failed = 1;
  
    }
    // test the second pdu
    pdu = pducidq->tail;

    generic_mac_hdr* post_gmh2 = (generic_mac_hdr *) malloc(sizeof(generic_mac_hdr));;
    parse_gmh(pdu->data, post_gmh2, &length);

    if (gmh2->ci != post_gmh2->ci || gmh2->cid != post_gmh2->cid || gmh2->ec != post_gmh2->ec ||
        gmh2->eks != post_gmh2->eks || gmh2->esf != post_gmh2->esf ||
        gmh2->ht != post_gmh2->ht || gmh2->len != post_gmh2->len || gmh2->rsv != post_gmh2->rsv ||
        gmh2->type != post_gmh2->type)
    {
        ERROR_TRACE("testconcat_parse: the second pdu mac header is not in accordance. \n");
        is_test_failed = 1;
  
    }

    
    // release the pdu queue
    release_pducidq(pducidq);
    
    get_pducidq(pdu_frameq, 130, &(pducidq));

    // now check the content of the pdu
    if (pducidq->pdu_num != 1)
    {
        ERROR_TRACE("testconcat_parse: parsed pdu number  is not in accordance. \n");
        is_test_failed = 1;
    }
    // test the first pdu
    pdu = pducidq->head;
    generic_mac_hdr* post_gmh3 = (generic_mac_hdr *) malloc(sizeof(generic_mac_hdr));
   
    parse_gmh(pdu->data, post_gmh3, &length);

    if (gmh3->ci != post_gmh3->ci || gmh3->cid != post_gmh3->cid || gmh3->ec != post_gmh3->ec ||
        gmh3->eks != post_gmh3->eks || gmh3->esf != post_gmh3->esf ||
        gmh3->ht != post_gmh3->ht || gmh3->len != post_gmh3->len || gmh3->rsv != post_gmh3->rsv ||
        gmh3->type != post_gmh3->type)
    {
        ERROR_TRACE("testconcat_parse: the third pdu mac header is not in accordance. \n");
        is_test_failed = 1;
  
    }

    // release the pdu queue

    release_pducidq(pducidq);

    release_pduframeq(pdu_frameq);

    release_pduq(ul_pduq_header);

    free(burst);

    // release the logical packet        
    free(le_frag1->data);
    free(le_frag2->data);
    free(le_frag3->data);
    free(le_frag4->data);
    release_logical_pdu_list(lp_header);

    if ( is_test_failed )
    { 
        printf("testconcat_parse: test failed! \n");
        return 1;
    } else {
        printf("testconcat_parse: test success! \n");
        return 0;
    }

}

int testcrc(){    
    u_int8_t is_test_failed = 0;
  /*     
    * Test vector from IEEE 802.16e standard 6.3.3.5.2.1     
    * add four bytes for CRC results     
    */    
    unsigned char test_vector[] = {0x40, 0x40, 0x1A, 0x06, 0xC4, 0x5A, 0xBC, 0xF6,
                                                    0x57, 0x21, 0xE7, 0x55, 0x36, 0xC8, 0x27, 0xA8, 
                                                    0xD7, 0x1B, 0x43, 0x2C, 0xA5, 0x48, 0x00, 0x00,
                                                    0x00, 0x00};    
    unsigned int length = 22;    

    /*     
      * Result of the test vector from IEEE 802.16e standard 6.3.3.5.2.1     
      */

#ifdef _OFDM_    
    unsigned char vector_result[] = {0xCB, 0xB6, 0x5F, 0x48};
#else    /* OFDM */    
    unsigned char vector_result[] = {0x1B, 0xD1, 0xBA, 0x21};
#endif    /* OFDMA */    

    /*     
      * Compute the CRC result of the test vector.     
      */    

    crc_init( POLY );    

    crc_calculation( test_vector, length, &test_vector[length] );    

    if (test_vector[length] != vector_result[0] ||test_vector[length+1] !=  vector_result[1] || 
        test_vector[length+2] != vector_result[2] ||test_vector[length+3] !=  vector_result[3] )
    {
        is_test_failed = 1;
        printf("testcrc: the crc calculation field is not in accordance! \n");
    }

    if (crc_verification ( test_vector, length, &test_vector[length]))
    {
        is_test_failed = 1;
        printf("testcrc: the crc verification is not correct! \n");
    }

    if ( is_test_failed )
    { 
        printf("testcrc: test failed! \n");
        return 1;
    } else {
        printf("testcrc: test success! \n");
        return 0;
    }  

} 

int testhcs(){    
    u_int8_t is_test_failed = 0;
    
    u_char test_vector[]={0x80, 0xAA, 0xAA, 0x0F, 0x0F, 0x00};
    u_int8_t test_result = 0xD5;
    int length = 5;
    hcs_calculation(test_vector, length, test_vector+length);

    if (test_vector[length] != test_result)
    {
        is_test_failed = 1;
        printf("testhcs: hcs calculation result is wrong! \n");
    }
    
    if (hcs_verification(test_vector, length, test_vector+length))
    {
        is_test_failed = 1;
    }
    // printf("testhcs: the hcs result is : %d \n", test_vector[5]);
    if ( is_test_failed )
    { 
        printf("testhcs: test failed! \n");
        return 1;
    } else {
        printf("testhcs: test success! \n");
        return 0;
    }  

} 

int testfrag_pack_assembe_fixsdu(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i;

    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 1;
    con1->is_frag_enabled = 0;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 500;
    con1->macsdu_size = 150;
    con1->modulo = 8;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 0, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 0;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 1;
    con2->is_frag_enabled = 0;
    con2->is_pack_enabled = 1;
    con2->macpdu_size = 210;
    con2->macsdu_size = 100;
    con2->modulo = 2048;    

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 0, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 0;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 1;
    con3->is_frag_enabled = 0;
    con3->is_pack_enabled = 0;
    con3->macpdu_size = 310;
    con3->macsdu_size = 100;
    con3->modulo = 2048;   

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;

    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1->length = 150;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = 150;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(150);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< 150; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2->length = 150;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = 150;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(150);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< 150; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3->length = 150;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = 150;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(150);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< 150; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4->length = 150;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = 150;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(150);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< 150; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1->length = 100;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = 100;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(100);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< 100; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2->length = 100;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = 100;
    cidq2_le2->start_bsn = 0;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(100);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< 100; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1->length = 100;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = 100;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(100);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< 100; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2->length = 100;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = 100;
    cidq3_le2->start_bsn = 0;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(100);
    cidq3_mod2 = 100;
    cidq3_inc2 = 0;
    for (i=0; i< 100; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    // 612+220+20 = 852
    burst1->burst_bytes_num = 852;
    burst1->pdu_num = 4;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = 600;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    burst1_trans_map2->num_bytes = 200;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 210+5 = 215
    burst2->burst_bytes_num = 215;
    burst2->pdu_num = 1;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = 200;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe = (physical_subframe *) malloc(sizeof(physical_subframe));
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    //5.1  generate the physical frame
    logical_element* le_tobe_discard = NULL;
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL;
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;
    // release the burst map
    release_logical_subframe_map(dl_map);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);

    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);

 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == 150)
        {
            for (i=0; i< 150; i++)
            {
                if (lp->element_head->data[i] != cidq1_data1[i]){
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the first sdu in the first connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == 150)
        {
            for (i=0; i< 150; i++)
            {
                if (lp->element_head->data[i] != cidq1_data2[i])
                {
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the second sdu in the first connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == 150)
        {
            for (i=0; i< 150; i++)
            {
                if (lp->element_head->data[i] != cidq1_data3[i])
                {
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the third sdu in the first connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == 150)
        {
            for ( i=0; i< 150; i++)
            {
                if (lp->element_head->data[i] != cidq1_data4[i])
                {
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the fourth sdu in the first connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fixsdu: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == 100)
        {
            for (i=0; i< 100; i++)
            {
                if (lp->element_head->data[i] != cidq3_data1[i]){
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the first sdu in the thirdconnection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == 100)
        {
            for (i=0; i< 100; i++)
            {
                if (lp->element_head->data[i] != cidq3_data2[i])
                {
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the second sdu in the third connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fixsdu: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == 100)
        {
            for (i=0; i< 100; i++)
            {
                if (lp->element_head->data[i] != cidq2_data1[i]){
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the first sdu in the second connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == 100)
        {
            for (i=0; i< 100; i++)
            {
                if (lp->element_head->data[i] != cidq2_data2[i])
                {
                    is_test_failed = 1;
                    printf("testfrag_pack_assemble_fixsdu: sdu content for the second sdu in the second connection is not correct! \n");
                    break;
                }
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fixsdu: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fixsdu: error cid and sdu num for the second sdu cid queue! \n");
    }   

    // release the connection
    release_connection_queue(con1);

    // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);
    
    // release the fragment queue
    release_fragq(fragq);

    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_fixsdu: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_fixsdu: test success! \n");
        return 0;
    }
    
}


int testfrag_pack_assembe_fragonly(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 0;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 8;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 0, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 0;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 1;
    con2->is_pack_enabled = 0;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 0, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 0;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 1;
    con3->is_pack_enabled = 0;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 150;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 96;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 201;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 100;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 300;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 0;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 88;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 2;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 0;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 750;
    burst1->pdu_num = 10;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 475;
    burst2->pdu_num = 6;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list) , &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

    // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);

 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);

    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);
    
    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fragonly: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fragonly: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragonly: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragonly: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fragonly: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq,0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_fragonly: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_fragonly: test success! \n");
        return 0;
    }
    
}

int testfrag_pack_assembe_packonly(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 0;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 8;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 0, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 0;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 0;
    con2->is_pack_enabled = 1;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 0, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 0;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 0;
    con3->is_pack_enabled = 1;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 30;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 10;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 60;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 10;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 30;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 0;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 70;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 0;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 350;
    burst1->pdu_num = 4;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 100;
    burst2->pdu_num = 1;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;


    // release the burst map
    release_logical_subframe_map(dl_map);
    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_packonly: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_packonly: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_packonly: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_packonly: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_packonly: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);
    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_packonly: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_packonly: test success! \n");
        return 0;
    }
    
}


int testfrag_pack_assembe_fragpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 8;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 0, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 0;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 1;
    con2->is_pack_enabled = 1;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 0, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 0;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 1;
    con3->is_pack_enabled = 1;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 200;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 100;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 100;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 100;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 0;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 70;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 0;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 600;
    burst1->pdu_num = 4;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 300;
    burst2->pdu_num = 1;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;


       // release the burst map
    release_logical_subframe_map(dl_map);
    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fragpack: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragpack: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragpack: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragpack: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragpack: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fragpack: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragpack: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragpack: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_fragpack: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_fragpack: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_fragpack: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);
    release_pduq(ul_pduq_header);
    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_fragpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_fragpack: test success! \n");
        return 0;
    }
    
}

int testfrag_pack_assembe_arqonly(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 0;
    con1->is_pack_enabled = 0;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 1, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 1;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 0;
    con2->is_pack_enabled = 0;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    
    con2->arq->arq_block_size = 30;

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 1, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 1;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 0;
    con3->is_pack_enabled = 0;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   
    con3->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 200;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 1;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 8;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 9;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 100;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 5;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 4;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 60;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 3;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

    // release the burst map
    release_logical_subframe_map(dl_map);
    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqonly: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqonly: error cid and sdu num for the third sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqonly: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqonly: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqonly: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);
    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_arqonly: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_arqonly: test success! \n");
        return 0;
    }

    return 0;
    
}

int testfrag_pack_assembe_arqfrag(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 0;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 1, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 1;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 1;
    con2->is_pack_enabled = 0;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    
    con2->arq->arq_block_size = 30;

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 1, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 1;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 1;
    con3->is_pack_enabled = 0;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   
    con3->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 200;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 1;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 8;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 9;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 100;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 5;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 4;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 60;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 3;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

    // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);

    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);
    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqfrag: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqfrag: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfrag: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfrag: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqfrag: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_arqfrag: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_arqfrag: test success! \n");
        return 0;
    }

    return 0;
    
}

int testfrag_pack_assembe_arqpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 0;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 1, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 1;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 0;
    con2->is_pack_enabled = 1;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    
    con2->arq->arq_block_size = 30;

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 1, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 1;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 0;
    con3->is_pack_enabled = 1;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   
    con3->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 1;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 4;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 5;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 63;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 5;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 3;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 60;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 3;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;
        // release the burst map
    release_logical_subframe_map(dl_map);
    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);

    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);
    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqpack: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqpack: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqpack: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqpack: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqpack: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_arqpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_arqpack: test success! \n");
        return 0;
    }

    return 0;
    
}

int testfrag_pack_assembe_arqfragpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 1, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 1;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 1;
    con2->is_pack_enabled = 1;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    
    con2->arq->arq_block_size = 30;

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 1, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 1;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 1;
    con3->is_pack_enabled = 1;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   
    con3->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 1;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 4;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 5;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 100;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 5;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 4;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 60;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 3;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+cidq1_lp4_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid3;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid3;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq3_lp1_len+cidq3_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid2;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid2;
    burst2_trans_map1->num_bytes = cidq2_lp1_len+cidq2_lp2_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqfragpack: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqfragpack: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_assemble_arqfragpack: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_assemble_arqfragpack: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_assemble_arqfragpack: error cid and sdu num for the second sdu cid queue! \n");
    } 
   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_assemble_arqfragpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_assemble_arqfragpack: test success! \n");
        return 0;
    }

    return 0;
    
    
}

int testcps_framework(){
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 8;

    // 1.2 the second connection
    connection* con2;
    con2 = NULL;
    u_int16_t cid2 = 113;
    add_connection(cid2, 0, &con2);
    con2->con_type = CONN_DATA;
    con2->current_seq_no = 0;
    con2->fsn_size = 11;
    con2->is_arq_enabled = 0;
    con2->is_crc_included = 1;
    con2->is_encrypt_enabled = 0;
    con2->is_fixed_macsdu_length = 0;
    con2->is_frag_enabled = 1;
    con2->is_pack_enabled = 1;
    con2->macpdu_size = 100;
    con2->macsdu_size = 0;
    con2->modulo = 2048;    

    // 1.3 the third connection
    connection* con3;
    con3 = NULL;
    u_int16_t cid3 = 112;
    add_connection(cid3, 0, &con3);
    con3->con_type = CONN_DATA;
    con3->current_seq_no = 0;
    con3->fsn_size = 11;
    con3->is_arq_enabled = 0;
    con3->is_crc_included = 1;
    con3->is_encrypt_enabled = 0;
    con3->is_fixed_macsdu_length = 0;
    con3->is_frag_enabled = 1;
    con3->is_pack_enabled = 1;
    con3->macpdu_size = 100;
    con3->macsdu_size = 0;
    con3->modulo = 2048;   

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 200;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 100;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 2.3 add the second sdu cid queue
    // 2.3.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid2);
    // 2.3.2 define the data structure
    logical_packet* cidq2_lp1;
    logical_packet* cidq2_lp2;
    logical_element* cidq2_le1;
    logical_element* cidq2_le2;
    u_char* cidq2_data1;
    u_char* cidq2_data2;
    int cidq2_lp1_len, cidq2_lp2_len;
    int cidq2_mod1, cidq2_mod2;
    int cidq2_inc1, cidq2_inc2;

    // 2.3.3 generate the first logical packet
    cidq2_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp1->cid = cid2;
    cidq2_lp1_len = 100;
    cidq2_lp1->length = cidq2_lp1_len;
    cidq2_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le1->type = MAC_SDU;
    cidq2_le1->blk_type = NO_FRAGMENTATION;
    cidq2_le1->length = cidq2_lp1_len;
    cidq2_le1->start_bsn = 0;
    cidq2_le1->next = NULL;
    
    cidq2_data1 = (u_char*) malloc(cidq2_lp1_len);
    cidq2_mod1 = 8;
    cidq2_inc1 = 0;
    for (i=0; i< cidq2_lp1_len ; i++)
    {
        cidq2_data1[i] = (cidq2_inc1++) % cidq2_mod1;
    }
    
    cidq2_le1->data = cidq2_data1;
    cidq2_lp1->element_head = cidq2_le1;

    // 2.3.4 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp1);

    // 2.3.5 generate the second logical packet
    cidq2_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq2_lp2->cid = cid2;
    cidq2_lp2_len = 100;
    cidq2_lp2->length = cidq2_lp2_len;
    cidq2_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq2_le2->type = MAC_SDU;
    cidq2_le2->blk_type = NO_FRAGMENTATION;
    cidq2_le2->length = cidq2_lp2_len;
    cidq2_le2->start_bsn = 0;
    cidq2_le2->next = NULL;
    
    cidq2_data2 = (u_char*) malloc(cidq2_lp2_len);
    cidq2_mod2 = 100;
    cidq2_inc2 = 0;
    for (i=0; i< cidq2_lp2_len; i++)
    {
        cidq2_data2[i] = (cidq2_inc2++) % cidq2_mod2;
    }
    
    cidq2_le2->data = cidq2_data2;
    cidq2_lp2->element_head = cidq2_le2;

    // 2.3.6 enqueue the logical packet
    enqueue_sduq(sduq, cid2, cidq2_lp2);
    
     // 2.4 add the second sdu cid queue
    // 2.4.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid3);
    // 2.4.2 define the data structure
    logical_packet* cidq3_lp1;
    logical_packet* cidq3_lp2;
    logical_element* cidq3_le1;
    logical_element* cidq3_le2;
    u_char* cidq3_data1;
    u_char* cidq3_data2;
    int cidq3_lp1_len, cidq3_lp2_len;
    int cidq3_mod1, cidq3_mod2;
    int cidq3_inc1, cidq3_inc2;

    // 2.4.3 generate the first logical packet
    cidq3_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp1->cid = cid3;
    cidq3_lp1_len = 70;
    cidq3_lp1->length = cidq3_lp1_len;
    cidq3_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le1->type = MAC_SDU;
    cidq3_le1->blk_type = NO_FRAGMENTATION;
    cidq3_le1->length = cidq3_lp1_len;
    cidq3_le1->start_bsn = 0;
    cidq3_le1->next = NULL;
    
    cidq3_data1 = (u_char*) malloc(cidq3_lp1_len);
    cidq3_mod1 = 8;
    cidq3_inc1 = 0;
    for (i=0; i< cidq3_lp1_len; i++)
    {
        cidq3_data1[i] = (cidq3_inc1++) % cidq3_mod1;
    }
    
    cidq3_le1->data = cidq3_data1;
    cidq3_lp1->element_head = cidq3_le1;

    // 2.4.4 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp1);

    // 2.4.5 generate the second logical packet
    cidq3_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq3_lp2->cid = cid3;
    cidq3_lp2_len = 70;
    cidq3_lp2->length = cidq3_lp2_len;
    cidq3_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq3_le2->type = MAC_SDU;
    cidq3_le2->blk_type = NO_FRAGMENTATION;
    cidq3_le2->length = cidq3_lp2_len;
    cidq3_le2->start_bsn = 0;
    cidq3_le2->next = NULL;
    
    cidq3_data2 = (u_char*) malloc(cidq3_lp2_len);
    cidq3_mod2 = cidq3_lp2_len;
    cidq3_inc2 = 0;
    for (i=0; i< cidq3_lp2_len; i++)
    {
        cidq3_data2[i] = (cidq3_inc2++) % cidq3_mod2;
    }
    
    cidq3_le2->data = cidq3_data2;
    cidq3_lp2->element_head = cidq3_le2;

    // 2.4.6 enqueue the logical packet
    enqueue_sduq(sduq, cid3, cidq3_lp2);

   //begin to invoke the dl and ul cps controller

    mac_cps_handler();

    // waiting for the initialization and the downlink scheduling
    usleep(10*1000);
    subframe_queue * dl_subframeq;
    subframe_queue * ul_subframeq;
    physical_subframe * phy_subframe;
    get_subframe_queue(1, &dl_subframeq);
    dequeue_subframe(dl_subframeq, &phy_subframe);

    get_subframe_queue(0, &ul_subframeq);
    enqueue_subframe(ul_subframeq, phy_subframe);
    
    // waiting for the uplink processing
    usleep(10*1000);
    // check if the reassembly is correctly executed
    // get the uplink sduq
    get_sduq(&ul_sduq, 0);
    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testcps_framework: error cid and sdu num for the first sdu cid queue! \n");
    }

    // check the second sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid3 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the first sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the first sdu in the third connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid3 && lp->length == cidq3_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq3_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the second sdu in the third connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the second sdu in the third connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testcps_framework: error cid and sdu num for the second sdu cid queue! \n");
    }

     // check the third sdu cid queue
    sducidq =sducidq->next;
    lp = sducidq->head;
    // check the second sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid2 && sducidq->sdu_num == 2)
    {
        // check the first sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the first sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the first sdu in the second connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid2 && lp->length == cidq2_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq2_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testcps_framework: sdu content for the second sdu in the second connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testcps_framework: error cid and length for the second sdu in the second connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testcps_framework: error cid and sdu num for the second sdu cid queue! \n");
    } 
   

    // release both the data structure used in upplink and downlink cps controller
    release_mac_cps_handler();
    // release the connection
    release_connection_queue(con1);

    
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);
    free(cidq2_data1);
    free(cidq2_data2);
    free(cidq3_data1);
    free(cidq3_data2);

     if ( is_test_failed )
    { 
        printf("testcps_framework: test failed! \n");
        return 1;
    } else {
        printf("testcps_framework: test success! \n");
        return 0;
    }
    usleep(1000); 
}

int testbrqueue(){

    /*
      data format that store in memory
      00011100  01000011
      11101000  00000000
      10000001  00111111
      integer format for these 6 bytes
      28 67 232 0 129 63

    */    
    // first generate the packing sub header
    u_char br[] = {28,67, 232, 0, 129, 63};
    u_int8_t is_test_failed = 0;


    br_queue* brq;

    initialize_br_queue(&brq);
    enqueue_br_queue(brq, 110, 128, 1, 6, br);

    enqueue_br_queue(brq, 110, 128, 1, 6, br);

    enqueue_br_queue(brq, 110, 129, 2, 6, br);
    
    get_br_queue(&brq);

    mgt_msg * br_msgs = NULL;
    mgt_msg * pre_br_msg = NULL;
    dequeue_br_cid_queue(brq,  128, &br_msgs);

    if (br_msgs->cid  != 128 || br_msgs->length != 6 || br_msgs->msg_type != 1)
    {
        ERROR_TRACE("testbrqueue: insert field is not correct. \n");
        is_test_failed = 1;
    }

    if (br[0] != ((u_char*)(br_msgs->data))[0] ||br[1] != ((u_char*)(br_msgs->data))[1] || 
        br[2] != ((u_char*)(br_msgs->data))[2] ||br[3] != ((u_char*)(br_msgs->data))[3] ||
        br[4] != ((u_char*)(br_msgs->data))[4] || br[5] != ((u_char*)(br_msgs->data))[5])
    {
        is_test_failed = 1;
        printf("testbrqueue: the data field is not in accordance! \n");
    }

    pre_br_msg = br_msgs;

    br_msgs = br_msgs->next;

    if (br_msgs->cid  != 128 || br_msgs->length != 6 || br_msgs->msg_type != 1)
    {
        ERROR_TRACE("testbrqueue: insert field is not correct. \n");
        is_test_failed = 1;
    }

    if (br[0] != ((u_char*)(br_msgs->data))[0] ||br[1] != ((u_char*)(br_msgs->data))[1] || 
        br[2] != ((u_char*)(br_msgs->data))[2] ||br[3] != ((u_char*)(br_msgs->data))[3] ||
        br[4] != ((u_char*)(br_msgs->data))[4] || br[5] != ((u_char*)(br_msgs->data))[5])
    {
        is_test_failed = 1;
        printf("testbrqueue: the data field is not in accordance! \n");
    }

    // release the br_msgs

    free(pre_br_msg->data);
    free(br_msgs->data);
    free(pre_br_msg);
    free(br_msgs);


    // dequeue the second br cid queue
    dequeue_br_cid_queue(brq,  129, &br_msgs);

    if (br_msgs->cid  != 129 || br_msgs->length != 6 || br_msgs->msg_type != 2)
    {
        ERROR_TRACE("testbrqueue: insert field is not correct. \n");
        is_test_failed = 1;
    }

    if (br[0] != ((u_char*)(br_msgs->data))[0] ||br[1] != ((u_char*)(br_msgs->data))[1] || 
        br[2] != ((u_char*)(br_msgs->data))[2] ||br[3] != ((u_char*)(br_msgs->data))[3] ||
        br[4] != ((u_char*)(br_msgs->data))[4] || br[5] != ((u_char*)(br_msgs->data))[5])
    {
        is_test_failed = 1;
        printf("testbrqueue: the data field is not in accordance! \n");
    }

    free(br_msgs->data);
    free(br_msgs);

    // release the br cid queue

    release_br_cid_queue(brq, 129);

    release_br_queue(brq);
    
    if ( is_test_failed )
    { 
        printf("testbrqueue: test failed! \n");
        return 1;
    } else {
        printf("testbrqueue: test success! \n");
        return 0;
    }
}

int testfrag_pack_fragments_fragonly(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 0;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 8;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 150;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 96;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 201;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 750;
    burst1->pdu_num = 10;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+cidq1_lp2_len+cidq1_lp3_len+2;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 1;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    burst1_trans_map2->num_bytes = 3;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 475;
    burst2->pdu_num = 6;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid1;
    burst2_pdu_map1->sdu_num = 1;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid1;
    burst2_trans_map1->num_bytes = 5;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

    // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);

 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);

    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);
    
    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragonly: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragonly: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragonly: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragonly: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragonly: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragonly: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == 10)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragonly: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragonly: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_fragments_fragonly: error cid and sdu num for the first sdu cid queue! \n");
    }

    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq,0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_fragments_fragonly: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_fragments_fragonly: test success! \n");
        return 0;
    }
    
}

int testfrag_pack_fragments_fragpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 0, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 3;
    con1->is_arq_enabled = 0;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 8;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 200;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 0;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 100;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 0;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 300;
    burst1->pdu_num = 4;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 2;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+100;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 3;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes =100+ 10;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 1;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid1;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid1;
    burst2_trans_map1->num_bytes = 90+10;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;


       // release the burst map
    release_logical_subframe_map(dl_map);
    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_fragpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_fragpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_fragments_fragpack: error cid and sdu num for the first sdu cid queue! \n");
    }
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);
    release_pduq(ul_pduq_header);
    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_fragments_fragpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_fragments_fragpack: test success! \n");
        return 0;
    }
    
}

int testfrag_pack_fragments_arqfragpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 0;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 1;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = MAC_SDU;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 4;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->cid = cid1;
    cidq1_lp4_len = 10;
    cidq1_lp4->length = cidq1_lp4_len;
    cidq1_le4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4->type = MAC_SDU;
    cidq1_le4->blk_type = NO_FRAGMENTATION;
    cidq1_le4->length = cidq1_lp4_len;
    cidq1_le4->start_bsn = 5;
    cidq1_le4->next = NULL;
    
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }
    
    cidq1_le4->data = cidq1_data4;
    cidq1_lp4->element_head = cidq1_le4;

    // 2.2.10 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_burst_map* burst2;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    logical_pdu_map* burst2_pdu_map1;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;
    transport_sdu_map* burst2_trans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 2;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 4;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len+30;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = 30;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;
    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid1;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid1;
    burst2_trans_map1->num_bytes = 29+ cidq1_lp3_len+cidq1_lp4_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
    burst1->next = burst2;
    burst2->next = NULL;

    burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    burst2_pdu_map1->arq_sdu_map = NULL;
    burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 2;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_arqfragpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_arqfragpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_arqfragpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_arqfragpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_arqfragpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_arqfragpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_fragments_arqfragpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_fragments_arqfragpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_fragments_arqfragpack: error cid and sdu num for the first sdu cid queue! \n");
    }

   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_fragments_arqfragpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_fragments_arqfragpack: test success! \n");
        return 0;
    }

    return 0;
    
    
}

int testfrag_pack_arqretrans_arqfragpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);


    
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4_arq1;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4_arq1;
    logical_element* cidq1_le4_arq2;
    logical_element* cidq1_le4_arq3;
    logical_element* cidq1_le4_arq4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 5;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 6;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = ARQ_BLOCK;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    // enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet, the fourth logical paket is used as retransmitted arq block

    cidq1_lp4_len = 100;
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }

    // the first arq block
    cidq1_lp4_arq1 = (logical_packet *) malloc(sizeof(logical_packet));

    cidq1_lp3->next = cidq1_lp4_arq1;
    
    cidq1_lp4_arq1->cid = cid1;
    cidq1_lp4_arq1->next = NULL;

    cidq1_lp4_arq1->length = 100;
    cidq1_le4_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq1->type = ARQ_BLOCK;
    cidq1_le4_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le4_arq1->length = 30;
    cidq1_le4_arq1->start_bsn = 1;
    cidq1_le4_arq1->next = NULL;
    
    cidq1_le4_arq1->data = cidq1_data4;
    cidq1_lp4_arq1->element_head = cidq1_le4_arq1;

    // the second arq block

    cidq1_le4_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq2->type = ARQ_BLOCK;
    cidq1_le4_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq2->length = 30;
    cidq1_le4_arq2->start_bsn = 2;
    cidq1_le4_arq2->next = NULL;
    
    cidq1_le4_arq2->data = cidq1_data4+30;
    cidq1_le4_arq1->next = cidq1_le4_arq2;

    // the third arq block
    cidq1_le4_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq3->type = ARQ_BLOCK;
    cidq1_le4_arq3->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq3->length = 30;
    cidq1_le4_arq3->start_bsn = 3;
    cidq1_le4_arq3->next = NULL;
    
    cidq1_le4_arq3->data = cidq1_data4+60;
     cidq1_le4_arq2->next = cidq1_le4_arq3;

    // the fourth arq block
    cidq1_le4_arq4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq4->type = ARQ_BLOCK;
    cidq1_le4_arq4->blk_type = LAST_FRAGMENT;
    cidq1_le4_arq4->length = 10;
    cidq1_le4_arq4->start_bsn = 4;
    cidq1_le4_arq4->next = NULL;
    
    cidq1_le4_arq4->data = cidq1_data4+90;
    cidq1_le4_arq3->next = cidq1_le4_arq4;
    cidq1_le4_arq4->next = NULL;

    // 2.2.10 enqueue the logical packet
   // enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;

    arq_retrans_sdu_map* burst1_retrans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 1;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 2;

    burst1_retrans_map1  =(arq_retrans_sdu_map*) malloc(sizeof(arq_retrans_sdu_map));
    burst1_retrans_map1->arq_retransmit_block = cidq1_lp3;
    burst1_pdu_map1->arq_sdu_map = burst1_retrans_map1;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq1_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;

/*    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid1;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid1;
    burst2_trans_map1->num_bytes = 29+ cidq1_lp3_len+cidq1_lp4_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
*/
    burst1->next = NULL;

    //burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    //burst2_pdu_map1->arq_sdu_map = NULL;
   // burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 1;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfragpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfragpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfragpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfragpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfragpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfragpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfragpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfragpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_arqretrans_arqfragpack: error cid and sdu num for the first sdu cid queue! \n");
    }

   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    release_sducontainer(cidq1_lp3, 0, NULL);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_arqretrans_arqfragpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_arqretrans_arqfragpack: test success! \n");
        return 0;
    }

    return 0;
    
    
}

int testfrag_pack_arqretrans_arqpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 0;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);


    
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4_arq1;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4_arq1;
    logical_element* cidq1_le4_arq2;
    logical_element* cidq1_le4_arq3;
    logical_element* cidq1_le4_arq4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 5;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 6;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = ARQ_BLOCK;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    // enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet, the fourth logical paket is used as retransmitted arq block

    cidq1_lp4_len = 100;
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }

    // the first arq block
    cidq1_lp4_arq1 = (logical_packet *) malloc(sizeof(logical_packet));

    cidq1_lp3->next = cidq1_lp4_arq1;
    
    cidq1_lp4_arq1->cid = cid1;
    cidq1_lp4_arq1->next = NULL;

    cidq1_lp4_arq1->length = 100;
    cidq1_le4_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq1->type = ARQ_BLOCK;
    cidq1_le4_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le4_arq1->length = 30;
    cidq1_le4_arq1->start_bsn = 1;
    cidq1_le4_arq1->next = NULL;
    
    cidq1_le4_arq1->data = cidq1_data4;
    cidq1_lp4_arq1->element_head = cidq1_le4_arq1;

    // the second arq block

    cidq1_le4_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq2->type = ARQ_BLOCK;
    cidq1_le4_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq2->length = 30;
    cidq1_le4_arq2->start_bsn = 2;
    cidq1_le4_arq2->next = NULL;
    
    cidq1_le4_arq2->data = cidq1_data4+30;
    cidq1_le4_arq1->next = cidq1_le4_arq2;

    // the third arq block
    cidq1_le4_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq3->type = ARQ_BLOCK;
    cidq1_le4_arq3->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq3->length = 30;
    cidq1_le4_arq3->start_bsn = 3;
    cidq1_le4_arq3->next = NULL;
    
    cidq1_le4_arq3->data = cidq1_data4+60;
     cidq1_le4_arq2->next = cidq1_le4_arq3;

    // the fourth arq block
    cidq1_le4_arq4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq4->type = ARQ_BLOCK;
    cidq1_le4_arq4->blk_type = LAST_FRAGMENT;
    cidq1_le4_arq4->length = 10;
    cidq1_le4_arq4->start_bsn = 4;
    cidq1_le4_arq4->next = NULL;
    
    cidq1_le4_arq4->data = cidq1_data4+90;
    cidq1_le4_arq3->next = cidq1_le4_arq4;
    cidq1_le4_arq4->next = NULL;

    // 2.2.10 enqueue the logical packet
   // enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;

    arq_retrans_sdu_map* burst1_retrans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 1;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 2;

    burst1_retrans_map1  =(arq_retrans_sdu_map*) malloc(sizeof(arq_retrans_sdu_map));
    burst1_retrans_map1->arq_retransmit_block = cidq1_lp3;
    burst1_pdu_map1->arq_sdu_map = burst1_retrans_map1;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq1_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;

/*    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid1;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid1;
    burst2_trans_map1->num_bytes = 29+ cidq1_lp3_len+cidq1_lp4_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
*/
    burst1->next = NULL;

    //burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    //burst2_pdu_map1->arq_sdu_map = NULL;
   // burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 1;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_arqretrans_arqpack: error cid and sdu num for the first sdu cid queue! \n");
    }

   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    release_sducontainer(cidq1_lp3, 0, NULL);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_arqretrans_arqpack: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_arqretrans_arqpack: test success! \n");
        return 0;
    }

    return 0;
    
    
}

int testfrag_pack_arqretrans_arqfrag(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 0;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);


    
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4_arq1;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4_arq1;
    logical_element* cidq1_le4_arq2;
    logical_element* cidq1_le4_arq3;
    logical_element* cidq1_le4_arq4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 5;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 1;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 6;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 2;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = ARQ_BLOCK;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 3;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    // enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet, the fourth logical paket is used as retransmitted arq block

    cidq1_lp4_len = 100;
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 4;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }

    // the first arq block
    cidq1_lp4_arq1 = (logical_packet *) malloc(sizeof(logical_packet));

    cidq1_lp3->next = cidq1_lp4_arq1;
    
    cidq1_lp4_arq1->cid = cid1;
    cidq1_lp4_arq1->next = NULL;

    cidq1_lp4_arq1->length = 100;
    cidq1_le4_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq1->type = ARQ_BLOCK;
    cidq1_le4_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le4_arq1->length = 30;
    cidq1_le4_arq1->start_bsn = 1;
    cidq1_le4_arq1->next = NULL;
    
    cidq1_le4_arq1->data = cidq1_data4;
    cidq1_lp4_arq1->element_head = cidq1_le4_arq1;

    // the second arq block

    cidq1_le4_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq2->type = ARQ_BLOCK;
    cidq1_le4_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq2->length = 30;
    cidq1_le4_arq2->start_bsn = 2;
    cidq1_le4_arq2->next = NULL;
    
    cidq1_le4_arq2->data = cidq1_data4+30;
    cidq1_le4_arq1->next = cidq1_le4_arq2;

    // the third arq block
    cidq1_le4_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq3->type = ARQ_BLOCK;
    cidq1_le4_arq3->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq3->length = 30;
    cidq1_le4_arq3->start_bsn = 3;
    cidq1_le4_arq3->next = NULL;
    
    cidq1_le4_arq3->data = cidq1_data4+60;
     cidq1_le4_arq2->next = cidq1_le4_arq3;

    // the fourth arq block
    cidq1_le4_arq4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq4->type = ARQ_BLOCK;
    cidq1_le4_arq4->blk_type = LAST_FRAGMENT;
    cidq1_le4_arq4->length = 10;
    cidq1_le4_arq4->start_bsn = 4;
    cidq1_le4_arq4->next = NULL;
    
    cidq1_le4_arq4->data = cidq1_data4+90;
    cidq1_le4_arq3->next = cidq1_le4_arq4;
    cidq1_le4_arq4->next = NULL;

    // 2.2.10 enqueue the logical packet
   // enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;

    arq_retrans_sdu_map* burst1_retrans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 1;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 2;

    burst1_retrans_map1  =(arq_retrans_sdu_map*) malloc(sizeof(arq_retrans_sdu_map));
    burst1_retrans_map1->arq_retransmit_block = cidq1_lp3;
    burst1_pdu_map1->arq_sdu_map = burst1_retrans_map1;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq1_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;

/*    
    // 3.5 generate the second burst map
    burst2 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst2->map_burst_index = 2;
    // 
    burst2->burst_bytes_num = 200;
    burst2->pdu_num = 2;
    burst2_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst2_pdu_map1->cid = cid1;
    burst2_pdu_map1->sdu_num = 2;

    burst2_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst2_trans_map1->cid = cid1;
    burst2_trans_map1->num_bytes = 29+ cidq1_lp3_len+cidq1_lp4_len;
    burst2_pdu_map1->transport_sdu_map = burst2_trans_map1;
    burst2_pdu_map1->next = NULL;
    burst2->pdu_map_header = burst2_pdu_map1;
*/
    burst1->next = NULL;

    //burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    //burst2_pdu_map1->arq_sdu_map = NULL;
   // burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 1;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfrag: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfrag: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfrag: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfrag: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfrag: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfrag: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqfrag: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqfrag: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_arqretrans_arqfrag: error cid and sdu num for the first sdu cid queue! \n");
    }

   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    release_sducontainer(cidq1_lp3, 0, NULL);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_arqretrans_arqfrag: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_arqretrans_arqfrag: test success! \n");
        return 0;
    }


    return 0;
    
    
}
int testfrag_pack_arqretrans_arqonly(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 0;
    con1->is_pack_enabled = 0;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);


    
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4_arq1;
    logical_element* cidq1_le1;
    logical_element* cidq1_le2;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4_arq1;
    logical_element* cidq1_le4_arq2;
    logical_element* cidq1_le4_arq3;
    logical_element* cidq1_le4_arq4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1_len = 20;
    cidq1_lp1->length = cidq1_lp1_len;
    cidq1_le1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1->type = MAC_SDU;
    cidq1_le1->blk_type = NO_FRAGMENTATION;
    cidq1_le1->length = cidq1_lp1_len;
    cidq1_le1->start_bsn = 5;
    cidq1_le1->next = NULL;
    
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 1;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1->data = cidq1_data1;
    cidq1_lp1->element_head = cidq1_le1;

    // 2.2.4 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp1);

    // 2.2.5 generate the second logical packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->cid = cid1;
    cidq1_lp2_len = 89;
    cidq1_lp2->length = cidq1_lp2_len;
    cidq1_le2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2->type = MAC_SDU;
    cidq1_le2->blk_type = NO_FRAGMENTATION;
    cidq1_le2->length = cidq1_lp2_len;
    cidq1_le2->start_bsn = 6;
    cidq1_le2->next = NULL;
    
    cidq1_data2 = (u_char*) malloc(cidq1_lp2->length);
    cidq1_mod2 = 100;
    cidq1_inc2 = 2;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }
    
    cidq1_le2->data = cidq1_data2;
    cidq1_lp2->element_head = cidq1_le2;

    // 2.2.6 enqueue the logical packet
    enqueue_sduq(sduq, cid1, cidq1_lp2);

    // 2.2.7 generate the third logical packet
    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->cid = cid1;
    cidq1_lp3_len = 30;
    cidq1_lp3->length = cidq1_lp3_len;
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = ARQ_BLOCK;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 0;
    cidq1_le3->next = NULL;
    
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 3;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    
    cidq1_le3->data = cidq1_data3;
    cidq1_lp3->element_head = cidq1_le3;

    // 2.2.8 enqueue the logical packet
    // enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet, the fourth logical paket is used as retransmitted arq block

    cidq1_lp4_len = 100;
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 4;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }

    // the first arq block
    cidq1_lp4_arq1 = (logical_packet *) malloc(sizeof(logical_packet));

    cidq1_lp3->next = cidq1_lp4_arq1;
    
    cidq1_lp4_arq1->cid = cid1;
    cidq1_lp4_arq1->next = NULL;

    cidq1_lp4_arq1->length = 100;
    cidq1_le4_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq1->type = ARQ_BLOCK;
    cidq1_le4_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le4_arq1->length = 30;
    cidq1_le4_arq1->start_bsn = 1;
    cidq1_le4_arq1->next = NULL;
    
    cidq1_le4_arq1->data = cidq1_data4;
    cidq1_lp4_arq1->element_head = cidq1_le4_arq1;

    // the second arq block

    cidq1_le4_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq2->type = ARQ_BLOCK;
    cidq1_le4_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq2->length = 30;
    cidq1_le4_arq2->start_bsn = 2;
    cidq1_le4_arq2->next = NULL;
    
    cidq1_le4_arq2->data = cidq1_data4+30;
    cidq1_le4_arq1->next = cidq1_le4_arq2;

    // the third arq block
    cidq1_le4_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq3->type = ARQ_BLOCK;
    cidq1_le4_arq3->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq3->length = 30;
    cidq1_le4_arq3->start_bsn = 3;
    cidq1_le4_arq3->next = NULL;
    
    cidq1_le4_arq3->data = cidq1_data4+60;
     cidq1_le4_arq2->next = cidq1_le4_arq3;

    // the fourth arq block
    cidq1_le4_arq4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq4->type = ARQ_BLOCK;
    cidq1_le4_arq4->blk_type = LAST_FRAGMENT;
    cidq1_le4_arq4->length = 10;
    cidq1_le4_arq4->start_bsn = 4;
    cidq1_le4_arq4->next = NULL;
    
    cidq1_le4_arq4->data = cidq1_data4+90;
    cidq1_le4_arq3->next = cidq1_le4_arq4;
    cidq1_le4_arq4->next = NULL;

    // 2.2.10 enqueue the logical packet
   // enqueue_sduq(sduq, cid1, cidq1_lp4);   
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_pdu_map* burst1_pdu_map1;
    logical_pdu_map* burst1_pdu_map2;
    transport_sdu_map* burst1_trans_map1;
    transport_sdu_map* burst1_trans_map2;

    arq_retrans_sdu_map* burst1_retrans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 1;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 2;

    burst1_retrans_map1  =(arq_retrans_sdu_map*) malloc(sizeof(arq_retrans_sdu_map));
    burst1_retrans_map1->arq_retransmit_block = cidq1_lp3;
    burst1_pdu_map1->arq_sdu_map = burst1_retrans_map1;

    burst1_trans_map1 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map1->cid = cid1;
    burst1_trans_map1->num_bytes = cidq1_lp1_len;
    burst1_pdu_map1->transport_sdu_map = burst1_trans_map1;

    burst1_pdu_map2 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map2->cid = cid1;
    burst1_pdu_map2->sdu_num = 2;

    burst1_trans_map2 = (transport_sdu_map *) malloc(sizeof(transport_sdu_map));
    burst1_trans_map2->cid = cid1;
    // test the only fragment is dequeued case
    burst1_trans_map2->num_bytes = cidq1_lp2_len;
    burst1_pdu_map2->transport_sdu_map = burst1_trans_map2;
    burst1_pdu_map2->next = NULL;

    burst1_pdu_map1->next = burst1_pdu_map2;

    burst1->pdu_map_header = burst1_pdu_map1;

    burst1->next = NULL;

    //burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;
    burst1_pdu_map2->arq_sdu_map = NULL;
    burst1_pdu_map2->mac_msg_map = NULL;
    //burst2_pdu_map1->arq_sdu_map = NULL;
   // burst2_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 1;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqonly: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqonly: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqonly: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqonly: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqonly: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqonly: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testfrag_pack_arqretrans_arqonly: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testfrag_pack_arqretrans_arqonly: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testfrag_pack_arqretrans_arqonly: error cid and sdu num for the first sdu cid queue! \n");
    }

   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    release_sducontainer(cidq1_lp3, 0, NULL);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testfrag_pack_arqretrans_arqonly: test failed! \n");
        return 1;
    } else {
        printf("testfrag_pack_arqretrans_arqonly: test success! \n");
        return 0;
    }
    
}

int testreassembly_arqretrans_arqfragpack(){
	DES_cblock key, schedule;
	DES_random_key(&key);
	DES_set_key(&key, &schedule);
    u_int8_t is_test_failed = 0;
    int i, j;
    
    // 1. generate several connection
    // 1.1 the first connection
    connection* con1;
    con1 = NULL;
    u_int16_t cid1 = 110;
    add_connection(cid1, 1, &con1);
    con1->con_type = CONN_DATA;
    con1->current_seq_no = 0;
    con1->fsn_size = 11;
    con1->is_arq_enabled = 1;
    con1->is_crc_included = 0;
    con1->is_encrypt_enabled = 0;
    con1->is_fixed_macsdu_length = 0;
    con1->is_frag_enabled = 1;
    con1->is_pack_enabled = 1;
    con1->macpdu_size = 100;
    con1->macsdu_size = 0;
    con1->modulo = 2048;
    con1->arq->arq_block_size = 30;
    con1->arq->is_order_preserved = 1;

    // 2. generate the related sdu queue

    // 2.1 initialize the sdu queue
    sdu_queue * sduq;
    initialize_sduq(&sduq, 1);

    // 2.2 add the first sdu cid queue
    // 2.2.1 generate the cid queue
    add_sdu_cid_queue(sduq, cid1);

    // the transmittion order is as follows
    // 01 45 3 2 89 6 710
    
    // 2.2.2 define the data structure
    logical_packet* cidq1_lp1; 
    logical_packet* cidq1_lp2;
    logical_packet* cidq1_lp3;
    logical_packet* cidq1_lp4;
    logical_packet* cidq1_lp5;
    logical_packet* cidq1_lp6;
    logical_packet* cidq1_lp7;
    logical_element* cidq1_le1_arq1;
    logical_element* cidq1_le1_arq2;
    logical_element* cidq1_le1_arq3;
    logical_element* cidq1_le2_arq1;
    logical_element* cidq1_le2_arq2;
    logical_element* cidq1_le2_arq3;
    logical_element* cidq1_le3;
    logical_element* cidq1_le4_arq1;
    logical_element* cidq1_le4_arq2;
    logical_element* cidq1_le4_arq3;
    logical_element* cidq1_le4_arq4;
    u_char* cidq1_data1;
    u_char* cidq1_data2;
    u_char* cidq1_data3;
    u_char* cidq1_data4;
    int cidq1_mod1, cidq1_mod2, cidq1_mod3, cidq1_mod4;
    int cidq1_inc1, cidq1_inc2, cidq1_inc3, cidq1_inc4;
    int cidq1_lp1_len, cidq1_lp2_len, cidq1_lp3_len, cidq1_lp4_len;
    // 2.2.3 generate the first logical packet

    cidq1_lp1_len = 65;
    //cidq1_lp1->length = cidq1_lp1_len;
    cidq1_data1 = (u_char*) malloc(cidq1_lp1_len);
    cidq1_mod1 = 8;
    cidq1_inc1 = 0;
    for (i=0; i< cidq1_lp1_len; i++)
    {
        cidq1_data1[i] = (cidq1_inc1++) % cidq1_mod1;
    }
    
    cidq1_le1_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1_arq1->type = ARQ_BLOCK;
    cidq1_le1_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le1_arq1->length = 30;
    cidq1_le1_arq1->start_bsn = 0;
    cidq1_le1_arq1->next = NULL;    
    cidq1_le1_arq1->data = cidq1_data1;

    cidq1_le1_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1_arq2->type = ARQ_BLOCK;
    cidq1_le1_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le1_arq2->length = 30;
    cidq1_le1_arq2->start_bsn = 1;
    cidq1_le1_arq2->next = NULL;    
    cidq1_le1_arq2->data = cidq1_data1+30;

    cidq1_le1_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le1_arq3->type = ARQ_BLOCK;
    cidq1_le1_arq3->blk_type = LAST_FRAGMENT;
    cidq1_le1_arq3->length = 5;
    cidq1_le1_arq3->start_bsn = 2;
    cidq1_le1_arq3->next = NULL;    
    cidq1_le1_arq3->data = cidq1_data1+60;


    // 2.2.5 generate the second logical packet

    cidq1_lp2_len = 89;
    cidq1_data2 = (u_char*) malloc(cidq1_lp2_len);
    cidq1_mod2 = 100;
    cidq1_inc2 = 0;
    for (i=0; i< cidq1_lp2_len; i++)
    {
        cidq1_data2[i] = (cidq1_inc2++) % cidq1_mod2;
    }

    cidq1_le2_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2_arq1->type = ARQ_BLOCK;
    cidq1_le2_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le2_arq1->length = 30;
    cidq1_le2_arq1->start_bsn = 3;
    cidq1_le2_arq1->next = NULL;
    cidq1_le2_arq1->data = cidq1_data2;

    cidq1_le2_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2_arq2->type = ARQ_BLOCK;
    cidq1_le2_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le2_arq2->length = 30;
    cidq1_le2_arq2->start_bsn = 4;
    cidq1_le2_arq2->next = NULL;
    cidq1_le2_arq2->data = cidq1_data2+30;

    cidq1_le2_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le2_arq3->type = ARQ_BLOCK;
    cidq1_le2_arq3->blk_type = LAST_FRAGMENT;
    cidq1_le2_arq3->length = 29;
    cidq1_le2_arq3->start_bsn = 5;
    cidq1_le2_arq3->next = NULL;
    cidq1_le2_arq3->data = cidq1_data2+60;


    cidq1_lp3_len = 30;
    cidq1_data3 = (u_char*) malloc(cidq1_lp3_len);
    cidq1_mod3 = 64;
    cidq1_inc3 = 0;
    for (i=0; i< cidq1_lp3_len; i++)
    {
        cidq1_data3[i] = (cidq1_inc3++) % cidq1_mod3;
    }
    cidq1_le3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le3->type = ARQ_BLOCK;
    cidq1_le3->blk_type = NO_FRAGMENTATION;
    cidq1_le3->length = cidq1_lp3_len;
    cidq1_le3->start_bsn = 6;
    cidq1_le3->next = NULL;
    cidq1_le3->data = cidq1_data3;


    // 2.2.8 enqueue the logical packet
    // enqueue_sduq(sduq, cid1, cidq1_lp3);

     // 2.2.9 generate the fourth logical packet, the fourth logical paket is used as retransmitted arq block

    cidq1_lp4_len = 100;
    cidq1_data4 = (u_char*) malloc(cidq1_lp4_len);
    cidq1_mod4 = 64;
    cidq1_inc4 = 0;
    for (i=0; i< cidq1_lp4_len; i++)
    {
        cidq1_data4[i] = (cidq1_inc4++) % cidq1_mod4;
    }

    //cidq1_lp4_arq1->length = 100;
    cidq1_le4_arq1 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq1->type = ARQ_BLOCK;
    cidq1_le4_arq1->blk_type = FIRST_FRAGMENT;
    cidq1_le4_arq1->length = 30;
    cidq1_le4_arq1->start_bsn = 7;
    cidq1_le4_arq1->next = NULL;
    
    cidq1_le4_arq1->data = cidq1_data4;
    //cidq1_lp4_arq1->element_head = cidq1_le4_arq1;

    // the second arq block

    cidq1_le4_arq2 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq2->type = ARQ_BLOCK;
    cidq1_le4_arq2->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq2->length = 30;
    cidq1_le4_arq2->start_bsn = 8;
    cidq1_le4_arq2->next = NULL;
    
    cidq1_le4_arq2->data = cidq1_data4+30;
    //cidq1_le4_arq1->next = cidq1_le4_arq2;

    // the third arq block
    cidq1_le4_arq3 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq3->type = ARQ_BLOCK;
    cidq1_le4_arq3->blk_type = CONTINUING_FRAGMENT;
    cidq1_le4_arq3->length = 30;
    cidq1_le4_arq3->start_bsn = 9;
    cidq1_le4_arq3->next = NULL;
    
    cidq1_le4_arq3->data = cidq1_data4+60;
    // cidq1_le4_arq2->next = cidq1_le4_arq3;

    // the fourth arq block
    cidq1_le4_arq4 = (logical_element *) malloc(sizeof(logical_element));
    cidq1_le4_arq4->type = ARQ_BLOCK;
    cidq1_le4_arq4->blk_type = LAST_FRAGMENT;
    cidq1_le4_arq4->length = 10;
    cidq1_le4_arq4->start_bsn = 10;
    cidq1_le4_arq4->next = NULL;
    
    cidq1_le4_arq4->data = cidq1_data4+90;
    //cidq1_le4_arq3->next = cidq1_le4_arq4;
    cidq1_le4_arq4->next = NULL;


   // link the necesary packet and block

   // the first packet
    cidq1_lp1 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->cid = cid1;
    cidq1_lp1->length = 60;
    cidq1_lp1->element_head = cidq1_le1_arq1;
    cidq1_le1_arq1->next = cidq1_le1_arq2;

    // the second packet
    cidq1_lp2 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp1->next = cidq1_lp2;
    cidq1_lp2->cid = cid1;
    cidq1_lp2->length = 59;
    cidq1_lp2->element_head = cidq1_le2_arq2;
    cidq1_le2_arq2->next = cidq1_le2_arq3;
    
    // the third packet

    cidq1_lp3 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp2->next = cidq1_lp3;
    cidq1_lp3->cid = cid1;
    cidq1_lp3->length = 30;
    cidq1_lp3->element_head = cidq1_le2_arq1;

    // the fourth packet
    cidq1_lp4 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp3->next = cidq1_lp4;
    cidq1_lp4->cid = cid1;
    cidq1_lp4->length = 5;
    cidq1_lp4->element_head = cidq1_le1_arq3;

    // the fifth packet
    cidq1_lp5 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp4->next = cidq1_lp5;
    cidq1_lp5->cid = cid1;
    cidq1_lp5->length = 60;
    cidq1_lp5->element_head = cidq1_le4_arq2;
    cidq1_le4_arq2->next = cidq1_le4_arq3;

    // the sixth packet
    cidq1_lp6 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp5->next = cidq1_lp6;
    cidq1_lp6->cid = cid1;
    cidq1_lp6->length = 30;
    cidq1_lp6->element_head = cidq1_le3;

    // the seventh packet
    cidq1_lp7 = (logical_packet *) malloc(sizeof(logical_packet));
    cidq1_lp6->next = cidq1_lp7;
    cidq1_lp7->cid = cid1;
    cidq1_lp7->length = 40;
    cidq1_lp7->element_head = cidq1_le4_arq1;
    cidq1_le4_arq1->next = cidq1_le4_arq4;
    cidq1_lp7->next = NULL;
    
    // 3. generate the logical_dl_subframe_map
    // 3.1 define the data structure
    logical_dl_subframe_map* dl_map;
    logical_burst_map* burst1;
    logical_pdu_map* burst1_pdu_map1;

    arq_retrans_sdu_map* burst1_retrans_map1;

    // 3.2 generate the dl map
    dl_map = (logical_dl_subframe_map *) malloc(sizeof(logical_dl_subframe_map));
    memset(dl_map, 0, sizeof(logical_dl_subframe_map));
    dl_map->num_bursts = 1;
    // 3.3 generate the first burst map
    burst1 = (logical_burst_map *) malloc(sizeof(logical_burst_map));
    burst1->map_burst_index = 1;
    burst1->burst_bytes_num = 500;
    burst1->pdu_num = 6;
    burst1_pdu_map1 = (logical_pdu_map *) malloc(sizeof(logical_pdu_map));
    burst1_pdu_map1->cid = cid1;
    burst1_pdu_map1->sdu_num = 2;

    burst1_retrans_map1  =(arq_retrans_sdu_map*) malloc(sizeof(arq_retrans_sdu_map));
    burst1_retrans_map1->arq_retransmit_block = cidq1_lp1;
    burst1_pdu_map1->arq_sdu_map = burst1_retrans_map1;
    burst1_pdu_map1->next = NULL;

    burst1_pdu_map1->next = NULL;

    burst1->pdu_map_header = burst1_pdu_map1;


    burst1->next = NULL;

    //burst1_pdu_map1->arq_sdu_map = NULL;
    burst1_pdu_map1->transport_sdu_map = NULL;
    burst1_pdu_map1->mac_msg_map = NULL;

    dl_map->burst_header = burst1;

    //4. prepare the data structure for the packing and fragmentation
    logical_burst_map* cur_burst;
    cur_burst = dl_map->burst_header;
    logical_packet* pdu_list;

    crc_init( POLY );

    // 5. define the physical subframe
    physical_subframe* phy_subframe;
    initialize_subframe(&phy_subframe);
    phy_subframe->bursts_num = 1;
    phy_subframe->frame_num = 78;
    phy_burst* phyburst;
    phy_burst* pre_phyburst;
    pre_phyburst = NULL;

    get_sduq(&sduq, 1);
    logical_element* le_tobe_discard = NULL;
    //5.1  generate the physical frame
    for (i=0; i<dl_map->num_bursts; i++)
    {
        // packing and fragmentation
        pdu_list = NULL; 
        fragpack(sduq, cur_burst, &(pdu_list), &le_tobe_discard, &status);
        phyburst = (phy_burst *) malloc(sizeof(phy_burst));
        phyburst->length = cur_burst->burst_bytes_num;
        phyburst->map_burst_index= cur_burst->map_burst_index;
        phyburst->burst_payload = (u_char *) malloc(phyburst->length);
        // concatenation
        concatenation(pdu_list, phyburst->burst_payload, phyburst->length, &schedule);
        release_logical_pdu_list(pdu_list);
        if (pre_phyburst == NULL){
            phy_subframe->burst_header = phyburst;
            pre_phyburst = phyburst;
        }
        else 
        {
            pre_phyburst->next = phyburst;
            pre_phyburst = phyburst;
        }
        cur_burst = cur_burst->next;
    }
    release_sdu_payload(le_tobe_discard);

    pre_phyburst->next = NULL;

        // release the burst map
    release_logical_subframe_map(dl_map);

    // 6 initialize the ul pduq
    // 6 initialize the ul pduq
    pdu_queue* ul_pduq_header = NULL;
    pdu_frame_queue* pdu_frameq = NULL;
    initialize_pduq(&(ul_pduq_header));
    initialize_pduframeq(&pdu_frameq, 78);
    enqueue_pduq(ul_pduq_header, pdu_frameq);

    dequeue_pduq(ul_pduq_header, &pdu_frameq);

    // initialize the physical subframe queue

    subframe_queue* dl_subframeq;

    initialize_subframe_queue(&dl_subframeq, 1);

    enqueue_subframe(dl_subframeq, phy_subframe);

    dequeue_subframe(dl_subframeq, &phy_subframe);
    
    // parsing the pdu from the burst
    phy_burst* phy_cur_burst;
    phy_cur_burst = phy_subframe->burst_header;
    for (i=0; i<phy_subframe->bursts_num; i++)
    {
        parse_burst_pdu(pdu_frameq->frame_no, phy_cur_burst->burst_payload, phy_cur_burst->length, pdu_frameq, NULL, NULL);

        phy_cur_burst = phy_cur_burst->next;
    }

    sdu_queue* ul_sduq;
    initialize_sduq(&ul_sduq, 0);
   
    // initialize the fragment queue
    frag_queue* fragq;
    initialize_fragq(&fragq);
 
    reassembly(pdu_frameq, ul_sduq, fragq, NULL, NULL);
    // now could release the memory of the pdu frame queue
    release_pduframeq(pdu_frameq);

    // check if the reassembly is correctly executed

    sdu_cid_queue* sducidq;
    sducidq = ul_sduq->sdu_cid_q;
    logical_packet* lp;
    lp = sducidq->head;
    logical_element* sdu_le;
    // check the first sducidq
    if (sducidq->sdu_cid_aggr_info->cid == cid1 && sducidq->sdu_num == 4)
    {
        // check the first sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp1_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for (j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data1[i])
                    {
                        is_test_failed = 1;
                        printf("testreassembly_arqretrans_arqfragpack: sdu content for the first sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;
            
                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testreassembly_arqretrans_arqfragpack: error cid and length for the first sdu in the first connection! \n");
        }
        // check the second sdu packet
        lp= lp->next;
        // check the second sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp2_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data2[i])
                    {
                        is_test_failed = 1;
                        printf("testreassembly_arqretrans_arqfragpack: sdu content for the second sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testreassembly_arqretrans_arqfragpack: error cid and length for the second sdu in the first connection! \n");
        }
        // check the third sdu packet
        lp= lp->next;
        // check the third sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp3_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data3[i])
                    {
                        is_test_failed = 1;
                        printf("testreassembly_arqretrans_arqfragpack: sdu content for the thrid sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }

        }
        else
        {
            is_test_failed = 1;
            printf("testreassembly_arqretrans_arqfragpack: error cid and length for the third sdu in the first connection! \n");
        }
        // check the fourth sdu packet
        lp= lp->next;
        // check the fourth sdu packet
        if (lp->cid == cid1 && lp->length == cidq1_lp4_len)
        {
            sdu_le = lp->element_head;
            i=0;
            while (sdu_le)
            {
                for(j=0; j<sdu_le->length; j++)
                {
                    if (sdu_le->data[j] != cidq1_data4[i])
                    {
                        is_test_failed = 1;
                        printf("testreassembly_arqretrans_arqfragpack: sdu content for the fourth sdu in the first connection is not correct! \n");
                        break;
                    }
                    i++;

                }
                sdu_le = sdu_le->next;
            }
        }
        else
        {
            is_test_failed = 1;
            printf("testreassembly_arqretrans_arqfragpack: error cid and length for the fourth sdu in the first connection! \n");
        }
        
    }
    else
    {
        is_test_failed = 1;
        printf("testreassembly_arqretrans_arqfragpack: error cid and sdu num for the first sdu cid queue! \n");
    }

   
    // release the fragment queue
    release_fragq(fragq);

        // release the physical phrame

    release_subframe( phy_subframe);

    // release the dl_subframeq;
    release_subframe_queue(dl_subframeq, 1);

    // release the connection
    release_connection_queue(con1);
    // release the sduq
    release_sduq(sduq, 1);
    release_sduq(ul_sduq, 0);

    release_pduq(ul_pduq_header);

    release_sducontainer(cidq1_lp1, 0, NULL);

    free(cidq1_data1);
    free(cidq1_data2);
    free(cidq1_data3);
    free(cidq1_data4);

     if ( is_test_failed )
    { 
        printf("testreassembly_arqretrans_arqfragpack: test failed! \n");
        return 1;
    } else {
        printf("testreassembly_arqretrans_arqfragpack: test success! \n");
        return 0;
    }

    return 0;
    
    
}

#endif

