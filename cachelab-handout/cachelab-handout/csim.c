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
#include <string.h>
#include <stdbool.h>

// Command line arguments structure
typedef struct commandArguments {
    bool hFlag;                 // Help flag
    bool vFlag;                 // Verbose flag
    int setIndexBits;           // Number of set index bits (s)
    int Associativity;          // Number of lines per set (E)
    int blockBits;              // Number of block bits (b)
    char traceFiles[100];       // Trace file name
    FILE *outputFile;           // Output file for verbose mode
} commandArguments;

// Cache statistics structure
typedef struct cacheStats {
    int hits;                   // Number of hits
    int misses;                 // Number of misses
    int evictions;              // Number of evictions
} cacheStats;

// Cache line structure
typedef struct {
    int valid;                  // Valid bit
    int tag;                    // Tag
    int last_used;              // LRU tracking
} CacheLine;

// Cache set structure
typedef struct {
    CacheLine *lines;           // Array of cache lines
} CacheSet;

// Cache structure
typedef struct {
    CacheSet *sets;             // Array of sets
    int s;                      // Number of set index bits
    int E;                      // Associativity
    int b;                      // Number of block bits
} Cache;

// CacheFields structure for parsed address
typedef struct {
    int tag;                    // Tag extracted from address
    int setIndex;               // Set index extracted from address
    int blockOffset;            // Block offset extracted from address
} CacheFields;

// Memory access structure
typedef struct {
    char action;                // Access type: 'L', 'S', 'M', or 'I'
    unsigned int address;       // Address being accessed
    unsigned int size;          // Size of access
} ParsedLine;

// Global cache instance
Cache cache;

ParsedLine parseLine(const char *line) {
    ParsedLine result = {0};
    sscanf(line, " %c %u,%u", &result.action, &result.address, &result.size);
    return result;
}
void parseArguments(int argc, char *argv[], commandArguments *args) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            args->hFlag = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            args->vFlag = true;
        } else if (strcmp(argv[i], "-s") == 0) {
            args->setIndexBits = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-E") == 0) {
            args->Associativity = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0) {
            args->blockBits = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0) {
            strncpy(args->traceFiles, argv[++i], sizeof(args->traceFiles) - 1);
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            exit(1);
        }
    }
}

/* Function to initialize the cache with given parameters */
void initializeCache(int s, int E, int b) {
    int num_sets = 1 << s; // Number of sets = 2^s
    cache.sets = malloc(num_sets * sizeof(CacheSet));
    cache.s = s;
    cache.E = E;
    cache.b = b;

    for (int i = 0; i < num_sets; i++) {
        cache.sets[i].lines = malloc(E * sizeof(CacheLine));
        for (int j = 0; j < E; j++) {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = -1;
            cache.sets[i].lines[j].last_used = 0;
        }
    }
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

void processCacheAccess(CacheFields fields, cacheStats *stats, bool *hit, bool *miss, bool *eviction) {
    // Get the relevant cache set
    CacheSet *set = &cache.sets[fields.setIndex];
    
}



void writeParsedLine(FILE *file, ParsedLine line, bool miss, bool eviction, bool hit) {
    if (file == NULL) {
        perror("Error: Invalid file pointer");
        return;
    }

    // Write the struct fields
    fprintf(file, "%c %u,%u", line.action, line.address, line.size);
    // Append the optional fields in the correct order
    if (miss) {
        fprintf(file, " miss");
    }
    if (eviction) {
        printf("%c %u , %u ",line.action,line.address,line.size);
        fprintf(file, " eviction");
    }
    if (hit) {
        fprintf(file, " hit");
    }

    // Newline at the end
    fprintf(file, "\n");
}

/*void processCacheAccess(CacheFields fields, cacheStats *stats, bool *hit, bool *miss, bool *eviction) {
/* Revised memory access handlers */
void handleLoad(ParsedLine parsed, cacheStats *stats, commandArguments args) {
    CacheFields fields = calculateCacheFields(parsed.address, args.blockBits, args.setIndexBits);
    bool hit = false, miss = false, eviction = false;
    
    processCacheAccess(fields, stats, &hit, &miss, &eviction);

    writeParsedLine(args.outputFile, parsed, miss, eviction, hit);
}

void handleStore(ParsedLine parsed, cacheStats *stats, commandArguments args) {
    CacheFields fields = calculateCacheFields(parsed.address, args.blockBits, args.setIndexBits);
    bool hit = false, miss = false, eviction = false;
    
    processCacheAccess(fields, stats, &hit, &miss, &eviction);
    writeParsedLine(args.outputFile, parsed, miss, eviction, hit);
}



void handleModify(ParsedLine parsed, cacheStats *stats, commandArguments args) {
    CacheFields fields = calculateCacheFields(parsed.address, args.blockBits, args.setIndexBits);

    // First phase: Load
    bool hit1 = false, miss1 = false, eviction1 = false;
    processCacheAccess(fields, stats, &hit1, &miss1, &eviction1);

    // Second phase: Store (guaranteed to be a hit after the first access)
    bool hit2 = false, miss2 = false, eviction2 = false;
    processCacheAccess(fields, stats, &hit2, &miss2, &eviction2);

    // Consolidate results for verbose output
    bool overall_hit = hit1 || hit2;
    bool overall_miss = miss1 || miss2;
    bool overall_eviction = eviction1 || eviction2;

    // Write the combined result
    writeParsedLine(args.outputFile, parsed, overall_miss, overall_eviction, overall_hit);
}
/* Main function to parse arguments, process trace file, and output results */
int main(int argc, char *argv[]) {
    commandArguments arguments = {0};
    cacheStats stats = {0};
    parseArguments(argc, argv, &arguments);

    initializeCache(arguments.setIndexBits, arguments.Associativity, arguments.blockBits);

    FILE *traceFile = fopen(arguments.traceFiles, "r");
    if (!traceFile) {
        fprintf(stderr, "Error opening trace file: %s\n", arguments.traceFiles);
        return 1;
    }

    // Set up the verbose output file
    FILE *vfile = fopen("vFile.txt", "w");
    if (!vfile) {
        fprintf(stderr, "Error creating verbose output file.\n");
        return 1;
    }
    arguments.outputFile = vfile;

    char line[256];
    while (fgets(line, sizeof(line), traceFile)) {
        ParsedLine parsed = parseLine(line);
        if (parsed.action == 'L') {
            handleLoad(parsed, &stats, arguments);
        } else if (parsed.action == 'S') {
            handleStore(parsed, &stats, arguments);
        } else if (parsed.action == 'M') {
            handleModify(parsed, &stats, arguments);
        }
    }

    fclose(traceFile);
    fclose(arguments.outputFile);

    if (arguments.vFlag) {
        arguments.outputFile = fopen("vFile.txt", "r");
        if (arguments.outputFile == NULL) {
            printf("Error reopening verbose output file.\n");
        } else {
            char verboseLine[256];
           // printf("\nVerbose Output:\n");
            while (fgets(verboseLine, sizeof(verboseLine), arguments.outputFile)) {
                printf("%s", verboseLine);
            }
            fclose(arguments.outputFile);
        }
    }

    printf("Hits: %d Misses: %d Evictions: %d\n", stats.hits, stats.misses, stats.evictions);
    return 0;
}

