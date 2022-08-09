#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <node.h>
#include <individual.h>
#include <STGP.h>
#include <time.h>

#include "arguments.c"


int length(char** c){
	int l = 0;
	while( c[l] != NULL){
		l++;
	}
	return l;
}

int get_DS_size(char* filename, int header){	
	char str[1028];
	FILE * file;
	file = fopen( filename , "r");
	int i;
	for (i = 0; fscanf(file, "%s", str)!=EOF; ++i){}
	fclose(file);	

	return i-header;
}

double** get_DS_X(char* filename, int n_samples, int header){	
	char str[1028];
	FILE * file;
	file = fopen( filename , "r");

	if (header) {
		fscanf(file, "%s", str);
		//printf("Skipped Header: %s\n", str );
	}

	double** ret = calloc( n_samples + 1 , sizeof(double*) );
	int i =0;
	for (i = 0; i < n_samples; ++i){
		fscanf(file, "%s", str);
		char** line = split(str, -1, ",");

		int len = length(line);
		ret[i] = calloc( len,  sizeof(double));
		for (int x = 0; x < len -1 ; ++x){ // -1 para ignorar a coluna da classe
			ret[i][x] = atof(line[x]);
		}
		string_array_destroy(line);
	}
	fclose(file);		

	return ret;
}

double* get_DS_Y(char* filename, int n_samples, int header){	
	char str[1028];
	FILE * file;
	file = fopen( filename , "r");

	if (header) {
		fscanf(file, "%s", str);
		//printf("Skipped Header: %s\n", str );
	}

	double* ret = calloc( n_samples + 1 , sizeof(double) );
	int i =0;
	for (i = 0; i < n_samples; ++i){
		fscanf(file, "%s", str);
		char** line = split(str, -1, ",");

		int len = length(line);
		ret[i] = atof( line[len-1] );

		string_array_destroy(line);
	}
	fclose(file);		

	return ret;
}

void shuffle_DS(double** X, double* Y, int n_samples){
    if (n_samples > 1) {
        for (int i = 0; i < n_samples - 1; i++) {
        	int j = i + rand() / (RAND_MAX / (n_samples - i) + 1);

        	double* tmpX = X[j];
        	X[j] = X[i];
        	X[i] = tmpX;

			double tmpY = Y[j];
        	Y[j] = Y[i];
        	Y[i] = tmpY;
        }
    }
}

double** getTrainingX(double** X, int n_samples){
	int limit = (int)( n_samples * TRAIN_FRACTION );

	double** ret = malloc( limit * sizeof(double*));
	for (int i = 0; i < limit; ++i){
		ret[i] = X[i];
	}
	return ret;
}

double* getTrainingY(double* Y, int n_samples){
	int limit = (int)( n_samples * TRAIN_FRACTION );

	double* ret = malloc( limit * sizeof(double));
	for (int i = 0; i < limit; ++i){
		ret[i] = Y[i];
	}
	return ret;
}

double** getTestX(double** X, int n_samples){
	int limit = (int)( n_samples * TRAIN_FRACTION );

	double** ret = malloc( (n_samples - limit) * sizeof(double*));
	for (int i = limit; i < n_samples; ++i){
		ret[i - limit] = X[i];
	}
	return ret;
}

double* getTestY(double* Y, int n_samples){
	int limit = (int)( n_samples * TRAIN_FRACTION );

	double* ret = malloc( (n_samples - limit) * sizeof(double));
	for (int i = limit; i < n_samples; ++i){
		ret[i - limit] = Y[i];
	}
	return ret;
}


