/* 
* By using this file, you are agreeing to this product's EULA
*
* This product can be obtained in https://github.com/jespb/C-StdGP
*
* Copyright ©2021-2022 J. E. Batista
*/


#ifndef _INDIVIDUAL_H
#define _INDIVIDUAL_H

struct individual_t {
	int n_samples;
	double** training_X;
	double*  training_Y;

	int n_op;
	char** operators;
	
	int n_term;
	char** terminals;

	int max_depth;

	struct node_t* head;

	int size;
	int depth;

	double* predictions_f;
	int* predictions_c;

	double fitness;
	int fit_calculated;
};


struct individual_t* individual_create(int n_op, char** operators, int n_term, char** terminals, int max_depth);

void change_head(struct individual_t* ind, struct node_t* n);
struct individual_t* clone_individual(struct individual_t* ind, int copy_head);

int compare(const void * a, const void * b);
/*
int compare( const void* a, const void* b){
     int int_a = * ( (int*) a );
     int int_b = * ( (int*) b );

     if ( int_a == int_b ) return 0;
     else if ( int_a < int_b ) return -1;
     else return 1;
}

qsort( a, 6, sizeof(int), compare )
*/


double getFitness(struct individual_t* ind, int n_samples, double** training_X, double*  training_Y);

double* getTrainingValuePredictions(struct individual_t* ind);
int* getTrainingClassPredictions(struct individual_t* ind);

double getAccuracy(struct individual_t* ind, int n_samples, double** X, double* Y);
double getWaF(struct individual_t* ind, int n_samples, double** X, double* Y);
double getKappa(struct individual_t* ind, int n_samples, double** X, double* Y);
double getRMSE(struct individual_t* ind, int n_samples, double** X, double* Y);

double* calculate_individual(struct individual_t* ind, int n_samples, double** X);
int* predict_individual(struct individual_t* ind, int n_samples, double** X);

int getSize(struct individual_t* ind);
int getDepth(struct individual_t* ind);

struct node_t* getHead(struct individual_t* ind);

char* toString_individual(struct individual_t* ind);

void individual_destroy(struct individual_t* ind);

#endif


