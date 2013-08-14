/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name:

   Function:

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */




#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "flog.h"
#include "viterbicore.h"

#if defined SSE2OPT
#include <xmmintrin.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#elif defined VSXOPT
#include <altivec.h>
#endif

#if !(defined SSE2OPT || defined VSXOPT)
#error "No scalar version viterbi decoder implemented, SSE2OPT or VSXOPT need be defined to have viterbi decoder work"
#endif

#if 0
typedef union {
	unsigned mtype  w[64];
} metric_t;     //w[64] save the metrices of  all the posible state

typedef union {
	unsigned mtype m[64];
} decision_t;

/* State info for instance of Viterbi decoder*/
struct va_ctx {
	metric_t metric_buf1; 		/* Path metric buffer 1 */
	metric_t metric_buf2; 		/* Path metric buffer 2 */
	metric_t *old_metrics,*new_metrics; /* Pointers to path metrics, swapped on every bit */
	decision_t *decisions;   	/* Beginning of decisions for block */
	decision_t *curr_decision;   	/* Pointer to current decision */
};
#endif

#if defined SSE2OPT
static union va_branchtab{
	unsigned char c[32];
	__m128i v[2];
} branch_tab[2] __attribute__ ((aligned (128)));
#elif defined VSXOPT
static union va_branchtab{
	unsigned char c[32];
	vector unsigned int v[2];
} branch_tab[2] __attribute__ ((aligned (128)));
#else
static union va_branchtab{
	unsigned char c[32];
} branch_tab[2] __attribute__ ((aligned (128)));
#endif

static int Init = 0;

static unsigned char Partab[] = {
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
};