int main(int argc, char* argv[]){

	initArguments();
	updateArguments(argc, argv);

	char* dir = calloc( (strlen(DATASETS_DIR) + strlen(DATASETS[0]) +1), sizeof(char) );
	strcat(dir, DATASETS_DIR);
	strcat(dir, DATASETS[0]);
	printf("DATASET: %s\n", dir );

	char str[1028];
	FILE * file;
	file = fopen( dir , "r");

	fscanf(file, "%s", str);

	int n_terminals = count(str, ",") -1 ; // Ignora a ultima coluna
	char** terminals = split( str , n_terminals, ",");


	int header = 1;
	int n_samples = get_DS_size( dir , header);
		
	fclose(file);



	struct STGP_t** models = calloc( RUNS+1, sizeof(struct STGP_t*));


	for (int run = 0; run < RUNS; run++){
		printf("\n\n\n---> RUN %2d <---\n", run);

		// --------------------------------------------  INIT RUN

		srand(run+1);

		double** X = get_DS_X( dir, n_samples, header);
		double*  Y = get_DS_Y( dir, n_samples, header);

		shuffle_DS(X,Y,n_samples);

		int Tr_samples = (int)( n_samples * TRAIN_FRACTION );
		int Te_samples = n_samples - (int)( n_samples * TRAIN_FRACTION );

		double** Tr_X = getTrainingX(X, n_samples);
		double*  Tr_Y = getTrainingY(Y, n_samples);
		double** Te_X = getTestX(X, n_samples);
		double*  Te_Y = getTestY(Y, n_samples);


		// --------------------------------------------  RUN
		
		struct STGP_t* model = stgp_create(N_OPERATORS, OPERATORS, n_terminals, terminals,
			MAX_DEPTH, POPULATION_SIZE, MAX_GENERATION, TOURNAMENT_SIZE, ELITISM_SIZE,
			LIMIT_DEPTH, THREADS, VERBOSE, Tr_samples, Tr_X, Tr_Y, Te_samples, Te_X, Te_Y);



    	clock_t before = clock();
		fit(model);
   		clock_t after = clock();
    	long duration = (after-before)*1000.0/CLOCKS_PER_SEC;

    	//for (int i = 0; i < MAX_GENERATION; ++i){
    	//	printf("%f,", model->timeOverTime[i]);
    	//}

		//if (model->verbose) printf("\n\nTotal Execution Time: %ldms\n", duration );


		//stgp_destroy(model);
		models[run] = model;
		

		// --------------------------------------------  CLOSE RUN

	
		//free dataset
		for (int i = 0; i < n_samples; ++i){
			free(X[i]);
		}
		free(X);
		free(Y);
		free(Tr_X);
		free(Tr_Y);
		free(Te_X);
		free(Te_Y);

		printf("RUN %d: MODEL#ID: %f\n", run, models[run]->testAccuracyOverTime[10]);

	}

	/*
	for (int i = 0; i < RUNS; ++i){
		printf("RUN %d: MODEL#ID: %f\n", i, models[i]->testAccuracyOverTime[10]);
	}
	*/

	// ---------------------------------------------- OUTPUT FILE
	/*
	DIR* outdir = opendir(OUTPUT_DIR);
	if (outdir) {
		/* Directory exists. /
		closedir(outdir);
	} else 
	if (errno == ENOENT) {
		/* Directory does not exist. /
		os.mkdir(OUTPUT_DIR, 0700);
	}
	*/
	mkdir(OUTPUT_DIR, 0700);
	
	char* outname = calloc( (strlen(OUTPUT_DIR) +7+ strlen(DATASETS[0]) +1), sizeof(char) );
	strcat(outname, OUTPUT_DIR);
	strcat(outname, "stgp_c_");
	strcat(outname, DATASETS[0]);
	FILE* out = fopen( outname,"w");//("%sstgp_c_%s",OUTPUT_DIR, outputname), "w");
	free(outname);
	char buff[2048];
	//fputs(string, out);

	printf("OUTPUT HEADER...\n");
	fputs("Attribute,Run,", out);
	for (int i = 0; i < MAX_GENERATION; ++i){
		sprintf(buff, "%d,", i);
		fputs(buff, out);
	}
	fputs("\n\n", out);

	printf("OUTPUT TR-ACC...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Training-Accuracy,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%f,", models[run]->trainingAccuracyOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT TE-ACC...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Test-Accuracy,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%f,", models[run]->testAccuracyOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT TR-RMSE...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Training-RMSE,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%f,", models[run]->trainingRMSEOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT TE-RMSE...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Test-RMSE,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%f,", models[run]->testRMSEOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT FITNESS...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Fitness,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%f,", models[run]->fitnessOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT SIZE...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Size,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%d,", models[run]->sizeOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT TIME...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Time,%d,", run);
		fputs(buff, out);	
		for (int i = 0; i < MAX_GENERATION; ++i){
			sprintf(buff, "%f,", models[run]->timeOverTime[i]);
			fputs(buff, out);
		}
		fputs("\n", out);
	}
	fputs("\n", out);

	printf("OUTPUT FINAL MODEL...\n");
	for (int run = 0; run < RUNS; run++){
		sprintf(buff, "Final_Model,%d,", run);
		fputs(buff, out);
		char* str = toString_individual(models[run]->bestIndividual);
		sprintf(buff, "%s,\n", str );
		free(str);
		fputs(buff, out);
	}
	fputs("\n", out);

	printf("OUTPUT PARAMETERS...\n");
	fputs("\nParameters:\n", out);
	sprintf(buff, "Operators,['+', '-', '*', '/']\n");	fputs(buff, out);
	sprintf(buff, "Max Initial Depth,%d\n", MAX_DEPTH);	fputs(buff, out);
	sprintf(buff, "Population Size,%d\n",POPULATION_SIZE);	fputs(buff, out);
	sprintf(buff, "Max Generation,%d\n", MAX_GENERATION);	fputs(buff, out);
	sprintf(buff, "Tournament Size,%d\n", TOURNAMENT_SIZE);	fputs(buff, out);
	sprintf(buff, "Elitism Size,%d\n", ELITISM_SIZE);	fputs(buff, out);
	sprintf(buff, "Depth Limit,%d\n", LIMIT_DEPTH);	fputs(buff, out);
	sprintf(buff, "Threads (not implemented),%d\n", THREADS);	fputs(buff, out);

	fclose(out);


	freeArguments();
	free(dir);

	for (int run = 0; run < RUNS; run++){
		stgp_destroy(models[run]);
	}
	string_array_destroy(terminals);
	free(models);

	return 0;
}
