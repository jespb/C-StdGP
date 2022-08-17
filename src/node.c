/* 
* By using this file, you are agreeing to this product's EULA
*
* This product can be obtained in https://github.com/jespb/C-StdGP
*
* Copyright ©2021-2022 J. E. Batista
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <node.h>


extern struct node_t Node;

struct node_t * node_create( int n_operators, int n_terminals, int max_depth, _Bool full ){
	//printf("A node has %lu bytes.\n",sizeof(Node));
	
	struct node_t * nt;
	nt = malloc( sizeof(Node) );
	
	if (max_depth > 1 && (rand()%2==0 || full)){
		nt->left  = node_create(n_operators, n_terminals, max_depth-1, full);
		nt->right = node_create(n_operators, n_terminals, max_depth-1, full);
		nt->value = rand() % n_operators;
	}else{
		nt->left  = NULL;
		nt->right = NULL;
		nt->value = rand() % n_terminals;
	}

	return nt;
}


struct node_t* clone(struct node_t* node){
	struct node_t * nt;
	nt = malloc( sizeof(Node) );
	
	if (node->left == NULL){
		nt->left  = NULL;
		nt->right = NULL;
		nt->value = node->value;
	}else{
		nt->left  = clone(node->left);
		nt->right = clone(node->right);
		nt->value = node->value;
	}
	return nt;
}

void copy(struct node_t* node, struct node_t* other){
	node->left  = other->left;
	node->value = other->value;
	node->right = other->right;
}

void swap(struct node_t* node, struct node_t* other){
	struct node_t * nt = malloc( sizeof(Node) );

	copy(nt, node);
	copy(node, other);
	copy(other, nt);

	free(nt);
}

		
int getSize_node(struct node_t *node){
	if (node->left == NULL){
		return 1;
	}else{
		return 1 + getSize_node(node->left) + getSize_node(node->right);  
	}
}

int getDepth_node(struct node_t *node){
	if (node->left == NULL){
		return 1;
	}else{
		int dl = getDepth_node(node->left);
		int dr = getDepth_node(node->right);
		return dl>dr ? 1+dl : 1+dr ;
	}
}


double calculate(struct node_t* node, double*  sample){
	if (node->left == NULL){
		return sample[node->value];
	}else{
		switch(node->value){
			case 0: // +
				return calculate(node->left, sample) + calculate(node->right, sample);

			case 1: // -
				return calculate(node->left, sample) - calculate(node->right, sample);
			
			case 2: // *
				return calculate(node->left, sample) * calculate(node->right, sample);
			
			case 3:; // /
				double r = calculate(node->right, sample);
				if (r == 0) r = 1; // protected division
				return calculate(node->left, sample) / r;

		}
	}
}



struct node_t * getRandomNode(struct node_t * node, int value){
	if (value == -1){
		value = rand() % getSize_node(node);
	}
	if (value == 0){
		return node;
	}

	int left_size = getSize_node(node->left);
	if (value <= left_size){
		return getRandomNode(node->left,  value-1);
	}else{
		return getRandomNode(node->right, value-left_size-1);
	}
}



void printNode(struct node_t *node, char** ops, char** term){
	char* str = toString_node(node, ops, term);
	printf("%s\n", str);
	free(str);
}


char* toString_node( struct node_t *node, char** ops, char** term){
	if (node->left != NULL){
		char* l = toString_node(node->left, ops, term);
		char* r = toString_node(node->right, ops, term);

		char* ret = calloc(6 + strlen(ops[node->value]) + strlen(l) + strlen(r) + 1, sizeof(char) );
		strcat(ret, "( ");
		strcat(ret, l );
		strcat(ret, " ");
		strcat(ret, ops[node->value]);
		strcat(ret, " ");
		strcat(ret, r );
		strcat(ret, " )");

		free(l);
		free(r);
		
		return ret;
	}else{
		char* ret = calloc(strlen(term[node->value]) + 1, sizeof(char));
		strcat(ret, term[node->value]);
		return ret;
	}
}


void node_destroy(struct node_t *node){
	if(node->left != NULL){
		node_destroy(node->left);
		node_destroy(node->right);
		free(node);	
	}else{
		free(node);
	}
}
