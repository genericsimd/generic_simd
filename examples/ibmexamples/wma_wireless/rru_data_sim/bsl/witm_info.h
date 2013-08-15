/**********************************************************
 * @file witm_info.h
 *  Header of Information Management Library
 *      Defines all constants and APIs for Information management
 *      Includes all header files of components in information management
 *
 *  Change History:
 *  Date            Description
 *  -----------     ---------------
 *  04-Mar. 10      Initial create
 */

#ifndef _WIT_INFORMATION_MANAGEMENT_H_INC_
#define _WIT_INFORMATION_MANAGEMENT_H_INC_

#include <sys/types.h>

#include "witm_rt.h"
#include "witm_types.h"

#include "info/store_software.h"
#include "info/store_monitor.h"
#include "info/store_freq.h"

/// Macro for Sync Storage
/// Blocks of storage, connects to specific access models
enum
{
    WMISF_SWDESC = 0,
    WMISF_SWRS,
    WMISF_TX_BLOCK,
    WMISF_RX_BLOCK,
    WMISF_RANGING,
    WMISF_SYNC,
    WMISF_BER,
    WMISF_BER_LINK,
    WMISF_FREQ,
    WMISF_ALARM,
    WMISF_SPEC_SETTING,
    WMISF_INVALID
};

/// Roles of Business Units for library initialization
/// The number of the enumeration meant to the redundant duplication of memory
enum
{
    WMI_ROLEMGMT = 1, WMI_ROLEBS = 2, WMI_ROLEINVALID
};

/// Procs of BS
enum
{
	WMI_SWRS_BSPROC = 0, WMI_SWRS_TESTUSRP, WMI_SWRS_TOTAL
};

/// Macro for Command Line Parameter Parser
/// None for information management
#define WMI_CLPARAMS			""WMRT_CLPARAMS

/// Global Variables defining of management informations.

extern commterm_t wmrt_localcmdterm;
extern commterm_t wmrt_localackterm;

#ifdef __cplusplus
extern C
{
#endif

    /// Information Management Fundamentals
    /**
     * @brief parse all command line parameters.
     * @param argc[I] - parameter count
     * @param argv[I] - parameters
     * @return success of execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wminfo_parsecmdline(int32_t argc, char **argv);

    /**
     * @brief parse a single command line parameter.
     * @param opt[I] - cmd
     * @param arg[I] - parameter of command
     * @return success of execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wminfo_parseparam(int32_t opt, char *arg);

    /**
     * @brief initialize the information management library.
     *      Initialize all resources
     *      Runtime Library is also initialized
     * @param role - role of this Business Unit: BS or Mgmt
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wminfo_init(uint32_t role);

    /**
     * @brief exit the information management library.
     *      Release all resources
     *      Information Library and Runtime Library are also exited
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wminfo_exit();
    /**
     * @brief start to receive and process messages from command terminal.
     *      This function enters a message processing loop and does not exit
     *      until a exit-signal is received.
     *      Set the dofork flag to 1 to stop blocking the main process.
     * @param dofork[I] - flags for creating new process. 0:no fork; other: fork
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_startmsgloop(int32_t dofork);

    /// Block Syncronization
    /**
     * @brief retrieve a block from destination terminal.
     * @param pdstterm[I] - pointer to descriptor of destination terminal
     * @param blockid[I] - id of block being retrieved
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_retrieveblock(commterm_t *pdstterm, int32_t blockid);

    /**
     * @brief submit a block to destination terminal
     * @param pdstterm[I] - pointer to descriptor of destination terminal
     * @param blockid[I] - id of block being submitted
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_submitblock(commterm_t *pdstterm, int32_t blockid);

    /// File Operation
    /**
     * @brief upload a file to destination terminal
     *      The file will be stored to a temporary path with same name and
     *  same mode.
     * @param pdstterm[I] - pointer to descriptor of destination terminal
     * @param filename[I] - file to be uploaded
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_uploadfile(commterm_t *pdstterm, char *filename);

    /**
     * @brief install a file to specific folder in destination terminal.
     *      The file should be uploaded already.
     * @param pdstterm[I] - pointer to descriptor of destination terminal
     * @param filename[I] - file to be install
     * @param destpath[I] - specific folder to put the file
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_installfile(commterm_t *pdstterm,
            char *filename, char *destpath);

    /// Process Operation
    /**
     * @brief start a process in destination terminal.
     *      The binary should be uploaded and has the permission to run.
     * @param pdstterm[I] - pointer to descriptor of destination terminal
     * @param filename[I] - file to be run
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_startproc(commterm_t *pdstterm, const char *filename, int32_t sw_proc);

    /**
     * @brief stop a process in destination terminal.
     * @param pdstterm[I] - pointer to descriptor of destination terminal
     * @param pid[I] - id of the target process
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmim_stopproc(commterm_t *pdstterm, int32_t pid, int32_t sw_proc);

    extern int32_t wmis_alterblock(int32_t id);

    extern int32_t wmir_createrep(commterm_t *dstterm, int32_t blockid, void *block);
    extern int32_t wmir_releaserep(int32_t id);
    extern int32_t wmir_replicate(int32_t id);


#ifdef __cplusplus
}
#endif

#endif
