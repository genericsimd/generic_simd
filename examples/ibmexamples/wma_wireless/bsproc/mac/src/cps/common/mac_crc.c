/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_crc.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

//  Function:  CRC-32 function for 802.16e

#include <stdlib.h>
#include <string.h>
#include "mac_config.h"
#include "mac_crc.h"


/* CRC Table pointer */
u_int32_t * crc_table = NULL;

/**
 * Function: flip()
 *
 * Description: flipping the bits of a binary sequence
 *
 * Parameters:
 *           input: [IN] input data to be reflected
 *           bits_num: [IN] bits number of the input data to be reflected
 *           output: [OUT] results
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int flipping (u_int32_t const input, u_int8_t const bits_num, u_int32_t * output)
{
    u_int32_t ref = 0x00000000;
    u_int8_t bit;
    u_int8_t sh_bits = bits_num - 1;
    u_int32_t tmp_input = input;

    /*
     * flipping for the center bit.
     */
    for (bit = 0; bit < bits_num; ++bit)
    {
        if (tmp_input & 0x01)
        {
            ref = ref | (1 << (sh_bits - bit));
        }

        tmp_input = (tmp_input >> 1);
    }

    *output = ref;

    return (0);

}	/* flipping() */


/**
 * Function: crc_init()
 *
 * Description: initialize a 256 entries Teable for CRC-32 algorithms
 *              it can be implemented offline.
 *
 * Parameters:
 *           poly: [IN] polynomial for CRC-32 calculation
 *
 * Global variable:
 *           crc_table: [IN] pointer which points to a 256 entries * 4Bytes Table
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int crc_init (u_int32_t const poly)
{
    u_int32_t rem;
    u_int32_t div_idx;
    u_int8_t bit_idx;

    /* use stack memory here, can be replace by heap */
    static u_int32_t crc_table_tmp[256];
//    crc_table_tmp = malloc(sizeof(u_int32_t) * 256);

    /*
     * Compute all the possible dividend.
     */
    for (div_idx = 0; div_idx < 256; ++div_idx)
    {
        rem = div_idx << (LEN - 8);

        for (bit_idx = 8; bit_idx > 0; --bit_idx)
        {
            if (rem & MSB)
            {
                rem = (rem << 1) ^ poly;
            }
            else
            {
                rem = (rem << 1);
            }
        }

        crc_table_tmp[div_idx] = rem;
    }

    crc_table = crc_table_tmp;

#ifdef _OFDM_
    FLOG_INFO("OFDM CRC32 initilization finished!\n");
#else  /* _OFDM_ */
    FLOG_INFO("OFDMA CRC32 initilization finished!\n");
#endif  /* _OFDMA_ */

    return (0);

}   /* crc_32_init() */

/**
 * Function: crc_calculation()
 *
 * Description: calculate the 32 bits CRC
 *
 * Parameters:
 *           input: [IN] input data for CRC-32 calculation
 *           length: [IN] Bytes number of input data
 *           output: [OUT] CRC-32 result
 *
 * Global variable:
 *           crc_table: [IN] pointer which points to a 256 entries * 4Bytes Table
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int crc_calculation (u_char const * input,
                     int const length,
                     u_char* output)
{
    u_int32_t rem = INIT_REM;
    u_int8_t uc_tmp0;
    u_int32_t ui_tmp0;
    u_int32_t byte_idx;

    if (crc_table == NULL)
    {
        FLOG_FATAL("crc table is not initilized, call crc_init first!\n");
        return 1;
    }

    for (byte_idx = 0; byte_idx < length; byte_idx++)
    {
#ifdef _OFDM_
        flipping(input[byte_idx], 8, &ui_tmp0);
        uc_tmp0 = ui_tmp0 ^ (rem >> (LEN - 8));
        rem = crc_table[uc_tmp0] ^ (rem << 8);
#else    /* OFDM */
        uc_tmp0 = input[byte_idx] ^ (rem >> (LEN - 8));
        rem = crc_table[uc_tmp0] ^ (rem << 8);
#endif   /* OFDMA  */
    }

#ifdef _OFDM_
    flipping(rem, 32, &ui_tmp0);
    ui_tmp0 = ui_tmp0 ^ INIT_XOR;

    /* Do reorder due to the reflect */
    output[0] = (ui_tmp0 & BYTE_D);
    output[1] = (ui_tmp0 & BYTE_C) >> 8;
    output[2] = (ui_tmp0 & BYTE_B) >> 16;
    output[3] = (ui_tmp0 & BYTE_A) >> 24;
#else    /* OFDM */
    ui_tmp0 = rem ^ INIT_XOR;

    /* extract Byte */
    output[0] = (ui_tmp0 & BYTE_A) >> 24;
    output[1] = (ui_tmp0 & BYTE_B) >> 16;
    output[2] = (ui_tmp0 & BYTE_C) >> 8;
    output[3] = (ui_tmp0 & BYTE_D);
#endif    /* OFDMA */

#ifdef _OFDM_
    FLOG_DEBUG("OFDM CRC32 calculation finished!\n");
#else  /* _OFDM_ */
    FLOG_DEBUG("OFDMA CRC32 calculation finished!\n");
#endif  /* _OFDMA_ */

    return (0);
}   /* crc_32_calc() */


/*
 * Function: crc_verification()
 *
 * Description: ckeck the 32 bits CRC results
 *
 * Parameters:
 *           input: [IN] input data for CRC-32 calculation
 *           length: [IN] Bytes number of input data
 *           output: [OUT] CRC-32 result, 0 for pass, other value for faild
 *
 * Global variable:
 *           crc_table: [IN] pointer which points to a 256 entries * 4Bytes Table
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int crc_verification (u_char const * input,
                      int const length,
                      u_char * output)
{
    u_char crc_result[4];

#ifdef _OFDM_
    u_char check_value[4] = {0x1c, 0xdf, 0x44, 0x21};
#else /* _OFDM_ */
    u_char check_value[4] = {0x38, 0xfb, 0x22, 0x84};
#endif /* _OFDMA_ */

    if (crc_calculation ( input, length, crc_result ) )
    {
        FLOG_DEBUG("error in CRC32 result check\n");
        return 1;
    }

    if ( (crc_result[0] == output[0]) && (crc_result[1] == output[1])
         && (crc_result[2] == output[2]) && (crc_result[3] == output[3]) )
    {
        return 0;
    }else if ((crc_result[0] == check_value[0]) && (crc_result[1] == check_value[1])         
        && (crc_result[2] == check_value[2]) && (crc_result[3] == check_value[3]))
    {
        return 0;
    }
    else 
    {
        return 1;
    }

}

