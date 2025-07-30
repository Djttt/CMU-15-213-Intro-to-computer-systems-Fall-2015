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
    
    char* tracefile;
    FILE* trace_file;       // file discriptor for trace file
    char identifier;        // identifier for access mode
    unsigned address;       // memory address
    int size;               // read bytes size 
    
    int hits, misses, evictions;

    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(opt) {
            case 'h':
                break;
            case 'v':
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
    cache_line** cache = (cache_line**)malloc(S * E * sizeof(cache_line));
    init_cache_blocks(cache, B);
    trace_file = fopen(tracefile, "r");
    
    // access main memory line by line
    while(fscanf(trace_file, " %c %x,%d", &identifier, &address, &size) > 0) {
        int s, tag, block_offset, cache_flag;
        s = get_address_set(address);
        tag = get_address_tag(address);
        block_offset = get_address_block_offset(address);
        switch (identifier) {
            case 'M':
                /* modify operation, M is treated as once L followed 
                by S at the same address */
                cache_flag = cache_access(cache, s, tag, block_offset);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);

                // S operation
                cache_flag = cache_access(cache, s, tag, block_offset);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                break;
            case 'L':
                /* load operation */
                cache_flag = cache_access(cache, s, tag, block_offset);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                break;
            case 'S':
                /* data store operation */
                cache_flag = cache_access(cache, s, tag, block_offset);
                modify_cache_state(cache_flag, &hits, &misses, &evictions);
                break;
            default:
                printf("wrong identifier for access mode!");
                break;
        }
    }


    printSummary(0, 0, 0);
    free_cache_blocks(cache, S, E);
    fclose(trace_file);
    free(cache);
    return 0;
}



unsigned get_address_tag(unsigned address, int b, int s) {
    return (address >> (b + s)) & 0xffffffff;
}


unsigned get_address_set(unsigned address, int b, int s) {
    address = address >> b;
    unsigned mask = (1u << s) - 1;
    return address & mask;
}


unsigned get_address_block_offset(unsigned address, int b, int s) {
    unsigned mask = (1u << b) - 1;
    return address & mask;
}


void modify_cache_state(int cache_flag, int* hits, int* misses, int* eviction) {
    if (cache_flag == 1) {
        *hits++;
    }
    else if (cache_flag == -1) {
        *misses++;
        *eviction++;
    }
    else {
        *misses++;
    }
}


static void check_Rep(cache_line* cache_line, int S, int B) {
    assert(cache_line->valid == 0 || cache_line == 1);
    assert(cache_line->lru_counter >= 0);
}


int init_cache_blocks(cache_line** cache, int S, int E, int B) {
    int i = 0;
    int j = 0;
    for (; i < S; i++) {
        for (; j < E; j++) {
            cache[i][j].block = (int*)malloc(B * sizeof(int));
        }
    }
}


int cache_access(cache_line** cache, int S, int E, int tag, int block_offset) {
    int i = 0;
    int cache_flag;
    for (; i < E; i++) {
        cache_line* cache_line = cache[S][i];
        if(cache_line->valid && cache_line->tag == tag) {
            // hits
            cache_line->lru_counter++;
            return 1;
        }
        else {
            cache_flag = cache_insert(cache, S, E, tag);
        }
    }
    return cache_flag;
}


int cache_insert(cache_line** cache, int S, int E, int tag) {
    int i = 0;
    int lru_index;
    for (; i < E; i++) {
        cache_line line = cache[S][i];
        if (!line.valid) {
            // this cache line can insert
            line.valid = 1;
            line.tag = tag;
            return 0;
        }
    }
    // run lru algorithm and update valid and tag field
    lru_index = LRU(cache, S, E);
    cache[S][lru_index].valid = 1;
    cache[S][lru_index].tag = tag;
    cache[S][lru_index].lru_counter = 1;
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
    int i = 0;
    int j = 0;
    for (; i < S; i++) {
        for (; j < E; j++) {
            free(cache[i][j].block);
        }
    }
}