#ifdef _MSC_VER
static _inline int parity(int x){
#else
static inline int parity(int x){
#endif
	/* Fold down to one byte */
	x ^= (x >> 16);
	x ^= (x >> 8);
	return Partab[x];
}

static void set_viterbi_polynomial( int polys[2] ){

	int state;

#ifdef CHARTYPE
	for(state=0;state < 32;state++){
		branch_tab[0].c[state] = (polys[0] < 0) ^ parity((2*state) & abs(polys[0])) ? SAMPLE_MASK_CHAR : 0;
		branch_tab[1].c[state] = (polys[0] < 0) ^ parity((2*state) & abs(polys[1])) ? SAMPLE_MASK_CHAR : 0;
	}
#else
	for(state=0;state < 32;state++){
		branch_tab[0].c[state] = (polys[0] < 0) ^ parity((2*state) & abs(polys[0])) ? SAMPLE_MASK_SHORT : 0;
		branch_tab[1].c[state] = (polys[0] < 0) ^ parity((2*state) & abs(polys[1])) ? SAMPLE_MASK_SHORT : 0;
	}
#endif
}

/* Initialize Viterbi decoder for start of new frame */
int init_viterbi(void *p, int starting_state)
{
	struct va_ctx *vp = p;
	int i;

	if(p == NULL)
		return -EPERM;

#if defined SSE2OPT
	{
#ifdef CHARTYPE
		__m128i Const63charv = _mm_set_epi8(63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63);

		for( i = 0; i < 64; i += 16 )
			_mm_store_si128((__m128i *)&vp->metric_buf1.w[i], Const63charv);
#endif

#ifdef SHORTTYPE
		__m128i Const63shortv = _mm_set_epi16(63, 63, 63, 63, 63, 63, 63, 63);
		for( i = 0; i < 64; i += 8 )
			_mm_store_si128((__m128i *)&vp->metric_buf1.w[i], Const63shortv);
#endif
	}
#elif defined VSXOPT
#ifdef CHARTYPE
                vector unsigned int Const63charv = {0x3F3F3F3F, 0x3F3F3F3F, 0x3F3F3F3F, 0x3F3F3F3F};

                for( i = 0; i < 64; i += 16 ) vec_st(Const63charv, i, (vector unsigned int *)vp->metric_buf1.w);
#endif
#else
	for( i = 0; i < 64; i++)
		vp->metric_buf1.w[i] = 1;
#endif

	vp->old_metrics = &vp->metric_buf1;
	vp->new_metrics = &vp->metric_buf2;
	vp->curr_decision = vp->decisions;

	vp->old_metrics->w[starting_state & 63] = 0;

	return 0;
}

/* Create a new instance of a Viterbi decoder */
void *create_viterbi(int len){
	struct va_ctx *vp;

	if(!Init){
		int polys[2] = {VITERBI_POLYA, VITERBI_POLYB };
		set_viterbi_polynomial(polys);
		Init++;
	}

#if defined SSE2OPT
	{
		void *p;
		FLOG_INFO("SSE2 Optimization Enable, use the vector optimized viterbi decoder\n");

		/* Ordinary malloc() only returns 8-byte alignment, we need 16 */
		if(posix_memalign(&p, sizeof(__m128i),sizeof(struct va_ctx)))
			return NULL;

		memset((void *)p, 0x00, sizeof(struct va_ctx));
		vp = (struct va_ctx *)p;

		if(posix_memalign(&p, sizeof(__m128i),len*sizeof(decision_t)))
		{
			free(vp);
			return NULL;
		}
		vp->decisions = (decision_t *)p;
	}
#elif defined VSXOPT
	{
		void *p;
		FLOG_INFO("VSX Optimization Enable, use the vector optimized viterbi decoder\n");

		/* Ordinary malloc() only returns 8-byte alignment, we need 16 */
		if(posix_memalign(&p, sizeof(vector unsigned int),sizeof(struct va_ctx)))
			return NULL;
		memset((void *)p, 0x00, sizeof(struct va_ctx));

		vp = (struct va_ctx *)p;

		if(posix_memalign(&p, sizeof(vector unsigned int),len*sizeof(decision_t)))
		{
			free(vp);
			return NULL;
		}
		vp->decisions = (decision_t *)p;
	}
#else
	FLOG_INFO("No Optimization, use the scalar viterbi decoder\n");
	if((vp = (struct va_ctx *)malloc(sizeof(struct va_ctx))) == NULL)
		return NULL;
	if((vp->decisions = (decision_t *) malloc(len*sizeof(decision_t))) == NULL){
		free(vp);
		return NULL;
	}
#endif

	init_viterbi(vp,0);

	return vp;
}

/* Viterbi chainback */
int chainback_viterbi_withtail(
		void *p,
		unsigned char *data,  /* Decoded output data */
		unsigned int nbits,   /* Number of data bits */
		unsigned int endstate)/* Terminal encoder state */
{
	struct va_ctx *vp = p;
	decision_t *d;
	unsigned short k;

	if(p == NULL)
		return -EPERM;
	d = vp->decisions;

	d += 4; /* Look past tail */
	endstate %= 64;

	while(nbits-- != 0){
		k = (d[nbits].m[endstate] ) & 1;
		//update the end state, k is the input and it is the highest bit of the 6 shift rigisters
		endstate = (endstate >> 1) | (k << 5);
		data[nbits] = k;
	}

	return 0;
}

static int least_metric( unsigned mtype *array, unsigned int num )
{
	unsigned mtype *min = array;
	unsigned mtype *cur = array;

	assert( num != 0 );

	while(num--)
	{
		cur++;
		min = (*min > *cur) ? cur : min;

	}
	return (min - array);
}


#ifdef ONETBBLOCK
/* Viterbi chainback */
int chainback_viterbi_notail(
		void *p,
		unsigned char *data, /* Decoded output data */
		unsigned int  nbits,  /* Number of data bits */
		unsigned int  tblen,  /* Trace Back block length */
		unsigned int  *end_state)
{
	struct va_ctx *vp = p;
	decision_t *d;
	unsigned int minindex;
	unsigned int endstate;
	unsigned short k;

	if(p == NULL)
		return -EPERM;

	d = vp->decisions;

	//Get the endstate of the notail bit stream with nbits length
	//get the path with the minmum metrics
	minindex = least_metric(vp->old_metrics->w,64);
	*end_state=endstate=minindex;

	endstate %= 64;

	/////////////////////////////////one TB block (TB0 TB0 TB0)//////////////////////////////
	/////////////algorighm: 3 duplication of TB0 and decode them for correct results//////////////
	/////////////the last TB0 is for training, ignore the output data ///////////////////////

	//the last TB0 is for training, no output
	while(nbits-- != 0){
		k = (d[nbits+tblen*2].m[endstate] ) & 1;
		//update the end state, k is the input and it is the highest bit of the 6 shift rigisters
		endstate = (endstate >> 1) | (k << 5);
	}

	/////////////the middle TB0 output data is valid ////////////////////////////////////////
	//output the data, the middle TB0
	nbits=tblen;
	while(nbits-- != 0){

		k = (d[nbits+tblen].m[endstate] ) & 1;
		data[nbits] = endstate & 1;	 //output data
		//update the end state, k is the input and it is the highest bit of the 6 shift rigisters
		endstate = (endstate >> 1) | (k << 5);
	}

	return 0;
}

#else
/* Viterbi chainback */
int chainback_viterbi_notail(
		void *p,
		unsigned char *data, /* Decoded output data */
		unsigned int  nbits,  /* Number of data bits */
		unsigned int  tblen,  /* Trace Back block length */
		unsigned int  *end_state)
{
	struct va_ctx *vp = p;
	decision_t *d;
	unsigned int minindex;
	unsigned int endstate;
	unsigned short k;
	unsigned int datalen;
	unsigned int notrainlen;
	unsigned int nopadlen;

	datalen = nbits+ 2* tblen;   //all data + training length  TB0 TB1 TB2 TB3 TB0 TB1
	notrainlen = nbits+tblen;    // TB0 TB1 TB2 TB3 TB0
	nopadlen = nbits;            // TB0 TB1 TB2 TB3

	if(p == NULL)
		return -EPERM;

	d = vp->decisions;

	//Get the endstate of the notail bit stream with nbits length
	//get the path with the minmum metrics
	minindex = least_metric(vp->old_metrics->w, 64);
	*end_state=endstate=minindex;

	endstate %= 64;
	///////////////////////////////////multi TB blocks (TB0 TB1 TB2 TB3 )///////////////////////////////
	///////////////////////////////////algorighm: multi TB blocks //////////////////////////////////////
	////////////////////////////////////TB0 TB1 TB2 TB3 TB0 TB1 ////////////////////////////////////////
	//////////////////// TB1 for training, and TB0 and the followed block output the data //////////////
	//////////////////// TB0 TB3 TB2 TB1, the TB0 data should be reordered /////////////////////////////

	nbits = tblen;
	while(nbits-- != 0){   //firt TB1 is for training, no output
		k = (d[nbits+ notrainlen].m[endstate] ) & 1;
		//update the end state, k is the input and it is the highest bit of the 4 shift rigisters
		endstate = (endstate >> 1) | (k << 5);
	}

	//output the data TB0
	nbits= tblen;
	while(nbits-- != 0){
		k = (d[nbits+nopadlen].m[endstate] ) & 1;
		data[nbits] = endstate & 1;	 //output data
		//update the end state, k is the input and it is the highest bit of the 6 shift rigisters
		endstate = (endstate >> 1) | (k << 5);
	}

	//output the data TB3 -> TB 2 -> TB1
	nbits= 3*tblen;  //TB3 TB2 TB1
	while(nbits-- != 0){
		k = (d[nbits+ tblen].m[endstate] ) & 1;
		data[nbits+tblen] = endstate & 1;	 //output data
		//update the end state, k is the input and it is the highest bit of the 6 shift rigisters
		endstate = (endstate >> 1) | (k << 5);
	}

	return 0;
}
#endif


#define SUM (2*SAMPLE_MASK_CHAR)

#if defined SSE2OPT
//in each loop, 8 butterflies are conducted
//but the offset should be 16 bits aligned, so this version can not work
//sse2 address should be 16 bytes aligned for data load and store
//this is a 16bytes version which is derived from 8 bytes version

#define _mm_sel(va, vb, vsel) \
	(\
	 _mm_or_si128( (_mm_and_si128(vsel, va)), (_mm_andnot_si128(vsel, vb)) ) \
	)


int update_viterbi_blk(void *p,unsigned char *syms,int nbits)
{
	decision_t *d;
	struct va_ctx *vp=p;

	unsigned char sym0, sym1;
	__m128i  *psyms_v = (__m128i *)syms;
	__m128i  sym_v;

	__m128i branchtab_0, branchtab_1, branchtab_2, branchtab_3;
	__m128i sym0vorg,sym1vorg;

	__m128i metricv_0, metricv_1, metricv_2, metricv_3;
	__m128i metricv_s_0, metricv_neg_0, metricv_s_1, metricv_neg_1;
	__m128i mv_0,mv_1, mv_2,mv_3;
	__m128i mv_4,mv_5, mv_6,mv_7;

	__m128i decisionv_0, decisionv_1, decisionv_2, decisionv_3;
	__m128i nmv_0, nmv_1, nmv_2, nmv_3;

	__m128i decisionv_pack_0, decisionv_pack_1, decisionv_pack_2, decisionv_pack_3;
	__m128i nmv_pack_0, nmv_pack_1, nmv_pack_2, nmv_pack_3;

	__m128i ConstSumshortv = _mm_set_epi8(SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM,SUM);

	unsigned short metric_0;
	unsigned short metric_1;
	__m128i cmp_0, cmp_1, cmp_2, cmp_3;

	int i;
	static char ctmp[][16] __attribute__ ((aligned(128))) = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
		{4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
		{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
		{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
		{10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
		{11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
		{12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12},
		{13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13},
		{14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14},
		{15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15}
	};
	__m128i *shuf_idx = (__m128i *)ctmp;

	if(p == NULL)
		return -EPERM;

	d = (decision_t *)vp->curr_decision;

	branchtab_0 = _mm_load_si128((__m128i*) &branch_tab[0].c[0]);  //branch_tab[0].c[0] ~ branch_tab[0].c[15]
	branchtab_1 = _mm_load_si128((__m128i*) &branch_tab[1].c[0]);  //branch_tab[1].c[0] ~ branch_tab[1].c[15]
	branchtab_2 = _mm_load_si128((__m128i*) &branch_tab[0].c[16]);  //branch_tab[0].c[16] ~ branch_tab[0].c[31]
	branchtab_3 = _mm_load_si128((__m128i*) &branch_tab[1].c[16]);  //branch_tab[1].c[16] ~ branch_tab[1].c[31]

	nmv_pack_0 = _mm_load_si128((__m128i*) &vp->old_metrics->w[0]);    //0~15
	nmv_pack_1 = _mm_load_si128((__m128i*) &vp->old_metrics->w[0+32]); //32~47
	nmv_pack_2 = _mm_load_si128((__m128i*) &vp->old_metrics->w[16]);    //16~31
	nmv_pack_3 = _mm_load_si128((__m128i*) &vp->old_metrics->w[16+32]); //4r87~63

	int nbits_blk, nbits_rmd;

	nbits_blk = nbits >> 3;
	nbits_rmd = nbits & (~7);

	while(nbits_blk-- > 0){

		sym_v = *psyms_v++;

		for(i = 0; i < 16; i+=2)
		{


			sym0vorg = _mm_shuffle_epi8(sym_v, shuf_idx[i]);
			sym1vorg = _mm_shuffle_epi8(sym_v, shuf_idx[i+1]);

			metricv_0 = _mm_xor_si128(branchtab_0, sym0vorg);
			metricv_1 = _mm_xor_si128(branchtab_1, sym1vorg);
			metricv_2 = _mm_xor_si128(branchtab_2, sym0vorg);
			metricv_3 = _mm_xor_si128(branchtab_3, sym1vorg);
			metricv_s_0 = _mm_add_epi8(metricv_0, metricv_1);
			metricv_neg_0 = _mm_sub_epi8(ConstSumshortv, metricv_s_0);
			metricv_s_1 = _mm_add_epi8(metricv_2, metricv_3);
			metricv_neg_1 = _mm_sub_epi8(ConstSumshortv, metricv_s_1);

			mv_0 = _mm_add_epi8(nmv_pack_0, metricv_s_0);
			mv_1 = _mm_add_epi8(nmv_pack_1, metricv_neg_0);
			mv_2 = _mm_add_epi8(nmv_pack_0, metricv_neg_0);
			mv_3 = _mm_add_epi8(nmv_pack_1, metricv_s_0);
			mv_4 = _mm_add_epi8(nmv_pack_2, metricv_s_1);
			mv_5 = _mm_add_epi8(nmv_pack_3, metricv_neg_1);
			mv_6 = _mm_add_epi8(nmv_pack_2, metricv_neg_1);
			mv_7 = _mm_add_epi8(nmv_pack_3, metricv_s_1);

			decisionv_0 = _mm_cmpgt_epi8(mv_0, mv_1);
			nmv_0 = _mm_sel(mv_1, mv_0, decisionv_0);
			decisionv_1 = _mm_cmpgt_epi8(mv_2, mv_3);
			nmv_1 = _mm_sel(mv_3, mv_2, decisionv_1);
			decisionv_2 = _mm_cmpgt_epi8(mv_4, mv_5);
			nmv_2 = _mm_sel(mv_5, mv_4, decisionv_2);
			decisionv_3 = _mm_cmpgt_epi8(mv_6, mv_7);
			nmv_3 = _mm_sel(mv_7, mv_6, decisionv_3);

			decisionv_pack_0=_mm_unpacklo_epi8(decisionv_0,decisionv_1);
			decisionv_pack_1=_mm_unpackhi_epi8(decisionv_0,decisionv_1);
			decisionv_pack_2=_mm_unpacklo_epi8(decisionv_2,decisionv_3);
			decisionv_pack_3=_mm_unpackhi_epi8(decisionv_2,decisionv_3);
			nmv_pack_0=_mm_unpacklo_epi8(nmv_0,nmv_1);
			nmv_pack_2=_mm_unpackhi_epi8(nmv_0,nmv_1);
			nmv_pack_1=_mm_unpacklo_epi8(nmv_2,nmv_3);
			nmv_pack_3=_mm_unpackhi_epi8(nmv_2,nmv_3);

			_mm_store_si128((__m128i*)&d->m[2*0],     decisionv_pack_0);
			_mm_store_si128((__m128i*)&d->m[2*0+16],  decisionv_pack_1);
			_mm_store_si128((__m128i*)&d->m[2*16],     decisionv_pack_2);
			_mm_store_si128((__m128i*)&d->m[2*16+16],  decisionv_pack_3);
			d++;

			metric_0 = _mm_extract_epi16(_mm_unpacklo_epi8(nmv_pack_0, ConstSumshortv), 0);
			if(metric_0 > 128) {
				cmp_2 = _mm_min_epu8(_mm_min_epu8(nmv_pack_0, nmv_pack_1), _mm_min_epu8(nmv_pack_2, nmv_pack_3));

				cmp_2 = _mm_min_epu8(cmp_2, _mm_shuffle_epi32( cmp_2, 0x4E));
				cmp_2 = _mm_min_epu8(cmp_2, _mm_shuffle_epi32( cmp_2, 0xB1));
				cmp_2 = _mm_min_epu8(cmp_2, _mm_shufflehi_epi16( cmp_2, 0xB1 ));
				cmp_1 = _mm_unpackhi_epi8(cmp_2, cmp_2);
				cmp_2 = _mm_min_epu8(cmp_2, _mm_shufflehi_epi16( cmp_1, 0xB1 ));

				cmp_2 = _mm_unpackhi_epi8(cmp_2, cmp_2);
				metric_1 = _mm_extract_epi16(cmp_2, 7);
				cmp_2 = _mm_set_epi16( metric_1, metric_1, metric_1, metric_1, metric_1, metric_1, metric_1, metric_1 );

				nmv_pack_0 = _mm_sub_epi8(nmv_pack_0, cmp_2);
				nmv_pack_1 = _mm_sub_epi8(nmv_pack_1, cmp_2);
				nmv_pack_2 = _mm_sub_epi8(nmv_pack_2, cmp_2);
				nmv_pack_3 = _mm_sub_epi8(nmv_pack_3, cmp_2);
			}
		} //for(i = 0; i < 16; i+=2)
		nbits-=8;
	}

	while(nbits-- > 0){

		sym0 = *syms++;
		sym1 = *syms++;

		sym0vorg = _mm_set_epi8(sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0, sym0);
		sym1vorg = _mm_set_epi8(sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1, sym1);

		metricv_0 = _mm_xor_si128(branchtab_0, sym0vorg);
		metricv_1 = _mm_xor_si128(branchtab_1, sym1vorg);
		metricv_2 = _mm_xor_si128(branchtab_2, sym0vorg);
		metricv_3 = _mm_xor_si128(branchtab_3, sym1vorg);
		metricv_s_0 = _mm_add_epi8(metricv_0, metricv_1);
		metricv_neg_0 = _mm_sub_epi8(ConstSumshortv, metricv_s_0);
		metricv_s_1 = _mm_add_epi8(metricv_2, metricv_3);
		metricv_neg_1 = _mm_sub_epi8(ConstSumshortv, metricv_s_1);

		mv_0 = _mm_add_epi8(nmv_pack_0, metricv_s_0);
		mv_1 = _mm_add_epi8(nmv_pack_1, metricv_neg_0);
		mv_2 = _mm_add_epi8(nmv_pack_0, metricv_neg_0);
		mv_3 = _mm_add_epi8(nmv_pack_1, metricv_s_0);
		mv_4 = _mm_add_epi8(nmv_pack_2, metricv_s_1);
		mv_5 = _mm_add_epi8(nmv_pack_3, metricv_neg_1);
		mv_6 = _mm_add_epi8(nmv_pack_2, metricv_neg_1);
		mv_7 = _mm_add_epi8(nmv_pack_3, metricv_s_1);

		decisionv_0 = _mm_cmpgt_epi8(mv_0, mv_1);
		nmv_0 = _mm_sel(mv_1, mv_0, decisionv_0);
		decisionv_1 = _mm_cmpgt_epi8(mv_2, mv_3);
		nmv_1 = _mm_sel(mv_3, mv_2, decisionv_1);
		decisionv_2 = _mm_cmpgt_epi8(mv_4, mv_5);
		nmv_2 = _mm_sel(mv_5, mv_4, decisionv_2);
		decisionv_3 = _mm_cmpgt_epi8(mv_6, mv_7);
		nmv_3 = _mm_sel(mv_7, mv_6, decisionv_3);

		decisionv_pack_0=_mm_unpacklo_epi8(decisionv_0,decisionv_1);
		decisionv_pack_1=_mm_unpackhi_epi8(decisionv_0,decisionv_1);
		decisionv_pack_2=_mm_unpacklo_epi8(decisionv_2,decisionv_3);
		decisionv_pack_3=_mm_unpackhi_epi8(decisionv_2,decisionv_3);
		nmv_pack_0=_mm_unpacklo_epi8(nmv_0,nmv_1);
		nmv_pack_2=_mm_unpackhi_epi8(nmv_0,nmv_1);
		nmv_pack_1=_mm_unpacklo_epi8(nmv_2,nmv_3);
		nmv_pack_3=_mm_unpackhi_epi8(nmv_2,nmv_3);

		_mm_store_si128((__m128i*)&d->m[2*0],     decisionv_pack_0);
		_mm_store_si128((__m128i*)&d->m[2*0+16],  decisionv_pack_1);
		_mm_store_si128((__m128i*)&d->m[2*16],     decisionv_pack_2);
		_mm_store_si128((__m128i*)&d->m[2*16+16],  decisionv_pack_3);
		d++;

		metric_0 = _mm_extract_epi16(_mm_unpacklo_epi8(nmv_pack_0, ConstSumshortv), 0);
		if(metric_0 > 128) {
			cmp_2 = _mm_min_epu8(_mm_min_epu8(nmv_pack_0, nmv_pack_1), _mm_min_epu8(nmv_pack_2, nmv_pack_3));

			cmp_2 = _mm_min_epu8(cmp_2, _mm_shuffle_epi32( cmp_2, 0x4E));
			cmp_2 = _mm_min_epu8(cmp_2, _mm_shuffle_epi32( cmp_2, 0xB1));
			cmp_2 = _mm_min_epu8(cmp_2, _mm_shufflehi_epi16( cmp_2, 0xB1 ));
			cmp_1 = _mm_unpackhi_epi8(cmp_2, cmp_2);
			cmp_2 = _mm_min_epu8(cmp_2, _mm_shufflehi_epi16( cmp_1, 0xB1 ));

			cmp_2 = _mm_unpackhi_epi8(cmp_2, cmp_2);
			metric_1 = _mm_extract_epi16(cmp_2, 7);
			cmp_2 = _mm_set_epi16( metric_1, metric_1, metric_1, metric_1, metric_1, metric_1, metric_1, metric_1 );

			nmv_pack_0 = _mm_sub_epi8(nmv_pack_0, cmp_2);
			nmv_pack_1 = _mm_sub_epi8(nmv_pack_1, cmp_2);
			nmv_pack_2 = _mm_sub_epi8(nmv_pack_2, cmp_2);
			nmv_pack_3 = _mm_sub_epi8(nmv_pack_3, cmp_2);
		}
	}

	_mm_store_si128((__m128i*)&vp->old_metrics->w[2*0],     nmv_pack_0); //0~15
	_mm_store_si128((__m128i*)&vp->old_metrics->w[2*0+16],  nmv_pack_2); //32~47
	_mm_store_si128((__m128i*)&vp->old_metrics->w[2*16],     nmv_pack_1); //16~31
	_mm_store_si128((__m128i*)&vp->old_metrics->w[2*16+16],  nmv_pack_3); //48~63

	vp->curr_decision = d;
	return 0;
}
#elif defined VSXOPT
int update_viterbi_blk(void *p,unsigned char *syms,int nbits)
{
	decision_t *d;
	struct va_ctx *vp=p;

	unsigned char sym0, sym1;
	vector unsigned char  *psyms_v = (vector unsigned char *)syms;
	vector unsigned char  sym_v;

	vector unsigned char branchtab_0, branchtab_1, branchtab_2, branchtab_3;
	vector unsigned char sym0vorg,sym1vorg;

	vector unsigned char metricv_0, metricv_1, metricv_2, metricv_3;
	vector unsigned char metricv_s_0, metricv_neg_0, metricv_s_1, metricv_neg_1;
	vector unsigned char mv_0,mv_1, mv_2,mv_3;
	vector unsigned char mv_4,mv_5, mv_6,mv_7;

	vector unsigned char decisionv_0, decisionv_1, decisionv_2, decisionv_3;
	vector unsigned char nmv_0, nmv_1, nmv_2, nmv_3;

	vector unsigned char decisionv_pack_0, decisionv_pack_1, decisionv_pack_2, decisionv_pack_3;
	vector unsigned char nmv_pack_0, nmv_pack_1, nmv_pack_2, nmv_pack_3;

	vector unsigned char ConstSumshortv = {SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM, SUM};

	unsigned char metric_0;
	unsigned char metric_1;
	vector unsigned char cmp_0, cmp_1, cmp_2, cmp_3;

	int i;
	static char ctmp[][16] __attribute__ ((aligned(128))) = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
		{4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
		{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
		{9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
		{10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
		{11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
		{12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12},
		{13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13},
		{14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14},
		{15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15}
	};
	vector unsigned char *shuf_idx = (vector unsigned char *)ctmp;

	if(p == NULL)
		return -EPERM;

	d = (decision_t *)vp->curr_decision;

	branchtab_0 = vec_ld( 0, (vector unsigned char*)branch_tab[0].v);  //branch_tab[0].c[0] ~ branch_tab[0].c[15]
	branchtab_1 = vec_ld( 0, (vector unsigned char*)branch_tab[1].v);  //branch_tab[1].c[0] ~ branch_tab[1].c[15]
	branchtab_2 = vec_ld(16, (vector unsigned char*)branch_tab[0].v);  //branch_tab[0].c[16] ~ branch_tab[0].c[31]
	branchtab_3 = vec_ld(16, (vector unsigned char*)branch_tab[1].v);  //branch_tab[1].c[16] ~ branch_tab[1].c[31]

	nmv_pack_0  = vec_ld( 0, (vector unsigned char*)vp->old_metrics->w); //0~15
	nmv_pack_1  = vec_ld(32, (vector unsigned char*)vp->old_metrics->w); //32~47
	nmv_pack_2  = vec_ld(16, (vector unsigned char*)vp->old_metrics->w); //16~31
	nmv_pack_3  = vec_ld(48, (vector unsigned char*)vp->old_metrics->w); //4r87~63

	int nbits_blk, nbits_rmd;

	nbits_blk = nbits >> 3;
	nbits_rmd = nbits & (~7);

	while(nbits_blk-- > 0){

		sym_v = *psyms_v++;

		for(i = 0; i < 16; i+=2){
      // splat i/i+1 byte in the vector
			sym0vorg = vec_perm(sym_v, sym_v, shuf_idx[i  ]);
			sym1vorg = vec_perm(sym_v, sym_v, shuf_idx[i+1]);

			metricv_0 = vec_xor(branchtab_0, sym0vorg);
			metricv_1 = vec_xor(branchtab_1, sym1vorg);
			metricv_2 = vec_xor(branchtab_2, sym0vorg);
			metricv_3 = vec_xor(branchtab_3, sym1vorg);
			metricv_s_0   = vec_add(metricv_0,      metricv_1);
			metricv_neg_0 = vec_sub(ConstSumshortv, metricv_s_0);
			metricv_s_1   = vec_add(metricv_2,      metricv_3);
			metricv_neg_1 = vec_sub(ConstSumshortv, metricv_s_1);

			mv_0 = vec_add(nmv_pack_0, metricv_s_0);
			mv_1 = vec_add(nmv_pack_1, metricv_neg_0);
			mv_2 = vec_add(nmv_pack_0, metricv_neg_0);
			mv_3 = vec_add(nmv_pack_1, metricv_s_0);
			mv_4 = vec_add(nmv_pack_2, metricv_s_1);
			mv_5 = vec_add(nmv_pack_3, metricv_neg_1);
			mv_6 = vec_add(nmv_pack_2, metricv_neg_1);
			mv_7 = vec_add(nmv_pack_3, metricv_s_1);

			decisionv_0 = (vector unsigned char)vec_cmpgt(mv_0, mv_1);
			nmv_0 = vec_sel(mv_0, mv_1, decisionv_0);
			decisionv_1 = (vector unsigned char)vec_cmpgt(mv_2, mv_3);
			nmv_1 = vec_sel(mv_2, mv_3, decisionv_1);
			decisionv_2 = (vector unsigned char)vec_cmpgt(mv_4, mv_5);
			nmv_2 = vec_sel(mv_4, mv_5, decisionv_2);
			decisionv_3 = (vector unsigned char)vec_cmpgt(mv_6, mv_7);
			nmv_3 = vec_sel(mv_6, mv_7, decisionv_3);

			decisionv_pack_0=vec_mergeh(decisionv_0, decisionv_1);
			decisionv_pack_1=vec_mergel(decisionv_0, decisionv_1);
			decisionv_pack_2=vec_mergeh(decisionv_2, decisionv_3);
			decisionv_pack_3=vec_mergel(decisionv_2, decisionv_3);
			nmv_pack_0=vec_mergeh(nmv_0,nmv_1);
			nmv_pack_2=vec_mergel(nmv_0,nmv_1);
			nmv_pack_1=vec_mergeh(nmv_2,nmv_3);
			nmv_pack_3=vec_mergel(nmv_2,nmv_3);

			vec_st(decisionv_pack_0, 2* 0   , (vector unsigned char*)d->m);
			vec_st(decisionv_pack_1, 2* 0+16, (vector unsigned char*)d->m);
			vec_st(decisionv_pack_2, 2*16   , (vector unsigned char*)d->m);
			vec_st(decisionv_pack_3, 2*16+16, (vector unsigned char*)d->m);
			d++;

			metric_0 = vec_extract(vec_mergeh(nmv_pack_0, ConstSumshortv), 0);
			if(metric_0 & 0x80) {
				cmp_2 = vec_min(vec_min(nmv_pack_0, nmv_pack_1), \
                        vec_min(nmv_pack_2, nmv_pack_3));

				cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 1));
				cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 2));
				cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 4));
				cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 8));
				cmp_2 = vec_splat(cmp_2, 0);

				nmv_pack_0 = vec_sub(nmv_pack_0, cmp_2);
				nmv_pack_1 = vec_sub(nmv_pack_1, cmp_2);
				nmv_pack_2 = vec_sub(nmv_pack_2, cmp_2);
				nmv_pack_3 = vec_sub(nmv_pack_3, cmp_2);
			}
		} //for(i = 0; i < 16; i+=2)
		nbits-=8;
	}

	while(nbits-- > 0){

		sym0 = *syms++;
		sym1 = *syms++;

		sym0vorg = (vector unsigned char){sym0};
		sym1vorg = (vector unsigned char){sym1};
    sym0vorg = vec_splat(sym0vorg, 0);
    sym1vorg = vec_splat(sym1vorg, 0);

		metricv_0 = vec_xor(branchtab_0, sym0vorg);
		metricv_1 = vec_xor(branchtab_1, sym1vorg);
		metricv_2 = vec_xor(branchtab_2, sym0vorg);
		metricv_3 = vec_xor(branchtab_3, sym1vorg);
		metricv_s_0   = vec_add(metricv_0,      metricv_1);
		metricv_neg_0 = vec_sub(ConstSumshortv, metricv_s_0);
		metricv_s_1   = vec_add(metricv_2,      metricv_3);
		metricv_neg_1 = vec_sub(ConstSumshortv, metricv_s_1);

		mv_0 = vec_add(nmv_pack_0, metricv_s_0);
		mv_1 = vec_add(nmv_pack_1, metricv_neg_0);
		mv_2 = vec_add(nmv_pack_0, metricv_neg_0);
		mv_3 = vec_add(nmv_pack_1, metricv_s_0);
		mv_4 = vec_add(nmv_pack_2, metricv_s_1);
		mv_5 = vec_add(nmv_pack_3, metricv_neg_1);
		mv_6 = vec_add(nmv_pack_2, metricv_neg_1);
		mv_7 = vec_add(nmv_pack_3, metricv_s_1);

		decisionv_0 = (vector unsigned char)vec_cmpgt(mv_0, mv_1);
		nmv_0 = vec_sel(mv_0, mv_1, decisionv_0);
		decisionv_1 = (vector unsigned char)vec_cmpgt(mv_2, mv_3);
		nmv_1 = vec_sel(mv_2, mv_3, decisionv_1);
		decisionv_2 = (vector unsigned char)vec_cmpgt(mv_4, mv_5);
		nmv_2 = vec_sel(mv_4, mv_5, decisionv_2);
		decisionv_3 = (vector unsigned char)vec_cmpgt(mv_6, mv_7);
		nmv_3 = vec_sel(mv_6, mv_7, decisionv_3);

		decisionv_pack_0=vec_mergeh(decisionv_0, decisionv_1);
		decisionv_pack_1=vec_mergel(decisionv_0, decisionv_1);
		decisionv_pack_2=vec_mergeh(decisionv_2, decisionv_3);
		decisionv_pack_3=vec_mergel(decisionv_2, decisionv_3);
		nmv_pack_0=vec_mergeh(nmv_0,nmv_1);
		nmv_pack_2=vec_mergel(nmv_0,nmv_1);
		nmv_pack_1=vec_mergeh(nmv_2,nmv_3);
		nmv_pack_3=vec_mergel(nmv_2,nmv_3);

		vec_st(decisionv_pack_0, 2* 0   , (vector unsigned char*)d->m);
		vec_st(decisionv_pack_1, 2* 0+16, (vector unsigned char*)d->m);
		vec_st(decisionv_pack_2, 2*16   , (vector unsigned char*)d->m);
		vec_st(decisionv_pack_3, 2*16+16, (vector unsigned char*)d->m);
		d++;

		metric_0 = vec_extract(vec_mergeh(nmv_pack_0, ConstSumshortv), 0);
		if(metric_0 & 0x80) {
			cmp_2 = vec_min(vec_min(nmv_pack_0, nmv_pack_1), \
                      vec_min(nmv_pack_2, nmv_pack_3));

			cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 1));
			cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 2));
			cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 4));
			cmp_2 = vec_min(cmp_2, vec_sld(cmp_2, cmp_2, 8));
			cmp_2 = vec_splat(cmp_2, 0);

			nmv_pack_0 = vec_sub(nmv_pack_0, cmp_2);
			nmv_pack_1 = vec_sub(nmv_pack_1, cmp_2);
			nmv_pack_2 = vec_sub(nmv_pack_2, cmp_2);
			nmv_pack_3 = vec_sub(nmv_pack_3, cmp_2);
		}
	}

	vec_st(nmv_pack_0, 2* 0   , (vector unsigned char*)vp->old_metrics->w); //0~15
	vec_st(nmv_pack_2, 2* 0+16, (vector unsigned char*)vp->old_metrics->w); //32~47
	vec_st(nmv_pack_1, 2*16   , (vector unsigned char*)vp->old_metrics->w); //16~31
	vec_st(nmv_pack_3, 2*16+16, (vector unsigned char*)vp->old_metrics->w); //48~63

	vp->curr_decision = d;
	return 0;
}
#endif

