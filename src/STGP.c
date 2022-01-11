#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <STGP.h>
#include <individual.h>
#include <geneticoperators.h>

struct STGP_t* stgp_create(int n_operators, char** operators, int n_terminals,
	char** terminals, int max_depth, int population_size, int max_generation, 
	int tournament_size, int elitism_size, int limit_depth, int threads, _Bool verbose,
	int n_training_samples, double** training_X, double* training_Y,
	int n_test_samples, double** test_X, double* test_Y){

	struct STGP_t* stgp = malloc( sizeof(struct STGP_t) );

	stgp->n_training_samples = n_training_samples;
	stgp->training_X = training_X;
	stgp->training_Y = training_Y;

	stgp->n_test_samples = n_test_samples;
	stgp->test_X = test_X;
	stgp->test_Y = test_Y;

	stgp->n_operators = n_operators;
	stgp->operators = operators;

	stgp->n_terminals = n_terminals;
	stgp->terminals = terminals;

	stgp->max_depth = max_depth;
	stgp->population_size = population_size;
	stgp->max_generation = max_generation;
	stgp->tournament_size = tournament_size;
	stgp->elitism_size = elitism_size;
	stgp->limit_depth = limit_depth;
	stgp->threads = threads;
	stgp->verbose = verbose;

	stgp->trainingAccuracyOverTime = calloc( max_generation +1, sizeof(double));
	stgp->testAccuracyOverTime = calloc( max_generation +1, sizeof(double));
	stgp->trainingRMSEOverTime = calloc( max_generation +1, sizeof(double));
	stgp->testRMSEOverTime = calloc( max_generation +1, sizeof(double));
	stgp->fitnessOverTime = calloc( max_generation +1, sizeof(double));
	stgp->sizeOverTime = calloc( max_generation +1, sizeof(double));
	stgp->timeOverTime = calloc( max_generation +1, sizeof(double));

	stgp->currentGeneration = 0;

	stgp->population = malloc( sizeof(struct individual_t*) * (population_size +1) );
	for (int i = 0; i < population_size; ++i){
		stgp->population[i] = individual_create(n_operators, operators, n_terminals, terminals, max_depth);
	}

	
	stgp->bestIndividual = clone_individual( stgp->population[0] ,1);

}

void stgp_destroy(struct STGP_t* stgp){
	free(stgp->trainingAccuracyOverTime);
	free(stgp->testAccuracyOverTime);
	free(stgp->trainingRMSEOverTime);
	free(stgp->testRMSEOverTime);
	free(stgp->fitnessOverTime);
	free(stgp->sizeOverTime);
	free(stgp->timeOverTime);

	for (int i = 0; i < stgp->population_size; i++){
		individual_destroy(stgp->population[i]);
	}
	free(stgp->population);

	individual_destroy(stgp->bestIndividual);

	free(stgp);
}



_Bool stoppingCriteria(struct STGP_t* stgp){
	_Bool genLimit = stgp->currentGeneration >= stgp->max_generation;
	_Bool perfectTraining = getFitness(stgp->bestIndividual, stgp->n_training_samples, stgp->training_X, stgp->training_Y) == 1;
		
	return genLimit || perfectTraining;
}


void fit(struct STGP_t* stgp){
	if (stgp->verbose){
		printf("Training a model with the following parameters:\n");
		printf("  > Population Size: %d\n", stgp->population_size);
		printf("  > Max Generation: %d\n", stgp->max_generation);
		printf("  > Tournament Size: %d\n", stgp->tournament_size);
		printf("  > Elitism Size: %d\n", stgp->elitism_size);
		printf("  > Depth Limit: %d\n", stgp->limit_depth);
		printf("  > Threads (not implemented): %d\n", stgp->threads);
		printf("\n");
		printf("Running Log:\n");
	}

	while (stgp->currentGeneration < stgp->max_generation){
		long duration = 0;

		if (!stoppingCriteria(stgp)) {
			if (stgp->verbose && stgp->currentGeneration%1==0) printf("Generation %3d:\n", stgp->currentGeneration );

    		clock_t before = clock();
			nextGeneration(stgp);
    		clock_t after = clock();
    		duration = (after-before)*1000.0/CLOCKS_PER_SEC;

			if (stgp->verbose && stgp->currentGeneration%1==0) printf("  > Execution Time: %ldms\n", duration );
    	}

    	stgp->trainingAccuracyOverTime[ stgp->currentGeneration ] = getAccuracy(stgp->bestIndividual, stgp->n_training_samples, stgp->training_X, stgp->training_Y);
		stgp->testAccuracyOverTime[ stgp->currentGeneration ] = getAccuracy(stgp->bestIndividual, stgp->n_test_samples, stgp->test_X, stgp->test_Y);
		stgp->trainingRMSEOverTime[ stgp->currentGeneration ] = getRMSE(stgp->bestIndividual, stgp->n_training_samples, stgp->training_X, stgp->training_Y);
		stgp->testRMSEOverTime[ stgp->currentGeneration ] = getRMSE(stgp->bestIndividual, stgp->n_test_samples, stgp->test_X, stgp->test_Y);
		stgp->fitnessOverTime[ stgp->currentGeneration ] = getFitness(stgp->bestIndividual, stgp->n_training_samples, stgp->training_X, stgp->training_Y);
		stgp->sizeOverTime[ stgp->currentGeneration ] = getSize(stgp->bestIndividual);
		stgp->timeOverTime[ stgp->currentGeneration ] = duration/1000.0;

		//printf("%f\n", getAccuracy(stgp->bestIndividual, stgp->n_training_samples, stgp->training_X, stgp->training_Y));

    	stgp->currentGeneration++;
	}

}

