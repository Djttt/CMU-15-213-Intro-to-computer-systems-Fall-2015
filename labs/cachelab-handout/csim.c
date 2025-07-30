/*
 * this csim.c is for cache simulator, taking valgrind memory trace as input
 * simulates the hit/miss behavir of cache memory, and ouputs the total number of hits, misses and evictions.
 */
#include "cachelab.h"
#include "csim.h"
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

static const char* HELP_STRING = "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n \
  -h\t\t Print this help message.\n \
  -v\t\t Optional verbose flag.\n \
  -s <num>\t Numberof of set index bits.\n \
  -E <num>\t Number of lines per set.\n \
  -b <num>\t Number of block offset bits.\n \
  -t <file>\t Trace file.\n\n \
Examples:\n \
linux> ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n \
linux> ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n";


static const char* NUM_TO_CACHE_INFO[] = {"miss", "hit", "miss eviction"};


int main(int argc, char** argv)
{
    // s represent set index bits(S = 2^s)
    // E represent number of lines per set 
    // b represent number of block bits(B = 2^b)
    int s, E, b;
    // S: number of sets
    // B: number of blocks
    int S, B;
    int opt;
    int cache_flag_intern;
    
    // global lru counter
    unsigned long time = 0;
    
    // default don't open verbose
    int verbose = 0;
    
    char* tracefile;
    FILE* trace_file;       // file discriptor for trace file
    char identifier;        // identifier for access mode
    unsigned long long address;       // memory address
    int size;               // read bytes size 
    
    int hits = 0, misses = 0, evictions = 0;

    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(opt) {
            case 'h':
                printf("%s", HELP_STRING);
                return 0;
                break;
            case 'v':
                verbose = 1;
                break;
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
                tracefile = optarg;
                break;
            default:
                printf("wrong argument\n");
                break;
        }
    }

    S = pow(2, s);
    B = pow(2, b);
    cache_line** cache = (cache_line**)malloc(S * sizeof(cache_line*));
    for (int i = 0; i < S; i++) {
        cache[i] = malloc(E * sizeof(cache_line));
        for (int j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = -1;
            cache[i][j].lru_counter = 0;
            cache[i][j].block = malloc(B);
        }
    }

    trace_file = fopen(tracefile, "r");
    
    // access main memory line by line
    // %llx don't use %x read 32 bits 
    // trans.trace's address length is 35
    while(fscanf(trace_file, " %c %llx,%d", &identifier, &address, &size) > 0) {
        int cache_flag;
        switch (identifier) {
            case 'M':
                /* modify operation, M is treated as once L followed 
                by S at the same address */
                time++;
                cache_flag = cache_access(cache, address, E, b, s, time);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                cache_flag_intern = cache_flag;
                // S operation
                time++;
                cache_flag = cache_access(cache, address, E, b, s, time);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                if (verbose) {
                    if (cache_flag_intern == 1) {
                        printf("M %llx, %d hits\n", address, size);
                    }
                    else {
                        if (cache_flag_intern == -1) {
                            printf("M %llx, %d miss eviction hit\n", address, size);
                        }
                        else {
                            printf("M %llx, %d miss hit\n", address, size);
                        }
                    }
                }
                break;
            case 'L':
                /* load operation */
                time++;
                cache_flag = cache_access(cache, address, E, b, s, time);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                break;
            case 'S':
                /* data store operation */
                time++;
                cache_flag = cache_access(cache, address, E, b, s, time);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                break;
            default:
                break;
        }
        if (verbose) {
            if (identifier != 'M' && identifier != 'I') {
                if (cache_flag == -1) {
                    cache_flag = 2;
                }
                printf("%c %llx, %d %s\n", identifier, address, size, NUM_TO_CACHE_INFO[cache_flag]);
            }
        }
    }


    printSummary(hits, misses, evictions);
    free_cache_blocks(cache, S, E);
    fclose(trace_file);
    free(cache);
    return 0;
}



unsigned get_address_tag(unsigned long long address, int b, int s) {
    return (address >> (b + s));
}


unsigned get_address_set(unsigned long long address, int b, int s) {
    address = address >> b;
    unsigned mask = (1u << s) - 1;
    return address & mask;
}


unsigned get_address_block_offset(unsigned long long address, int b, int s) {
    unsigned mask = (1u << b) - 1;
    return address & mask;
}


void modify_cache_state(int cache_flag, int* hits, int* misses, int* eviction) {
    if (cache_flag == 1) {
        (*hits)++;
    }
    else if (cache_flag == -1) {
        (*misses)++;
        (*eviction)++;
    }
    else {
        (*misses)++;
    }
}


// static void check_Rep(cache_line* cache_line, int S, int B) {
//     assert(cache_line->valid == 0 || cache_line->valid == 1);
//     assert(cache_line->lru_counter >= 0);
// }



int cache_access(cache_line** cache, unsigned long long address, int E, int b, int s, unsigned time) {
    int i = 0;
    int cache_flag;
    unsigned long long tag = get_address_tag(address, b, s);
    unsigned long long set_index = get_address_set(address, b, s);

    for (; i < E; i++) {
        cache_line* cache_line = &cache[set_index][i];
        if(cache_line->valid && cache_line->tag == tag) {
            // hits
            cache_line->lru_counter = time;
            return 1;
        }
    }
    
    cache_flag = cache_insert(cache, set_index, E, tag, time);

    return cache_flag;
}


int cache_insert(cache_line** cache, int set_index, int E, int target_tag, unsigned time) {
    int i = 0;
    int lru_index;
    for (; i < E; i++) {
        cache_line* line = &cache[set_index][i];
        if (!line->valid) {
            // this cache line can insert
            line->valid = 1;
            line->tag = target_tag;
            line->lru_counter = time;
            return 0;
        }
    }
    // run lru algorithm and update valid and tag field
    lru_index = LRU(cache, set_index, E);
    cache[set_index][lru_index].valid = 1;
    cache[set_index][lru_index].tag = target_tag;
    cache[set_index][lru_index].lru_counter = time;
    return -1;
}


int LRU(cache_line** cache, int s, int E) {
    int i = 0;
    // lru index for replacement cache line
    int lru_index = 0;
    int min_lru_counter = cache[s][0].lru_counter;
    for (; i < E; i++) {
        cache_line line = cache[s][i];
        if (line.lru_counter < min_lru_counter) {
            min_lru_counter = line.lru_counter;
            lru_index = i;
        }
    }
    return lru_index;
}


void free_cache_blocks(cache_line** cache, int S, int E) {

    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            free(cache[i][j].block);
        }
    }
}




