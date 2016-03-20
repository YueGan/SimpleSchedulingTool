#include <stdio.h>
#include <stdlib.h>
#include "helper.h"

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: displayfile <filename>\n");
        exit(1);
    }

    FILE *fp;
    if((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen");
        exit(1);
    }

    struct rec r;
    while(!feof(fp) && (fread(&r, sizeof(struct rec), 1, fp) == 1)) {
        printf("%d  %s\n", r.freq, r.word);
    }

    return 0;
}
