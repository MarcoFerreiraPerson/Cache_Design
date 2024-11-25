/*
FOR SET ASSOCIATIVE CACHE 
GROUP MEMBERS: 
#FILL IN

LOCATION: Rutgers University
CLASS: CS211 Computer Architecture
SESSION: Fall 2024
PROFESSOR: Dr. Tina Burns

DESCRIPTION:


NOTES:
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"
#include <stdbool.h>


/*
 * main - Main routine 
 */
typedef struct commandArguments{
    bool hFlag;
    bool vFlag;
    int setIndexBits;
    int Associativity;
    int blockBits;
    char traceFiles[100];
}commandArguments;
typedef struct cacheStats{
    int hits;
    int misses;
    int evictions;
}cacheStats;
typedef struct {
    int tag;
    int setIndex;
    int blockOffset;
} CacheFields;
typedef struct {
    char action;         // 'L', 'S', or 'M'
    unsigned int address; // Address value
    unsigned int size;    // Size value
} ParsedLine;
void setHelpFlag(commandArguments *args) {
    args->hFlag = true;
}

void setVerboseFlag(commandArguments *args) {
    args->vFlag = true;
}

void setIndexBits(commandArguments *args, const char *value) {
    args->setIndexBits = atoi(value);
}

void setAssociativity(commandArguments *args, const char *value) {
    args->Associativity = atoi(value);
}

void setBlockBits(commandArguments *args, const char *value) {
    args->blockBits = atoi(value);
}
void setTraceFile(commandArguments *args, const char *value) {
    strncpy(args->traceFiles, value, sizeof(args->traceFiles) - 1);
    args->traceFiles[sizeof(args->traceFiles) - 1] = '\0'; 
}
void parseArguments(int argc, char *argv[], commandArguments *args) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            setHelpFlag(args);
        } else if (strcmp(argv[i], "-v") == 0) {
            setVerboseFlag(args);
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                setIndexBits(args, argv[++i]);
            } else {
                fprintf(stderr, "Error: Missing value for -s\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-E") == 0) {
            if (i + 1 < argc) {
                setAssociativity(args, argv[++i]);
            } else {
                fprintf(stderr, "Error: Missing value for -E\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 < argc) {
                setBlockBits(args, argv[++i]);
            } else {
                fprintf(stderr, "Error: Missing value for -b\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                setTraceFile(args, argv[++i]);
            } else {
                fprintf(stderr, "Error: Missing value for -t\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            exit(1);
        }
    }
}
FILE *grabRows(const char *fileName) {
    // Ensure the file name is not empty
    if (strcmp(fileName, "") == 0) {
        fprintf(stderr, "Error: File name is empty.\n");
        return NULL;
    }

    // Attempt to open the file
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", fileName);
        return NULL;
    }

    return file; // Return the file pointer if successful
}
CacheFields calculateCacheFields(int address, int b, int s) {
    CacheFields fields;

    // Calculate block offset (last b bits)
    fields.blockOffset = address & ((1 << b) - 1);

    // Calculate set index (next s bits)
    fields.setIndex = (address >> b) & ((1 << s) - 1);

    // Calculate tag (remaining high-order bits)
    fields.tag = address >> (b + s);

    return fields;
}
ParsedLine parseLine(const char *line) {
    ParsedLine result = {0}; // Initialize struct to default values

    // Parse the line into the struct
    if (sscanf(line, " %c %u,%u", &result.action, &result.address, &result.size) != 3) {
        fprintf(stderr, "Error: Failed to parse line: %s", line);
    }

    return result;
}
// Function to handle 'L' (Load) operation
void handleLoad(ParsedLine parsed, cacheStats *stats, commandArguments args) {
    // Implementation goes here
    CacheFields result = calculateCacheFields(parsed.address,args.blockBits, args.setIndexBits);
}

// Function to handle 'I' (Instruction) operation
void handleInstruction(ParsedLine parsed, cacheStats *stats,commandArguments args) {
    // Implementation goes here
    CacheFields result = calculateCacheFields(parsed.address,args.blockBits, args.setIndexBits);
}

// Function to handle 'S' (Store) operation
void handleStore(ParsedLine parsed, cacheStats *stats,commandArguments args) {
    // Implementation goes here
    CacheFields result = calculateCacheFields(parsed.address,args.blockBits, args.setIndexBits);
}

// Function to handle 'M' (Modify) operation
void handleModify(ParsedLine parsed, cacheStats *stats,commandArguments args) {
    // Implementation goes here
    CacheFields result = calculateCacheFields(parsed.address,args.blockBits, args.setIndexBits);
}

void printVerbose(){
    printf("Implement Later but this is what should be printed if the -v Flag is present");
}
int main(int argc, char* argv[])
{
    commandArguments arguments = {0}; // Initialize struct to default values
    cacheStats stats = {0}; // Set defualt stats to 0 and will be updated in functions
    // Parse command-line arguments
    parseArguments(argc, argv, &arguments);

    // Print parsed arguments for debugging
    printf("Parsed Arguments:\n");
    printf("  hFlag: %s\n", arguments.hFlag ? "true" : "false");
    printf("  vFlag: %s\n", arguments.vFlag ? "true" : "false");
    printf("  setIndexBits: %d\n", arguments.setIndexBits);
    printf("  associativity: %d\n", arguments.Associativity);
    printf("  blockBits: %d\n", arguments.blockBits);
    printf("  traceFiles: %s\n", arguments.traceFiles);
   
    FILE *file = grabRows(arguments.traceFiles);
    if (file == NULL) {
        printf("Invalid trace file was provided %s",arguments.traceFiles);
        return 1; // Exit if there was an error
    }

    // Read and print each line from the file
    char line[256];
     while (fgets(line, sizeof(line), file)) {
        ParsedLine parsed = parseLine(line); // Parse the line

        // Handle each case based on the action
        if (parsed.action == 'L') {
            handleLoad(parsed,&stats,arguments);
            printf("Load: Address = %u, Size = %u\n", parsed.address, parsed.size);
        } else if (parsed.action == 'S') {
            handleStore(parsed,&stats,arguments);
            printf("Store: Address = %u, Size = %u\n", parsed.address, parsed.size);
        } else if (parsed.action == 'M') {
            handleModify(parsed,&stats,arguments);
            printf("Modify: Address = %u, Size = %u\n", parsed.address, parsed.size);
        }else if (parsed.action == 'I'){
            continue;// Part A skip 
            handleInstruction(parsed,&stats,arguments);
            printf("Instruction: Address = %u, Size = %u\n", parsed.address, parsed.size);
        } else {
            printf("Unknown action: %c\n", parsed.action);
        }
    }
    if (arguments.vFlag){
        printVerbose();
    }
    fclose(file);
    printf("Done\n");
    return 0;
}


