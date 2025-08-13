/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "sun",
    /* First member's full name */
    "joash",
    /* First member's email address */
    "joash@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Tunable constants */
#define ALIGNMENT 8
#define CHUNKSIZE (1<<12) /* 4KB extend size */
#define MIN_BLOCK_SIZE 32

/* Align `x` up to nearest multiple of ALIGNMENT */
#define ALIGN(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic header/footer operations (hdr/ftr point to size_t words) */
#define PACK(size, alloc) ((size) | (alloc))
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given a payload pointer `bp`, compute header/footer addresses.  In our
   layout payload is after: HDR | NEXT | PREV  (3 size_t-sized fields). */
#define HDRP(bp) ((char *)(bp) - 3 * SIZE_T_SIZE)
#define FTRP(bp) (HDRP(bp) + GET_SIZE(HDRP(bp)) - SIZE_T_SIZE)

/* Header-based navigation */
#define HDR_FROM_HDR_NEXT(hdr) ((char *)(hdr) + GET_SIZE(hdr))
#define FTR_FROM_HDR(hdr) ((char *)(hdr) + GET_SIZE(hdr) - SIZE_T_SIZE)

/* next header from payload */
#define NEXT_HDRP(bp) (HDRP(bp) + GET_SIZE(HDRP(bp)))

/* previous footer/header calculation (safe: prologue is allocated) */
#define PRE_FTRP(bp) ((char *)HDRP(bp) - SIZE_T_SIZE)
#define PRE_HDRP(bp) ((char *)PRE_FTRP(bp) - GET_SIZE(PRE_FTRP(bp)) + SIZE_T_SIZE)

/* Free-list link accessors: these operate on the header-pointer address
   where the next and prev pointers are stored at offsets +1*SIZE_T_SIZE,
   +2*SIZE_T_SIZE respectively. We access them as void* pointers. */
static inline void *get_next_free(char *hdr) { return *(void **)(hdr + SIZE_T_SIZE); }
static inline void *get_prev_free(char *hdr) { return *(void **)(hdr + 2 * SIZE_T_SIZE); }
static inline void set_next_free(char *hdr, void *ptr) { *(void **)(hdr + SIZE_T_SIZE) = ptr; }
static inline void set_prev_free(char *hdr, void *ptr) { *(void **)(hdr + 2 * SIZE_T_SIZE) = ptr; }

/* separate list number */
#define LIST_CLASS 10

/* Global free-list head (points to header of a free block) */
static char *free_list[LIST_CLASS]; 

static int get_list_index(size_t size) {
    int idx = 0;
    size_t s = 16;
    while (idx < LIST_CLASS - 1 && size > s) {
        s <<= 2;
        idx++;
    }
    return idx;
}

/* Forward declarations */
static void *extend_heap(size_t words);
static void insert_free_block(char *hdr);
static void remove_free_block(char *hdr);
static void *coalesce(char *hdr);
static char *find_fit(size_t asize);
static char *place_and_split(char *hdr, size_t asize);

/* mm_init: create initial empty heap with prologue and epilogue */
int mm_init(void)
{
    char *heap_start = mem_sbrk(5 * SIZE_T_SIZE);
    if (heap_start == (void *) -1) return -1;

    /* Prologue block: header | next | prev | footer (allocated) */
    PUT(heap_start, PACK(4 * SIZE_T_SIZE, 1));              /* prologue header */
    /* next / prev pointers of prologue (unused) */
    set_next_free(heap_start, NULL);
    set_prev_free(heap_start, NULL);
    PUT(heap_start + 3 * SIZE_T_SIZE, PACK(4 * SIZE_T_SIZE, 1)); /* prologue footer */

    /* Epilogue header */
    PUT(heap_start + 4 * SIZE_T_SIZE, PACK(0, 1));

    for (int i = 0; i < LIST_CLASS; i++) { free_list[i] = NULL; }

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / SIZE_T_SIZE) == NULL) return -1;
    return 0;
}

/* extend_heap: extend heap by `words` words (size_t sized units). returns
   pointer to payload of new free block or NULL on error. */
static void *extend_heap(size_t words)
{
    size_t size = ALIGN(words * SIZE_T_SIZE);
    if (size < MIN_BLOCK_SIZE) size = MIN_BLOCK_SIZE;

    char *bp = mem_sbrk(size);
    if (bp == (void *) -1) return NULL;

    /* bp now points to where we'll write the new header. We'll set header
       and footer then new epilogue. Note: mem_sbrk returns start of new area. */
    PUT(bp, PACK(size, 0));                         /* free block header */
    PUT(bp + size - SIZE_T_SIZE, PACK(size, 0));   /* free block footer */
    PUT(bp + size, PACK(0, 1));                     /* new epilogue header */

    /* coalesce and insert into free list */
    char *new_hdr = coalesce(bp);
    return (new_hdr == NULL) ? NULL : (void *)(new_hdr + 3 * SIZE_T_SIZE);
}

