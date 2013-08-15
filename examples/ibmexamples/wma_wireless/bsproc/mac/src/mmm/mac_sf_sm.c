/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_sf_sm.c

   Change Activity:

   Date             Description of Change                               By
   -----------      ---------------------           --------
   1-Mar.2011       Created                                             Parul Gupta
   30-Jan.2012  Modified to make it support dsc         Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_sf_sm.h"
#include "debug.h"
#include "thread_sync.h"

#include "mac_sf_api.h"
#include "mac_dsd_processing.h"

#include "mac_dsc_sm.h"

/*
  * find_sf_in_peer - find service flow in peer device
  * @sf_id: service flow id
  * @peer_mac: peer's mac address
  *
  * The API is used to find service flow in peer device
  *
  * Return:
  *     service flow if successful
  *     NULL if no such service flow failed
  */
struct service_flow * find_sf_in_peer
(
    int         sf_id, 
    u_int64_t   peer_mac
) 
{
    struct service_flow     *sf_flow;
    bs_ss_info              *ss_info;

    sf_flow = NULL;
    
    ss_info = find_bs_ss_info(peer_mac);
    if (ss_info != NULL) 
    {
        sf_flow = ss_info->sf_list_head;
        while (sf_flow != NULL)
        {
            if (sf_flow->sfid == sf_id)
            {
                break;
            }
            else
            {
                sf_flow = sf_flow->next;
            }
        }
    }

    return sf_flow;
}

extern pthread_mutex_t bs_ss_list_mutex;
void release_ss_sfs(bs_ss_info *ss_info)
{
    serviceflow *curr_flow;
    
    if (ss_info != NULL)
    {
        while (1)
        {
            pthread_mutex_lock(&bs_ss_list_mutex);
            curr_flow = ss_info->sf_list_head;
            if (curr_flow != NULL)
            {
                ss_info->sf_list_head = curr_flow->next;
            }
            pthread_mutex_unlock(&bs_ss_list_mutex);

            if (curr_flow != NULL)
            {
                FLOG_INFO("++++++++ %s: release sf: sf_id = %d, cid = %d ++++++++\n", __FUNCTION__, curr_flow->sfid, curr_flow->cid);

                pthread_rwlock_wrlock(&conn_info_rw_lock);
                delete_connection(curr_flow->cid);
                if (curr_flow->service_class_name != NULL)
                {
                    free(curr_flow->service_class_name);
                }
                free(curr_flow);
                pthread_rwlock_unlock(&conn_info_rw_lock);
            }
            else
            {
                break;
            }
        }
    }
}

/*
  * dsa_req_handler - to process the dsa request
  * @mm: management message which contains dsa request
  *
  * The API is used to to process the dsa request, which is from the peer
  *
  * Return:
  *     none
  */
static void dsa_req_handler(mgt_msg *mm)
{
    dsa_req_msg                 *dsa_req;
    struct transaction_node     *trans_node;
    struct service_flow         *sf_node;
    int                         ret;
    bs_ss_info                  *ss_info;

    assert(mm != NULL);

    dsa_req = (dsa_req_msg *)malloc(sizeof(dsa_req_msg));
    if (NULL != dsa_req)
    {
        ret = parse_dsa_req (mm->data, mm->length, dsa_req);
        if (ret == 0)
        {
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                /* find if transaction node already exist*/
                trans_node = get_trans_node(&dsa_trans_list, ss_info->mac_addr, dsa_req->trans_id);

                /* if doesn't exist, create new transaction */
                if (trans_node == NULL) 
                {
                    sf_node = (struct service_flow *)malloc(sizeof(struct service_flow));
                    if (sf_node != NULL)
                    {
                        /* fill the temporary service flow using dsa request */
                        dsa_req_to_sf(dsa_req, sf_node);

                        /* create a new transaction node and initialize its members */
                        trans_node = alloc_trans_node();
                        if (trans_node != NULL)
                        {
                            trans_node->sf = sf_node;
                            trans_node->trans_id = dsa_req->trans_id;

                            /* set peer address and notify function */
                            trans_node->initiator_mac = ss_info->mac_addr;
                            trans_node->peer_mac = ss_info->mac_addr;
                            trans_node->peer_primary_cid = mm->cid;
                            trans_node->trans_status = DSA_BEGIN;
                            trans_node->notify = NULL;

                            add_trans_node(&dsa_trans_list, trans_node);

                            ret = dsa_msg_handler(dsa_req, DSA_REQ, trans_node);
                            if (ret != 0)
                            {
                                delete_trans_node(&dsa_trans_list, trans_node);
                            }
                        }
                        else
                        {
                            free(sf_node);
                        }
                    }
                }
                else
                {
                    dsa_msg_handler(dsa_req, DSA_REQ, trans_node);
                    put_trans_node(trans_node);
                }
            }
        }
        free_dsa_req(dsa_req);
    }

    free(mm->data);
}

