/* 
* By using this file, you are agreeing to this product's EULA
*
* This product can be obtained in https://github.com/jespb/C-StdGP
*
* Copyright ©2021-2022 J. E. Batista
*/


#ifndef _NODE_H
#define _NODE_H

/* Estrutura que define os dados.  
 */
struct node_t {
	struct node_t* left;
	struct node_t* right;
	int value; /* Tamanho do bloco de dados */
};


struct node_t * node_create( int n_operators,  int n_terminals, int max_depth, _Bool full );
struct node_t* clone(struct node_t* node);

void copy(struct node_t* node, struct node_t* other);
void swap(struct node_t* node, struct node_t* other);

double calculate(struct node_t* node, double* sample);

int getSize_node(struct node_t *node);
int getDepth_node(struct node_t *node);

struct node_t * getRandomNode(struct node_t * node, int value);

void printNode(struct node_t *node, char** ops, char** term);
char* toString_node( struct node_t *node, char** operators, char** terminals);

void node_destroy(struct node_t *node);


#endif