/* insert_free_block: insert header `hdr` at front of explicit free list */
static void insert_free_block(char *hdr)
{
    if (!hdr) return;
    int idx = get_list_index(GET_SIZE(hdr));
    set_next_free(hdr, free_list[idx]);
    set_prev_free(hdr, NULL);
    if (free_list[idx]) set_prev_free(free_list[idx], hdr);
    free_list[idx] = hdr;
}

/* remove_free_block: unlink `hdr` from explicit free list */
static void remove_free_block(char *hdr)
{
    if (!hdr) return;
    int idx = get_list_index(GET_SIZE(hdr));
    char *prev = get_prev_free(hdr);
    char *next = get_next_free(hdr);

    if (prev) set_next_free(prev, next);
    else free_list[idx] = next;

    if (next) set_prev_free(next, prev);
}

/* coalesce: hdr points to header of block being freed (or newly created free
   block). Merge with adjacent free blocks if possible and return header of
   resulting (coalesced) block. */
static void *coalesce(char *hdr)
{
    /* payload pointer for convenience */
    char *bp = hdr + 3 * SIZE_T_SIZE;
    char *next_hdr = NEXT_HDRP(bp);

    int prev_alloc = GET_ALLOC(PRE_FTRP(bp));
    int next_alloc = GET_ALLOC(next_hdr);
    size_t size = GET_SIZE(hdr);

    if (prev_alloc && next_alloc) {
        /* Case 1: both allocated, nothing to do */
    } else if (prev_alloc && !next_alloc) {
        /* Case 2: merge with next */
        remove_free_block(next_hdr);
        size += GET_SIZE(next_hdr);
        PUT(hdr, PACK(size, 0));
        PUT(hdr + size - SIZE_T_SIZE, PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {
        /* Case 3: merge with previous */
        char *prev_hdr = PRE_HDRP(bp);
        remove_free_block(prev_hdr);
        size += GET_SIZE(prev_hdr);
        hdr = prev_hdr;
        PUT(hdr, PACK(size, 0));
        PUT(hdr + size - SIZE_T_SIZE, PACK(size, 0));
    } else {
        /* Case 4: merge both prev and next */
        char *prev_hdr = PRE_HDRP(bp);
        remove_free_block(prev_hdr);
        remove_free_block(next_hdr);
        size += GET_SIZE(prev_hdr) + GET_SIZE(next_hdr);
        hdr = prev_hdr;
        PUT(hdr, PACK(size, 0));
        PUT(hdr + size - SIZE_T_SIZE, PACK(size, 0));
    }

    insert_free_block(hdr);
    return hdr;
}

/* find_fit: first-fit search through free list for block >= asize; returns
   header pointer or NULL if none found. */
static char *find_fit(size_t asize)
{
    int idx = get_list_index(asize);
    for (; idx < LIST_CLASS; idx ++) {
        for (char *hdr = free_list[idx]; hdr != NULL; hdr = get_next_free(hdr)) {
            size_t bsize = GET_SIZE(hdr);
            if (!GET_ALLOC(hdr) && bsize >= asize) return hdr;
        }
    }

    return NULL;    
}

/* place_and_split: mark [hdr] as allocated; if remainder >= MIN_BLOCK_SIZE,
   split and insert remaining free portion. Returns payload pointer. */
static char *place_and_split(char *hdr, size_t asize)
{
    size_t bsize = GET_SIZE(hdr);
    remove_free_block(hdr);

    if (bsize - asize >= MIN_BLOCK_SIZE) {
        /* Split: allocate front portion, leave remainder as free block */
        PUT(hdr, PACK(asize, 1));
        PUT(hdr + asize - SIZE_T_SIZE, PACK(asize, 1));

        char *rem_hdr = hdr + asize;
        size_t rem_size = bsize - asize;
        PUT(rem_hdr, PACK(rem_size, 0));
        PUT(rem_hdr + rem_size - SIZE_T_SIZE, PACK(rem_size, 0));
        insert_free_block(rem_hdr);
    } else {
        /* No split; allocate whole block */
        PUT(hdr, PACK(bsize, 1));
        PUT(hdr + bsize - SIZE_T_SIZE, PACK(bsize, 1));
    }

    return hdr + 3 * SIZE_T_SIZE; /* payload pointer */
}

/* mm_malloc: allocate size bytes and return payload pointer */
void *mm_malloc(size_t size)
{
    if (size == 0) return NULL;

    /* Compute adjusted block size: header + next + prev + payload + footer */
    size_t asize = ALIGN(size + 4 * SIZE_T_SIZE);
    if (asize < MIN_BLOCK_SIZE) asize = MIN_BLOCK_SIZE;

    /* Search free list */
    char *hdr = find_fit(asize);
    if (hdr) return place_and_split(hdr, asize);

    /* No fit found --- extend heap */
    size_t extend_size = (asize > CHUNKSIZE) ? asize : CHUNKSIZE;
    char *bp = extend_heap(extend_size / SIZE_T_SIZE);
    if (!bp) return NULL;

    /* bp returned is payload pointer; header is bp - 3*SIZE_T_SIZE */
    return place_and_split(HDRP(bp), asize);
}

/* mm_free: free a previously allocated payload pointer */
void mm_free(void *ptr)
{
    if (!ptr) return;

    char *hdr = HDRP(ptr);
    if (!GET_ALLOC(hdr)) return; /* already free */

    size_t size = GET_SIZE(hdr);
    PUT(hdr, PACK(size, 0));
    PUT(hdr + size - SIZE_T_SIZE, PACK(size, 0));
    coalesce(hdr);
}

/* mm_realloc: simple implement using malloc/copy/free */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) return mm_malloc(size);
    if (size == 0) { mm_free(ptr); return NULL; }

    size_t old_size = GET_SIZE(HDRP(ptr));
    size_t copy_size = old_size - 4 * SIZE_T_SIZE; /* payload bytes available */
    if (copy_size > size) copy_size = size;

    void *newptr = mm_malloc(size);
    if (!newptr) return NULL;
    memcpy(newptr, ptr, copy_size);
    mm_free(ptr);
    return newptr;
}

