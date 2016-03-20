#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "helper.h"
#include <sys/wait.h>
#include <math.h>

int compare_helper(struct rec *rec_list, int size){

	int to_merge = 0;

	int i;
	for(i = 0; i < size; i ++){

		if(rec_list[i].freq == -1){
			if(i == size){
				return -1;
			}
			else{
				continue;
			}
		}
		else if((compare_freq(&rec_list[to_merge], &rec_list[i]) == 1) ||  rec_list[to_merge].freq == -1){

			to_merge = i;
		}
	}
	return to_merge;
}

int main(int argc, char ** argv){

	int num_pa = 0;
	struct rec *priority_array = NULL;
	struct rec *sorted_records = NULL;
	int num_rec = 0;

	/* take arguments after "-n" "-f" and "-o" */
	int opt;
	while ((opt = getopt(argc, argv, "n:f:o:")) != -1) {
	
		switch (opt) {
		case 'n':

			// Set the global variable number of partial array to N.
			num_pa = atoi(optarg);

			// Malloc for priority array with N records.
			priority_array = malloc(sizeof(struct rec) * num_pa);
			break;

		case 'f':

			printf("files to open: %s\n", optarg);
			

			pid_t pid;
			int status;
			int current = 0;

			// Make multiple pipe between N processes and parent.
			int **fd = malloc(sizeof(int*) * num_pa);


			// Number of Records in total
			num_rec = get_file_size(optarg)/sizeof(struct rec);

			// Malloc space for output file, should be same as input file.
			sorted_records = malloc(get_file_size(optarg));

			/* Create list of each part size */
			int *array_size;
			array_size = malloc(sizeof(int) * num_pa);

			int i, k;
			int temp_num_rec = num_rec;
			/* Divide up the size of chunk for Nth child processes */
			for (i = 0; i < num_pa; i++){
				array_size[i] = ceil(temp_num_rec/(num_pa - i));
				temp_num_rec -= array_size[i];
			}

			/* start looping N times to create processes */
			for (i = 0; i < num_pa; i++){
				fd[i] = malloc(sizeof(int) * 2);
				if (pipe(fd[i]) == -1){

		        	perror("pipe");
		    	}

		    	// START FORKING
				pid = fork();
				if (pid < 0){
					perror("fork\n");
				}

				/* Children */
				else if (pid == 0){

					close(fd[i][0]);
					struct rec * albumn = NULL;
					int temp = array_size[i];
					
					albumn = malloc(sizeof(struct rec) * temp);
					

					// READ BIANARY FILE START
					FILE *fp;
    				if((fp = fopen(optarg, "rb")) == NULL) {
        				perror("fopen");
        				exit(1);
    				}


    				/* Reading from certain lines of struct */
					struct rec r;
					fseek(fp, current * sizeof(struct rec), SEEK_SET);
    				while(temp > 0 && (fread(&r, sizeof(struct rec), 1, fp) == 1)) {
        				
        				albumn[temp-1] = r;
        				temp -= 1;

   					}

   					// Sorting the array with compare_freq compare method.
   					qsort(albumn, array_size[i], sizeof(struct rec), compare_freq);					

   					// Read binary file ends.
					fclose(fp);

					for(k = 0; k < array_size[i]; k++){	
						if(write(fd[i][1], &albumn[k], sizeof(struct rec)) == -1) {
							printf("write to pipe\n");
						}
					}

					close(fd[i][1]);
					exit(0);
				}

				/* Parent */
				else if(pid > 0){
					close(fd[i][1]);
					/*
					from here, parent check if this children's (n_records[i]) item is smallest
					if so take out and append into large array and take children's first item and 
					put into n_records[i]
					*/
					struct rec *record = malloc(sizeof(struct rec));

					/* At the last fork, and thus all child done sorting */
					struct rec empty;
					empty.freq = -1;

					if(i == num_pa - 1){
						int to_merge = 0;
						if(read(fd[i][0], record, sizeof(struct rec)) > 0){
							priority_array[i] = *record;
						}
						int index = 0;
						int all_empty = num_pa; 

						while(all_empty){

							to_merge = compare_helper(priority_array, num_pa);

							if(to_merge == -1){
								break;
							}
							sorted_records[index] = priority_array[to_merge];

							if(read(fd[to_merge][0], record, sizeof(struct rec)) > 0){
								priority_array[to_merge] = *record;
							} 
							else{
								// Closeing pipe 
								close(fd[to_merge][0]);
								priority_array[to_merge] = empty; 
								all_empty -= 1;
							}
							index += 1;

						}

					}
					else{
						if(read(fd[i][0], record, sizeof(struct rec)) > 0){
							priority_array[i] = *record;
						} 
					}

					free(record);
				}

				current += array_size[i];
			}

			for(i = 0; i < num_pa; i++){
				wait(&status);
			}

			// wait for all child to finish
			free(array_size);
			free(fd);

			break;

		case 'o':
			printf("print to: %s\n", optarg);
			/* write the all records into target file as binary file*/
			FILE *ptr_myfile;
    		ptr_myfile = fopen(optarg, "wa");

    		/*for each rec and do the wrtie method*/
			int items;
			for (items=0; items < num_rec; items++){

				struct rec r = sorted_records[items];
    		
   		    	fwrite(&r, sizeof(struct rec), 1, ptr_myfile);
			}
			fclose(ptr_myfile);


			/* Freeing the priority array.*/
			free(priority_array);
			free(sorted_records);
			break;

		default:
			abort();
		}
	}


	return 0;
}
