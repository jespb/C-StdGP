/*
 * By using this file, you are agreeing to this product's EULA
 *
 * This product can be obtained in https://github.com/jespb/C-StdGP
 *
 * Copyright ©2021-2022 J. E. Batista
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <StdGP.h>
#include <geneticoperators.h>
#include <individual.h>

struct StdGP_t *stdgp_create(int n_operators, char **operators, int n_terminals,
                             char **terminals, int max_depth,
                             int population_size, int max_generation,
                             int tournament_size, int elitism_size,
                             int limit_depth, int threads, _Bool verbose,
                             int n_training_samples, double **training_X,
                             double *training_Y, int n_test_samples,
                             double **test_X, double *test_Y) {

  struct StdGP_t *stdgp = malloc(sizeof(struct StdGP_t));

  stdgp->n_training_samples = n_training_samples;
  stdgp->training_X = training_X;
  stdgp->training_Y = training_Y;

  stdgp->n_test_samples = n_test_samples;
  stdgp->test_X = test_X;
  stdgp->test_Y = test_Y;

  stdgp->n_operators = n_operators;
  stdgp->operators = operators;

  stdgp->n_terminals = n_terminals;
  stdgp->terminals = terminals;

  stdgp->max_depth = max_depth;
  stdgp->population_size = population_size;
  stdgp->max_generation = max_generation;
  stdgp->tournament_size = tournament_size;
  stdgp->elitism_size = elitism_size;
  stdgp->limit_depth = limit_depth;
  stdgp->threads = threads;
  stdgp->verbose = verbose;

  stdgp->trainingAccuracyOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->testAccuracyOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->trainingWaFOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->testWaFOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->trainingKappaOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->testKappaOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->trainingRMSEOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->testRMSEOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->fitnessOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->sizeOverTime = calloc(max_generation + 1, sizeof(double));
  stdgp->timeOverTime = calloc(max_generation + 1, sizeof(double));

  stdgp->currentGeneration = 0;

  stdgp->population =
      malloc(sizeof(struct individual_t *) * (population_size + 1));
  for (int i = 0; i < population_size; ++i) {
    stdgp->population[i] = individual_create(n_operators, operators,
                                             n_terminals, terminals, max_depth);
  }

  stdgp->bestIndividual = clone_individual(stdgp->population[0], 1);

  return stdgp;
}

void stdgp_destroy(struct StdGP_t *stdgp) {
  free(stdgp->trainingAccuracyOverTime);
  free(stdgp->testAccuracyOverTime);
  free(stdgp->trainingWaFOverTime);
  free(stdgp->testWaFOverTime);
  free(stdgp->trainingKappaOverTime);
  free(stdgp->testKappaOverTime);
  free(stdgp->trainingRMSEOverTime);
  free(stdgp->testRMSEOverTime);
  free(stdgp->fitnessOverTime);
  free(stdgp->sizeOverTime);
  free(stdgp->timeOverTime);

  for (int i = 0; i < stdgp->population_size; i++) {
    individual_destroy(stdgp->population[i]);
  }
  free(stdgp->population);

  individual_destroy(stdgp->bestIndividual);

  free(stdgp);
}

_Bool stoppingCriteria(struct StdGP_t *stdgp) {
  _Bool genLimit = stdgp->currentGeneration >= stdgp->max_generation;
  _Bool perfectTraining =
      getFitness(stdgp->bestIndividual, stdgp->n_training_samples,
                 stdgp->training_X, stdgp->training_Y) == 1;

  return genLimit || perfectTraining;
}

void fit(struct StdGP_t *stdgp) {
  if (stdgp->verbose) {
    printf("Training a model with the following parameters:\n");
    printf("  > Population Size: %d\n", stdgp->population_size);
    printf("  > Max Generation: %d\n", stdgp->max_generation);
    printf("  > Tournament Size: %d\n", stdgp->tournament_size);
    printf("  > Elitism Size: %d\n", stdgp->elitism_size);
    printf("  > Depth Limit: %d\n", stdgp->limit_depth);
    printf("  > Threads (not implemented): %d\n", stdgp->threads);
    printf("\n");
    printf("Running Log:\n");
  }

  while (stdgp->currentGeneration < stdgp->max_generation) {
    long duration = 0;

    if (!stoppingCriteria(stdgp)) { 
      if (stdgp->verbose && stdgp->currentGeneration % 1 == 0)
        printf("Generation %3d:", stdgp->currentGeneration);

      clock_t before = clock();
      nextGeneration(stdgp);
      clock_t after = clock();
      duration = (after - before) * 1000.0 / CLOCKS_PER_SEC;

      if (stdgp->verbose && stdgp->currentGeneration % 1 == 0)
        printf("  > Execution Time: %ldms\n", duration);
    }

    stdgp->trainingAccuracyOverTime[stdgp->currentGeneration] =
        getAccuracy(stdgp->bestIndividual, stdgp->n_training_samples,
                    stdgp->training_X, stdgp->training_Y);
    stdgp->testAccuracyOverTime[stdgp->currentGeneration] =
        getAccuracy(stdgp->bestIndividual, stdgp->n_test_samples, stdgp->test_X,
                    stdgp->test_Y);
    stdgp->trainingWaFOverTime[stdgp->currentGeneration] =
        getWaF(stdgp->bestIndividual, stdgp->n_training_samples,
               stdgp->training_X, stdgp->training_Y);
    stdgp->testWaFOverTime[stdgp->currentGeneration] =
        getWaF(stdgp->bestIndividual, stdgp->n_test_samples, stdgp->test_X,
               stdgp->test_Y);
    stdgp->trainingKappaOverTime[stdgp->currentGeneration] =
        getKappa(stdgp->bestIndividual, stdgp->n_training_samples,
                 stdgp->training_X, stdgp->training_Y);
    stdgp->testKappaOverTime[stdgp->currentGeneration] =
        getKappa(stdgp->bestIndividual, stdgp->n_test_samples, stdgp->test_X,
                 stdgp->test_Y);
    stdgp->trainingRMSEOverTime[stdgp->currentGeneration] =
        getRMSE(stdgp->bestIndividual, stdgp->n_training_samples,
                stdgp->training_X, stdgp->training_Y);
    stdgp->testRMSEOverTime[stdgp->currentGeneration] =
        getRMSE(stdgp->bestIndividual, stdgp->n_test_samples, stdgp->test_X,
                stdgp->test_Y);
    stdgp->fitnessOverTime[stdgp->currentGeneration] =
        getFitness(stdgp->bestIndividual, stdgp->n_training_samples,
                   stdgp->training_X, stdgp->training_Y);
    stdgp->sizeOverTime[stdgp->currentGeneration] =
        getSize(stdgp->bestIndividual);
    stdgp->timeOverTime[stdgp->currentGeneration] = duration / 1000.0;

    // printf("%f\n", getAccuracy(stdgp->bestIndividual,
    // stdgp->n_training_samples, stdgp->training_X, stdgp->training_Y));

    stdgp->currentGeneration++;
  }
}

/*
qsort( a, 6, sizeof(int), compare )

// USE this to sort the population
*/
void nextGeneration(struct StdGP_t *stdgp) {
  // Calculate the individual's fitness
  for (int i = 0; i < stdgp->population_size; ++i) {
    getFitness(stdgp->population[i], stdgp->n_training_samples,
               stdgp->training_X, stdgp->training_Y);
    // printf("%ld\n", stdgp->population[i]);
  }

  // Sorts the population
  qsort(stdgp->population, stdgp->population_size,
        sizeof(struct individual_t *), compare);

  // Updates the best individual
  if (stdgp->population[0]->fitness > stdgp->bestIndividual->fitness) {
    individual_destroy(stdgp->bestIndividual);
    stdgp->bestIndividual = clone_individual(stdgp->population[0], 1);
  }

  // Generates next generation
  struct individual_t **new_population =
      calloc(stdgp->population_size + 1, sizeof(struct individual_t *));
  struct individual_t **elite =
      getElite(stdgp->population_size, stdgp->population, stdgp->elitism_size);
  for (int i = 0; i < stdgp->elitism_size; ++i) {
    new_population[i] = elite[i];
  }
  free(elite);

  for (int i = stdgp->elitism_size; i < stdgp->population_size;) {
    struct individual_t **offspring = getOffspring(
        stdgp->population_size, stdgp->population, stdgp->tournament_size);
    struct individual_t **afterDiscard =
        discardDeep(offspring, stdgp->limit_depth);
    free(offspring);

    int j = 0;
    while (afterDiscard[j] != NULL) {
      if (i + j < stdgp->population_size) {
        new_population[i + j] = afterDiscard[j];
      } else {
        individual_destroy(afterDiscard[j]);
      }
      j++;
    }

    free(afterDiscard);

    i += j;
  }

  // Deletes and replaces the previous generation
  for (int i = 0; i < stdgp->population_size; ++i) {
    individual_destroy(stdgp->population[i]);
  }
  free(stdgp->population);
  stdgp->population = new_population;

  // Generation Log
  if (stdgp->verbose && stdgp->currentGeneration % 1 == 0)
    printf("  > Training: %.4f  // Test: %.4f      ",
           getAccuracy(stdgp->bestIndividual, stdgp->n_training_samples,
                       stdgp->training_X, stdgp->training_Y),
           getAccuracy(stdgp->bestIndividual, stdgp->n_test_samples,
                       stdgp->test_X, stdgp->test_Y));
}

