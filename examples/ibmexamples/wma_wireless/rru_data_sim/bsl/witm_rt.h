/**********************************************************
 * @file witm_rt.h
 *  Header of Managerial Runtime Library
 *      Defines all constants and APIs for Managerial Runtime
 *      Includes all header files of components in managerial runtime
 *
 *  Change History:
 *  Date            Description
 *  -----------     ---------------
 *  04-Mar. 10      Initial create
 */

#ifndef _WIT_MANAGEMENT_RUNTIME_H_INC_
#define _WIT_MANAGEMENT_RUNTIME_H_INC_

#include <stdio.h>
#include <time.h>

#include "witm_types.h"

/// Process scheduling policy
#define WMRTP_SPNORMAL          SCHED_OTHER
#define WMRTP_SPFIFO            SCHED_FIFO
#define WMRTP_SPRR              SCHED_RR

/// Queue flags
#define WMRTQ_BLOCKING          0
#define WMRTQ_NONBLOCKING       1

/// Communication Related
#ifndef WMRT_COMMDEVICE
#define WMRT_COMMDEVICEETH
#endif

/**
 * @brief Macro for Command Line Parameter Parser.
 * f - filename of system configuration
 */
#define WMRT_CLPARAMS			"f:d"

#define WMRT_COMMSYSTERMTOTAL   2

/**
 * @brief signal processor function.
 * @param cmd - signal, required by Linux
 */
typedef int32_t sigprocessor();
typedef void *wmrt_threadfunc(void *arg);

/// Terminals for system usage
extern commterm_t wmrttu_localterm;
extern commterm_t wmrttu_localsysterm[WMRT_COMMSYSTERMTOTAL];

struct wmrt_ringbuf
{
    int32_t head;
    int32_t tail;
    int32_t size;
    uint8_t buf[];
};

struct wmrt_registry
{
    int current;
    int regsize;
    int items[];
};

