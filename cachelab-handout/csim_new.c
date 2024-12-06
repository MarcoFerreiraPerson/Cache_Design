/*
FOR SET ASSOCIATIVE CACHE 
GROUP MEMBERS: 
* Marco Ferreira - maf407
* Katelyn DePaul kmd390
* Ethan Shindin es1231
* Adrian Delgado aed136
 * Richard Baah rbb98
#FILL IN

LOCATION: Rutgers University
CLASS: CS211 Computer Architecture
SESSION: Fall 2024
PROFESSOR: Dr. Tina Burns

DESCRIPTION:
Uses a dynamic growth strategy with frequency-based quartile removal

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


#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line with frequency counter */
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned int frequency; // used to get how frequently a datapoint to accessed
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* Stats tracking per set */
typedef struct cache_stats {
    int current_size;
    int max_size;
} cache_stats_t;

/* Globals set by command line args */
int verbosity = 0;
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;

/* The cache we are simulating */
cache_t cache;
cache_stats_t* set_stats;
mem_addr_t set_index_mask;

/* Removal percentage for top and bottom % of the cache based on frequency */
const float REMOVAL_AMOUNT = 0.01;

/* Initialize cache */
void initCache() {
    int i;
    cache = (cache_set_t*) malloc(sizeof(cache_set_t) * S);
    set_stats = (cache_stats_t*) malloc(sizeof(cache_stats_t) * S);
    
    if (!cache || !set_stats) {
        fprintf(stderr, "memory allocation failed\n");
        exit(1);
    }

    // each set has E lines so it, they have their own current_size and max_size associated with it, this expands
    for (i = 0; i < S; i++) {
        set_stats[i].current_size = E;
        set_stats[i].max_size = E;
        
        cache[i] = (cache_line_t*) malloc(sizeof(cache_line_t) * E);
        if (!cache[i]) {
            fprintf(stderr, "Failed to allocate set %d\n", i);
            exit(1);
        }

    }

     /* Computes set index mask */
    set_index_mask = (mem_addr_t)(pow(2, s) - 1);
}

/* Free allocated memory */
void freeCache() {
    if (cache) {
        for (int i = 0; i < S; i++) {
            if (cache[i]) {
                free(cache[i]);
            }
        }
        free(cache);
    }
    if (set_stats) {
        free(set_stats);
    }
}

/* Handle memory pressure by removing top and bottom percentages if we run out of memory, this function should never run theoretically, its only if we run out of ram space*/
void handle_memory_overflow(cache_set_t cache_set, int set_index) {
    int size = set_stats[set_index].current_size;
    int remove_count = size * REMOVAL_AMOUNT;
    int new_size = size - (remove_count * 2);

    // bubble sort by frequency
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (cache_set[j].valid && cache_set[j+1].valid && 
                cache_set[j].frequency > cache_set[j+1].frequency) {
                // Swap entries
                cache_line_t temp = cache_set[j];
                cache_set[j] = cache_set[j+1];
                cache_set[j+1] = temp;
            }
        }
    }

    // Compact the middle entries (removing top and bottom frequencies)
    int write_pos = 0;
    for (int read_pos = remove_count; read_pos < size - remove_count; read_pos++) {
        if (write_pos != read_pos) {
            cache_set[write_pos] = cache_set[read_pos];
        }
        write_pos++;
    }

    // Reallocate to smaller size to save room
    cache_line_t* new_set = realloc(cache_set, sizeof(cache_line_t) * new_size);
    if (new_set) {
        cache[set_index] = new_set;
        set_stats[set_index].current_size = new_size;
        eviction_count += (size - new_size); // how many entries got purged
    } else {
        fprintf(stderr, "this should never happen\n");
    }
}
/* Access data at memory address addr */
void accessData(mem_addr_t addr) {
    mem_addr_t set_index = (addr >> b) & set_index_mask;
    mem_addr_t tag = addr >> (s + b);
    cache_set_t cache_set = cache[set_index];
    int current_size = set_stats[set_index].current_size;
    
    // Check for hit
    for (int i = 0; i < current_size; i++) {
        if (cache_set[i].valid && cache_set[i].tag == tag) {
            hit_count++;
            cache_set[i].frequency++;
            if (verbosity)
                printf("hit ");
            return;
        }
    }

    /* If we reach this line, then we have a cache miss */
    miss_count++;
    if (verbosity)
        printf("miss ");

    // Find empty spot
    int empty_index = -1;
    for (int i = 0; i < current_size; i++) {
        if (!cache_set[i].valid) {
            empty_index = i;
            break;
        }
    }

    // If no empty spot -> try to grow, if fails then evict to save memory
    if (empty_index == -1) {
        int new_size = current_size + 1; // allocating space for one more item
        cache_line_t* new_set = realloc(cache_set, sizeof(cache_line_t) * new_size);
        
        if (new_set) {
            cache[set_index] = new_set;
            cache_set = new_set;
            empty_index = current_size;
            set_stats[set_index].current_size = new_size; // updating the size
            set_stats[set_index].max_size = new_size;
        } else { // this should never happen
            handle_memory_overflow(cache_set, set_index);
            // After mass eviction -> find empty spot again
            for (int i = 0; i < current_size; i++) {
                if (!cache_set[i].valid) {
                    empty_index = i;
                    break;
                }
            }
        }
    }

    // Add the new  entry
    if (empty_index != -1) {
        cache_set[empty_index].valid = 1;
        cache_set[empty_index].tag = tag;
        cache_set[empty_index].frequency = 1;
    }
}

/*
 * replayTrace - replays the given trace file against the cache 
 */
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);
      
            if(verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

            accessData(addr);

            /* If the instruction is R/W then access again */
            if(buf[1]=='M')
                accessData(addr);
            
            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

void printUsage(char* argv[]) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}



int main(int argc, char* argv[]) {
    char c;
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch(c) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = 1;
                break;
            case 'h':
                printUsage(argv);
                exit(0);
            default:
                printUsage(argv);
                exit(1);
        }
    }

    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    S = (unsigned int) pow(2, s);
    B = (unsigned int) pow(2, b);
 
    initCache();
    replayTrace(trace_file);
    freeCache();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}