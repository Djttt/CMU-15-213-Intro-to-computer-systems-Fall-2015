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
                break;
            case 'L':
                cache_flag = cache_access(cache, s, tag, block_offset);
                if (cache_flag) {
                    hits++;
                }
                else {
                    misses++;
                }
                break;
            case 'S':
                break;
            default:
                printf("wrong identifier for access mode!");
                break;
        }
    }

    

    
    printSummary(0, 0, 0);
    fclose(trace_file);
    free(cache);
    return 0;
}

/**
 * Get main memory address' tag number which is used in cache
 * @param address unsigned hex number of main memory address 
 * @return tag symbol for cache 
 */
unsigned get_address_tag(unsigned address) {
    
}

/**
 * Get main memory address' set number which is used in cache
 * @param address unsigned hex number of main memory address 
 * @return set number for cache 
 */
unsigned get_address_set(unsigned address) {

}


/**
 * Get main memory address' block offset number which is used in cache
 * @param address unsigned hex number of main memory address 
 * @return block offset for cache
 */
unsigned get_address_block_offset(unsigned address) {

}