#ifdef __cplusplus
extern C
{
#endif

/// Runtime Management

/**
 * @brief parse all command line parameters.
 * @param argc[I] - parameter count
 * @param argv[I] - parameters
 * @return success of execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_parsecmdline(int32_t argc, char **argv);

/**
 * @brief parse a single command line parameter.
 * @param opt[I] - cmd
 * @param arg[I] - parameter of command
 * @return success of execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_parseparam(int32_t opt, char *arg);

/**
 * @brief initialize the managerial runtime.
 *      Initialize all resources
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_init();

/**
 * @brief exit the managerial runtime.
 *      Release all resources
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_exit();

/// Functions for Configuration files
/**
 * @brief Load the system configuration file.
 *      Open the file; Load all config items; Close the file
 *      Configuration Items stored in memory during whole application
 *  lifecycle
 * @param filename[I] - of the configuration
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_loadsysconfig(const char *filename);

/**
 * @brief Load the user configuration file.
 *      Load all config items from the FILE stream
 *      Runtime keeps only 1 copy of user configuration info, and it
 *  does not last long. Users need to copy the info to their own place
 *      A END-mark is required at the end of the FILE stream
 * @param fp[I] - pointer of the input FILE stream
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_loadusrconfig(FILE *fp);

/**
 * @brief Save the user configuration to FILE stream.
 *      Save all user config items to the FILE stream
 *      A END-mark will be put at the end of the output
 * @param fp[I] - pointer of the output FILE stream
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_saveusrconfig(FILE *fp);

/**
 * @brief Output a value of string to specific stream
 * @param fp[I] - pointer of the output FILE stream
 * @param name[I] - name of the item
 * @param value[I] - value of the item
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t
wmrt_formatstritem(FILE* fp, const char *name, const char *value);

/**
 * @brief Output a value of int32 to specific stream
 * @param fp[I] - pointer of the output FILE stream
 * @param name[I] - name of the item
 * @param value[I] - value of the item
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_formatint32item(FILE* fp, const char *name, int32_t value);

/**
 * @brief Output a value of date and time to specific stream
 * @param fp[I] - pointer of the output FILE stream
 * @param name[I] - name of the item
 * @param value[I] - value of the item
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_formatdtitem(FILE* fp, const char *name,
        const struct datetime *value);

/**
 * @brief Output a END-mark to specific stream
 * @param fp[I] - pointer of the output FILE stream
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_completeconfig(FILE* fp);

/**
 * @brief find the specific config item in system config storage.
 *      All values are stored as a string, users need to
 *  convert the type by themselves
 * @param name[I] - name of the config item
 * @return pointer to the result string
 * @retval NULL - item was not found
 * @retval others - pointer to the result string
 */
extern char *wmrt_getsysconfig(const char *name);

/**
 * @brief find the specific config item in user config storage
 *      All values are stored as a string, users need to
 *  convert the type by themselves
 * @param name[I] - name of the config item
 * @return pointer to the result string
 * @retval NULL - item was not found
 * @retval others - pointer to the result string
 */
extern char *wmrt_getusrconfig(const char *name);

/// Functions for Queue Management. According to Linux IPC
/**
 * @brief request queue from managerial runtime
 * @param key - key to differentiate this queue from others
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_reqqueue(uint32_t key);

/**
 * @brief test the queue, check if there is pending message
 * @param qid - id of the queue
 * @return pending message count
 */
extern int32_t wmrt_testqueue(int32_t qid);

/**
 * @brief flush the queue, all existing messages will be lost
 * @param qid - id of the queue
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_fluqueue(int32_t qid);

/**
 * @brief release a block of shared memory
 * @param qid - id of the queue
 * @return void
 */
extern void wmrt_relqueue(int32_t qid);

/**
 * @brief enqueue a message
 * @param qid - id of the queue
 * @param message - message to enqueue
 * @param size - message size
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_enqueue(int32_t qid, const void *message, size_t size);

/**
 * @brief request queue from managerial runtime
 * @param qid - id of the queue
 * @param message - message to enqueue
 * @param size - message size
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_dequeue(int32_t qid, void *message, size_t size);

/// Functions for Memory Management. According to Linux IPC
/**
 * @brief request a block of shared memory
 * @param shmid[O] - id of the allocated shared memory block
 * @param key[I] - key to differentiate this block from others
 * @param size[I] - size of the block being requested
 * @return pointer to the shared memory block
 * @retval NULL - Failed to allocate
 * @retval others - pointer to the shared memory block
 */
extern void *wmrt_reqshm(int32_t *shmid, uint32_t key, size_t size);

/**
 * @brief release a block of shared memory
 * @param memaddr[I] - memory address of the allocated shared memory
 * @param shmid[I] - id of the allocated shared memory block
 * @return void
 */
extern void wmrt_relshm(void *memaddr, int32_t shmid);

/**
 * @brief check the owner ship of the shared memory block.
 * @param shmid[I] - id of the allocated shared memory block
 * @return ownship of the
 * @retval 0 - owned by myself
 * @revtal others - owner pid
 */
extern int32_t wmrt_isshmowner(int32_t shmid);

/// Functions for Process Management
/**
 * @brief start a process.
 * @param pathname[I] - the pathname of the binary to be started
 * @return process id
 * @retval -1 - errors
 */
extern int32_t wmrt_startproc(const char *pathname);

/**
 * @brief stop a process.
 * @param procid[I] - the process id to be stopped
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_stopproc(int32_t procid);

/**
 * @brief set process scheduler
 * @param pid - target process id
 * @param policy - scheduling policy
 * @param priority - scheduling priority
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_setprocschd(int32_t pid, int32_t policy, int32_t priority);

struct wmrt_tcpudpterm
{
    ipaddress_t ip;
    uint16_t port;
};

/**
 * @brief construct the communication terminal from ethernet information.
 * @param pterm[O] - pointer to the buffer to store the result
 * @param paddr[I] - IP address of the terminal
 * @param port[I] - port of the terminal
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_filltcpudpterm(commterm_t *pterm, char *paddr,
        uint16_t port);

/**
 * @brief ListenAt.
 * @param pterm[I] - terminal to be listen at
 * @return id of listen handler
 * @retval -1 - errors
 * @retval others - id of listen handler
 */
extern int32_t wmrt_tcplistenat(commterm_t *pterm);

/**
 * @brief Accept connection.
 * @param pclient_term[O] - terminal descriptor of accepted client
 * @param listen_id[I] - id of listen handler
 * @return id of communication handler
 * @retval -1 - errors
 * @retval others - id of communication handler
 */
extern int32_t wmrt_tcpacceptconn(commterm_t *client_term, int32_t listen_id);

/**
 * @brief Connect to target terminal.
 * @param pterm[I] - terminal descriptor of target server
 * @return id of communication handler
 * @retval -1 - errors
 * @retval others - id of communication handler
 */
extern int32_t wmrt_tcpconnectto(commterm_t *pterm);

/**
 * @brief Disconnect the communication handler
 * @param id[I] - id of communication handler
 * @return success of execution
 * @retval -1 - errors
 * @retval 0 - success
 */
extern void wmrt_tcpdisconnect(int32_t id);

/**
 * @brief Write data to specific communication handler.
 * @param id[I] - id of communication handler
 * @param pdata[I] - pointer to the data buffer
 * @param len[I] - size to write
 * @return size wrote
 * @retval -1 - errors
 * @retval others - size wrote
 */
extern int32_t wmrt_tcpwritedata(int32_t id, const void *pdata, size_t len);

/**
 * @brief Read data from specific communication handler.
 * @param id[I] - id of communication handler
 * @param pdata[I] - pointer to the data buffer
 * @param len[I] - size to write
 * @return size wrote
 * @retval -1 - errors
 * @retval others - size wrote
 */
extern int32_t wmrt_tcpreaddata(int32_t id, void *pdata, size_t len);

/**
 * @brief initialize management channel on specific terminal
 * @param term[I] - terminal for management communication
 * @return id of communication handler
 * @retval -1 - errors
 * @retval others - id of communication handler
 */
extern int32_t wmrt_udpinitreceiver(commterm_t *pterm);

/**
 * @brief send a message to specific peer terminal
 * @param content[I] - message body
 * @param size[I] - message size
 * @param dstterm[I] - destination terminal
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_udppostmsg(const void *content, size_t size,
        commterm_t *dstterm);

/**
 * @brief recv a message from command port
 * @param content[O] - place to store the message
 * @param size[I/O] - I:size expected, O:size received
 * @param fromterm[O] - originating terminal, ignore if it is NULL
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_udprecvmsg(int32_t rid, void *content, size_t *size,
        commterm_t *fromterm);

/**
 * @brief exchange a message with specific peer terminal
 *      Send a message, and receieve a message from the same terminal
 *      The new message overrides the original one
 * @param content[I/O] - place to store the message
 * @param size[I/O] - I:size expected, O:size received
 * @param dstterm[I] - destination terminal
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_udpexchangemsg(void *content, size_t *size,
        commterm_t *dstterm, int32_t rid);
extern int32_t wmrt_udptryrecvmsg(int32_t rid, void *content, size_t *size,
        commterm_t *fromterm);
extern int32_t wmrt_udpsendmsg(int32_t id, const void *data, size_t datalen);
extern int32_t wmrt_udpconnectto(commterm_t *dstterm);
extern void wmrt_udpdisconnect(int32_t id);

/**
 * @brief set user exit-function.
 *  Signal processor answers:
 *      SIGQUIT
 *      SIGTERM
 *      SIGINT
 * @param proc[I] - pointer to user functions
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_regusrexitfunc(sigprocessor *proc);

/**
 * @brief set system exit-function.
 *  Signal processor answers:
 *      SIGQUIT
 *      SIGTERM
 *      SIGINT
 * @param proc[I] - pointer to user functions
 * @return success of the execution
 * @retval 0 - success
 * @retval -1 - errors
 */
extern int32_t wmrt_setsysexitfunc(sigprocessor *proc);

/**
 * @brief daemonize the process
 * @return success of the execution
 * @retval 0 - success
 * @retval 1 - failed
 * */
extern int32_t wmrt_daemonize();

extern int32_t wmrt_forkproc();

extern int32_t wmrt_waitproc(int *status);

extern int32_t wmrt_sleep(int32_t seconds, int32_t mseconds);

extern int32_t wmrt_killproc(int32_t pid);

extern int32_t wmrt_startthread(wmrt_threadfunc *func, void *arg);

extern int32_t wmrt_killthread(int32_t pid);

extern int32_t wmrt_waitthread(int32_t tid, void *retp);

extern struct wmrt_registry* wmrt_initreg(int32_t size);
extern int32_t wmrt_freereg(struct wmrt_registry *reg);

extern int32_t wmrt_allocregitem(struct wmrt_registry *reg);
extern int32_t wmrt_relregitem(struct wmrt_registry *reg, int32_t id);
extern int32_t wmrt_itemallocated(struct wmrt_registry* reg, int32_t id);

#define wmrt_buflength(rbuf)    ((rbuf->tail + rbuf->size - rbuf->head) % rbuf->size)
extern struct wmrt_ringbuf* wmrt_initrbuf(int32_t size);
extern int32_t wmrt_freerbuf(struct wmrt_ringbuf *rbuf);
extern int32_t wmrt_storetorbuf(struct wmrt_ringbuf *rbuf, uint8_t *content,
        int32_t size);

extern int32_t wmrt_migudpinitprimary(commterm_t *term);
extern int32_t wmrt_migudpinitside(int32_t migid, commterm_t *term);
extern int32_t wmrt_migudpconnectprimary(commterm_t *term);
extern int32_t wmrt_migudpconnectside(int32_t migid, commterm_t *term);
extern void wmrt_migudpdisconnectprimary(int32_t migid);
extern void wmrt_migudpdisconnectside(int32_t migid);
extern int32_t wmrt_migudprecv(int32_t migid, void *content, size_t size);
extern int32_t wmrt_migudpsend(int32_t migid, void *content, size_t size);

extern int32_t wmrt_migtcpinitprimary(commterm_t *term);
extern int32_t wmrt_migtcpinitside(int32_t migid, commterm_t *term);
extern int32_t wmrt_migtcpconnectprimary(commterm_t *term);
extern int32_t wmrt_migtcpconnectside(int32_t migid, commterm_t *term);
extern void wmrt_migtcpdisconnectprimary(int32_t migid);
extern void wmrt_migtcpdisconnectside(int32_t migid);
extern int32_t wmrt_migtcprecv(int32_t migid, void *content, size_t size);
extern int32_t wmrt_migtcpsend(int32_t migid, void *content, size_t size);

#ifdef __cplusplus
}
#endif

#endif
