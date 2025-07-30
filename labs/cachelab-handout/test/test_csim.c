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


void test_init_cache_blocks() {
    
}

void test_cache_access() {

}


void test_cache_insert() {

}


void test_LRU() {

}

void test_free_cache_blocks() {

}