#if defined SSE2OPT
void prt_xmm(__m128i vx)
{
	//int i;
	__m128i tmp1, tmp2;
	__m128i Zero = _mm_set_epi8(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	tmp1 = _mm_unpacklo_epi8(vx, Zero);
	tmp2 = _mm_unpackhi_epi8(vx, Zero);
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 0));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 1));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 2));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 3));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 4));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 5));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 6));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp1, 7));
	FLOG_INFO(" | ");
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 0));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 1));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 2));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 3));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 4));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 5));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 6));
	FLOG_INFO("0x%x, ", _mm_extract_epi16(tmp2, 7));
	FLOG_INFO("\n");
	return;
}
#elif defined VSXOPT
void prt_xmm(vector unsigned int vx)
{
	FLOG_INFO("0x%08x, ", vec_extract(vx, 0));
	FLOG_INFO("0x%08x, ", vec_extract(vx, 1));
	FLOG_INFO("0x%08x, ", vec_extract(vx, 2));
	FLOG_INFO("0x%08x, ", vec_extract(vx, 3));
	FLOG_INFO("\n");
	return;
}
#endif


/* Delete instance of a Viterbi decoder */
void delete_viterbi(void *p){
	struct va_ctx *vp = p;

	if(vp != NULL){
		free(vp->decisions);
		free(vp);
	}
}




