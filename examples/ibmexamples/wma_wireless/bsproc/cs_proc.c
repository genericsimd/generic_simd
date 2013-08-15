/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: cs_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>

#include "hash_table.h"
#include "addr_con_inter.h"

#include "cs_proc.h"

#include "bs_cfg.h"
#include "queue_util.h"
#include "flog.h"

#include "memmgmt.h"
#include "CS.h"
#include "ul_cs_consumer.h"
#include "mac.h"

#include "dl_exp_params.h"

#include "metric_proc.h"

//struct hash_table * gp_hash_table = NULL;
void * gp_hash_table = NULL;

static struct cs_global_param * gp_cs_param = NULL;

int tap_fd = 0;

pthread_t pkt_forward_thd = 0;
pthread_t pkt_classify_thd = 0;

void *process_pkt_forward (void *arg __attribute__ ((unused)));
void *process_pkt_classify (void *arg __attribute__ ((unused)));
int connect_tap (char *if_name, int flags);


int connect_tap (char *if_name, int flags)
{
    struct ifreq ifr;
    int fd;
    int ret;
    char *dev = "/dev/net/tun";

    if ( ( fd = open (dev, O_RDWR) ) < 0)
    {
        FLOG_ERROR ("Open /dev/net/tun falied");
        return fd;
    }

    memset (&ifr, 0, sizeof ( ifr ));

    ifr.ifr_flags = flags;

    if (*dev)
    {
        strncpy (ifr.ifr_name, if_name, IFNAMSIZ);
    }

    if ( ( ret = ioctl (fd, TUNSETIFF, (void *) &ifr) ) < 0)
    {
        FLOG_ERROR ("ioctl(TUNSETIFF) failed");
        close (fd);
        return ret;
    }

    strcpy (if_name, ifr.ifr_name);

    return fd;
}

int init_cs_table (void)
{
    int ret = 0;
    char tmp_s[128];
    int bbu_id = 0;

    ret = get_global_param ("BBU_SERVER_ID", & ( bbu_id ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_SERVER_ID error\n");
    }

    ret = init_addr_con_info ((void **) ( &gp_hash_table ));

    if (ret != 0)
    {
        FLOG_ERROR ("Init CS Table failed");

        return 1;
    }

    sprintf (tmp_s, "%p", gp_hash_table);

    ret = set_global_param ("CS_HASH_TABLE", (void *) tmp_s);

    if (ret != 0)
    {
        FLOG_ERROR ("set cs hash table error");
        return 1;
    }

    gp_cs_param = (struct cs_global_param *) malloc (
            sizeof(struct cs_global_param));

    if (gp_cs_param == NULL)
    {
        FLOG_ERROR ("malloc cs_param error");
        return 1;
    }

    ret = get_global_param ("BS_IFDEV_NAME", (void *) gp_cs_param->if_name);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BS_IFDEV_NAME error\n");
    }

//    sprintf(gp_cs_param->if_name, "tap%d", bbu_id);

    ret = get_global_param ("BS_IP_SUBNET", (void *) gp_cs_param->wma_subnet);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BS_IP_SUBNET error\n");
    }

    /*
     ret = get_global_param ("CS_HASH_TABLE", (void *)tmp_s);

     if (ret != 0)
     {
     FLOG_WARNING ("get parameters CS_HASH_TABLE error\n");
     }

     void * p = NULL;
     sscanf(tmp_s, "%p", &p);
     printf("%p, %p\n", p, gp_hash_table);
     */

#ifdef _IP_ENABEL_
    if ( ( tap_fd = connect_tap (gp_cs_param->if_name, DEV_TYPE | IFF_NO_PI) )
            < 0)
    {
        FLOG_ERROR ("Connect to tap interface %s failed!\n",
                gp_cs_param->if_name);

        return 1;
    }
#endif

    return 0;
}

int release_cs_table (void)
{
    int ret = 0;

    if (gp_cs_param != NULL)
    {
        free (gp_cs_param);
    }

    ret = del_addr_con_info ((void **) ( &gp_hash_table ));

    if (ret != 0)
    {
        FLOG_ERROR ("Release CS Table failed");

        return 1;
    }

    if (tap_fd != 0)
    {
        close(tap_fd);
    }

    return 0;
}

