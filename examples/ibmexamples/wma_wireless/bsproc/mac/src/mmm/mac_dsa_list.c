/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsa_list.c

   Change Activity:

   Date             Description of Change                   		By
   -----------      ---------------------		--------
   1-Mar.2011       Created                                		Parul Gupta
   03-Feb.2012	Modified to make it support dsc	 	Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_dsa_list.h"

void initiate_trans_list(struct transaction_list *trans_list)
{
	if (trans_list != NULL)
	{
    	trans_list->head = NULL;
    	pthread_mutex_init(&trans_list->trans_list_mutex, NULL);
	}
}

void add_trans_node
(
	struct transaction_list		*trans_list, 
	struct transaction_node		*trans_node
)
{
    pthread_mutex_lock(&(trans_list->trans_list_mutex));
    trans_node->next = trans_list->head;
    trans_list->head = trans_node;
    pthread_mutex_unlock(&(trans_list->trans_list_mutex));
}

static void add_trans_node_ref(struct transaction_node *trans_node)
{
	if (trans_node != NULL)
	{
		pthread_mutex_lock(&trans_node->mutex);
		trans_node->ref_count++;
		pthread_mutex_unlock(&trans_node->mutex);
	}
}

/* do not free the transaction list itself */
void free_trans_list(struct transaction_list *trans_list)
{	
	struct transaction_node *trans_node;

	if (trans_list != NULL)
	{
		/* free all transaction nodes in the transaction list */
		pthread_mutex_lock(&trans_list->trans_list_mutex);
		while (trans_list->head != NULL)
		{
			trans_node = trans_list->head;
			trans_list->head = trans_node->next;

			if (trans_node->sf != NULL)
			{
				if (trans_node->sf->service_class_name != NULL)
				{
					free(trans_node->sf->service_class_name);
				}
				free(trans_node->sf);
				trans_node->sf = NULL;
			}
			
			if (trans_node->re_msg.msg != NULL)
			{	
				free(trans_node->re_msg.msg);
			}
			free(trans_node);
		}
		pthread_mutex_unlock(&trans_list->trans_list_mutex);
		pthread_mutex_destroy(&trans_list->trans_list_mutex);

		/* do not free the transaction list itself, because it is static variable now */
	}
}

struct transaction_node* get_trans_node
(
	struct transaction_list	*trans_list,
	u_int64_t				initiator_mac,
	u_int16_t 				trans_id
)
{
	struct transaction_node*	trans_node;
	struct transaction_node*	result;

	result = NULL;
	if (trans_list != NULL)
	{
		pthread_mutex_lock(&trans_list->trans_list_mutex);
		trans_node = trans_list->head;
		while (trans_node != NULL) 
		{
        	if ((trans_node->initiator_mac == initiator_mac) && (trans_node->trans_id == trans_id))
        	{
				add_trans_node_ref(trans_node);
				result = trans_node;
				break;
        	}
          	trans_node = trans_node->next;
        }

		pthread_mutex_unlock(&trans_list->trans_list_mutex);
    }

    return  result;
}

struct transaction_node* find_trans_node_from_sfid 
(
	struct transaction_list		*trans_list, 
	u_int64_t					initiator_mac,
	u_int32_t 					sfid
)
{
	struct transaction_node*	trans_node;
	struct transaction_node*	result;

	result = NULL;
	if (trans_list != NULL)
	{
		pthread_mutex_lock(&trans_list->trans_list_mutex);

		trans_node = trans_list->head;
		while (trans_node != NULL) 
		{
        	if ((trans_node->initiator_mac == initiator_mac) && (trans_node->sf->sfid == sfid))
        	{
        		result = trans_node;
				break;
        	}
          	trans_node = trans_node->next;
        }

		pthread_mutex_unlock(&trans_list->trans_list_mutex);
    }

    return  result;
}

struct transaction_node* alloc_trans_node(void)
{
	struct transaction_node*	trans_node;
	int							ret;
	
