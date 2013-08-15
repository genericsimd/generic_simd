/**********************************************************
 * @file witm_func.h
 *  Header of Function Management Library
 *      Defines all constants and APIs for function management
 *
 *  Change History:
 *  Date            Description
 *  -----------     ---------------
 *  04-Mar. 10      Initial create
 */

#ifndef _WIT_FUNCTION_MANAGEMENT_H_INC_
#define _WIT_FUNCTION_MANAGEMENT_H_INC_

#include "witm_info.h"

/**
 * @brief Macro for Command Line Parameter Parser.
 * None for Function Management now
 */
#define WMF_CLPARAMS			""WMI_CLPARAMS

#ifdef __cplusplus
extern C
{
#endif

    /// Functional Management Fundamentals
    /**
     * @brief parse all command line parameters.
     * @param argc[I] - parameter count
     * @param argv[I] - parameters
     * @return success of execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfunc_parsecmdline(int32_t argc, char **argv);

    /**
     * @brief parse a single command line parameter.
     * @param opt[I] - cmd
     * @param arg[I] - parameter of command
     * @return success of execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfunc_parseparam(int32_t opt, char *arg);

    /**
     * @brief initialize the functional management library.
     *      Initialize all resources
     *      Information Library and Runtime Library are also initialized
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfunc_init();

    /**
     * @brief exit the functional management library.
     *      Release all resources
     *      Information Library and Runtime Library are also exited
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfunc_exit();

    /// Keeping all funcs here for now.
    /// May split them into specific headers in the future if too many

    /// Configuration Management
    /**
     * @brief upgrade software to target server.
     *      Assuming using TCP/IP (addr:port pair to identify the server)
     * @param filename[I] - file to be loaded to target
     * @param pdestaddr[I] - IP address of destination
     * @param destport[I] - Command Port of destination
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfcsw_upgrade(const char *filename,
            char *pdestaddr, uint16_t destport);

    /**
     * @brief Get software info on target server.
     *      Assuming using TCP/IP (addr:port pair to identify the server)
     * @param desc[O] - software descriptor, place to store the information
     * @param pdestaddr[I] - IP address of destination
     * @param destport[I] - Command Port of destination
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfcsw_getswinfo(struct wmis_swdesc *desc,
            char *pdestaddr, uint16_t destport);

    /**
     * @brief Set software info on target server.
     *      Assuming using TCP/IP (addr:port pair to identify the server)
     * @param desc[I] - software descriptor, carrying the information to be set
     * @param pdestaddr[I] - IP address of destination
     * @param destport[I] - Command Port of destination
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfcsw_setswinfo(const struct wmis_swdesc *desc,
            char *pdestaddr, uint16_t destport);

    /**
     * @brief Start the service on target server.
     *      Assuming using TCP/IP (addr:port pair to identify the server)
     *      Service has a universial constat name and path
     * @param pdestaddr[I] - IP address of destination
     * @param destport[I] - Command Port of destination
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfcsw_startservice(char *pdestaddr, uint16_t destport, const char *pathname, int32_t sw_proc);

    /**
     * @brief Stop the service on target server.
     *      Assuming using TCP/IP (addr:port pair to identify the server)
     *      Service has a universial constat name and path
     * @param destport[I] - Command Port of destination
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfcsw_stopservice(char *pdestaddr, uint16_t destport, int32_t sw_proc);

    /**
     * @brief Get service status on target server.
     *      Assuming using TCP/IP (addr:port pair to identify the server)
     * @param desc[O] - status descriptor, place to store the information
     * @param pdestaddr[I] - IP address of destination
     * @param destport[I] - Command Port of destination
     * @return success of the execution
     * @retval 0 - success
     * @retval -1 - errors
     */
    extern int32_t wmfcsw_getsvcstatus(struct wmis_swrs *desc,
            char *pdestaddr, uint16_t destport, int32_t sw_proc);

#ifdef __cplusplus
}
#endif

#endif
