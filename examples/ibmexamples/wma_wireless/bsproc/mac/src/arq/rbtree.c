/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2008,2009,2010,2011,2012

All Rights Reserved

File Name: rbtree.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
03-May-2012 	     Created		   Mukundan Madhavan

---------------------------------------------------------------
*/
//#define RBTREE_TEST
#include "rbtree.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "flog.h"
int compare_int(void* leftp, void* rightp) 
{
#ifdef INTEGRATION_TEST
    long long int left = (long long int)(leftp);
	long long int right = (long long int)(rightp);
#else
    int left = (int)(leftp);
	int right = (int)(rightp);
#endif
/*
#ifdef RBTREE_TEST
	printf("Compare_int :%d %d \n",left,right);
#endif
*/
	if (left < right)
		return -1;
	else if (left > right)
		return 1;
	else {
		assert (left == right);
		return 0;
	}
}
rbtree* rbtree_create()
{
	rbtree* temp;
	temp = (rbtree*) malloc(sizeof(rbtree));
	temp->root = NULL;
	return temp;
}
enum rbtree_node_color node_color(rbtree_node* n)
{
	return n == NULL ? BLACK:n->color;
}
void rbtree_destroy(rbtree* t)
{
	if (t) free(t);
}
rbtree_node* rbtree_search(rbtree_node* n, void* key)
{	
	if (n == NULL) return NULL;
	int temp = compare_int(n->key, key);
	if (temp == 0) {if (n!= NULL) assert(n->value != NULL);return n;}
	rbtree_node *a = NULL;
	if (temp > 0)
	{
		if (n->left != NULL)
		{
			a = rbtree_search(n->left, key);
		}
	}
	if (temp < 0)
	{
		if (n->right != NULL)
		{
			a = rbtree_search(n->right, key);
		}
	}
	return a;
}
void* rbtree_lookup(rbtree* t, void* key)
{
	rbtree_node* n = t->root;
 	rbtree_node* my_node = rbtree_search(n,key);
	return ((my_node == NULL)? NULL :my_node->value);
}
void rotate_left(rbtree* t, rbtree_node *n)
{
	rbtree_node* r = n->right;

	if (n->parent == NULL) 
	{
		//n is root node.
		t->root = r;
	}
	else
	{
		if (n == n->parent->left) n->parent->left = r;
		else n->parent->right = r;
	}	
	if (r != NULL) r->parent = n->parent;
	n->right = r->left;
	if (r->left != NULL) r->left->parent = n;
	r->left = n;
	n->parent = r;


}
void rotate_right(rbtree* t, rbtree_node* n)
{
	rbtree_node* l = n->left;
	
	if (n->parent == NULL) 
	{
		//n is root node.
		t->root = l;
	}
	else
	{
		if (n == n->parent->left) n->parent->left = l;
		else n->parent->right = l;
	}	
	if (l != NULL) l->parent = n->parent;
	
	n->left = l->right;
	if (l->right != NULL) l->right->parent = n;
	l->right = n;
	n->parent = l;
}
rbtree_node* uncle(rbtree_node* node)
{
	if (node->parent == node->parent->parent->right)
	return node->parent->parent->left;
	else
	return node->parent->parent->right;
}
	
