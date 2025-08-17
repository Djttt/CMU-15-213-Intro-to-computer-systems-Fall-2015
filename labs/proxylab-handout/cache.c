#include "cache.h"

/**
 * init cache
 */
void cache_init(cache_t *cache) {
    cache->head = NULL;
    cache->tail = NULL;
    cache->total_size = 0;

    Sem_init(&cache->mutex, 0, 1);
}

/**
 * insert a new cache obj into cache and eviction policy by LRU
 * @param cache a pointer to point web object cache
 * @param url new object's url(as key)
 * @param buf the buffer to save the web object
 * @param size  web object's size
 */
void cache_insert(cache_t *cache, char *url, char *buf, int size) {
    if (size > MAX_OBJECT_SIZE) return;
    
    while (cache->total_size + size > MAX_CACHE_SIZE && cache->tail) {
        cache_obj_t *old = cache->tail;
        cache->tail = old->prev;
        if (cache->tail) cache->tail->next = NULL;
        cache->total_size -= old->size;
        free(old->buf);
        free(old);
    }

    cache_obj_t *obj = malloc(sizeof(cache_obj_t));
    strcpy(obj->url, url);
    obj->buf = malloc(size);
    memcpy(obj->buf, buf, size);
    obj->size = size;
    obj->prev = NULL;
    obj->next = cache->head;

    if (cache->head) cache->head->prev = obj;
    cache->head = obj;
    if (!cache->tail) cache->tail = obj;
    cache->total_size += obj->size;
}

/**
 * iterate cache to find target url cache obj 
 * if find target, put it int the head. Else, return NULL
 * @param cache a pointer to point web object cache
 * @param url look up target url
*/
cache_obj_t *cache_lookup(cache_t *cache, char *url) {
    cache_obj_t *p = cache->head;
    while (p) {
        if (!strcmp(p->url, url)) {
            if (p != cache->head) {
                if (p->prev) p->prev->next = p->next;
                if (p->next) p->next->prev = p->prev;
                if (p == cache->tail) cache->tail = p->prev; 

                /* insert p into head */
                p->next = cache->head;
                p->prev = NULL; 
                cache->head->prev = p;
                cache->head = p;
            }
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/**********************************
 * Wrappers for cache package
 **********************************/
cache_obj_t *Cache_lookup(cache_t *cache, char *url) {
    cache_obj_t *p = NULL;
    P(&cache->mutex);
    p = cache_lookup(cache, url);
    V(&cache->mutex);
    return p;
}


void Cache_insert(cache_t *cache, char *url, char *buf, int size) {
    P(&cache->mutex);
    cache_insert(cache, url, buf, size);
    V(&cache->mutex);
}
