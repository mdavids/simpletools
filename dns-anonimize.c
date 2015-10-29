/*
 * Copyright (c) 2001-2013, NLnet Labs. All rights reserved.
 * 
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Version 0.0.4
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>


/** Node colour black */
#define	BLACK	0
/** Node colour red */
#define	RED	1

typedef struct rbnode_t rbnode_t;
/**
 * The rbnode_t struct definition.
 */
struct rbnode_t {
	/** parent in rbtree, RBTREE_NULL for root */
	rbnode_t   *parent;
	/** left node (smaller items) */
	rbnode_t   *left;
	/** right node (larger items) */
	rbnode_t   *right;
	/** pointer to sorting key */
	uint8_t     key[16];
	/** pointer to data */
	uint32_t    value;
	/** colour of this node */
	uint8_t     color;
};

#define RBTREE_NULL &rbtree_null_node
/** the NULL node, global alloc */
rbnode_t	rbtree_null_node = {
	RBTREE_NULL,	/* Parent.  */
	RBTREE_NULL,	/* Left.  */
	RBTREE_NULL,	/* Right.  */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* Key.  */
	0,               /* Data. */
	BLACK		     /* Color.  */
};


/** An entire red black tree */
typedef struct rbtree_t rbtree_t;
/** definition for tree struct */
struct rbtree_t {
	/** The root of the red-black tree */
	rbnode_t    *root;
	/** The number of the nodes in the tree */
	size_t       count;
	/**
	 * Key compare function. <0,0,>0 like strcmp.
	 * Return 0 on two NULL ptrs.
	 */
	int (*cmp) (const void *, const void *);
};

/** rotate subtree left (to preserve redblack property) */
void rbtree_rotate_left(rbtree_t *rbtree, rbnode_t *node);
/** rotate subtree right (to preserve redblack property) */
void rbtree_rotate_right(rbtree_t *rbtree, rbnode_t *node);
/** Fixup node colours when insert happened */
void rbtree_insert_fixup(rbtree_t *rbtree, rbnode_t *node);
/** Fixup node colours when delete happened */
void rbtree_delete_fixup(rbtree_t* rbtree, rbnode_t* child, rbnode_t* child_parent);

void 
rbtree_init(rbtree_t *rbtree, int (*cmpf)(const void *, const void *))
{
	/* Initialize it */
	rbtree->root = RBTREE_NULL;
	rbtree->count = 0;
	rbtree->cmp = cmpf;
}

/*
 * Creates a new red black tree, intializes and returns a pointer to it.
 *
 * Return NULL on failure.
 *
 */
rbtree_t *
rbtree_create (int (*cmpf)(const void *, const void *))
{
	rbtree_t *rbtree;

	/* Allocate memory for it */
	rbtree = (rbtree_t *) malloc(sizeof(rbtree_t));
	if (!rbtree) {
		return NULL;
	}

	/* Initialize it */
	rbtree_init(rbtree, cmpf);

	return rbtree;
}

void 
rbtree_free(rbtree_t *rbtree)
{
	free(rbtree);
}

/*
 * Rotates the node to the left.
 *
 */
void
rbtree_rotate_left(rbtree_t *rbtree, rbnode_t *node)
{
	rbnode_t *right = node->right;
	node->right = right->left;
	if (right->left != RBTREE_NULL)
		right->left->parent = node;

	right->parent = node->parent;

	if (node->parent != RBTREE_NULL) {
		if (node == node->parent->left) {
			node->parent->left = right;
		} else  {
			node->parent->right = right;
		}
	} else {
		rbtree->root = right;
	}
	right->left = node;
	node->parent = right;
}

/*
 * Rotates the node to the right.
 *
 */
void
rbtree_rotate_right(rbtree_t *rbtree, rbnode_t *node)
{
	rbnode_t *left = node->left;
	node->left = left->right;
	if (left->right != RBTREE_NULL)
		left->right->parent = node;

	left->parent = node->parent;

	if (node->parent != RBTREE_NULL) {
		if (node == node->parent->right) {
			node->parent->right = left;
		} else  {
			node->parent->left = left;
		}
	} else {
		rbtree->root = left;
	}
	left->right = node;
	node->parent = left;
}

