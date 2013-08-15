/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_acm_test.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Sep.2012       Created                                 Mukundan Madhavan

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <unistd.h>
#include "mac_amc.h"
#include "thread_sync.h"
#include "mac.h"
#include "CS.h"

extern sdu_queue *dl_sdu_queue;

int bs_amc_test()
{
	int i;int x;int count = 0;
	while (can_sync_continue())
	{
		x = rand();x = x%30;
		count++; 
		if (count > 4) x = 10;
		if (count == 1 || count > 10) x = 26;
		//Put in update statements
		for (i =0;i<NUM_SS;i++)
		{
			FLOG_INFO("will call update_ss_ul_link_quality with %d cinr\n", x);
			update_ss_ul_link_quality(i+BASIC_CID_MIN_VALUE, x, 0);
			//update_ss_dl_link_quality_cpe(i+BASIC_CID_MIN_VALUE, x, 0);
		}		

		sleep(1);
	}
	return 0;
}
int ss_amc_test()
{
	int i;int x;
	while (can_sync_continue())
	{
			i = rand();x = i%10;
			update_ss_dl_link_quality_cpe(BASIC_CID_MIN_VALUE, 24 + x - 10, 0);

		sleep(1);
	}
	return 0;
}
