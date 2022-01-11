#include <stdio.h>
#include <stdlib.h>

#include <node.h>
#include <individual.h>
#include <geneticoperators.h>

struct individual_t** getElite(int p_size, struct individual_t** population, int e_size){
	struct individual_t** ret = calloc( e_size +1 , sizeof(struct individual_t*) );
	for (int i = 0; i < e_size; ++i){	
		ret[i] = clone_individual( population[i] , 1);
	}
	return ret;
}

struct individual_t* tournament(int population_size, struct individual_t** population, int t_size){
	int picked = rand() % population_size;
	for (int i = 1; i < t_size; ++i){
		int p = rand() % population_size;
		if (p < picked){
			picked = p;
		}
	}
	return population[picked];
}


struct individual_t** getOffspring(int p_size, struct individual_t** population, int t_size){
	int isCross = rand()%2;
	struct individual_t** ret;

	if (isCross){
		ret = STXO(p_size, population, t_size);
	}else{
		ret = STMUT(p_size, population, t_size);		
	}

	return ret;
}


struct individual_t** STXO(int p_size, struct individual_t** population, int t_size){
	struct individual_t* i1 = tournament(p_size, population, t_size);
	struct individual_t* i2 = tournament(p_size, population, t_size);

	struct node_t* h1 = clone(getHead(i1));
	struct node_t* h2 = clone(getHead(i2));

	struct node_t* r1 = getRandomNode(h1, -1);
	struct node_t* r2 = getRandomNode(h2, -1);

	swap(r1,r2);

	struct individual_t** ret = calloc( 2 +1,  sizeof(struct individual_t*) );

	ret[0] = clone_individual(i1, 0);
	change_head(ret[0], h1);
	//node_destroy(h1);

	ret[1] = clone_individual(i2, 0);
	change_head(ret[1], h2);
	//node_destroy(h2);

	return ret;
}

struct individual_t** STMUT(int p_size, struct individual_t** population, int t_size){
	struct individual_t* i1 = tournament(p_size, population, t_size);
	
	struct node_t* h1 = clone(getHead(i1));
	
	struct node_t* r1 = getRandomNode(h1, -1);

	struct node_t* nt = node_create(i1->n_op, i1->n_term, i1->max_depth, 0);

	swap(r1,nt); // swap instead of copy so it's easier to destroy the old branch

	node_destroy(nt);

	struct individual_t** ret = calloc( 1+1, sizeof(struct individual_t*) );

	ret[0] = clone_individual(i1, 0);
	change_head(ret[0], h1);
	//node_destroy(h1);

	return ret;
}



struct individual_t** discardDeep(struct individual_t** population, int max_depth){
	int MAX_CHILDREN = 2;
	struct individual_t** ret = calloc( MAX_CHILDREN +1 ,  sizeof(struct individual_t*) );
	int track = 0;
	
	for (int i = 0; population[i] != NULL; i++){
	 	if ( getDepth(population[i]) > max_depth ){
	 		individual_destroy(population[i]);
	 	}else{
	 		ret[track++] = population[i];
	 	}
	} 

	return ret;
}