void rbtree_insert(rbtree* t, void* key, void* value )
{
	int cmp;
//Creating a new node.
	rbtree_node* node = (rbtree_node*) malloc(sizeof(rbtree_node));
	node->key = key;
	node->value = value;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->color = RED;

//Find where to insert
	if (t->root == NULL) t->root = node;
	else
	{
		rbtree_node*  n = t->root;
		while(1)
		{
			cmp = compare_int(node->key,n->key);
			if (cmp ==0)
			{
				FLOG_WARNING("Trying to insert arq connection, but cid is already part of RB Tree\n");
				n->value = node->value;
				free(node);
				return;
			}
			if (cmp < 0)
			{
#ifdef RBTREE_TEST
				FLOG_DEBUG("Insert : Going left\n");
#endif
				if (n->left == NULL) 
				{
					n->left = node;
					node->parent = n;
					break;
				}
				else n = n->left;
			}
			if (cmp > 0)
			{
#ifdef RBTREE_TEST
				FLOG_DEBUG("Insert : Going right\n");
#endif
				if (n->right == NULL) 
				{
					n->right = node;
					node->parent = n;
					break;
				}
				else n = n->right;
			}
		}
		
		
	}
	//Node has been inserted according to Binary Search Tree rules. Now, to work towards preserving RB Tree rules.
	rbtree_node* uncle_node;
	while ( (node != t->root) && (node_color(node->parent) == RED))
	{
		if (node->parent == node->parent->parent->left)
		{
			//Case A - Parent is left child. 
			uncle_node = uncle(node);
			if (node_color(uncle_node) == RED)
			{
				//If uncle is red, and parent is red. Change colors of parent, uncle to black and grandparent to red. this preservres black height.
				node->parent->color = BLACK;
				uncle_node->color = BLACK;
				node->parent->parent->color = RED;
				node = node->parent->parent; //Move up the tree and fix grandparent relations.
			}	
			else
			{
				//Uncle is black
				if (node == node->parent->right)
				{
					//parent is left child, you are right child - zigzag pattern
					//node = node->parent;
					rotate_left(t,node->parent);
					node = node->left;
				}
				//case 3. zigzag straightened up
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rotate_right(t, node->parent->parent);	
			}
		}
		else
		{
			//Case B - Parent is right child. 
			uncle_node = uncle(node);
			if (node_color(uncle_node) == RED)
			{
				//If uncle is red, and parent is red. Change colors of parent, uncle to black and grandparent to red. this preservres black height.
				node->parent->color = BLACK;
				uncle_node->color = BLACK;
				node->parent->parent->color = RED;
				node = node->parent->parent; //Move up the tree and fix grandparent relations.
			}	
			else
			{
				//Uncle is black
				if (node == node->parent->left)
				{
					//parent is left child, you are right child - zigzag pattern
					//node = node->parent;
					rotate_right(t,node->parent);
					node = node->right;
				}
				//case 3. zigzag straightened up
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rotate_left(t, node->parent->parent);	
			}
		}

	}
	t->root->color = BLACK;
	print_rbtree(t->root);	
}
void replace_node(rbtree* t, rbtree_node* old_node, rbtree_node* new_node)
{

	//replace node with child
	if (old_node->parent == NULL)
	{
		//n is root
		t->root = new_node;
	}
	else
	{
		if (old_node == old_node->parent->left)
		{
			old_node->parent->left = new_node;
		}
		else
		{
			old_node->parent->right = new_node;
		}
	}
		if (new_node != NULL) new_node->parent = old_node->parent;
	//end of replacement
}
void delete_fix_tree(rbtree* t, rbtree_node* n)
{
	rbtree_node* sib;
	while (n != t->root)
	{
		if (n == n->parent->left)
		{
			//case A : node is left child.
			sib = n->parent->right;
			if (node_color(sib) == RED)
			{
				//if sibling is red, do an adjustment and do a delete(node) using the sibling = black cases.
#ifdef RBTREE_TEST
				FLOG_DEBUG("\t\t\t Case A1\n");
#endif
				sib->color = BLACK;
				n->parent->color = RED;
				rotate_left(t,n->parent);
				assert(n == n->parent->left);
				sib = n->parent->right;
			}
			//what if sibling is null ???	
			if (node_color(sib->left) == BLACK && node_color(sib->right) == BLACK)
			{
#ifdef RBTREE_TEST
				FLOG_DEBUG("\t\t\t Case A2\n");
#endif
				//sib has only black children
				sib->color = RED;
				if (node_color(n->parent) == BLACK)
				{
					n = n->parent;
					continue;	
				}
				else
				{
					n->parent->color = BLACK; break;
				}
			}
			else if(node_color(sib->right) == BLACK)
			{
#ifdef RBTREE_TEST
				FLOG_DEBUG("\t\t\t Case A3\n");
#endif
				sib->color = RED;
				sib->left->color = BLACK;
				rotate_right(t, sib);
				sib = n->parent->right;
			}
			sib->color = node_color(n->parent);
			n->parent->color = BLACK;
			if (sib->right) sib->right->color = BLACK;
			rotate_left(t, n->parent);
			n = t->root;
		}
		else
		{
			//case B : node is right child.
			sib = n->parent->left;
			if (node_color(sib) == RED)
			{
				//if sibling is red, do an adjustment and do a delete(node) using the sibling = black cases.
#ifdef RBTREE_TEST
				FLOG_DEBUG("\t\t\tCaseB1\n");
#endif
				sib->color = BLACK;
				n->parent->color = RED;
				rotate_right(t,n->parent);
				assert(n == n->parent->right);
				sib = n->parent->left;
			}
			//what if sibling is null ???	
			if (node_color(sib->right) == BLACK && node_color(sib->left) == BLACK)
			{
#ifdef RBTREE_TEST
				FLOG_DEBUG("\t\t\tCaseB2\n");
#endif
				//sib has only black children
				sib->color = RED;
				if (node_color(n->parent) == BLACK)
				{
					n = n->parent;
					continue;	
				}
				else
				{
					n->parent->color = BLACK; break;
				}
			}
			else if(node_color(sib->left) == BLACK)
			{
#ifdef RBTREE_TEST
				FLOG_DEBUG("\t\t\tCaseB3\n");
#endif
				sib->color = RED;
				sib->right->color = BLACK;
				rotate_left(t, sib);
				sib = n->parent->left;
			}
			sib->color = node_color(n->parent);
			n->parent->color = BLACK;
			if (sib->left) sib->left->color = BLACK;
			rotate_right(t, n->parent);
			n = t->root;

		}	

	}


}
void rbtree_delete(rbtree* t, void* key)
{
	if (t->root == NULL)
	{
		FLOG_WARNING("ARQ RB Tree empty\n");
		return;
	}
	rbtree_node* n = t->root;
	int cmp;
	while(n != NULL)
	{
		cmp = compare_int(n->key, key);
		if (cmp == 0) break;
		if (cmp < 0) n=n->right;
		else if (cmp > 0) n = n->left;
	}
	if (n == NULL)
	{
		FLOG_WARNING("ARQ RB Tree: Key not found in tree for deletion\n");
		return;
	}
	
//Convert such that node-to-be-deleted is parent to atleast one null-leaf. Swap data.
	if (n->left != NULL && n->right != NULL)
	{
#ifdef RBTREE_TEST
		FLOG_DEBUG("Delete is for a non-leaf node\n"); 
#endif
		rbtree_node* swap_node = n->left;
		while (swap_node->right != NULL)
		{
			swap_node = swap_node->right;
		}
		n->key = swap_node->key;
		n->value = swap_node->value;
		n = swap_node;
	}
	//Swap is done. Now we will work with this node having one null-leaf for deletion.
	rbtree_node* n_child = n->right == NULL? n->left: n->right;

	if (node_color(n) == BLACK)
	{
		n->color = node_color(n_child);
		delete_fix_tree(t,n);
		//Going to do color-fixing
	}
	replace_node(t,n,n_child);
	if (n->parent == NULL && n_child != NULL) n_child->color = BLACK;
	free(n); 
}
void check_rb_property(rbtree_node* n)
{
	if (node_color(n) == RED)
	{
		assert(node_color(n->left) == BLACK);
		assert(node_color(n->right) == BLACK);
		assert(node_color(n->parent) == BLACK);
	}
	if (n == NULL) return;
	check_rb_property(n->left);
	check_rb_property(n->right);
}
void check_black_height(rbtree_node* n, int count, int* path_count)
{
	if (node_color(n) == BLACK) count++;
	if (n == NULL)
	{
		if (*path_count == -1) *path_count = count;
		else assert(count == *path_count);
		return;
	}
	check_black_height(n->left, count, path_count);
	check_black_height(n->right, count, path_count);
}


int check_properties(rbtree* t)
{
	rbtree_node* n = t->root;
	assert(node_color(n) == BLACK); //root is black
	check_rb_property(n);
	int path_count = -1;
	check_black_height(n,0,&path_count);
}
print_rbtree(rbtree_node *n)
{
	
	if (n== NULL) return;
#ifdef RBTREE_TEST
#ifdef INTEGRATION_TEST
	long long int key = ((long long int)n->key);
	if (n->parent != NULL &&  n == n->parent->left) printf("Left node to %d: ",(long long int)(n->parent->key)); 
	else if(n->parent != NULL)  printf("Right node to %d: ", (long long int)(n->parent->key));
#else
	int key = ((int)n->key);
	if (n->parent != NULL &&  n == n->parent->left) printf("Left node to %d: ",(int)(n->parent->key)); 
	else if(n->parent != NULL)  printf("Right node to %d: ", (int)(n->parent->key));
#endif
	 printf("%d %d\n",key,n->color);
#endif
	print_rbtree(n->left);
	print_rbtree(n->right);
}
