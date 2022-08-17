/* 
* By using this file, you are agreeing to this product's EULA
*
* This product can be obtained in https://github.com/jespb/C-StdGP
*
* Copyright ©2021-2022 J. E. Batista
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <individual.h>
#include <node.h>

extern struct individual_t Individual;

struct individual_t* individual_create(int n_op, char** operators, int n_term, 
	char** terminals, int max_depth){
	struct individual_t* ret = malloc( sizeof(Individual) );

	ret->n_op = n_op;
	ret->operators = operators;

	ret->n_term = n_term;
	ret->terminals = terminals;

	ret->max_depth = max_depth;

	ret->head = node_create(ret->n_op, ret->n_term, ret->max_depth, 0);

	ret->size = -1;
	ret->depth = -1;
	ret->fitness = -1;
	ret->fit_calculated = 0;

	ret->predictions_f = NULL;
	ret->predictions_c = NULL;

	return ret;
}

void change_head(struct individual_t* ind, struct node_t* n){
	if( ind->head != NULL) node_destroy(ind->head);
	if( ind->predictions_f != NULL) free(ind->predictions_f);
	if( ind->predictions_c != NULL) free(ind->predictions_c);

	ind->head = n;
}

struct individual_t* clone_individual(struct individual_t* ind, int copy_head){
	struct individual_t* ret = malloc( sizeof(Individual) );

	ret->n_op = ind->n_op;
	ret->operators = ind->operators;

	ret->n_term = ind->n_term;
	ret->terminals = ind->terminals;

	ret->max_depth = ind->max_depth;

	ret->head = copy_head ? clone( ind->head ):NULL;

	ret->size = -1;
	ret->depth = -1;
	ret->fitness = -1;
	ret->fit_calculated = 0;

	ret->predictions_f = NULL;
	ret->predictions_c = NULL;

	return ret;
}

/*
* The individuals MUST have calculates the fitness beforehand
* bigger fitness = better
*/
int compare(const void * a, const void * b){
	struct individual_t* this  = ((struct individual_t*) ((const long int*) a)[0]);
	struct individual_t* other = ((struct individual_t*) ((const long int*) b)[0]);
	if (this->fitness > other->fitness ){
		return -1;
	}else if(this->fitness < other->fitness ){
		return 1;
	}else{
		//return 0;
		return getSize(this) - getSize(other);
	}
}


double getFitness(struct individual_t* ind, int n_samples, double** training_X, double*  training_Y){

	if (!ind->fit_calculated){
		ind->n_samples = n_samples;
		ind->training_X = training_X;
		ind->training_Y = training_Y;

		int isFitnessRMSE = 0;
		if (isFitnessRMSE){
			ind->fitness = getRMSE(ind, n_samples, training_X, training_Y) * (-1) +1; // negative for fitness, +1 to ser maximum as 1 (like accuracy) 
		}else{
			ind->fitness = getAccuracy(ind, n_samples, training_X, training_Y);
		}

		ind->fit_calculated = 1;
	}
	return ind->fitness;
}

double* getTrainingValuePredictions(struct individual_t* ind){
	return ind->predictions_f;
}

int* getTrainingClassPredictions(struct individual_t* ind){
	return ind->predictions_c;
}

double getAccuracy(struct individual_t* ind, int n_samples, double** X, double* Y){
	int* pred_c = predict_individual(ind, n_samples, X);

	int hits = 0;
	for (int i = 0; i < n_samples; ++i){
		if (pred_c[i] == Y[i]){
			hits++;
		}
	}

	free(pred_c);

	return hits*1.0/n_samples;

}

double getRMSE(struct individual_t* ind, int n_samples, double** X, double* Y){
	double* pred = calculate_individual(ind, n_samples, X);

	double rmse = 0.0;
	for (int i = 0; i < n_samples; ++i){
		rmse += pow(pred[i] - Y[i], 2);
	}

	free(pred);

	return pow(rmse/n_samples, 0.5);
}

double* calculate_individual(struct individual_t* ind, int n_samples, double** X){
	double* ret = malloc( n_samples * sizeof(double));

	for (int i = 0; i < n_samples; ++i){
		ret[i] = calculate(ind->head, X[i]);
	}

	return ret;
}

int* predict_individual(struct individual_t* ind, int n_samples, double** X){
	int* ret = malloc( n_samples * sizeof(int));

	for (int i = 0; i < n_samples; ++i){
		ret[i] = calculate(ind->head, X[i]) > 0.5? 1:0;
	}

	return ret;
}


int getSize(struct individual_t* ind){
	if (ind->size == -1){
		ind->size = getSize_node(ind->head);
	}
	return ind->size;
}

int getDepth(struct individual_t* ind){
	if (ind->depth == -1){
		ind->depth = getDepth_node(ind->head);
	}
	return ind->depth;
}

struct node_t* getHead(struct individual_t* ind){
	return ind->head;
}

char* toString_individual(struct individual_t* ind){
	return toString_node(ind->head, ind->operators, ind->terminals);
}


void individual_destroy(struct individual_t* ind){
	if( ind->head != NULL)	node_destroy(ind->head);
	if( ind->predictions_f != NULL) free(ind->predictions_f);
	if( ind->predictions_c != NULL) free(ind->predictions_c);

	free(ind);
}

