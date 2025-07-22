/*
 * By using this file, you are agreeing to this product's EULA
 *
 * This product can be obtained in https://github.com/jespb/C-StdGP
 *
 * Copyright ©2021-2022 J. E. Batista
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <StdGP.h>
#include <individual.h>
#include <node.h>
#include <time.h>

#include "arguments.c"

int getArrayLength(char **c) {
  int l = 0;

  while (c[l] != NULL) {
    l++;
  }
  return l;
}

int getDatasetLength(char *filename, int header) {
  FILE *file;
  int count = 0;
  char c;

  file = fopen(filename, "r");

  for ( c = getc(file); c != EOF; c = getc(file)){
  	if (c == '\n'){
  		count++;
  	}
  }

  return count - header;
}


double **getDataset(char *filename, int n_samples, int header) {
  char str[1028];
  char **line;
  FILE *file;
  double **ret;
  int i, len, x;


  file = fopen(filename, "r");

  if (header) {
    if (fscanf(file, "%s", str) != 1) {
      fprintf(stderr, "Error reading string from file\n");
      exit(EXIT_FAILURE);
    }
  }

  ret = calloc(n_samples + 1, sizeof(double *));
  for (i = 0; i < n_samples; ++i) {
    if (fscanf(file, "%s", str) != 1) {
      fprintf(stderr, "Error reading string from file\n");
      exit(EXIT_FAILURE);
    }
    line = split(str, -1, ",");

    len = getArrayLength(line);
    ret[i] = calloc(len, sizeof(double));
    for (x = 0; x < len; ++x) {
      ret[i][x] = atof(line[x]);
    }
    string_array_destroy(line);
  }
  fclose(file);

  return ret;
}


void shuffleDataset(double **ds, int n_samples) {
  int i, j;
  double *tmp;

  if (n_samples > 1) {
    for (i = 0; i < n_samples - 1; i++) {
      j = i + rand() / (RAND_MAX / (n_samples - i) + 1);

      tmp = ds[j];
      ds[j] = ds[i];
      ds[i] = tmp;
    }
  }
}


double **getDatasetX(double **ds, int n_samples, int isTraining) {
  int mini, maxi, i;
  double **ret;

  if (isTraining  == 0){
  	mini = 0;
  	maxi = (int)(n_samples * TRAIN_FRACTION);
  }else{
  	mini = (int)(n_samples * TRAIN_FRACTION);
  	maxi = n_samples;
  }

  ret = calloc( maxi-mini, sizeof(double *));
  for (i = mini; i < maxi; i++) {
  	// The target value is passed but not used by the classifier
    ret[i-mini] = ds[i]; 
  }
  return ret;
}

double *getDatasetY(double **ds, int n_samples, int n_terminals, int isTraining) {
  int mini, maxi, i;
  double *ret;

  if (isTraining == 0){
  	mini = 0;
  	maxi = (int)(n_samples * TRAIN_FRACTION);
  }else{
  	mini = (int)(n_samples * TRAIN_FRACTION);
  	maxi = n_samples;
  }

  ret = calloc( maxi-mini, sizeof(double));
  for (i = mini; i < maxi; i++) {
    ret[i-mini] = ds[i][n_terminals]; // Target value
  }
  return ret;
}







int main(int argc, char *argv[]) {
  char *dir, *outname, *ind_str ;
  char **terminals;
  char str[1028], buff[2048];
  FILE *file, *out;
  int n_terminals, header, n_samples, run, Tr_samples, Te_samples, i;
  struct StdGP_t *model;
  struct StdGP_t **models;
  double **dataset, **Tr_X, **Te_X;
  double *Tr_Y, *Te_Y;


  initArguments();
  updateArguments(argc, argv);

  dir = calloc((strlen(DATASETS_DIR) + strlen(DATASETS[0]) + 1), sizeof(char));
  strcat(dir, DATASETS_DIR);
  strcat(dir, DATASETS[0]);
  printf("DATASET: %s\n", dir);

  file = fopen(dir, "r");

  if (fscanf(file, "%s", str) != 1) {
    fprintf(stderr, "Error reading string from file\n");
    exit(EXIT_FAILURE);
  }

  n_terminals = count(str, ",") - 1; // Ignores last column (Target)
  terminals = split(str, n_terminals, ",");

  header = 1;
  n_samples = getDatasetLength(dir, header);

  fclose(file);

  models = calloc(RUNS + 1, sizeof(struct StdGP_t *));

  for (run = 0; run < RUNS; run++) {
    printf("\n\n\n---> RUN %2d <---\n", run);

    // --------------------------------------------  INIT RUN

    srand(run + 1);

    dataset = getDataset(dir, n_samples, header);
    shuffleDataset(dataset, n_samples);

    Tr_samples = (int)(n_samples * TRAIN_FRACTION);
    Te_samples = n_samples - (int)(n_samples * TRAIN_FRACTION);

    Tr_X = getDatasetX(dataset, n_samples,0);
    Tr_Y = getDatasetY(dataset, n_samples, n_terminals,0);
    Te_X = getDatasetX(dataset, n_samples,1);
    Te_Y = getDatasetY(dataset, n_samples, n_terminals,1);

    // --------------------------------------------  RUN

    model =
        stdgp_create(N_OPERATORS, OPERATORS, n_terminals, terminals, MAX_DEPTH,
                     POPULATION_SIZE, MAX_GENERATION, TOURNAMENT_SIZE,
                     ELITISM_SIZE, LIMIT_DEPTH, THREADS, VERBOSE, Tr_samples,
                     Tr_X, Tr_Y, Te_samples, Te_X, Te_Y);


    fit(model);


    // stdgp_destroy(model);
    models[run] = model;

    // --------------------------------------------  CLOSE RUN

    // free dataset
    for (i = 0; i < n_samples; ++i) {
      free(dataset[i]);
    }
    free(dataset);
    free(Tr_X);
    free(Tr_Y);
    free(Te_X);
    free(Te_Y);

    printf("RUN %d: MODEL#ID: %f\n", run,
           models[run]->testAccuracyOverTime[10]);
  }




  mkdir(OUTPUT_DIR, 0700);

  outname =
      calloc((strlen(OUTPUT_DIR) + 7 + strlen(DATASETS[0]) + 2), sizeof(char));
  strcat(outname, OUTPUT_DIR);
  strcat(outname, "stdgp_c_");
  strcat(outname, DATASETS[0]);
  out = fopen(outname, "w"); //("%sstdgp_c_%s",OUTPUT_DIR, outputname), "w");
  free(outname);
  // fputs(string, out);

  printf("OUTPUT HEADER...\n");
  fputs("Attribute,Run,", out);
  for (i = 0; i < MAX_GENERATION; ++i) {
    sprintf(buff, "%d,", i);
    fputs(buff, out);
  }
  fputs("\n\n", out);

  printf("OUTPUT TR-Accuracy...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Training-Accuracy,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->trainingAccuracyOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TE-Accuracy...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Test-Accuracy,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->testAccuracyOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TR-WaF...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Training-WaF,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->trainingWaFOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TE-WaF...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Test-WaF,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->testWaFOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TR-Kappa...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Training-Kappa,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->trainingKappaOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TE-Kappa...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Test-Kappa,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->testKappaOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TR-RMSE...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Training-RMSE,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->trainingRMSEOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TE-RMSE...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Test-RMSE,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->testRMSEOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT FITNESS...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Fitness,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->fitnessOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT SIZE...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Size,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%d,", models[run]->sizeOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT TIME...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Time,%d,", run);
    fputs(buff, out);
    for (i = 0; i < MAX_GENERATION; ++i) {
      sprintf(buff, "%f,", models[run]->timeOverTime[i]);
      fputs(buff, out);
    }
    fputs("\n", out);
  }
  fputs("\n", out);

  printf("OUTPUT FINAL MODEL...\n");
  for (run = 0; run < RUNS; run++) {
    sprintf(buff, "Final_Model,%d,", run);
    fputs(buff, out);
    ind_str = toString_individual(models[run]->bestIndividual);
    sprintf(buff, "%s,\n", ind_str);
    free(ind_str);
    fputs(buff, out);
  }
  fputs("\n", out);

  printf("OUTPUT PARAMETERS...\n");
  fputs("\nParameters:\n", out);
  sprintf(buff, "Operators,['+', '-', '*', '/']\n");
  fputs(buff, out);
  sprintf(buff, "Max Initial Depth,%d\n", MAX_DEPTH);
  fputs(buff, out);
  sprintf(buff, "Population Size,%d\n", POPULATION_SIZE);
  fputs(buff, out);
  sprintf(buff, "Max Generation,%d\n", MAX_GENERATION);
  fputs(buff, out);
  sprintf(buff, "Tournament Size,%d\n", TOURNAMENT_SIZE);
  fputs(buff, out);
  sprintf(buff, "Elitism Size,%d\n", ELITISM_SIZE);
  fputs(buff, out);
  sprintf(buff, "Depth Limit,%d\n", LIMIT_DEPTH);
  fputs(buff, out);
  sprintf(buff, "Threads (not implemented),%d\n", THREADS);
  fputs(buff, out);

  fclose(out);

  freeArguments();
  free(dir);

  for (int run = 0; run < RUNS; run++) {
    stdgp_destroy(models[run]);
  }
  string_array_destroy(terminals);
  free(models);

  return 0;
}