/*


                # Update best individual
                if self.population[0] > self.bestIndividual:
                        self.bestIndividual = self.population[0]

                # Generating Next Generation
                newPopulation = []
                newPopulation.extend(getElite(self.population,
   self.elitism_size)) while len(newPopulation) < self.population_size:
                        offspring = getOffspring(self.population,
   self.tournament_size) offspring = discardDeep(offspring, self.limit_depth)
                        newPopulation.extend(offspring)
                self.population = newPopulation[:self.population_size]



                # Debug
                if self.verbose and self.currentGeneration%5==0:
                        if not self.Te_x is None:
                                print("   > Gen
   #"+str(self.currentGeneration)+":  Tr-Acc: "+ "%.6f"
   %self.bestIndividual.getAccuracy(self.Tr_x, self.Tr_y)+" // Te-Acc: "+ "%.6f"
   %self.bestIndividual.getAccuracy(self.Te_x, self.Te_y) + " // Time: " +
   str(end- begin) ) else: print("   > Gen #"+str(self.currentGeneration)+":
   Tr-Acc: "+ "%.6f" %self.bestIndividual.getAccuracy(self.Tr_x, self.Tr_y))


*/

double *predict_regression(struct StdGP_t *stdgp, int n_samples, double **x) {
  return calculate_individual(stdgp->bestIndividual, n_samples, x);
}

int *predict_classification(struct StdGP_t *stdgp, int n_samples, double **x) {
  return predict_individual(stdgp->bestIndividual, n_samples, x);
}

struct individual_t *getBestIndividual(struct StdGP_t *stdgp) {
  return stdgp->bestIndividual;
}

double *getTrainingAccuracyOverTime(struct StdGP_t *stdgp) {
  return stdgp->trainingAccuracyOverTime;
}

double *getTestAccuracyOverTime(struct StdGP_t *stdgp) {
  return stdgp->testAccuracyOverTime;
}

double *getTrainingRMSEOverTime(struct StdGP_t *stdgp) {
  return stdgp->trainingRMSEOverTime;
}

double *getTestRMSEOverTime(struct StdGP_t *stdgp) {
  return stdgp->testRMSEOverTime;
}

double *getFitnessOverTime(struct StdGP_t *stdgp) {
  return stdgp->fitnessOverTime;
}

int *getSizeOverTime(struct StdGP_t *stdgp) { return stdgp->sizeOverTime; }

double *getTimeOverTime(struct StdGP_t *stdgp) { return stdgp->timeOverTime; }

char *toString_StdGP(struct StdGP_t *stdgp) {
  return toString_individual(stdgp->bestIndividual);
}
