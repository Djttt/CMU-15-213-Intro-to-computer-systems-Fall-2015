/**
 * prototypes for cache used to proxy
 */
#include "csapp.h"
#define MAX_OBJECT_SIZE 102400  // 100 KB
#define MAX_CACHE_SIZE 1048576  // 1 MB

/* include url, buf, size, and prev, next pointer*/
typedef struct cache_obj
{
    char url[MAXLINE];
    char *buf;
    int size;
    struct cache_obj *prev;
    struct cache_obj *next;
}cache_obj_t;


typedef struct {
    cache_obj_t *head;
    cache_obj_t *tail;
    sem_t mutex;
    int total_size;
    
}cache_t;

void cache_init(cache_t *cache);
cache_obj_t *cache_lookup(cache_t *cache, char *url);
void cache_insert(cache_t *cache, char *url, char *buf, int size);
cache_obj_t *Cache_lookup(cache_t *cache, char *url);
void Cache_insert(cache_t *cache, char *url, char *buf, int size);












