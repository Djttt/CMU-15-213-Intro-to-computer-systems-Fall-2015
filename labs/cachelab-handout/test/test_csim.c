/**
 * This file is to test all function in csim.c
 */
#include <stdlib.h>
#include <assert.h>
#include "csim.h"


// test strategy
// 


void test_get_address_tag() {
    unsigned address = 0x0000;
    assert(get_address_tag(address, 2, 1) == 0);
    address = 0x1000;
    assert(get_address_tag(address, 2, 1) == 1);
    printf("All get_address_tag() tests passed!\n");
}


void test_get_address_set() {
    unsigned address = 0x0421c7f0;
    assert(get_address_set(address, 4, 2) == 33);

    printf("All get_address_set() tests passed!\n");
}


void test_get_address_block_offset() {
    unsigned address = 0x04f6b868;
    assert(get_address_block_offset(address, 1, 2) == 8);

    address = 0x04f6b018;
    assert(get_address_block_offset(address, 3, 2) == 24);
    printf("All get_address_block_offset() tests passed!\n");
}


void test_modify_cache_state() {
    int cache_flag = 1;
    int hits = 0;
    int misses = 0;
    int eviction = 0;
    modify_cache_state(cache_flag, &hits, &misses, &eviction);
    assert(hits == 1);
    assert(misses == 0);
    assert(eviction == 0);

    cache_flag = -1;
    modify_cache_state(cache_flag, &hits, &misses, &eviction);
    assert(misses == 1);
    assert(eviction == 1);
    assert(hits == 1);

    cache_flag = 0;
    modify_cache_state(cache_flag, &hits, &misses, &eviction);
    assert(hits == 1);
    assert(misses == 2);
    assert(eviction == 1);

    printf("All modify_cache_state tests passed!\n");

}


void test_cache_access() {
    int S = 4;
    int E = 2;
    int s = 2;
    int b = 2;
    int cache_flag;
    unsigned address = 0x0400d7d4;  
    unsigned tag = get_address_tag(address, b, s);
    unsigned set_index = get_address_set(address, b, s);
    unsigned block_offset = get_address_block_offset(address, b, s);
    cache_line** cache = (cache_line**)malloc(S * E * sizeof(cache_line));
    
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 0);
    
    cache_flag = cache_access(cache, set_index, E, tag, block_offset);
    assert(cache_flag == 1);

    free(cache);

}


void test_cache_insert() {
    int s = 4;
    int E = 1;
    int b = 4;
    int S = 16;
    int tag;
    int cache_insert_flag;
    cache_line** cache = (cache_line**)malloc(S * E * sizeof(cache_line));

    unsigned address = 10;
    tag = get_address_tag(address, b, s);
    cache_insert_flag = cache_insert(cache, S, E, tag);
    assert(cache_insert_flag == 0);

    free(cache);
}


void test_LRU() {
    int s = 4;
    int E = 1;
    int b = 4;
    int S = 16;
    unsigned address = 10;
    int cache_flag;
    cache_line** cache = (cache_line**)malloc(S * E * sizeof(cache_line));

    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 0);
    

    // M access mode
    address = 20;
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 0);
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 1);

    address = 22;
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 1);

    address = 18;
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 1);

    address = 110;
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == -1);

    address = 210;
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == -1);

    // M access mode
    address = 12;
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == -1);
    cache_flag = cache_access(cache, address, E, b, s);
    assert(cache_flag == 1);


    free(cache);
}