void
rbtree_insert_fixup(rbtree_t *rbtree, rbnode_t *node)
{
	rbnode_t	*uncle;

	/* While not at the root and need fixing... */
	while (node != rbtree->root && node->parent->color == RED) {
		/* If our parent is left child of our grandparent... */
		if (node->parent == node->parent->parent->left) {
			uncle = node->parent->parent->right;

			/* If our uncle is red... */
			if (uncle->color == RED) {
				/* Paint the parent and the uncle black... */
				node->parent->color = BLACK;
				uncle->color = BLACK;

				/* And the grandparent red... */
				node->parent->parent->color = RED;

				/* And continue fixing the grandparent */
				node = node->parent->parent;
			} else {				/* Our uncle is black... */
				/* Are we the right child? */
				if (node == node->parent->right) {
					node = node->parent;
					rbtree_rotate_left(rbtree, node);
				}
				/* Now we're the left child, repaint and rotate... */
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rbtree_rotate_right(rbtree, node->parent->parent);
			}
		} else {
			uncle = node->parent->parent->left;

			/* If our uncle is red... */
			if (uncle->color == RED) {
				/* Paint the parent and the uncle black... */
				node->parent->color = BLACK;
				uncle->color = BLACK;

				/* And the grandparent red... */
				node->parent->parent->color = RED;

				/* And continue fixing the grandparent */
				node = node->parent->parent;
			} else {				/* Our uncle is black... */
				/* Are we the right child? */
				if (node == node->parent->left) {
					node = node->parent;
					rbtree_rotate_right(rbtree, node);
				}
				/* Now we're the right child, repaint and rotate... */
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rbtree_rotate_left(rbtree, node->parent->parent);
			}
		}
	}
	rbtree->root->color = BLACK;
}

/*
 * Inserts a node into a red black tree.
 *
 * Returns NULL on failure or the pointer to the newly added node
 * otherwise.
 */
