/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_preamble.c

   Function: Generate the preamble OFDMA symbols used in DL frame.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "phy_dl_preamble.h"
#include "phy_dl_tx_common.h"
#include "phy_dl_utility.h"
#include "flog.h"

/* turn on/off dump according to DUMP_PHY_DL_PREAMPLE setting */
#ifndef DUMP_PHY_DL_PREAMPLE

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

/** Preamble modulation series per index for 1024-FFT mode */
const char wk_hex_1024fft[114][WK_HEX_LENGTH+10] =
{
"A6F294537B285E1844677D133E4D53CCB1F182DE00489E53E6B6E77065C7EE7",
"668321CBBE7F462E6C2A07E8BBDA2C7F7946D5F69E35AC8ACF7D64AB4A33C46",
"1C75D30B2DF72CEC9117A0BD8EAF8E0502461FC07456AC906ADE03E9B5AB5E1",
"5F9A2E5CA7CC69A5227104FB1CC2262809F3B10D0542B9BDFDA4A73A7046096",
"82F8A0AB918138D84BB86224F6C342D81BC8BFE791CA9EB54096159D672E91C",
"EE27E59B84CCF15BB1565EF90D478CD2C49EE8A70DE368EED7C9420B0C6FFAF",
"C1DF5AE28D1CA6A8917BCDAF4E73BD93F931C44F93C3F12F0132FB643EFD588",
"FCA36CCCF7F3E0602696DF745A68DB948C57DFA9575BEA1F05725C42155898F",
"024B0718DE6474473A08C8B151AED124798F15D1FFCCD0DE574C5D2C52A42EE",
"D4EBFCC3F5A0332BEA5B309ACB04685B8D1BB4CB49F9251461B4ABA25589714",
"EEA213F429EB926D1BDEC03ABB67D1DE47B4738F3E929854F83D18B216095E6",
"C03036FA9F253045DF6C0889A8B83BAEFCF90EB993C2D79BD911CA84075061A",
"1E68EC22E5E2947FB0A29E4CC70597254B36C60331EACF779FE752D3F55DC41",
"63A57E75A0434F035AAC4504B265081D497F10C77928B71797C5D6C6824DC0F",
"C57C4612816DE981C58FD6F8DE9DD41F2422ADBC522B0CE31F9A6D5F2A126DC",
"978256AF184E7ED17789B33D324C711B36BFBCCE5446EB03687E9A0A839C7CE",
"011EC823157DD73150640CEB7DDB0A1F8F91E09599A851D5C7CAF687CFB752D",
"C6DE82BEB7F57B9120E8A376D85C8F70FDC65BC660402DAC4AE6002EA2740C4",
"4C74929D6F9FAB9E5BB761026038E076F6824295E0AF397806ECEBC6DC713F0",
"13E1E85C2234D0F3418001A35F135E10C6C918C36BC659FDA9D655D288A0BDA",
"FD4AF2D8F4F08F1A7DF59291C9AEE788F641B8231CFB813376E0BEB68DFCFCB",
"EBBC77A493AA0C62C62F25EE5E8D0701F50386F49026FA31487C9FD5C5206CE",
"134F936F9E875842587ADCA92187F2FC6D62FFC3A833D8CDE465F9972ABAA83",
"01E5EBA1594FB8A0420A3CD1DA70670BC73363D1B4A66D280FF6AA7636D07EC",
"918296B2937C2B6F73CF98F85A81B723D1C69DBDF3E019749C582DA22E78956",
"C323981B8B2240865F48D61AE1B3B61D88522B7358952F949D4308CA15D1EE8",
"7514A6FA5FBB250C5C8CE96F791D676036C344A44B24284477B44CB3E758F8B",
"84C7FEC6E977FA1EC0C7CC9E0D067C73D8F846F82ABB3456D2104E1448D5A58",
"4841AFC277B86A0E067AF319422F501C87ACBFBDD66BFEA3644F879AE98BA8C",
"F35EA87318E459138A2CE69169AD5FD9F30B62DA04ED21320A9F59893F0D176",
"A0C5F35C5971CD3DC55D7D2B9FD27AA17A198583F580EB0800744EE5B6B3648",
"A6D3D33AD9B56862DBF076E3ACE6A3150510CCC8BE77DE4E6E10EB5FE163765",
"52849D8F020EA6583032917F36E8B62DFD18AD4D77A7D2D8EC2D4F20CC0C75B",
"CC53A152209DEC7E61A06195E3FA633076F7AE1BAFFE83CE565087C0507BA59",
"17D98A7E32CCA9B142FE32DB37B2BF726E25AA7A557FFB5C400B47A38B16CF1",
"A5BA8C7E2C795C9F84EBBD425992766BDE5549A7A9F7EF7E44AFD941C608456",
"33E57E78A5696255CA61AE36027036DA619E493A0A8F95D9915C6E61F3006CB",
"09961E7309A9B7F3929C370C51910EBAB1B4F409FA976AE8679F354C84C4051",
"508A9EBAEF3C7E09CFCFC0B6F444A09B45A130EFC8C5B22BCE87213854E7C9D",
"AACEEF9BCDC82E4AD525185B07CBABCB74861D16F7C25CFBA917B05463AD653",
"23060ACC5A125DAB207EEEE47B4EEE1E8466BD17DDA2EB3CD90D2AB7A758C21",
"CA55521667BDA8B6F1B205201A51B3A0C05DE9EA06BC73268730A81A9927770",
"05ADFCA2F8207DC6FF8D1A85A1DD4694D4C48A838C4F833C532710021AC448A",
"218C951223D7B712DC98F8B5217388A830003C5F2A00F232DD3475D2FC78C25",
"79B94D24D721121EF678B7156F8D2666DE712BBF3837C85A9518781903146A7",
"58AABEF6A6BDE4011CAC583C5104B2C6FC5A2980F856373E5931A3C69024532",
"427D1AD18E338E16FCE6E23B4AD6D82A2144D53048F2665AA94577AFABD2688",
"337FE0E4C15A22471AE0F6B6F91161A7DE2E1403D73587D5C8355105D2F7064",
"A3FCAA311B536AC9DB39FED9F4E996506B3181C58D6B7E04157A3FD463F6046",
"F484FD1F57F53A4A749B86148E0B1D0653667CE1393198875DDB0AE9179BBBD",
"A3E9ECF1E6048562BC89DB6168E708855F0D4AD29F859EF36C9160DF407D854",
"890519376D1FFAA2894EABCD6663B0A3C2411982C17B01270E0FB0B289D4BC8",
"09847B6187BB5F6F6728B4ED610088FAD9DADFC00748E9DCD8A0CE320D6C991",
"3285AE0A3D196313659C37BE1C94D61D20F11FD49D9FDF9D1026FF5763F02CB",
"0069D3F34D0D455AFB45FEFDF716333B785C6BDA90DA23F1CC68BC6A1DBC916",
"AA977A8BCA39381E7C35A1ACC7C4F60421C0862BFD6106C7C025B0676EA0EF6",
"F310745C497094ABE56E0490C0800319DBE290553E696B6859635AF03B121F7",
"964DFD350B9C7DFDC7F6F7C43283A76F0D613E48A5520D1DAF761C6F47E389B",
"6D767B88D28A455CC3B56C942BAFD8E465A50FD2C22FE6162E03A9AAC3C1CC8",
"C5491C6CA3D998906EC1482F815B74B7C2E3816B682ACC6009AB7EFF34BF0E9",
"6D8EE32D30E19D93A0E5AD8226BAE9CF6FCBA17CF6E67FDC5A15A81ECB8908B",
"98F8BFDF774C7A249418E6FF4723D6E6AB2F091CDE4DE1CE11D3BD463B509FB",
"65300BAD8FFA21BC7DC2C1F79FA97A9F469CCC9E270A61759F34D6276F57CBE",
"6F36BB6D5A7DC4FB720439E91FF0DE86DD6C4B93CFC4271F2BCC6169616E3AE",
"D27B00C70A8AA2C036ADD4E99D047A376B363FEDC287B8FD1A7794818C5873E",
"E7FDDCEED8D31B2C0752D976DE92BEA241A713CF818C274AA1C2E3862C7EB70",
"87BF4954022D30549DF7348477EACB97AC3565B838460CC62F242883313B15C",
"82DD830BEDE4F13C76E4CF9AEF5E42609F0BDDCB000A742B6372DD5225B0C31",
"4E06E4CF46E1F5691938D7F40179D8F79A85216775384BD97966DB4BBF49FB6",
"64164534569A5E670FDB390D09C04802DD6A16B022CADC77EDD7464AFED43C7",
"FB8769A81AA9DB607F14A6A95948401F83057CDC9C9C3996BA5821403A49F00",
"77710D6F40B4F79CC63F678551C3EC18FA9DF2C82E6C8F415DADFD63264B751",
"503F196BBF93C238BFD5E735E5AE52E0DAE64F5E2F4C3B92E553F51303C4A64",
"5FD4A6894566678C95B9D5A59DDE5366799045FEB03A2BAA74094140E9068C6",
"95B584DC40C8B5DEAD63D48FCE65B1E61BAB4C597D921DB12677141E2FFE7C0",
"985763AB6CC8934DB8A0BE738A7AF1D1FA3958C1F9E2D6A51A163E47A0A6E5F",
"FD8D45F00D943AD986BD353D61C6746DBF8A309B6AE1C173B880D957B76DC03",
"AE4323534F6EFB1A20169328417885EF304FA220389FA9C2607E5A406F4CE4A",
"E5205579893BE184CB9948C28E2F9AAF699D47B6E5E0B219CBEAFE4BEC8D561",
"AB11D6941478D36D5695CE813070DC1E32122A39083E53FE373660AEB125D83",
"188A09C46F1F11206FF9F15CFB5F6CD2F26C4BF485EE37D3650A595064F76CE",
"4B1CDE25539A56CEDC45FE7F54C38CF155F4FB1AE868F6C3952D07014BF828E",
"16CA8F8C6A879E865E3611EAC389D56AFA3E4E84CDBB73567BA4A160249C4B6",
"39D2B08AA0E2E8781476027B41AD72F8D9838B7001AADFD33A92D81E56ECBB2",
"8C258BC80D4AD125F335A5151EDF9E9A463E06C5C8D046F82E5DC3D73EF4D22",
"41A029C6356C825585179C5348EDF07A3AC2022539AC28DC4CD3C1DFADC8EE9",
"0D70A77CBE9804913BFBEC4FBF917C5CD3580F6062BBAD3F99ECEBB4A9EBB87",
"6A00A30901F9FDE44B4F1ECED44E0BCB943B29519F313BE4496D34F39B154FC",
"95351107A8BE6ABFC24C1292FE1A0FE677CBFD04F2E81178CAA9D294730EF9C",
"01F21470FD9B1E0B3C6B2F7C0412A15764C277D61BA2EE3B3769DE7ADACB2BB",
"A578ABFE155369440FA3D4DF757CCA596469B80A0E56BFE6010DD63E67CEDB8",
"1E1CFFAB031836777DE5D168A9246C559574C74CCC06405EB406B8DDB7C9A6E",
"354149C2CA19A735F9CD04AF4922E8ECE6509B978B951F946FD4AD36C7F9C83",
"5A27E60DEA547D0D41897A03199F28A967AC51728E3B38325B4FBECF1B85A7E",
"784DA3B16B810FE3B851060AD7BD27D9D9457F6C8899A13D311E531B855C15E",
"D7DFBC65797633A8C13D3EEC781D48952338136063B579D69437B28B744B5A4",
"61AF26BD39A9FFF52826625E04ADA299385A373FA946D837D754E6CFEBB26F5",
"D77D97CDB93DBEAA65CAFA146F40D72B5E80944F750E07325DC164ED60F3243",
"4529D9CA65AF49C1C39BDC18CFAB87E03FE4DAFC0A48FF1457D46B0DF66B414",
"33AC0261DAA57C1D611EBA1C730D50AFEE5BE3E849030A4E891BC8C5F4C78DC",
"BED48C704F02A84F03BCD299D919DA56F7B71EDF8A0F8A25E8F8496F95A44CE",
"0ECCBE0902EBF4B4C29506014A3706622784B7B2D5153E10AD3112DC5E45277",
"7CB4937889C7DFD9AA2D37235E06F993D3D4F5D515B39CA652F62397C08457D",
"43F23F6CAC6C43896B3EDBF00E1CBD42E2CC75E2A996448F0FCF17F6779DD6E",
"72C8A209FBC4A568BEF03BCFE1B0D959F977B0963780B4E54E2B9A1016344AC",
"77AEB9E50DC3727849A94FBFFCDB5B9589AF50ABD8A58808B9663058E17A2EB",
"667123C89077FE4AAAEF15C635E976C6811682D478FFC7B721A76B5A38697DF",
"CBD6C5C9BE55B0BE76AD03392E8A8AB9A86063DB31B79280B447980BB841FD7",
"C7D7DEF8B3C9C8667D8D65063B4DAD1FF69445C87CA71DA955D0CA23970E988",
"FB246ABD92F9E560CB2BEC2317204C9CE22AD3BD19EA02E90F5F3B7F4F65538",
"29E74579472FDD8FFC2700B2BF33C649989DD8153093A7CA08B50F7A5E4BAED",
"A27F29D8D6CCD7EB4BBE303C3E9E95802DB98BFD5B8ED03B88304359D92E3EC",
"3FE70E26FA00327FE3B2BE6BC5D5014F588F09C17D222C146DD68B4824692A6",
"41E91307EC58801CFF2C7E9CFEFBEB71681FAE2BEAEC72D4E4556E99345D3BA"};

