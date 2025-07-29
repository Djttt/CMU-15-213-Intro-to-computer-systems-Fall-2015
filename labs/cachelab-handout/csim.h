/* 
 * csim.h - Prototypes for csim.c helper function
 */

#ifndef CACHE_SIMILATOR_H
#define CACHE_SIMILATOR_H

/*
 * Represent one cache line in cache 
 */
typedef struct cache_line{
    static int valid;               // valid bit for cache line
    static int tag;                 // tag symbol 
    static int lru_counter;         // for LRU replace policy
    static int* block;              // block area for cache line
} cache_line;

/**
 * rep invariant: valid = 0 or 1, 0 < tag < 2^(address_length - s - b),
 * lru_counter >= 0, 0 < block.length < 2^b
 * abstraction function: represent one cache line 
 * safety from exposure: 
 * all fields are static, that means only can access the field in this file.
 */

/**
 * check rep_invariant 
 */
static void check_Rep() {

}


/**
 * Allocate B blocks in each cache line
 * @param cache target cache
 * @param B each cache init with B blocks
 * @return allocate memory success return 1, else return 0.
 */
int init_cache_blocks(cache_line** cache, int B);

/**
 * access cache by inputing memory address, and memory address divided into three parts,
 * S, tag, block_offset. if access success, return 1. otherwise return 0.
 * @param cache target cache is 2D array of cache_line object
 * @param S set number of this main memory address for cache(0 < S < cache.length)
 * @param tag tag number of main memory address (0 < tag < 2^(address_length - s - b))
 * @param block_offset block offset in cache line (0 < block_offset < 2^b)
 * @return if this memory value already exited in cache(hit), return 1. otherwise(miss) return 0.
 */
int cache_access(cache_line** cache, int S, int tag, int block_offset);


/**
 * if cache missed, insert memory address's value to cache, 
 * and set up responding arguments
 * @param S insert target set number in cache(0 < S < cache.length)
 * @param E insert line number of set(0 < E < cache[0].length)
 * @param cache insert target cache
 * @return insert with eviction return 1, insert without eviction return 0, else failed return -1.
 */
int cache_insert(cache_line** cache, int S, int E);


/**
 * least-recently used replacement policy implemention for choosing cache line to evict
 * @param cache target cache
 * @param S target set number for replace new main address value(0 < S < cache.length)
 * @return return replacement index in cache if success, otherwise return 0.
 */
int LRU(cache_line** cache, int S);

#endif /* CACHE_SIMILATOR_H  */

