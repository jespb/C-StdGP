/* 
* By using this file, you are agreeing to this product's EULA
*
* This product can be obtained in https://github.com/jespb/C-StdGP
*
* Copyright ©2021-2022 J. E. Batista
*/


#ifndef _StdGP_H
#define _StdGP_H

struct StdGP_t {
	int n_training_samples;
	double** training_X;
	double*  training_Y;

	int n_test_samples;
	double** test_X;
	double*  test_Y;

	int n_operators;
	char** operators;

	int n_terminals;
	char** terminals;

	int max_depth;
	int population_size;
	int max_generation;
	int tournament_size;
	int elitism_size;
	int limit_depth;
	int threads;

	_Bool verbose;

	struct individual_t** population;
	struct individual_t* bestIndividual;

	int currentGeneration;

	double* trainingAccuracyOverTime;
	double* testAccuracyOverTime; 
	double* trainingWaFOverTime;
	double* testWaFOverTime; 
	double* trainingKappaOverTime;
	double* testKappaOverTime; 
	double* trainingRMSEOverTime; 
	double* testRMSEOverTime;
	double* fitnessOverTime;
	int* sizeOverTime; 
	double* timeOverTime; 
};

struct StdGP_t* stdgp_create(int n_operators, char** operators, int n_terminals,
	char** terminals, int max_depth, int population_size, int max_generation, 
	int tournament_size, int elitism_size, int limit_depth, int threads, _Bool verbose,
	int n_training_samples, double** training_X, double* training_Y,
	int n_test_samples, double** test_X, double* test_Y);

void stdgp_destroy(struct StdGP_t* stdgp);

char* toString_StdGP(struct StdGP_t* stdgp);

_Bool stoppingCriteria(struct StdGP_t* stdgp);

void fit(struct StdGP_t* stdgp);

void nextGeneration(struct StdGP_t* stdgp);


double* predict_regression(struct StdGP_t* stdgp, int n_samples, double** x);
int* predict_classification(struct StdGP_t* stdgp, int n_samples, double** x);

struct individual_t* getBestIndividual(struct StdGP_t* stdgp);

double* getTrainingAccuracyOverTime(struct StdGP_t* stdgp);
double* getTestAccuracyOverTime(struct StdGP_t* stdgp);
double* getTrainingRMSEOverTime(struct StdGP_t* stdgp);
double* getTestRMSEOverTime(struct StdGP_t* stdgp);
double* getFitnessOverTime(struct StdGP_t* stdgp);
int* getSizeOverTime(struct StdGP_t* stdgp);
double* getTimeOverTime(struct StdGP_t* stdgp);

#endif
