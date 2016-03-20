#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "helper.h"

#define UPPER 30000


/* Return a randomly generated number, uniformly distributed between
 * lower and upper.
 */
int uniform(int lower,int upper) {
    int r;
    r = (int) (floor ( drand48() * (upper - lower + 1) ) + lower);
    return(r);
}


int main(int argc, char *argv[]) {

    extern char *optarg;
    int ch;
    FILE *infp, *outfp;
    struct rec record;

    char *infile = NULL, *outfile = NULL;

    if (argc != 5) {
        fprintf(stderr, "Usage:  mkwords -f <input file name> "
                        "-o <output file name>\n");
        exit(1);
    }

    /* read in arguments */
    while ((ch = getopt(argc, argv, "f:o:")) != -1) {
        switch(ch) {
        case 'f':
            infile = optarg;
            break;
        case 'o':
            outfile = optarg;
            break;
        default : fprintf(stderr, "Usage:  mkwords -f <input file name> "
                          "-o <output file name>\n");
            exit(1);
        }
    }


    /* seed the random number generator */
    srand48(131);

    if ((infp = fopen(infile, "r")) == NULL ) {
        fprintf(stderr, "Could not open %s\n", infile);
        exit(1);
    }
    if ((outfp = fopen(outfile, "w")) == NULL ) {
        fprintf(stderr, "Could not open %s\n", infile);
        exit(1);
    }

    /* read a word from the input file, and make up a rank for it */
    while (( fgets(record.word, sizeof(record.word), infp) ) != NULL) {
        record.word[strlen(record.word) - 1] = '\0';
        int r = uniform(0, UPPER);
        record.freq = r;

        if ((fwrite(&record, sizeof(record), 1, outfp)) != 1){
            fprintf(stderr, "Could not write to %s\n", outfile);
        }
    }

    return 0;
}