int pkt_classify_process (void)
{
    pthread_attr_t tattr;

    //    cpu_set_t cpuset;

    pthread_attr_init (&tattr);
    /*
     __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
     __CPU_SET_S(0, sizeof (cpu_set_t), &cpuset);
     pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
     */
    pthread_create (&pkt_classify_thd, NULL, process_pkt_classify, NULL);
    pthread_attr_destroy (&tattr);

    return 0;
}

int pkt_classify_release (void)
{
    if (pkt_classify_thd != 0)
    {
        pthread_cancel (pkt_classify_thd);
        pthread_join (pkt_classify_thd, NULL);
    }

    return 0;
}

int pkt_forward_process (void)
{
    pthread_attr_t tattr;
    //    cpu_set_t cpuset;

    pthread_attr_init (&tattr);
    /*
     __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
     __CPU_SET_S(0, sizeof (cpu_set_t), &cpuset);
     pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
     */
    pthread_create (&pkt_forward_thd, NULL, process_pkt_forward, NULL);
    pthread_attr_destroy (&tattr);

    return 0;

}

int pkt_forward_release (void)
{
    FLOG_INFO ("RRH Rx thread Release");

    if (pkt_forward_thd != 0)
    {
        pthread_cancel (pkt_forward_thd);
        pthread_join (pkt_forward_thd, NULL);
    }

    return 0;
}

void *process_pkt_forward (void *arg __attribute__ ((unused)))
{
    FLOG_INFO ("Pkt forward process Started");

    struct queue_msg cs_msg;
    sdu_queue* ul_sduq = NULL;
    int cid = 0;
    char tmp_buf[LINE_MTU_SIZE];
    int buf_count = 0;
    int ret = 0;
    sdu_cid_queue* sq;
    logical_packet* sdu;
    logical_packet* next_sdu;
    logical_element* le;
    logical_element* next_le;

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
        return NULL;
    }

    get_sduq (&ul_sduq, 0);

    cs_msg.my_type = csforw_de_id;

    while (1)
    {
        if (wmrt_dequeue (csforw_de_id, &cs_msg, sizeof(struct queue_msg))
                == -1)
        {
            FLOG_ERROR ("DEQUEUE ERROR\n");
        }

        buf_count = 0;
        cid = (int) cs_msg.len;

        sq = ul_sduq->sdu_cid_q[cid];

        if (sq == NULL)
        {
            continue;
        }
        pthread_mutex_lock (& ( sq->qmutex ));
        sdu = sq->head;

        while (sdu)
        {
            next_sdu = sdu->next;

            le = sdu->element_head;

            while (le)
            {
                next_le = le->next;

                if (le->length != 0)
                {
                    memcpy ( tmp_buf + buf_count, le->data,
                            le->length );
                    buf_count += le->length;
                }

                if (le->data != NULL)
                {
                    free (le->data);
                }

                free (le);
                le = next_le;
            }

            ul_total_bytes += (float)buf_count;

#ifdef _IP_ENABEL_
	    FLOG_DEBUG("R: %d\n", buf_count);
            ret = write(tap_fd, tmp_buf, buf_count);

            if (ret != buf_count)
            {
                FLOG_WARNING("Write to TAP failed");
            }
#endif
            buf_count = 0;
            free (sdu);
            sdu = next_sdu;
        }

        sq->head = NULL;
        sq->sdu_num = 0;
        sq->tail = NULL;

        pthread_mutex_unlock (& ( sq->qmutex ));
    }

    (void)ret;

    return NULL;
}