/** To convert hexadecimal to binary */

int hex_to_binary(const char *data, unsigned int *dest)
{
    int value, count;
    unsigned int *binary_array;

    if(data == NULL) {
        FLOG_ERROR("E001_preamble: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    binary_array = dest;
    count = 0;

    while(*data != '\0') {
        if (*data >= '0' && *data <= '9')
            value = *data - '0';
        else if (*data >= 'A' && *data <= 'F')
            value = *data - 'A' + 10;
        else if (*data >= 'a' && *data <= 'f')
            value = *data - 'a' + 10;
        else
            return ERROR_CODE;

        /* MSB bit */

        binary_array[count] = value>>3;
        value = value - (binary_array[count] << 3);
        count++;

        /* The second MSB bit */

        binary_array[count] = value>>2;
        value = value - (binary_array[count] << 2);
        count++;

        /* The third MSB bit */

        binary_array[count] = value>>1;
        value = value - (binary_array[count] << 1);
        count++;

        /* LSB bit */

        binary_array[count] = value;
        count++;

        data++;
    }
    return SUCCESS_CODE;
}

/**----------------------------------------------------------------------------
   Function:    phy_dl_preamble_fgen()

   Description: To generate frequency domain DL preamble.

   Parameters:
                Input-  [const struct phy_dl_tx_syspara *para]  The pointer refers to the
                        struct of system parameters which include the para to
                        include preamble index to indicate which WK should be
                        used.

                Output- [float *output_fr]  The pointer refers to the real part
                        of frequency domain preamble symbol, 1 OFDMA symbol of
                        1024 samples.
                        [float *output_fi]  The pointer refers to the imaginary
                        part of frequency domain preamble symbol, 1 OFDMA symbol
                        of 1024 samples.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */
int phy_dl_preamble_fgen(const struct phy_dl_tx_syspara *para, 
			 char *active_band,
			 float *output_fr, 
                         float *output_fi)
{
    const char *p_wk_hex;
    unsigned int wk_bi[WK_HEX_LENGTH*4], sample_count, offset;
    int err_code;
    float wk_modulation[WK_HEX_LENGTH*4];
    float preamble_unshift[OFDMA_SYMBOL_SIZE];
    float preamble_temp[OFDMA_SYMBOL_SIZE];
    float preamble_boost;
    
    u_int32_t i = 0, j = 0; 
    int n=0;
    u_int32_t count = 0;
    char *pch = NULL;
	    

    if (para == NULL || output_fr == NULL || output_fi == NULL) {
        FLOG_ERROR("E002_preamble: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }
    if (active_band == NULL)
    {
        FLOG_ERROR("E002_preamble: the pointer refer to dts_info struct is null!\n");
	return ERROR_CODE;
    }
    else
    {
	pch = active_band;
    }

    p_wk_hex = &wk_hex_1024fft[para->preamble_index][0];

    /* Convert wk_hex to wk_bi */
    err_code = hex_to_binary(p_wk_hex, wk_bi);
    if(err_code) return err_code;

    /* Modulation */
    for (sample_count=0; sample_count<WK_HEX_LENGTH*4; sample_count++) {
        wk_modulation[sample_count] = 4 * SQRT2 * (0.5-wk_bi[sample_count]); /* 4*sqrt(2)*(1/2-wk_bi) */
    }
    
    /* 3 segment index mapping*/
    memset(preamble_unshift,0,para->ofdma_nfft*sizeof(float));
    offset = para->segment_index ;
    for (n=0; n<WK_HEX_LENGTH*4; n++) {
        preamble_unshift[n*3 + offset] = wk_modulation[n];
    }

   /* insert zeros for interference channel*/
    for (i=0; i<para->num_phyband; i++ )
    {
	if ( *pch++ )
        {
            for (j=36*i; j<36*(i+1); j++)
            {
	        preamble_unshift[j] =0.0;
            }
            count++;
        }
    }
	DO_DUMP(DUMP_PHY_TX_PREAMBLE_UNSHIFT_ID, 0, preamble_unshift, 756);

    memset(preamble_temp,0,para->ofdma_nfft*sizeof(float));
    offset = para->preamble_guard_band;
    for(i=0; i<WK_HEX_LENGTH*4*3; i++)
    {
        preamble_temp[i+offset] = preamble_unshift[i];
    }
    
    if ( para->segment_index == 0 ) preamble_temp[para->ofdma_dc] = 0;

	DO_DUMP(DUMP_PHY_TX_PREAMBLE_TEMP_ID, 0, preamble_temp, 1024);


    /* Shift zero-frequency component of discrete Fourier transform to center of spectrum */
    memset(output_fi,0,para->ofdma_nfft*sizeof(float));
    
    preamble_boost = sqrt((float)para->num_phyband/(float)(para->num_phyband - count));
  //  printf("--------------preamble_boost = %f-------------------------\n", preamble_boost);
    for (sample_count=0; sample_count<para->ofdma_dc; sample_count++) {
        output_fr[sample_count] = preamble_boost * preamble_temp[sample_count+para->ofdma_dc];
        output_fr[sample_count+para->ofdma_dc] = preamble_boost * preamble_temp[sample_count];
    }

	DO_DUMP(DUMP_PHY_TX_PREAMBLE_FDOMAIN_ID, 0, output_fr, 1024);

    return SUCCESS_CODE;
}

/**----------------------------------------------------------------------------
   Function:    phy_dl_preamble_gen()

   Description: To generate preamble symbol in time domain.

   Parameters:
                Input-  [const struct phy_dl_tx_syspara *para]  The pointer refers to the
                        struct of system parameters which include the para to
                        include preamble index to indicate which WK should be
                        used.

                Output- [float *output_tr]  The pointer refers to the real
                        part of time domain preamble symbol, 1 OFDMA symbol
                        of 1088 samples.
                        [float *output_ti]  The pointer refers to the imaginary
                        part of time domain preamble symbol, 1 OFDMA symbol of
                        1088 samples.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

#ifdef IPP_OPT_TEMP730

int phy_dl_preamble_gen(const struct phy_dl_tx_syspara *para, 
                        char *active_band,
                        float *output_tr, 
                        float *output_ti)
{
    int err_code;
    unsigned int sample_count;

    Ipp32f preamble_fr[OFDMA_SYMBOL_SIZE], preamble_fi[OFDMA_SYMBOL_SIZE];
    Ipp32f *dst_r, *dst_i;

    if (para == NULL || output_tr == NULL || output_ti == NULL) {
        FLOG_ERROR("E004_preamble: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }


    err_code = phy_dl_preamble_fgen(para,active_band, preamble_fr, preamble_fi);
    if(err_code) {
        FLOG_ERROR("E005_preamble: error %d when generate frequency domain preamble!\n",err_code);
        return err_code;
    }



	DO_DUMP(DUMP_PHY_TX_FREQ_PREAMBLE_FGEN_ID, 0, preamble_fr, para->ofdma_nfft);

                
    /* Time domain symbol */
    dst_r = output_tr+para->ofdma_ncp;
    dst_i = output_ti+para->ofdma_ncp;
    ippsFFTInv_CToC_32f(preamble_fr, preamble_fi, dst_r, dst_i, g_pFFTSpecInv, g_BufInv);

    /* Add CP */
    for (sample_count=0; sample_count<para->ofdma_ncp; sample_count++) {
        output_tr[sample_count] =  output_tr[para->ofdma_nfft + sample_count];
        output_ti[sample_count] =  output_ti[para->ofdma_nfft + sample_count];
    }

    return SUCCESS_CODE;
}

/* Preamble Generation under CDD mode */

int phy_dl_preamble_gen_cdd(const struct phy_dl_tx_syspara *para,
                            char *active_band,
                            float *output_tr, float *output_ti)
{
    int err_code;
    unsigned int sample_count;

    Ipp32f preamble_fr[OFDMA_SYMBOL_SIZE], preamble_fi[OFDMA_SYMBOL_SIZE];
    Ipp32f dst_r_temp[1200], dst_i_temp[1200];/*temp handle */

    if (para == NULL || output_tr == NULL || output_ti == NULL) {
        FLOG_ERROR("E004_preamble: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }


    err_code = phy_dl_preamble_fgen(para,active_band, preamble_fr, preamble_fi);
    if(err_code) {
        FLOG_ERROR("E005_preamble: error %d when generate frequency domain preamble!\n",err_code);
        return err_code;
    }


    /* Time domain symbol */
    ippsFFTInv_CToC_32f(preamble_fr, preamble_fi, dst_r_temp+para->ofdma_ncp+para->cdd_num, dst_i_temp+para->ofdma_ncp+para->cdd_num, g_pFFTSpecInv, g_BufInv);
    

     /* Add CP */
    for (sample_count=0; sample_count<para->ofdma_ncp+para->cdd_num; sample_count++)
    {
        dst_r_temp[sample_count] =  dst_r_temp[para->ofdma_nfft + sample_count];
        dst_i_temp[sample_count] =  dst_i_temp[para->ofdma_nfft + sample_count];
    }

    for (sample_count=0; sample_count< para->ofdma_symlen_with_guard; sample_count++)
    {
        output_tr[sample_count] =  dst_r_temp[sample_count];
        output_ti[sample_count] =  dst_i_temp[sample_count];
    }
    
    return SUCCESS_CODE;
}



#else
#include "fft.h"
int phy_dl_preamble_gen(const struct phy_dl_tx_syspara *para,
                        char *active_band, 
                        float *output_tr, 
                        float *output_ti)
{
    int err_code;
    unsigned int sample_count;
#ifdef VSXOPT
    float preamble_fr[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), preamble_fi[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
    float preamble_fr[OFDMA_SYMBOL_SIZE], preamble_fi[OFDMA_SYMBOL_SIZE];
#endif

    if (para == NULL || output_tr == NULL || output_ti == NULL) {
        FLOG_ERROR("E004_preamble: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    err_code = phy_dl_preamble_fgen(para, active_band, preamble_fr, preamble_fi);
    if(err_code) {
        FLOG_ERROR("E005_preamble: error when generate frequency domain preamble!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX_FREQ_PREAMBLE_FGEN_ID, 0, preamble_fr, para->ofdma_nfft);

    /* Time domain symbol */
    ifft(para->ofdma_nfft,
         output_tr+para->ofdma_ncp,
         output_ti+para->ofdma_ncp,
         preamble_fr,
         preamble_fi,
         para->XX,
         para->x,
         para->X);

    /* Add CP */
    for (sample_count=0; sample_count<para->ofdma_ncp; sample_count++) {
        output_tr[sample_count] =  output_tr[para->ofdma_nfft + sample_count];
        output_ti[sample_count] =  output_ti[para->ofdma_nfft + sample_count];
    }

    return SUCCESS_CODE;
}


int phy_dl_preamble_gen_cdd(const struct phy_dl_tx_syspara *para,
                            char *active_band, 
                            float *output_tr, 
                            float *output_ti)
{
    int err_code;
    unsigned int sample_count;
#ifdef VSXOPT
    float preamble_fr[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), preamble_fi[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float output_temp_r[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), output_temp_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
    float preamble_fr[OFDMA_SYMBOL_SIZE], preamble_fi[OFDMA_SYMBOL_SIZE];
    float output_temp_r[1200], output_temp_i[1200];
#endif

    if (para == NULL || output_tr == NULL || output_ti == NULL) {
        FLOG_ERROR("E004_preamble: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    err_code = phy_dl_preamble_fgen(para, active_band, preamble_fr, preamble_fi);
    if(err_code) {
        FLOG_ERROR("E005_preamble: error when generate frequency domain preamble!\n");
        return err_code;
    }

    /* Time domain symbol */
#ifdef VSXOPT
    ifft_p2(para->ofdma_nfft,
         output_temp_r,
         output_temp_i,
         preamble_fr,
         preamble_fi,
         para->XX,
         para->x,
         para->X);
#else
    ifft(para->ofdma_nfft,
         output_temp_r+para->ofdma_ncp+para->cdd_num,
         output_temp_i+para->ofdma_ncp+para->cdd_num,
         preamble_fr,
         preamble_fi,
         para->XX,
         para->x,
         para->X);
#endif
    /* Add CP */
#ifdef VSXOPT
    if(para->ofdma_symlen_with_guard > (para->ofdma_ncp + para->cdd_num))
    {
        int len1 = para->ofdma_ncp + para->cdd_num;
        int len2 = para->ofdma_symlen_with_guard - len1;

        memcpy( output_tr, output_temp_r + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( output_tr + len1, output_temp_r, sizeof(float) * len2);
        memcpy( output_ti, output_temp_i + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( output_ti + len1, output_temp_i, sizeof(float) * len2);
    }
    else
    {
        int len1 = para->ofdma_symlen_with_guard;

        memcpy( output_tr, output_temp_r + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( output_ti, output_temp_i + ( para->ofdma_nfft - len1), sizeof(float) * len1);
    }

#else
     for (sample_count=0; sample_count<para->ofdma_ncp+para->cdd_num; sample_count++)
    {
        output_temp_r[sample_count] =  output_temp_r[para->ofdma_nfft + sample_count];
        output_temp_i[sample_count] =  output_temp_i[para->ofdma_nfft + sample_count];
    }


    for (sample_count=0; sample_count<para->ofdma_symlen_with_guard; sample_count++) {
        output_tr[sample_count] =  output_temp_r[sample_count];
        output_ti[sample_count] =  output_temp_i[sample_count];
    }

#endif
     
    return SUCCESS_CODE;
}


#endif

int dump_phy_tx_freq_preamble_fgen(int flag, char * name, FILE * fd, int len, void *p_buf)
{
	(void) flag;
	(void) name;
	int count;
	float * data = (float *)p_buf;
	
	for (count=0; count<len; count++)
	{
  	  	fprintf(fd, "%0.6f\n", data[count]);
  	}
	
	return 0;
}



/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_DL_PREAMPLE

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif

