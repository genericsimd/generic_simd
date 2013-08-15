/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: textlogger.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "log.h"

#define CHARFILE	"char.logs.out"

int main(int argc, char *argv[])
{
	char outfile[256];
	FILE *fp, *fp2, *fp_perf, *fp_burst;

	assert (argc > 1);

	fp = fopen(argv[1], "r");

	assert (fp != NULL);

	strcpy(outfile, argv[1]);
	strcat(outfile, ".char");
	fp2 = fopen(outfile, "w");

	/* write the performance output*/
	fp_perf = fopen("perf_profile.out","w");
	fp_burst = fopen("burst_profile.out", "w");

	do {
		trace_event_type_t et;

		int ret;
		char out_buf[128];

		ret = fread(&et, sizeof(int), 1, fp);

		if (!feof(fp)) {
			fseek(fp, -1*sizeof(int), SEEK_CUR);

			lock_trace_info_t lti;
			switch(et) {
			case LOCK_TRYING:
			case LOCK_ACQUIRED:
			case LOCK_RELEASED: {
				ret = fread(&lti, sizeof(lock_trace_info_t), 1, fp);
				if (1 == ret) {
					sprintf(out_buf, "%d %d %lld %p\n", lti.ev_type, lti.thread_id, lti.ev_ts, lti.lock_addr);
					fputs(out_buf, fp2);
				}
			}
			break;

			case SDU_QUEUE_LENGTH: {
				sdu_q_trace_info_t sti;
				ret = fread(&sti, sizeof(sdu_q_trace_info_t), 1, fp);
				if (1 == ret) {
					sprintf(out_buf, "%d %d %lld %d %d\n", sti.ev_type, sti.thread_id, sti.ev_ts, sti.cid, sti.sdu_q_len);
					fputs(out_buf, fp2);
				}
			}
			break;

			case FRAME_CREATION_START:
			case FRAME_CREATION_END: {
				frame_trace_info_t fti;
				ret = fread(&fti, sizeof(frame_trace_info_t), 1, fp);
				if (1 == ret) {
					sprintf(out_buf, "%d %d %lld %d\n", fti.ev_type, fti.thread_id, fti.ev_ts, fti.frame_num);
					fputs(out_buf, fp2);
				}
			}
			break;

			case SDU_ALLOCED:
			case SDU_FREED: {
				sdu_mem_scope_info_t smi;
				ret = fread(&smi, sizeof(sdu_mem_scope_info_t), 1, fp);
				if (1 == ret) {
					sprintf(out_buf, "%d %d %lld %p %d\n", smi.ev_type, smi.thread_id, smi.ev_ts, smi.sdu_start_addr, smi.class_type);
					fputs(out_buf, fp2);
				}
			}
			break;
			case PERF_PROFILE_EVENT:
			{
				perf_trace_info_t pti;
				ret = fread(&pti, sizeof(perf_trace_info_t), 1, fp);
				if (1 == ret) {
					sprintf(out_buf, "%d %d %lld\n", pti.frame_id, pti.event_id, pti.ev_ts);
					fputs(out_buf, fp_perf);
				}
				break;
			}
			case BURST_PROFILE_EVENT:
			{
				burst_trace_info_t pti;
				ret = fread(&pti, sizeof(burst_trace_info_t), 1, fp);
				if (1 == ret) {
					sprintf(out_buf, "%d %d %d %d %lld\n", pti.frame_id, pti.event_id, pti.burst_id, pti.num_bytes, pti.ev_ts);
					fputs(out_buf, fp_burst);
				}
				break;
			}
			default:
				// Skip this event and advance the pointer by sizeof int,
				// since the event type is not valid
				fseek(fp, sizeof(int), SEEK_CUR);
				long offset = ftell(fp);
				printf("Current Offset: %ld , %d\n", offset, et);
				break;
			}
		} else {
			break;
		}
	} while(1);

	// close
	fclose(fp2);
	fclose(fp_perf);
	fclose(fp_burst);
}