void *process_pkt_classify (void *arg __attribute__ ((unused)))
{
    int ret = 0;
    fd_set rd_set;
    short class_type = 0;
    char tmp_buf[LINE_MTU_SIZE];
    char * p_buf = NULL;
    char * p_sdu = NULL;
    int len = 0;
    int cid = 0;

    struct ethhdr *p_eth;
    struct iphdr *p_ip;
    struct tcphdr *p_tcp;
    struct udphdr *p_udp;
    char search_key[128];
    char ss[32];
    char dd[32];
    int sp;
    int dp;

    FLOG_INFO ("Pkt classify process Started");

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
        return NULL;
    }

    (void) p_tcp;
    (void) p_udp;
    (void) ss;
    (void) sp;
    (void) dp;

    FD_ZERO (&rd_set);
    FD_SET (tap_fd, &rd_set);

    while (1)
    {
        ret = select (tap_fd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0)
        {
            FLOG_WARNING ("select TAP error\n");
            continue;
        }

        if (FD_ISSET (tap_fd, &rd_set))
        {
            len = read (tap_fd, tmp_buf, LINE_MTU_SIZE);
            if (ret <= 0)
            {
                FLOG_WARNING ("Reading data error");
                continue;
            }

            p_buf = tmp_buf;
            p_eth = (struct ethhdr *) p_buf;

            p_buf += sizeof(struct ethhdr);

            p_ip = (struct iphdr *) p_buf;

            p_buf += sizeof(struct iphdr);

            strcpy (dd, inet_ntoa (*(struct in_addr*) & ( p_ip->daddr )));
            /*
             strcpy(ss, inet_ntoa(*(struct in_addr*)&(p_ip->saddr)));
             switch(p_ip->protocol)
             {
             case IPPROTO_UDP:
             p_udp = (struct udphdr *)p_buf;
             sp = ntohs(p_udp->source);
             dp = ntohs(p_udp->dest);
             break;

             case IPPROTO_TCP:
             p_tcp = (struct tcphdr *)p_buf;
             sp = ntohs (p_tcp.source);
             dp = ntohs (p_tcp.dest);
             break;
             default:
             break;
             }
             */

#if 0
            sprintf (search_key, "%02x%02x%02x%02x%02x%02x%s",
                    p_eth->h_dest[0], p_eth->h_dest[1], p_eth->h_dest[2],
                    p_eth->h_dest[3], p_eth->h_dest[4], p_eth->h_dest[5], dd);

            cid = get_addr_con_id (gp_hash_table, search_key);

            if (ret != 0)
            {
                p_sdu = (char *) mac_sdu_malloc (len, class_type);
                memcpy (p_sdu, tmp_buf, len);

                enqueue_transport_sdu_queue (dl_sdu_queue, cid, len, p_sdu);

                continue;
            }

            sprintf (search_key, "%02x%02x%02x%02x%02x%02x", p_eth->h_dest[0],
                    p_eth->h_dest[1], p_eth->h_dest[2], p_eth->h_dest[3],
                    p_eth->h_dest[4], p_eth->h_dest[5]);

            cid = get_addr_con_id (gp_hash_table, search_key);

            if (ret != 0)
            {
                p_sdu = (char *) mac_sdu_malloc (len, class_type);
                memcpy (p_sdu, tmp_buf, len);

                enqueue_transport_sdu_queue (dl_sdu_queue, cid, len, p_sdu);

                dl_total_bytes += (float)len;

                continue;
            }
#else
            memset (search_key, 0, 128);
#ifdef MULTIPLE_CPE
            sprintf (search_key, "%02x:%02x:%02x:%02x:%02x:%02x",
                    p_eth->h_dest[0], p_eth->h_dest[1], p_eth->h_dest[2],
                    p_eth->h_dest[3], p_eth->h_dest[4], p_eth->h_dest[5]);

//            printf("in the cs part %s\n", search_key);
#else
            sprintf(search_key, "%d", param_MAX_VALID_BASIC_CID);
#endif
            cid = get_addr_con_id (gp_hash_table, search_key);
	    FLOG_DEBUG("[%d]: %d\n", cid, len);

            if (cid != 0)
            {
                p_sdu = (char *) mac_sdu_malloc (len, class_type);
                memcpy (p_sdu, tmp_buf, len);

                ret = enqueue_transport_sdu_queue (dl_sdu_queue, cid, len, p_sdu);

                if (ret != 0)
                {
                    mac_sdu_free(p_sdu, len, 0);
                }

                dl_total_bytes += (float)len;
            }
#endif
        }
    }

    return NULL;
}