rbnode_t *
rbtree_insert (rbtree_t *rbtree, rbnode_t *data)
{
	/* XXX Not necessary, but keeps compiler quiet... */
	int r = 0;

	/* We start at the root of the tree */
	rbnode_t	*node = rbtree->root;
	rbnode_t	*parent = RBTREE_NULL;

	/* Lets find the new parent... */
	while (node != RBTREE_NULL) {
		/* Compare two keys, do we have a duplicate? */
		if ((r = rbtree->cmp(data->key, node->key)) == 0) {
			return NULL;
		}
		parent = node;

		if (r < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	/* Initialize the new node */
	data->parent = parent;
	data->left = data->right = RBTREE_NULL;
	data->color = RED;
	rbtree->count++;

	/* Insert it into the tree... */
	if (parent != RBTREE_NULL) {
		if (r < 0) {
			parent->left = data;
		} else {
			parent->right = data;
		}
	} else {
		rbtree->root = data;
	}

	/* Fix up the red-black properties... */
	rbtree_insert_fixup(rbtree, data);

	return data;
}


void
rbtree_insert_vref(rbnode_t *data, void *rbtree)
{
	(void) rbtree_insert((rbtree_t *) rbtree,
						 data);
}

/** helpers for delete: swap node colours */
void swap_int8(uint8_t* x, uint8_t* y) 
{ 
	uint8_t t = *x; *x = *y; *y = t; 
}

/** helpers for delete: swap node pointers */
void swap_np(rbnode_t** x, rbnode_t** y) 
{
	rbnode_t* t = *x; *x = *y; *y = t; 
}

/** Update parent pointers of child trees of 'parent' */
void change_parent_ptr(rbtree_t* rbtree, rbnode_t* parent, rbnode_t* old, rbnode_t* new)
{
	if(parent == RBTREE_NULL)
	{
		if(rbtree->root == old) rbtree->root = new;
		return;
	}
	if(parent->left == old) parent->left = new;
	if(parent->right == old) parent->right = new;
}
/** Update parent pointer of a node 'child' */
void change_child_ptr(rbnode_t* child, rbnode_t* old, rbnode_t* new)
{
	if(child == RBTREE_NULL) return;
	if(child->parent == old) child->parent = new;
}

rbnode_t * rbtree_search (rbtree_t *rbtree, const void *key);
rbnode_t* 
rbtree_delete(rbtree_t *rbtree, const void *key)
{
	rbnode_t *to_delete;
	rbnode_t *child;
	if((to_delete = rbtree_search(rbtree, key)) == 0) return 0;
	rbtree->count--;

	/* make sure we have at most one non-leaf child */
	if(to_delete->left != RBTREE_NULL &&
	   to_delete->right != RBTREE_NULL)
	{
		/* swap with smallest from right subtree (or largest from left) */
		rbnode_t *smright = to_delete->right;
		while(smright->left != RBTREE_NULL)
			smright = smright->left;
		/* swap the smright and to_delete elements in the tree,
		 * but the rbnode_t is first part of user data struct
		 * so cannot just swap the keys and data pointers. Instead
		 * readjust the pointers left,right,parent */

		/* swap colors - colors are tied to the position in the tree */
		swap_int8(&to_delete->color, &smright->color);

		/* swap child pointers in parents of smright/to_delete */
		change_parent_ptr(rbtree, to_delete->parent, to_delete, smright);
		if(to_delete->right != smright)
			change_parent_ptr(rbtree, smright->parent, smright, to_delete);

		/* swap parent pointers in children of smright/to_delete */
		change_child_ptr(smright->left, smright, to_delete);
		change_child_ptr(smright->left, smright, to_delete);
		change_child_ptr(smright->right, smright, to_delete);
		change_child_ptr(smright->right, smright, to_delete);
		change_child_ptr(to_delete->left, to_delete, smright);
		if(to_delete->right != smright)
			change_child_ptr(to_delete->right, to_delete, smright);
		if(to_delete->right == smright)
		{
			/* set up so after swap they work */
			to_delete->right = to_delete;
			smright->parent = smright;
		}

		/* swap pointers in to_delete/smright nodes */
		swap_np(&to_delete->parent, &smright->parent);
		swap_np(&to_delete->left, &smright->left);
		swap_np(&to_delete->right, &smright->right);

		/* now delete to_delete (which is at the location where the smright previously was) */
	}

	if(to_delete->left != RBTREE_NULL) child = to_delete->left;
	else child = to_delete->right;

	/* unlink to_delete from the tree, replace to_delete with child */
	change_parent_ptr(rbtree, to_delete->parent, to_delete, child);
	change_child_ptr(child, to_delete, to_delete->parent);

	if(to_delete->color == RED)
	{
		/* if node is red then the child (black) can be swapped in */
	}
	else if(child->color == RED)
	{
		/* change child to BLACK, removing a RED node is no problem */
		if(child!=RBTREE_NULL) child->color = BLACK;
	}
	else rbtree_delete_fixup(rbtree, child, to_delete->parent);

	/* unlink completely */
	to_delete->parent = RBTREE_NULL;
	to_delete->left = RBTREE_NULL;
	to_delete->right = RBTREE_NULL;
	to_delete->color = BLACK;
	return to_delete;
}

void rbtree_delete_fixup(rbtree_t* rbtree, rbnode_t* child, rbnode_t* child_parent)
{
	rbnode_t* sibling;
	int go_up = 1;

	/* determine sibling to the node that is one-black short */
	if(child_parent->right == child) sibling = child_parent->left;
	else sibling = child_parent->right;

	while(go_up)
	{
		if(child_parent == RBTREE_NULL)
		{
			/* removed parent==black from root, every path, so ok */
			return;
		}

		if(sibling->color == RED)
		{	/* rotate to get a black sibling */
			child_parent->color = RED;
			sibling->color = BLACK;
			if(child_parent->right == child)
				rbtree_rotate_right(rbtree, child_parent);
			else	rbtree_rotate_left(rbtree, child_parent);
			/* new sibling after rotation */
			if(child_parent->right == child) sibling = child_parent->left;
			else sibling = child_parent->right;
		}

		if(child_parent->color == BLACK 
			&& sibling->color == BLACK
			&& sibling->left->color == BLACK
			&& sibling->right->color == BLACK)
		{	/* fixup local with recolor of sibling */
			if(sibling != RBTREE_NULL)
				sibling->color = RED;

			child = child_parent;
			child_parent = child_parent->parent;
			/* prepare to go up, new sibling */
			if(child_parent->right == child) sibling = child_parent->left;
			else sibling = child_parent->right;
		}
		else go_up = 0;
	}

	if(child_parent->color == RED
		&& sibling->color == BLACK
		&& sibling->left->color == BLACK
		&& sibling->right->color == BLACK) 
	{
		/* move red to sibling to rebalance */
		if(sibling != RBTREE_NULL)
			sibling->color = RED;
		child_parent->color = BLACK;
		return;
	}

	/* get a new sibling, by rotating at sibling. See which child
	   of sibling is red */
	if(child_parent->right == child
		&& sibling->color == BLACK
		&& sibling->right->color == RED
		&& sibling->left->color == BLACK)
	{
		sibling->color = RED;
		sibling->right->color = BLACK;
		rbtree_rotate_left(rbtree, sibling);
		/* new sibling after rotation */
		if(child_parent->right == child) sibling = child_parent->left;
		else sibling = child_parent->right;
	}
	else if(child_parent->left == child
		&& sibling->color == BLACK
		&& sibling->left->color == RED
		&& sibling->right->color == BLACK)
	{
		sibling->color = RED;
		sibling->left->color = BLACK;
		rbtree_rotate_right(rbtree, sibling);
		/* new sibling after rotation */
		if(child_parent->right == child) sibling = child_parent->left;
		else sibling = child_parent->right;
	}

	/* now we have a black sibling with a red child. rotate and exchange colors. */
	sibling->color = child_parent->color;
	child_parent->color = BLACK;
	if(child_parent->right == child)
	{
		sibling->left->color = BLACK;
		rbtree_rotate_right(rbtree, child_parent);
	}
	else
	{
		sibling->right->color = BLACK;
		rbtree_rotate_left(rbtree, child_parent);
	}
}

int
rbtree_find_less_equal(rbtree_t *rbtree, const void *key, rbnode_t **result)
{
	int r;
	rbnode_t *node;

	/* We start at root... */
	node = rbtree->root;

	*result = NULL;

	/* While there are children... */
	while (node != RBTREE_NULL) {
		r = rbtree->cmp(key, node->key);
		if (r == 0) {
			/* Exact match */
			*result = node;
			return 1;
		} 
		if (r < 0) {
			node = node->left;
		} else {
			/* Temporary match */
			*result = node;
			node = node->right;
		}
	}
	return 0;
}

/*
 * Searches the red black tree, returns the data if key is found or NULL otherwise.
 *
 */
rbnode_t *
rbtree_search (rbtree_t *rbtree, const void *key)
{
	rbnode_t *node;

	if (rbtree_find_less_equal(rbtree, key, &node)) {
		return node;
	} else {
		return NULL;
	}
}

/*
 * Finds the first element in the red black tree
 *
 */
rbnode_t *
rbtree_first (rbtree_t *rbtree)
{
	rbnode_t *node = rbtree->root;

	if (rbtree->root != RBTREE_NULL) {
		for (node = rbtree->root; node->left != RBTREE_NULL; node = node->left);
	}
	return node;
}

rbnode_t *
rbtree_last (rbtree_t *rbtree)
{
	rbnode_t *node = rbtree->root;

	if (rbtree->root != RBTREE_NULL) {
		for (node = rbtree->root; node->right != RBTREE_NULL; node = node->right);
	}
	return node;
}

/*
 * Returns the next node...
 *
 */
rbnode_t *
rbtree_next (rbnode_t *node)
{
	rbnode_t *parent;

	if (node->right != RBTREE_NULL) {
		/* One right, then keep on going left... */
		for (node = node->right;
			node->left != RBTREE_NULL;
			node = node->left);
	} else {
		parent = node->parent;
		while (parent != RBTREE_NULL && node == parent->right) {
			node = parent;
			parent = parent->parent;
		}
		node = parent;
	}
	return node;
}

rbnode_t *
rbtree_previous(rbnode_t *node)
{
	rbnode_t *parent;

	if (node->left != RBTREE_NULL) {
		/* One left, then keep on going right... */
		for (node = node->left;
			node->right != RBTREE_NULL;
			node = node->right);
	} else {
		parent = node->parent;
		while (parent != RBTREE_NULL && node == parent->left) {
			node = parent;
			parent = parent->parent;
		}
		node = parent;
	}
	return node;
}

/**
 * split off elements number of elements from the start
 * of the name tree and return a new tree 
 */
rbtree_t *
rbtree_split(rbtree_t *tree,
			   size_t elements)
{
	rbtree_t *new_tree;
	rbnode_t *cur_node;
	rbnode_t *move_node;
	size_t count = 0;

	new_tree = rbtree_create(tree->cmp);

	cur_node = rbtree_first(tree);
	while (count < elements && cur_node != RBTREE_NULL) {
		move_node = rbtree_delete(tree, cur_node->key);
		(void)rbtree_insert(new_tree, move_node);
		cur_node = rbtree_first(tree);
		count++;
	}

	return new_tree;
}

/** recursive descent traverse */
void 
traverse_post(void (*func)(rbnode_t*, void*), void* arg, 
	rbnode_t* node)
{
	if(!node || node == RBTREE_NULL)
		return;
	/* recurse */
	traverse_post(func, arg, node->left);
	traverse_post(func, arg, node->right);
	/* call user func */
	(*func)(node, arg);
}

void 
traverse_postorder(rbtree_t* tree, 
	void (*func)(rbnode_t*, void*), void* arg)
{
	traverse_post(func, arg, tree->root);
}

/*
 * add all node from the second tree to the first (removing them from the
 * second), and fix up nsec(3)s if present
 */
void
rbtree_join(rbtree_t *tree1, rbtree_t *tree2)
{
	traverse_postorder(tree2, rbtree_insert_vref, tree1);
}

int ipv6cmp(const void* addr1, const void* addr2)
{
	return memcmp(addr1, addr2, 16);
}

int ipv6netcmp(const void* addr1, const void* addr2)
{
	return memcmp(addr1, addr2, 6);
}

int ipv4cmp(const void* addr1, const void* addr2)
{
	return memcmp(addr1, addr2, 4);
}

struct pcap_file_header {
	uint32_t magic;
	uint16_t version_major;
	uint16_t version_minor;
	int32_t  thiszone;
	int32_t  sigfigs;
	int32_t  snaplen;
	int32_t  linktype;
};
struct pcap_pkthdr {
	uint32_t sec;
	uint32_t usec;
	uint32_t caplen;
	uint32_t len;
};

rbtree_t*  ipv6nets  = NULL;
rbtree_t*  ipv6nodes = NULL;
uint32_t n_ipv6nets  = 0;
uint32_t n_ipv6nodes = 0;

void lookup_and_replace6(uint8_t* ipv6, uint16_t ipv6type)
{
	uint32_t  ipv6net;
	uint32_t  ipv6node;
	rbnode_t* node;

	ipv6type = ((ipv6type & 15) << 12) 
		 | ((ipv6type & 15) <<  8)
		 | ((ipv6type & 15) <<  4)
		 |  (ipv6type & 15);
	node = rbtree_search(ipv6nets, ipv6);
	if (node) {
		ipv6net = node->value;
	} else {
		node = malloc(sizeof(rbnode_t));
		if (! node) {
			fprintf(stderr, "mem allocation error\n");
			exit(EXIT_FAILURE);
		}
		memcpy(node->key, ipv6, 6);
		ipv6net = node->value = n_ipv6nets++;
		rbtree_insert(ipv6nets, node);
	}
	node = rbtree_search(ipv6nodes, ipv6);
	if (node) {
		ipv6node = node->value;
	} else {
		node = malloc(sizeof(rbnode_t));
		if (! node) {
			fprintf(stderr, "mem allocation error\n");
			exit(EXIT_FAILURE);
		}
		memcpy(node->key, ipv6, 16);
		ipv6node = node->value = n_ipv6nodes++;
		rbtree_insert(ipv6nodes, node);
	}

	/* anonymize */
	ipv6[ 0] = 0xca;
	ipv6[ 1] = 0xfe;
	ipv6[ 2] = (ipv6net  & 0xff000000) >> 24;
	ipv6[ 3] = (ipv6net  & 0x00ff0000) >> 16;
	ipv6[ 4] = (ipv6net  & 0x0000ff00) >>  8;
	ipv6[ 5] =  ipv6net  & 0x000000ff;
	ipv6[ 6] = (ipv6type & 0xff00) >> 8;
	ipv6[ 7] =  ipv6type & 0x00ff;
	ipv6[ 8] = (ipv6node & 0xff000000) >> 24;
	ipv6[ 9] = (ipv6node & 0x00ff0000) >> 16;
	ipv6[10] = (ipv6node & 0x0000ff00) >>  8;
	ipv6[11] =  ipv6node & 0x000000ff;
	ipv6[12] = 0;
	ipv6[13] = 0;
	ipv6[14] = 0;
	ipv6[15] = 0;
}

rbtree_t*  ipv4nodes = NULL;
uint32_t n_ipv4nodes = 0;

void lookup_and_replace4(uint8_t* ipv4, uint16_t ipv4type)
{
	uint32_t  ipv4node;
	rbnode_t* node;

	node = rbtree_search(ipv4nodes, ipv4);
	if (node) {
		ipv4node = node->value;
	} else {
		node = malloc(sizeof(rbnode_t));
		if (! node) {
			fprintf(stderr, "mem allocation error\n");
			exit(EXIT_FAILURE);
		}
		memcpy(node->key, ipv4, 4);
		ipv4node = node->value = n_ipv4nodes++;
		rbtree_insert(ipv4nodes, node);
	}
	if (ipv4type == 2)
		ipv4node |= 0x80000000;
	else
		ipv4node &= 0x7FFFFFFF;

	/* anonymize */
	ipv4[0] = (ipv4node & 0xff000000) >> 24;
	ipv4[1] = (ipv4node & 0x00ff0000) >> 16;
	ipv4[2] = (ipv4node & 0x0000ff00) >>  8;
	ipv4[3] =  ipv4node & 0x000000ff;
}

int main(int argc, char** argv)
{
	FILE* in, *out;
	struct pcap_file_header file_header;
	struct pcap_pkthdr pkthdr;
	size_t sz, hsz, hsz2;
	uint8_t buf[16384];
	uint16_t ethertype;
	uint16_t src_port;
	uint16_t dst_port;

	uint8_t* router;
	uint8_t* dst_ns;
	uint8_t* src_ns;
	uint8_t* client;

	/* Handle arguments
	 */
	if (argc != 3) {
		fprintf(stderr, "%s in.pcap out.anonymized.pcap\n", *argv);
		return 1;
	}
	if (argv[1][0] == '-' && argv[1][1] == 0) {
		in = stdin;
	} else {
		in = fopen(argv[1], "r");
		if (! in) {
			perror("could not open input");
			exit(EXIT_FAILURE);
		}
	}
	if (argv[2][0] == '-' && argv[2][1] == 0) {
		out = stdout;
	} else {
		out = fopen(argv[2], "w");
		if (! in) {
			perror("could not open output");
			exit(EXIT_FAILURE);
		}
	}
	
	/* Check and copy header
	 */
	sz = fread(&file_header, sizeof(file_header), 1, in);
	if (sz < 1) {
		perror("could not read file header");
		exit(EXIT_FAILURE);
	}
	if (file_header.magic == (uint32_t)0xd4c3b2a1) {
		fprintf(stderr, "cannot handle different byte orders yet\n");
		exit(EXIT_FAILURE);
	}
	if (file_header.magic != 0xa1b2c3d4) {
		fprintf(stderr, "input is not in pcap format\n");
		exit(EXIT_FAILURE);
	}
	sz = fwrite(&file_header, sizeof(file_header), 1, out);
	if (sz < 1) {
		perror("could not write file header");
		exit(EXIT_FAILURE);
	}
	ipv6nets  = rbtree_create(ipv6netcmp);
	ipv6nodes = rbtree_create(ipv6cmp);
	ipv4nodes = rbtree_create(ipv4cmp);

	/* Modify and copy packets
	 */
	while (! feof(in)) {
		sz = fread(&pkthdr, sizeof(pkthdr), 1, in);
		if (sz < 1) {
			break;
		}
		sz = fread(buf, pkthdr.caplen, 1, in);
		if (sz < 1) {
			break;
		}
		ethertype = buf[12] << 8 | buf[13];

		switch (ethertype) {
		case 0x0800: /* IPv4 */

			if ((hsz = (buf[14] & 0x0F) * 4) < 20)
				hsz = 20;

			/* UDP || TCP */
			if (buf[23] == 17 || buf[23] == 6) { 
				src_port = buf[hsz + 14] << 8 | buf[hsz + 15];
				dst_port = buf[hsz + 16] << 8 | buf[hsz + 17];

				if (src_port == 53) { /* dns response */
					lookup_and_replace4(&buf[26], 2);
					lookup_and_replace4(&buf[30], 1);
				} else if (dst_port == 53) { /* dns request */
					lookup_and_replace4(&buf[26], 1);
					lookup_and_replace4(&buf[30], 2);
				} else { /* non-dns packet! */
					continue;
				}
				break;

			} else if (buf[23] != 1)
				continue;

			/* Assume sender is the server. */
			lookup_and_replace4(&buf[26], 2);
			lookup_and_replace4(&buf[30], 1);

			if (buf[hsz + 14] != 3 && buf[hsz + 14] !=  4 &&
			    buf[hsz + 14] != 5 && buf[hsz + 14] != 11)
				/* ICMP without IP header payload */
				break;

			/* ICMP with IP header payload
			 * Check if it involves DNS traffic and anonimize
			 * accordingly.
			 */

			/* Non UDP or TCP payload, continue */
			if (buf[hsz + 31] != 17 && buf[hsz + 31] != 6)
				continue;

			if ((hsz2 = (buf[hsz + 22] & 0x0F) * 4) < 20)
				hsz2 = 20;

			src_port = buf[hsz + 22 + hsz2] << 8 | buf[hsz + 23 + hsz2];
			dst_port = buf[hsz + 24 + hsz2] << 8 | buf[hsz + 25 + hsz2];


			if (src_port == 53) { /* dns response */
				lookup_and_replace4(&buf[hsz + 34], 2);
				lookup_and_replace4(&buf[hsz + 38], 1);
			} else if (dst_port == 53) { /* dns request */
				lookup_and_replace4(&buf[hsz + 34], 1);
				lookup_and_replace4(&buf[hsz + 38], 2);
			} else { /* non-dns payload! */
				continue;
			}
			break;

		case 0x86DD: /* IPv6 */

			if (buf[20] == 58) { /* Next header == IPv6-ICMP */
				if (buf[54] >= 100) {
					/* ICMPv6 type without payload */
					continue;
				}
				router = &buf[22];
				dst_ns = &buf[38];
				src_ns = &buf[70];
				client = &buf[86];

				lookup_and_replace6(client, 1);
				lookup_and_replace6(src_ns, 2);
				lookup_and_replace6(dst_ns, 2);
				lookup_and_replace6(router, 3);
			} else if (buf[20] == 17 || buf[20] == 6) {
				/* UDP || TCP */

				src_port = buf[54] << 8 | buf[55];
				dst_port = buf[56] << 8 | buf[57];

				if (src_port == 53) { /* dns response */
					lookup_and_replace6(&buf[22], 2);
					lookup_and_replace6(&buf[38], 1);
				} else if (dst_port == 53) { /* dns request */
					lookup_and_replace6(&buf[22], 1);
					lookup_and_replace6(&buf[38], 2);
				} else { /* non-dns packet! */
					continue;
				}
			} else if (buf[20] == 44) { /* Next hdr == Fragment */
				/* To identify the role's of the IP addresses
				 * for fragments except the first, they need toxi
				 * be correlated (or reassembled).
				 *
				 * Tag them with code 4444 for now.
				 */
				lookup_and_replace6(&buf[22], 4);
				lookup_and_replace6(&buf[38], 4);

				/* Though...
				 * when a fragmented ICMPv6 with type < 100
				 *
				 * is fragmented (impossible in theory),
				 * we should anonymize the payload...
				 */
				if (buf[54] == 58 && buf[56] == 0 &&
				    buf[57] ==  0 && buf[62] < 100 ) {
					/* First fragment for 
					 * IPv6-ICMP type < 100
					 */
					src_ns = &buf[78];
					client = &buf[94];
					lookup_and_replace6(client, 1);
					lookup_and_replace6(src_ns, 2);
				}
			} else {
				fprintf( stderr
				       , "Unknown next header protocol: %d\n"
				       , (int)buf[20]);
			}
			break;
		default:
			continue;
		}

		fprintf( stderr
		       , "pos: %ld, pkt, len: %u, caplen: %u\n"
		       , ftell(in) , pkthdr.len , pkthdr.caplen );

		sz = fwrite(&pkthdr, sizeof(pkthdr), 1, out);
		if (sz < 1) {
			perror("could not write packet header");
			exit(EXIT_FAILURE);
		}
		sz = fwrite(buf, pkthdr.caplen, 1, out);
		if (sz < 1) {
			perror("could not write packet");
			exit(EXIT_FAILURE);
		}
	}
	if (in != stdin) {
		fclose(in);
	}
	if (out != stdout) {
		fclose(out);
	}
	return 0;
}

