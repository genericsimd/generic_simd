#ifndef _WNT_FRAMEWORK_KERNEL_QUEUE_H_INC_
#define _WNT_FRAMEWORK_KERNEL_QUEUE_H_INC_

#include <stdint.h>
#include <sys/ipc.h>

#define WNTQF_BLOCKING				0
#define WNTQF_NONBLOCKING			IPC_NOWAIT

#define WNTQ_GENMTYPE(qid)			(0l + qid)

// All Queue messages need to be defined as follow
typedef struct stru_wntcommon_command_header
{
	long mtype;

	char mbody[0];
}WNTM_COMMONHEADER;

#ifdef __cplusplus
extern C {
#endif

// queue related
extern int wnt_initqueue(int id);
extern int wnt_closequeue(int qid);
extern int wnt_attachqueue(int id);
extern int wnt_detachqueue(int qid);
extern int wnt_enqueue(
		int qid, const void *msgp, size_t msgsz, uint32_t msgflg);
extern int wnt_dequeue(
		int qid, void *msgp, size_t msgsz, uint32_t msgflg);

#ifdef __cplusplus
}
#endif

#endif
