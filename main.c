#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Open the input file
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
      printf("%s",line);
    }

    fclose(file);
    return 0;
}
