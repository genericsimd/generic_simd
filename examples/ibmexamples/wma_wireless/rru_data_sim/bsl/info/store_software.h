/**********************************************************
 * @file store_software.h
 *  Header of software related data structures and related constants
 *
 *  Change History:
 *  Date            Description
 *  -----------     ---------------
 *  04-Mar. 10      Initial create
 */

#ifndef _WIT_MANAGEMENT_INFORMATION_MANAGEMENT_SOFTWARE_DESCRIPTION_H_INC_
#define _WIT_MANAGEMENT_INFORMATION_MANAGEMENT_SOFTWARE_DESCRIPTION_H_INC_

#include <time.h>

#include "../witm_types.h"

/// Macro for Software download status
#define WMIDSW_DSNULL                   0
#define WMIDSW_DSDOWNLOAD               1
#define WMIDSW_DSACTIVATED              2

/// Item names for Software Descriptor
#define WMIDN_SWDESC_DEVINDEX           "dev_index"
#define WMIDN_SWDESC_VENDORID           "vendor_id"
#define WMIDN_SWDESC_HWID               "hardware_id"
#define WMIDN_SWDESC_CURVERSION         "current_version"
#define WMIDN_SWDESC_DLVERSION          "download_version"
#define WMIDN_SWDESC_DLSTATUS           "download_status"
#define WMIDN_SWDESC_DLPROGRESS         "download_progress"
#define WMIDN_SWDESC_UPGTIME            "upgrade_timestamp"
#define WMIDN_SWDESC_MEMO               "memo"

/// Definition of software descriptor
struct wmis_swdesc
{
    int dev_index;
    char vendor_id[WITM_STRLEN_LONG];
    char hardware_id[WITM_STRLEN_LONG];
    char current_version[WITM_STRLEN_LONG];
    char download_version[WITM_STRLEN_LONG];
    int download_status; // 0: null; 1: download, 2:activate
    int download_progress; // 0 ~ 100 (%)
    struct datetime upgrade_timestamp;

    char memo[WITM_STRLEN_HUGE];
};

/// Item Names for Software Running Status
#define WMIDN_SWRS_PID			"service_pid"
#define WMIDN_SWRS_FILENAME     "file_name"

/// Definitions of software definitions
struct wmis_swrs
{
    int32_t pid;
    int32_t policy;
    int32_t priority;

	char filename[WITM_STRLEN_LONG];
};

/// Software descriptor
extern const struct wmis_swdesc *g_wmisr_swdesc;
extern struct wmis_swdesc *g_wmisw_swdesc;
/// Software runtime status
extern const struct wmis_swrs *g_wmisr_swrs;
extern struct wmis_swrs *g_wmisw_swrs;

#endif
