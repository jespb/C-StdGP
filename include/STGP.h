#ifndef _STGP_H
#define _STGP_H

struct STGP_t {
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
	double* trainingRMSEOverTime; 
	double* testRMSEOverTime;
	double* fitnessOverTime;
	int* sizeOverTime; 
	double* timeOverTime; 
};

struct STGP_t* stgp_create(int n_operators, char** operators, int n_terminals,
	char** terminals, int max_depth, int population_size, int max_generation, 
	int tournament_size, int elitism_size, int limit_depth, int threads, _Bool verbose,
	int n_training_samples, double** training_X, double* training_Y,
	int n_test_samples, double** test_X, double* test_Y);

void stgp_destroy(struct STGP_t* stgp);

char* toString_STGP(struct STGP_t* stgp);

_Bool stoppingCriteria(struct STGP_t* stgp);

void fit(struct STGP_t* stgp);

void nextGeneration(struct STGP_t* stgp);


double* predict_regression(struct STGP_t* stgp, int n_samples, double** x);
int* predict_classification(struct STGP_t* stgp, int n_samples, double** x);

struct individual_t* getBestIndividual(struct STGP_t* stgp);

double* getTrainingAccuracyOverTime(struct STGP_t* stgp);
double* getTestAccuracyOverTime(struct STGP_t* stgp);
double* getTrainingRMSEOverTime(struct STGP_t* stgp);
double* getTestRMSEOverTime(struct STGP_t* stgp);
double* getFitnessOverTime(struct STGP_t* stgp);
int* getSizeOverTime(struct STGP_t* stgp);
double* getTimeOverTime(struct STGP_t* stgp);

#endif
