#ifndef _WIT_MONITOR_TOOL_FREQ_H_
#define _WIT_MONITOR_TOOL_FREQ_H_

#include "../witm_types.h"

#define WITM_FREQ_MAX_POINT      25600

#define WMMSG_INVALID       0x00000000 /* 0 */
#define WMMSG_WARNING		0x00000001 /* 1 */

#define WMIDN_SPEC_CENTREALFREQ       "central_freq"
#define WMIDN_SPEC_DATFILENAME        "datfile_name"

/*
 *  Definition of plot struct
 *  current means the index of latest 256 points
 *  eg: for MAX_POINT is 25600 (2.3G ~~ 2.7G)
 *      since 400M/4M = 100
 *  then current rangs from 1 to 100
 */
struct wmtf_freq {
	float freq[WITM_FREQ_MAX_POINT];
	float fft[WITM_FREQ_MAX_POINT];

	int current;
};

/*
 *  Definition of alarm msg notify
 *   MSG_TYPE:
 *   	WMMSG_INVALID  --- no alarm msg
 *   	WMMSG_WARNING  --- warning msg
 */
struct wmtf_alarm {
	int32_t msg_type;

	char message[WITM_STRLEN_LONG]; /* the alarm message */
};

/*
 *  Definition of setting list
 */
struct wmis_specset {
	double central_freq;
	char datfile[WITM_STRLEN_LONG];
};

extern const struct wmtf_freq *g_wmtfr_freq;
extern struct wmtf_freq *g_wmtfw_freq;

extern const struct wmtf_alarm *g_wmtfr_alarm;
extern struct wmtf_alarm *g_wmtfw_alarm;

extern const struct wmis_specset *g_wmisr_specset;
extern struct wmis_specset *g_wmisw_specset;

#endif /* _WIT_MONITOR_TOOL_FREQ_H_ */