/*
qsort( a, 6, sizeof(int), compare )

// USE this to sort the population
*/
void nextGeneration(struct STGP_t* stgp){
	// Calculate the individual's fitness
	for (int i = 0; i < stgp->population_size; ++i){
		getFitness(stgp->population[i], stgp->n_training_samples, stgp->training_X, stgp->training_Y );
		//printf("%ld\n", stgp->population[i]);
	}


	// Sorts the population
	qsort( stgp->population, stgp->population_size, sizeof(struct individual_t*), compare);



	// Updates the best individual
	if ( stgp->population[0]->fitness > stgp->bestIndividual->fitness){
		individual_destroy(stgp->bestIndividual);
		stgp->bestIndividual = clone_individual( stgp->population[0] ,1);
	}

	
	// Generates next generation
	struct individual_t** new_population = calloc( stgp->population_size +1, sizeof(struct individual_t*));
	struct individual_t** elite = getElite(stgp->population_size, stgp->population, stgp->elitism_size);
	for (int i = 0; i < stgp->elitism_size; ++i){
		new_population[i] = elite[i];
	}
	free(elite);
	
	
	for (int i = stgp->elitism_size; i < stgp->population_size; ){
		struct individual_t** offspring = getOffspring(stgp->population_size, stgp->population, stgp->tournament_size);
		struct individual_t** afterDiscard = discardDeep(offspring, stgp->limit_depth);
		free(offspring);

		int j = 0;
		while (afterDiscard[j] != NULL){
			if (i+j < stgp->population_size){
				new_population[i+j] = afterDiscard[j];
			}else{
				individual_destroy(afterDiscard[j]);
			}
			j++;
		}

		free(afterDiscard);

		i+=j;
	}
	
	
	// Deletes and replaces the previous generation
	for (int i = 0; i < stgp->population_size; ++i){
		individual_destroy(stgp->population[i]);
	}
	free(stgp->population);
	stgp->population = new_population;
	

	// Generation Log
	if (stgp->verbose && stgp->currentGeneration%1==0) printf("  > Training: %.4f  // Test: %.4f      ", 
		getAccuracy(stgp->bestIndividual, stgp->n_training_samples, stgp->training_X, stgp->training_Y), 
		getAccuracy(stgp->bestIndividual, stgp->n_test_samples, stgp->test_X, stgp->test_Y) 
		);

	

}


/*


		# Update best individual
		if self.population[0] > self.bestIndividual:
			self.bestIndividual = self.population[0]

		# Generating Next Generation
		newPopulation = []
		newPopulation.extend(getElite(self.population, self.elitism_size))
		while len(newPopulation) < self.population_size:
			offspring = getOffspring(self.population, self.tournament_size)
			offspring = discardDeep(offspring, self.limit_depth)
			newPopulation.extend(offspring)
		self.population = newPopulation[:self.population_size]



		# Debug
		if self.verbose and self.currentGeneration%5==0:
			if not self.Te_x is None:
				print("   > Gen #"+str(self.currentGeneration)+":  Tr-Acc: "+ "%.6f" %self.bestIndividual.getAccuracy(self.Tr_x, self.Tr_y)+" // Te-Acc: "+ "%.6f" %self.bestIndividual.getAccuracy(self.Te_x, self.Te_y) + " // Time: " + str(end- begin) )
			else:
				print("   > Gen #"+str(self.currentGeneration)+":  Tr-Acc: "+ "%.6f" %self.bestIndividual.getAccuracy(self.Tr_x, self.Tr_y))


*/





double* predict_regression(struct STGP_t* stgp, int n_samples, double** x){
	return calculate_individual(stgp->bestIndividual, n_samples, x);
}

int* predict_classification(struct STGP_t* stgp, int n_samples, double** x){
	return predict_individual(stgp->bestIndividual, n_samples, x);
}

struct individual_t* getBestIndividual(struct STGP_t* stgp){
	return stgp->bestIndividual;
}

double* getTrainingAccuracyOverTime(struct STGP_t* stgp){
	return stgp->trainingAccuracyOverTime;
}

double* getTestAccuracyOverTime(struct STGP_t* stgp){
	return stgp->testAccuracyOverTime;
}

double* getTrainingRMSEOverTime(struct STGP_t* stgp){
	return stgp->trainingRMSEOverTime;
}

double* getTestRMSEOverTime(struct STGP_t* stgp){
	return stgp->testRMSEOverTime;
}

double* getFitnessOverTime(struct STGP_t* stgp){
	return stgp->fitnessOverTime;
}

int* getSizeOverTime(struct STGP_t* stgp){
	return stgp->sizeOverTime;
}

double* getTimeOverTime(struct STGP_t* stgp){
	return stgp->timeOverTime;
}

char* toString_STGP(struct STGP_t* stgp){
	return toString_individual(stgp->bestIndividual);
}