/*
  * dsa_rsp_handler - to process the dsa response
  * @mm: management message which contains dsa response
  *
  * The API is used to to process the dsa response, which is from the peer
  *
  * Return:
  *     none
  */
static void dsa_rsp_handler(mgt_msg *mm)
{
    dsa_rsp_msg                 *dsa_rsp;
    struct transaction_node     *trans_node;
    bs_ss_info                  *ss_info;
    int                         ret;
    u_int64_t                   own_mac_addr;

    assert(mm != NULL);
    
    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                    ((u_int64_t)param_MY_MAC[1]) << 32U | \
                    ((u_int64_t)param_MY_MAC[2]) << 24U | \
                    ((u_int64_t)param_MY_MAC[3]) << 16U | \
                    ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                    ((u_int64_t)param_MY_MAC[5]) << 0U;
    
    dsa_rsp = (dsa_rsp_msg*)malloc(sizeof(dsa_rsp_msg));
    if (dsa_rsp != NULL)
    {
        ret = parse_dsa_rsp(mm->data, mm->length, dsa_rsp);
        if (ret == 0)
        {
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                /* find the transaction node for this trans_id */
                trans_node = get_trans_node(&dsa_trans_list, own_mac_addr, dsa_rsp->trans_id);
                if (trans_node != NULL)
                {
                    /* call the DSA transaction state machine */
                    ret = dsa_msg_handler(dsa_rsp, DSA_RSP, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: dsa response can't be handled\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
                else
                {
                    FLOG_INFO("DSA_RSP received for non-existing transaction %d  Discarding\n", dsa_rsp->trans_id);
                }
            }
        }
        free_dsa_rsp(dsa_rsp);
    }

    free(mm->data);
}

/*
  * dsx_rvd_handler - to process the dsx received message
  * @mm: management message which contains dsx received message
  *
  * The API is used to to process the dsx received message, which is from BS
  *
  * Return:
  *     none
  */
static void dsx_rvd_handler(mgt_msg *mm)
{
    dsx_rvd_msg                 dsx_rvd;
    struct transaction_node     *trans_node;
    int                         ret;
    bs_ss_info                  *ss_info;
    u_int64_t                   own_mac_addr;

    assert(mm != NULL);
    
    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                    ((u_int64_t)param_MY_MAC[1]) << 32U | \
                    ((u_int64_t)param_MY_MAC[2]) << 24U | \
                    ((u_int64_t)param_MY_MAC[3]) << 16U | \
                    ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                    ((u_int64_t)param_MY_MAC[5]) << 0U;
    
    ret = parse_dsx_rvd(mm->data, mm->length, &dsx_rvd);
    
    if (ret == 0)   
    {
        ss_info = get_ssinfo_from_pcid(mm->cid);
        if (ss_info != NULL)
        {
            /* find the transaction id */
            trans_node = get_trans_node(&dsa_trans_list, own_mac_addr, dsx_rvd.trans_id);
            if (trans_node != NULL)
            {
                /* call the DSA transaction state machine */
                ret = dsa_msg_handler(&dsx_rvd, DSX_RVD, trans_node);
                if (ret != 0)
                {
                    FLOG_INFO("%s: error in dsa_msg_handler\n", __FUNCTION__);
                }
                put_trans_node(trans_node);
            }
            else
            {
                trans_node = get_trans_node(&dsc_trans_list, own_mac_addr, dsx_rvd.trans_id);
                if (trans_node != NULL)
                {
                    /* call the DSC transaction state machine */
                    ret = dsc_msg_handler(&dsx_rvd, DSX_RVD, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
                else
                {
                    FLOG_INFO("%s: dsx-rvd message has no correspondent \n", __FUNCTION__);
                }
            }
        }
    }

    free(mm->data);
}

/*
  * dsa_ack_handler - to process the dsa acknowledge
  * @mm: management message which contains dsa acknowledge
  *
  * The API is used to to process the dsa acknowledge, which is from the peer
  *
  * Return:
  *     none
  */
static void dsa_ack_handler(mgt_msg *mm)
{
    dsa_ack_msg                 *dsa_ack;
    struct transaction_node     *trans_node;    
    bs_ss_info                  *ss_info;
    int                         ret;

    assert(mm != NULL);
    
    dsa_ack = (dsa_ack_msg*)malloc(sizeof(dsa_ack_msg));
    if (dsa_ack != NULL)
    {
        ret = parse_dsa_ack(mm->data, mm->length, dsa_ack);
        if (ret == 0)
        {
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                /* find the transaction node for this trans id */
                trans_node = get_trans_node(&dsa_trans_list, ss_info->mac_addr, dsa_ack->trans_id);
                if (trans_node != NULL)
                {
                    /* call the DSA transaction state machine */
                    ret = dsa_msg_handler(dsa_ack, DSA_ACK, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsa_msg_handler", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
                else
                {
                    FLOG_INFO("%s: no transaction node found for ID: %d", __FUNCTION__, dsa_ack->trans_id);
                }
            }
        }

        free_dsa_ack(dsa_ack);
    }

    free(mm->data);
}

/*
  * dsc_req_handler - to process the dsc request
  * @mm: management message which contains dsc request
  *
  * The API is used to to process the dsc request, which is from the peer
  *
  * Return:
  *     none
  */
static void dsc_req_handler(mgt_msg *mm)
{
    dsc_req_msg                 *dsc_req;
    struct transaction_node     *trans_node;
    bs_ss_info                  *ss_info;
    struct service_flow         *old_sf_node;
    struct service_flow         *new_sf_node;
    int                         ret;
    
    assert(mm != NULL);

    dsc_req = (dsc_req_msg *)malloc(sizeof(dsc_req_msg));
    if (dsc_req != NULL)
    {
        memset(dsc_req, 0U, sizeof(dsc_req_msg));
        ret = parse_dsc_req(mm->data, mm->length, dsc_req);
        if (ret == 0)
        {           
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                trans_node = get_trans_node(&dsc_trans_list, ss_info->mac_addr, dsc_req->trans_id);
        
                /* if doesn't exist, create a new transaction node */
                if (trans_node == NULL) 
                {
                    old_sf_node = find_sf_in_peer(dsc_req->sf_id, ss_info->mac_addr);
                    if (old_sf_node != NULL)
                    {
                        trans_node = alloc_trans_node();
                        new_sf_node = (struct service_flow *)malloc(sizeof(struct service_flow));
                        if ((trans_node != NULL) && (new_sf_node != NULL))
                        {
                            trans_node->trans_id = dsc_req->trans_id;
                            trans_node->initiator_mac = ss_info->mac_addr;
                            trans_node->peer_mac = ss_info->mac_addr;
                            trans_node->peer_primary_cid = mm->cid;
                            trans_node->trans_status = DSC_BEGIN;
                            dsc_req_to_sf(dsc_req, new_sf_node);
                            trans_node->sf = new_sf_node;
            
                            add_trans_node(&dsc_trans_list, trans_node);
                            ret = dsc_msg_handler(dsc_req, DSC_REQ, trans_node);
                            if (ret != 0)
                            {
                                FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                            }
                        }
                        else
                        {
                            put_trans_node(trans_node);
                            free(new_sf_node);
                        }
                    }
                    else
                    {
                        FLOG_INFO("Can't find service flow %d", dsc_req->sf_id);
                    }
                }
                else
                {
                    ret = dsc_msg_handler(dsc_req, DSC_REQ, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
            }
            else
            {
                FLOG_INFO("Error in %s: can't parse dsc request correctly", __FUNCTION__);
            }
        }
        free_dsc_req(dsc_req);
    }

    free(mm->data);
}

/*
  * dsc_rsp_handler - to process the dsc response
  * @mm: management message which contains dsc response
  *
  * The API is used to to process the dsc response, which is from the peer
  *
  * Return:
  *     none
  */
static void dsc_rsp_handler(mgt_msg *mm)
{
    dsc_rsp_msg                 *dsc_rsp;
    struct transaction_node     *trans_node;
    int                         ret;
    bs_ss_info                  *ss_info;
    u_int64_t                   own_mac_addr;

    assert(mm != NULL);
    
    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                    ((u_int64_t)param_MY_MAC[1]) << 32U | \
                    ((u_int64_t)param_MY_MAC[2]) << 24U | \
                    ((u_int64_t)param_MY_MAC[3]) << 16U | \
                    ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                    ((u_int64_t)param_MY_MAC[5]) << 0U;

    dsc_rsp = (dsc_rsp_msg*)malloc(sizeof(dsc_rsp_msg));
    if (dsc_rsp != NULL)
    {
        ret = parse_dsc_rsp(mm->data, mm->length, dsc_rsp);
        if (ret == 0)
        {
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                trans_node = get_trans_node(&dsc_trans_list, own_mac_addr, dsc_rsp->trans_id);
                if (trans_node != NULL)
                {
                    /* call the DSC transaction state machine */
                    ret = dsc_msg_handler(dsc_rsp, DSC_RSP, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
                else
                {
                    FLOG_INFO("DSA_RSP received for non-existing transaction %d Discarding\n", dsc_rsp->trans_id);
                }
            }
            else
            {
                FLOG_INFO("DSA_RSP received for non-existing ss: %d, Discarding\n", mm->cid);
            }
        }
        free_dsc_rsp(dsc_rsp);
    }

    free(mm->data);
}

/*
  * dsc_ack_handler - to process the dsc acknowledge
  * @mm: management message which contains dsc acknowledge
  *
  * The API is used to to process the dsc acknowledge, which is from the peer
  *
  * Return:
  *     none
  */
static void dsc_ack_handler(mgt_msg *mm)
{
    dsc_ack_msg                 *dsc_ack;
    struct transaction_node     *trans_node;
    int                         ret;
    bs_ss_info                  *ss_info;

    dsc_ack = (dsc_ack_msg*)malloc(sizeof(dsc_ack_msg));
    if (dsc_ack != NULL)
    {
        memset(dsc_ack, 0, sizeof(dsc_ack_msg));
        ret = parse_dsc_ack(mm->data, mm->length, dsc_ack);

        if (ret == 0)
        {
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                /* find the txn_node for this trans_id */
                trans_node = get_trans_node(&dsc_trans_list, ss_info->mac_addr, dsc_ack->trans_id);
                if (trans_node != NULL)
                {
                    /* call the DSA transaction state machine */
                    ret = dsc_msg_handler(dsc_ack, DSC_ACK, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
                else
                {
                    FLOG_INFO("Error in dsc_ack_handler: No transaction node found for ID: %d", dsc_ack->trans_id);
                }
            }
            else
            {
                FLOG_INFO("error in parse_dsc_ack: No ss node found for ID: %d", dsc_ack->trans_id);
            }
        }
        else
        {
            FLOG_INFO("error in parse_dsc_ack\n");
        }
        free_dsc_ack(dsc_ack);
    }

    free(mm->data);
}

/*
  * dsd_req_handler - to process the dsd request
  * @mm: management message which contains dsd request
  *
  * The API is used to to process the dsd request, which is from the peer
  *
  * Return:
  *     none
  */
static void dsd_req_handler(mgt_msg *mm)
{
    char                        *temp_var;
    int                         sfid;
    struct transaction_node     *trans_node;
    dsd_req_msg                 *dsd_req;
    bs_ss_info                  *ss_info;
    struct service_flow         *sf_node;
    int                         ret;

    assert(mm != NULL);
    
    temp_var = (char*)mm->data;
    sfid = (temp_var[3] << 24)+ (temp_var[4] << 16) + (temp_var[5] << 8) + (temp_var[6]);

    FLOG_DEBUG("MAC SF SM: Received DSD REQ message for SFID %d\n",sfid);
    dsd_req = (dsd_req_msg *)malloc(sizeof(dsd_req_msg));
    if (dsd_req != NULL)
    {
        memset(dsd_req, 0U, sizeof(dsd_req_msg));
        ret = parse_dsd_req(mm->data, mm->length, dsd_req);
        if (ret == 0)
        {           
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                trans_node = get_trans_node(&dsd_trans_list, ss_info->mac_addr, dsd_req->trans_id);
            
                /* if doesn't exist, create a new transaction node */
                if (trans_node == NULL) 
                {
                    sf_node = find_sf_in_peer(dsd_req->sfid, ss_info->mac_addr);
                    if (sf_node != NULL)
                    {
                        trans_node = alloc_trans_node();
                        if (trans_node != NULL)
                        {
                            trans_node->trans_id = dsd_req->trans_id;
                            trans_node->initiator_mac = ss_info->mac_addr;
                            trans_node->peer_mac = ss_info->mac_addr;
                            trans_node->peer_primary_cid = mm->cid;
                            trans_node->trans_status = DSD_BEGIN;
                            trans_node->sf = sf_node;
                
                            add_trans_node(&dsd_trans_list, trans_node);
                            ret = dsd_msg_handler(dsd_req, DSD_REQ, trans_node);
                            if (ret != 0)
                            {
                                FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                                delete_trans_node(&dsd_trans_list, trans_node);
                            }
                        }
                    }
                }
                else
                {
                    ret = dsd_msg_handler(dsd_req, DSD_REQ, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsc_msg_handler\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
            }
        }
        free(dsd_req);
    }

    free(mm->data);
}

/*
  * dsd_rsp_handler - to process the dsd response
  * @mm: management message which contains dsd response
  *
  * The API is used to to process the dsd response, which is from the peer
  *
  * Return:
  *     none
  */
static void dsd_rsp_handler(mgt_msg *mm)
{
    dsd_rsp_msg                 *dsd_rsp;
    struct transaction_node     *trans_node;
    int                         ret;
    bs_ss_info                  *ss_info;
    u_int64_t                   own_mac_addr;

    assert(mm != NULL);
    
    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                    ((u_int64_t)param_MY_MAC[1]) << 32U | \
                    ((u_int64_t)param_MY_MAC[2]) << 24U | \
                    ((u_int64_t)param_MY_MAC[3]) << 16U | \
                    ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                    ((u_int64_t)param_MY_MAC[5]) << 0U;

    dsd_rsp = (dsd_rsp_msg *)malloc(sizeof(dsd_rsp_msg));
    if (dsd_rsp != NULL)
    {
        ret = parse_dsd_rsp((unsigned char *)mm->data, mm->length, dsd_rsp);
        if (ret == 0)
        {
            ss_info = get_ssinfo_from_pcid(mm->cid);
            if (ss_info != NULL)
            {
                trans_node = get_trans_node(&dsd_trans_list, own_mac_addr, dsd_rsp->trans_id);
                if (trans_node != NULL)
                {
                    /* call the DSC transaction state machine */
                    ret = dsd_msg_handler(dsd_rsp, DSD_RSP, trans_node);
                    if (ret != 0)
                    {
                        FLOG_INFO("%s: error in dsd_msg_handler\n", __FUNCTION__);
                    }
                    put_trans_node(trans_node);
                }
                else
                {
                    FLOG_INFO("DSD_RSP received for non-existing transaction %d Discarding\n", dsd_rsp->trans_id);
                }
            }
            else
            {
                FLOG_INFO("DSD_RSP received for non-existing ss: %d, Discarding\n", mm->cid);
            }
        }
        free(dsd_rsp);
    }

    free(mm->data);
}

/*
  * sf_add_handler - to process service flow add request
  * @mm: management message which contains service flow
  *
  * The API is used to process the service flow add request, which is from the APP layer
  *
  * Return:
  *     none
  */
static void sf_add_handler(mgt_msg *mm)
{
    sf_dsx_param                *dsx_param;
    struct transaction_node     *trans_node;
    int                         ret;
    u_int64_t                   own_mac_addr;
    sf_result                   add_result;

    /* create a new transaction node and initialize its members */
    dsx_param = (sf_dsx_param *)mm->data;
    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                    ((u_int64_t)param_MY_MAC[1]) << 32U | \
                    ((u_int64_t)param_MY_MAC[2]) << 24U | \
                    ((u_int64_t)param_MY_MAC[3]) << 16U | \
                    ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                    ((u_int64_t)param_MY_MAC[5]) << 0U;
    
    trans_node = alloc_trans_node();
    if (trans_node != NULL)
    {
        trans_node->trans_id = incr_and_read_tid();
        trans_node->trans_status = DSA_BEGIN;

        trans_node->sf = dsx_param->sf;
        trans_node->sf->trans_id = trans_node->trans_id;
        trans_node->initiator_mac = own_mac_addr;
        trans_node->peer_mac = dsx_param->peer->mac_addr;
        trans_node->peer_primary_cid = dsx_param->peer->primary_cid;
        trans_node->notify = dsx_param->notify;

        /* add the new transaction to dsa list */
        add_trans_node(&dsa_trans_list, trans_node);

        ret = dsa_primitive_handler(SF_ADD, trans_node);
        if (ret != 0)
        {
            FLOG_INFO("%s: error in dsa_primitive_handler\n", __FUNCTION__);

            if (trans_node->notify != NULL)
            {
                add_result.peer_mac= dsx_param->peer->mac_addr;
                add_result.sf_id = trans_node->sf->sfid;
                add_result.cfm_code = CC_REJECT_OTHER;
                (*trans_node->notify)(&add_result);
            }
            delete_trans_node(&dsa_trans_list, trans_node);
        }
    }
    else
    {
        if (dsx_param->notify != NULL)
        {
            add_result.peer_mac= dsx_param->peer->mac_addr;
            add_result.sf_id = dsx_param->sf->sfid;
            add_result.cfm_code = CC_REJECT_OTHER;
            (*dsx_param->notify)(&add_result);
        }
    }
}

/*
  * sf_change_handler - to process service flow changed request
  * @mm: management message which contains changed service flow
  *
  * The API is used to process the service flow change request, which is from the APP layer
  *
  * Return:
  *     none
  */
static void sf_change_handler(mgt_msg *mm)
{
    sf_dsx_param                *dsx_param;
    struct service_flow         *sf_node;
    struct transaction_node     *trans_node;
    struct transaction_node     *own_trans_node;
    u_int64_t                   own_mac_addr;
    int                         ret;
    sf_result                   change_result;

    assert(mm != NULL);

    dsx_param = (sf_dsx_param *)mm->data;
    sf_node = dsx_param->sf;

    /* make sure only one dsc transaction on the same service flow in one time */
    trans_node = find_trans_node_from_sfid(&dsc_trans_list, dsx_param->peer->mac_addr, sf_node->sfid);

    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                                ((u_int64_t)param_MY_MAC[1]) << 32U | \
                                ((u_int64_t)param_MY_MAC[2]) << 24U | \
                                ((u_int64_t)param_MY_MAC[3]) << 16U | \
                                ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                                ((u_int64_t)param_MY_MAC[5]) << 0U;
    own_trans_node = find_trans_node_from_sfid(&dsc_trans_list, own_mac_addr, sf_node->sfid);
                                
    if ((trans_node == NULL) && (own_trans_node == NULL))
    {
        /* create a new transaction node and initialize its members */
        trans_node = alloc_trans_node();
        if (trans_node != NULL)
        {
            trans_node->trans_id = incr_and_read_tid();
            trans_node->trans_status = DSC_BEGIN;

            trans_node->sf = sf_node;
            trans_node->sf->trans_id = trans_node->trans_id;
            trans_node->initiator_mac = own_mac_addr;
            trans_node->peer_mac = dsx_param->peer->mac_addr;
            trans_node->peer_primary_cid = dsx_param->peer->primary_cid;
            trans_node->notify = dsx_param->notify;

            /* add the new transaction to dsa list */
            add_trans_node(&dsc_trans_list, trans_node);

            ret = dsc_primitive_handler(SF_CHANGE, trans_node);
            if (ret != 0)
            {
                FLOG_INFO("%s: error in dsa_primitive_handler\n", __FUNCTION__);
                
                if (trans_node->notify != NULL)
                {
                    change_result.peer_mac= dsx_param->peer->mac_addr;
                    change_result.sf_id = sf_node->sfid;
                    change_result.cfm_code = CC_REJECT_OTHER;
                    (*trans_node->notify)(&change_result);
                }

                delete_trans_node(&dsc_trans_list, trans_node);
            }
        }
        else
        {
            if (dsx_param->notify != NULL)
            {
                change_result.peer_mac= dsx_param->peer->mac_addr;
                change_result.sf_id = sf_node->sfid;
                change_result.cfm_code = CC_REJECT_OTHER;
                (*dsx_param->notify)(&change_result);
            }
        }
    }
    else
    {
        if (dsx_param->notify != NULL)
        {
            change_result.peer_mac= dsx_param->peer->mac_addr;
            change_result.sf_id = sf_node->sfid;
            change_result.cfm_code = CC_REJECT_SF_EXISTS;
            (*dsx_param->notify)(&change_result);
        }
    }
}

/*
  * sf_delete_handler - to process service flow delete request
  * @mm: management message which contains service flow id
  *
  * The API is used to process the service flow delete request, which is from the APP layer
  *
  * Return:
  *     none
  */
static void sf_delete_handler(mgt_msg *mm)
{
    struct transaction_node     *own_trans_node;
    u_int64_t                   own_mac_addr;
    struct transaction_node     *trans_node;
    int                         ret;
    sf_dsx_param                *dsx_param;
    sf_result                   delete_result;

    /* make sure only one dsd transaction on the same service flow in one time */
    dsx_param = (sf_dsx_param *)mm->data;
    own_mac_addr = ((u_int64_t)param_MY_MAC[0]) << 48U |  \
                                ((u_int64_t)param_MY_MAC[1]) << 32U | \
                                ((u_int64_t)param_MY_MAC[2]) << 24U | \
                                ((u_int64_t)param_MY_MAC[3]) << 16U | \
                                ((u_int64_t)param_MY_MAC[4]) << 8U |    \
                                ((u_int64_t)param_MY_MAC[5]) << 0U;
    own_trans_node = find_trans_node_from_sfid(&dsd_trans_list, own_mac_addr, dsx_param->sf->sfid);
    trans_node = find_trans_node_from_sfid(&dsd_trans_list, dsx_param->peer->mac_addr, dsx_param->sf->sfid);
    if ((own_trans_node == NULL) && (trans_node == NULL))
    {
        /* create a new transaction node and initialize its members */
        trans_node = alloc_trans_node();
        if (trans_node != NULL)
        {
            trans_node->trans_id = incr_and_read_tid();
            trans_node->trans_status = DSD_BEGIN;
            
            trans_node->initiator_mac = own_mac_addr;
            trans_node->sf = dsx_param->sf;
            trans_node->sf->trans_id = trans_node->trans_id;
            trans_node->peer_mac = dsx_param->peer->mac_addr;
            trans_node->peer_primary_cid = dsx_param->peer->primary_cid;
            trans_node->notify = dsx_param->notify;
                
            /* add the new transaction to dsa list */
            add_trans_node(&dsd_trans_list, trans_node);
            ret = dsd_primitive_handler(trans_node, SF_DELETE);
            if (ret != 0)
            {
                if (trans_node->notify != NULL)
                {
                    delete_result.peer_mac= dsx_param->peer->mac_addr;
                    delete_result.sf_id =  dsx_param->sf->sfid;
                    delete_result.cfm_code = CC_REJECT_OTHER;
                    (*trans_node->notify)(&delete_result);
                }

                delete_trans_node(&dsd_trans_list, trans_node);
            }
        }
        else
        {
            if (dsx_param->notify != NULL)
            {
                delete_result.peer_mac= dsx_param->peer->mac_addr;
                delete_result.sf_id =  dsx_param->sf->sfid;
                delete_result.cfm_code = CC_REJECT_OTHER;
                (*dsx_param->notify)(&delete_result);
            }
        }
    }
    else
    {
        if (dsx_param->notify != NULL)
        {
            delete_result.peer_mac= dsx_param->peer->mac_addr;
            delete_result.sf_id =  dsx_param->sf->sfid;
            delete_result.cfm_code = CC_REJECT_OTHER;
            (*dsx_param->notify)(&delete_result);
        }
    }

    free(mm->data);
}

void* sf_state_machine()
{
    mgt_msg *mm;
    
    if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        pthread_exit(NULL);
    }

    while (can_sync_continue())
    {
        /* wait on the management message queue */
        mm = dequeue_ul_mgt_msg_queue(&ul_msg_queue[DS_MMM_INDEX]);
        if (mm == NULL) 
        {
            break;
        }
        FLOG_INFO("++++++++ Received in SF SM, msg type: %d ++++++++\n", mm->msg_type);
        
        switch(mm->msg_type)
        {
            case DSA_REQ:
                dsa_req_handler(mm);
                break;
            case DSA_RSP:
                dsa_rsp_handler(mm);
                break;
            case DSX_RVD:
                dsx_rvd_handler(mm);
                break;
            case DSA_ACK:
                dsa_ack_handler(mm);
                break;
            case DSC_REQ:
                dsc_req_handler(mm);
                break;
            case DSC_RSP:
                dsc_rsp_handler(mm);
                break;
            case DSC_ACK:
                dsc_ack_handler(mm);
                break;
            case DSD_REQ:
                dsd_req_handler(mm);
                break;
            case DSD_RSP:
                dsd_rsp_handler(mm);
                break;
            case SF_ADD:
                sf_add_handler(mm);
                break;
            case SF_CHANGE:
                sf_change_handler(mm);
                break;
            case SF_DELETE:
                sf_delete_handler(mm);
                break;
            default:
                FLOG_ERROR("Error in sf_state_machine: Incorrect input\n");
                free(mm->data);
                break;
        }
        free(mm);
    }
    pthread_exit(NULL);
}

void dsx_utm(void)
{
        FLOG_WARNING("-------- %s enter --------\n", __FUNCTION__);
#ifdef DSC_TEST
        FLOG_WARNING("-------- DSC_TEST defined --------\n", __FUNCTION__);
        #ifdef SS_TX
                #ifdef SS_RX
            FLOG_WARNING("-------- SS TX and SS RX defined --------\n", __FUNCTION__);
                        mac_dsc_utm();
                #endif
        #else
                FLOG_WARNING("-------- SS_TX not defined --------\n", __FUNCTION__);
                #ifndef SS_RX
                        FLOG_WARNING("-------- SS_RX not defined --------\n", __FUNCTION__);
                        mac_dsc_utm();
                #endif
        #endif
#endif

#ifdef DSD_TEST
        FLOG_WARNING("-------- DSD_TEST defined --------\n", __FUNCTION__);
        #ifdef SS_TX
                #ifdef SS_RX
            FLOG_WARNING("-------- SS TX and SS RX defined --------\n", __FUNCTION__);
                        mac_dsd_utm();
                #endif
        #else
                FLOG_WARNING("-------- SS_TX not defined --------\n", __FUNCTION__);
                #ifndef SS_RX
                        FLOG_WARNING("-------- SS_RX not defined --------\n", __FUNCTION__);
                        mac_dsd_utm();
                #endif
        #endif
#endif
        FLOG_WARNING("-------- %s exit --------\n", __FUNCTION__);
}