/* Optional: utility to dump free list (for debugging) */
#ifdef DEBUG
void dump_free_list(void)
{
    fprintf(stderr, "Free list:");
    for (char *hdr = free_list_head; hdr != NULL; hdr = get_next_free(hdr)) {
        fprintf(stderr, " -> [%p sz=%zu]", hdr, GET_SIZE(hdr));
    }
    fprintf(stderr, "\n");
}
#endif

/**
 * Memory checker for checking memory invariant.
 */
int mm_check(void) {
    void *start = NULL;
    void *end = mem_heap_hi();
    char *p = (char *)start + 4 * SIZE_T_SIZE;
    int block_counter = 0;

    while (GET_SIZE(p) > 0) {                                                                       
        int blk_size = GET_SIZE(p);
        int alloc = GET_ALLOC(p);

        void *next_blk = NEXT_HDRP(p + SIZE_T_SIZE);
        void *cur_header = p;
        void *cur_footer = FTRP(p + SIZE_T_SIZE);
        void *pre_blk, *pre_footer;

        int pre_alloc = 1;
        if (block_counter > 0) {
            pre_blk = PRE_HDRP(p + SIZE_T_SIZE);
            pre_footer = PRE_FTRP(p + SIZE_T_SIZE);
            pre_alloc = GET_ALLOC(pre_blk);
        }
        int next_alloc = GET_ALLOC(next_blk);
        int cur_alloc = GET_ALLOC(p);

        
        /* check out overlap wiht prev and next block */
        if ((char *)cur_footer > (char *)next_blk) {
            printf("currently block overlap with next block\n");
            break;
        }
        if (!pre_footer) {
            if ((char *)pre_footer > (char *)cur_header) {
                printf("currently block overlap with prev block\n");
                break;
            }
        }


        /* check out heap whether exite contiguous free block */
        if (!cur_alloc && (!pre_alloc || !next_alloc)) {
            printf("Heap exites contiguous free blocks! Please \
                check out coalescing in free function\n");
            break;
        }
        
        
        /* check out pointer in heap point to valid heap address */
        if (p < (char *)start || p > (char*) end) {
            printf("Pointer in heap point to invalid heap address!\n");
            break;
        }
        
        // if (!alloc) {
        //     printf("free block at %p\n", p);
        //     printf("block id: %d, block alloc: %d, block size: %d\n", block_counter, cur_alloc, blk_size);
        //     printf("block header: %p, block footer: %p\n", p, cur_footer);
        // }

        p += blk_size;
        block_counter++;
        
    }
}


/**
 * helper function - print memory blocks address and other helpful information for debugging
 */
void print_memory_block() {
    void *start = mem_heap_lo();
    void *end = mem_heap_hi();
    char *p = (char *)start + 3 * SIZE_T_SIZE;
    int block_counter = 0;


    while (GET_SIZE(p) > 0) {
        int blk_size = GET_SIZE(p);
        void *cur_header = p;
        void *cur_footer = FTRP(p + SIZE_T_SIZE);

        int cur_alloc = GET_ALLOC(p);

        printf("block id: %d, block alloc: %d, block size: %d\n", block_counter, cur_alloc, blk_size);
        printf("block header: %p, block footer: %p\n", p, cur_footer);
        p += blk_size;
        block_counter++;
    }
}