	trans_node = (struct transaction_node *)malloc(sizeof(struct transaction_node));
	if (trans_node != NULL)
	{
		memset(trans_node, 0, sizeof(struct transaction_node));
		trans_node->t7_timer = NULL;
		trans_node->t8_timer = NULL;
		trans_node->t10_timer = NULL;
		trans_node->t14_timer = NULL;
		trans_node->sf = NULL;
		trans_node->next = NULL;
		trans_node->ref_count = 1U;

		ret= pthread_mutex_init(&trans_node->mutex, NULL);
		if (ret != 0)
		{
			free(trans_node);
			trans_node = NULL;
		}
	}

	return trans_node;
}

void put_trans_node(struct transaction_node *trans_node)
{
	sf_result				sf_result;

	if (trans_node != NULL)
	{
		pthread_mutex_lock(&trans_node->mutex);
		trans_node->ref_count--;
		if (trans_node->ref_count == 0)
		{
			FLOG_INFO("++++++++  trans_node(%p) no ref any nore ++++++++\n", trans_node);
			/* free the resources owned by transaction node */
			if (trans_node->sf != NULL) 
			{
				if (trans_node->sf->service_class_name != NULL)
				{
					free(trans_node->sf->service_class_name);
				}
				free(trans_node->sf);
				trans_node->sf = NULL;
			}

			/* clear all the timers */
			if (trans_node->t7_timer != NULL)
			{
				app_timer_delete(trans_node->t7_timer);
				trans_node->t7_timer = NULL;
			}
			if (trans_node->t8_timer != NULL)
			{
				app_timer_delete(trans_node->t8_timer);
				trans_node->t8_timer = NULL;
			}
			if (trans_node->t10_timer != NULL)
			{
				app_timer_delete(trans_node->t10_timer);
				trans_node->t10_timer = NULL;
			}
			if (trans_node->t14_timer != NULL)
			{
				app_timer_delete(trans_node->t14_timer);
				trans_node->t14_timer = NULL;
			}

			if (trans_node->re_msg.msg != NULL) 
			{
				free(trans_node->re_msg.msg);
			}
			pthread_mutex_unlock(&trans_node->mutex);

			pthread_mutex_destroy(&trans_node->mutex);
			
			free(trans_node);
		}
		else
		{
			pthread_mutex_unlock(&trans_node->mutex);
		}
	}
}

/*
  * delete_trans_node -delete transaction node
  * @trans_list: transaction node list
  * @delete_node: transaction node to be deleted
  *
  * The API is used to remove transation node from transaction list, and free it
  *
  * Return:
  *		none
  */
void delete_trans_node
(
	struct transaction_list 	*trans_list,
	struct transaction_node		*delete_node
)
{
	struct transaction_node*	trans_node;
	struct transaction_node*	prev_node;

	assert(delete_node != NULL);
	
    if (trans_list->head != NULL)
    {
    	/* remove it from transaction list */
        pthread_mutex_lock(&trans_list->trans_list_mutex);
		trans_node = trans_list->head;
		prev_node = NULL;
		while (trans_node != NULL) 
		{
        	if (trans_node == delete_node)
        	{
				break;
        	}
			prev_node = trans_node;
          	trans_node = trans_node->next;
        }

		if (trans_node != NULL)
		{
	        if (prev_node == NULL)
	        {
	            trans_list->head = trans_node->next;
	        }
	        else
	        {
	            prev_node->next = trans_node->next;
	        }
		}
		pthread_mutex_unlock(&(trans_list->trans_list_mutex));
    }

	put_trans_node(trans_node);
}

int incr_and_read_sfid()
{
	int tsfid;
	
	pthread_mutex_lock(&(last_sfid_mutex));
	last_sfid++;
	tsfid = last_sfid;
	pthread_mutex_unlock(&(last_sfid_mutex));
	
	return tsfid;
}

int incr_and_read_tid()
{
	int tid;
	
	pthread_mutex_lock(&(last_tid_mutex));
	last_tid++;
	tid = last_tid;
	pthread_mutex_unlock(&(last_tid_mutex));
	
	return tid;
}
