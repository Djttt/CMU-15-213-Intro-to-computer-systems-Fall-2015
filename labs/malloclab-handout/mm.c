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

#define MAX_BLOCK_SIZE 16

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 读写 header/footer（p 必须指向 header 或 footer） */
#define GET(p)        (*(size_t *)(p))
#define PUT(p, val)   (*(size_t *)(p) = (val))

/* 打包 / 解包 */
#define PACK(size, alloc) ((size) | (alloc))
#define GET_SIZE(p)   (GET(p) & ~0x7)   /* p = header 或 footer */
#define GET_ALLOC(p)  (GET(p) & 0x1)    /* 0/1 */

/* 设置/清除 alloc 位（写回内存）*/
#define SET_ALLOC_FLAG(p)   (PUT((p), GET(p) | (size_t)1))
#define CLEAR_ALLOC_FLAG(p) (PUT((p), GET(p) & ~(size_t)1))

/* 基于 payload bp 的常用偏移（bp 是 payload 指针） */
#define HDRP(bp) ((char *)(bp) - SIZE_T_SIZE)                     /* payload -> header */
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - 2*SIZE_T_SIZE) /* payload -> footer */

/* 基于 header 的邻居计算（更直观、常用） */
#define HDR_FROM_HDR_NEXT(hdr) ((char *)(hdr) + GET_SIZE(hdr))            /* next header */
#define FTR_FROM_HDR(hdr)      ((char *)(hdr) + GET_SIZE(hdr) - SIZE_T_SIZE) /* footer of same block */

/* 基于 payload 计算前一块和下一块（替代复杂宏） */
#define NEXT_HDRP(bp)   (HDRP(bp) + GET_SIZE(HDRP(bp)))                       /* payload -> next header */
#define NEXT_FTRP(bp)   (NEXT_HDRP(bp) + GET_SIZE(NEXT_HDRP(bp)) - SIZE_T_SIZE) /* payload -> next footer */

/* 更安全地计算 prev footer/header：先用 cur header，然后回退 */
#define PRE_FTRP(bp)    ((char *)HDRP(bp) - SIZE_T_SIZE) /* previous block's footer */
#define PRE_HDRP(bp)    ((char *)PRE_FTRP(bp) - GET_SIZE(PRE_FTRP(bp)) + SIZE_T_SIZE)


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

    char *heap_start = mem_sbrk(4 * SIZE_T_SIZE);
    if (heap_start == (void *)-1) {
        return -1;
    }
    PUT(heap_start, 0);
    PUT(heap_start + SIZE_T_SIZE, PACK(SIZE_T_SIZE, 1));
    PUT(heap_start + 2 * SIZE_T_SIZE, PACK(SIZE_T_SIZE, 1));
     PUT(heap_start + 3 * SIZE_T_SIZE, PACK(SIZE_T_SIZE, 1)); 

    size_t extend_size = 4096 + SIZE_T_SIZE;
    char *bp = mem_sbrk(extend_size);
    if (bp == (void *)-1) return -1;
    PUT(bp, PACK(extend_size - SIZE_T_SIZE, 0));
    PUT(bp + extend_size - 2 * SIZE_T_SIZE, PACK(extend_size - SIZE_T_SIZE, 0));
    PUT(bp + extend_size - SIZE_T_SIZE, PACK(0, 1)); 
    

    return 0;

}

/* 
 * mm_malloc - Allocate a block by implicit list.
 * find a fist fit block and return this bolck address pointer.
 * each block have three parts: header, playload, footer. 
 * if not have fitted block, then use mm_sbrk to allcate heap memory.
 */
void *mm_malloc(size_t size)
{
    /* header  + playload + footer */
    int newsize = ALIGN(size + SIZE_T_SIZE + SIZE_T_SIZE);
  
    char *start = mem_heap_lo();
    char *end = mem_heap_hi();
    int block_id = 0;
    // printf("Heap start: %p\n", start);
    // printf("Heap end: %p\n", end);
    // printf("Heap size:  %zu bytes\n", (char *)end - (char *)start + 1);
    char *p;     
    p = start + 4 * SIZE_T_SIZE;
    // printf("start malloc: origin size = %d, newsize = %d\n", size, newsize);
    char *found = NULL;
    // printf("init block size: %d\n", GET_SIZE(p));
    while (GET_SIZE(p) > 0) { 
        size_t block_size = GET_SIZE(p);
        // printf("block id: %d, block alloc: %d, size: %d, header address: %p, footer address: %p\n", block_id, GET_ALLOC(p), GET_SIZE(p), p, FTRP(p + SIZE_T_SIZE));
        if (block_size == 0) {
            printf("Wrong! block size equals 0\n");
            break;
        }
        
        if (!GET_ALLOC(p) && block_size >= newsize) {
            found = p;
            // printf("find block: %d to allocate\n", block_id);
            // break;
        }
        // printf("block id: %d, block size: %d, block header address: %p, block footer address: %p\n",
        // block_id, block_size, p, FTRP(p + SIZE_T_SIZE);
        block_id++;
        p = (char *)p + block_size;
    }

    /* don't find fitted block */
    if (!found) {   
        char *newptr = mem_sbrk(newsize);
        // printf("sbrk new block address: %p\n", newptr);
        if (newptr == (void *)-1) {
            fprintf(stderr, "mem_sbrk failed: request %d bytes, heap size = %ld\n", newsize, mem_heapsize());
            return NULL;
        }

        PUT(newptr - SIZE_T_SIZE, PACK(newsize, 1));
        PUT(newptr + newsize - 2 * SIZE_T_SIZE, PACK(newsize, 1));
        /* update epilogue block */
        PUT(newptr + newsize - SIZE_T_SIZE, PACK(0, 1));
        return (void *)((char *)newptr);
    }

    size_t orig = GET_SIZE(found);
    if (orig - newsize > MAX_BLOCK_SIZE) {
        /* split the block */
        PUT(found, PACK(newsize, 1));
        PUT(found + newsize - SIZE_T_SIZE, PACK(newsize, 1));

        PUT(found + newsize, PACK(orig - newsize, 0));
        PUT(found + orig - SIZE_T_SIZE, PACK(orig - newsize, 0));
    }
    else {
        PUT(found, PACK(orig, 1));
        PUT(found + orig - SIZE_T_SIZE, PACK(orig, 1));
    }

    // mm_check();
    return (void *)((char *)found + SIZE_T_SIZE);
}

/**
 * select coalesce policy, there are four situations.
 * if memory is like [alloc, alloc(to free), alloc] return 0.
 * [alloc, alloc(to free), free] return 1.
 * [free, alloc(to free), alloc] return 2.
 * [free, alloc(to free), free] return 3.      
 * @param bp indicate the start address of one block, which will be to free
 * @return return different number represents different situations.
 */
int coalesce_policy(void *bp) {
    void *pre_footer, *next_header;
    pre_footer = PRE_FTRP(bp);
    next_header = NEXT_HDRP(bp);

    int pre_alloc = GET_ALLOC(pre_footer);
    int next_alloc = GET_ALLOC(next_header);

    if (pre_alloc && next_alloc) {
        return 0;
    }
    else if (pre_alloc && !next_alloc) {
        return 1;
    }
    else if (!pre_alloc && next_alloc) {
        return 2;
    }
    else if (!pre_alloc && !next_alloc) {
        return 3;
    }
    else {
        return -1;
    }
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    int situation_flag;
    void* cur_header = (char *)ptr - SIZE_T_SIZE;
    void* cur_footer = (char *)ptr + GET_SIZE(cur_header) - 2 * SIZE_T_SIZE;
    // printf("Free one block at adress: %p\n", cur_header);
    if (!GET_ALLOC(cur_header)) {
        return NULL;
    }
    
    /* if block allocated, there are four situation to coalescing */
    /* situation one */
    situation_flag = coalesce_policy(ptr);
    if (situation_flag == 0) {
        PUT(cur_header, PACK(GET_SIZE(cur_header), 0));
        PUT(cur_footer, PACK(GET_SIZE(cur_header), 0));
    }
    else if (situation_flag == 1) {
        void *next_footer = NEXT_FTRP(ptr);
        int next_block_len = GET_SIZE(next_footer);
        int cur_block_len = GET_SIZE(cur_header);

        int total_len = next_block_len + cur_block_len;
        PUT(cur_header, PACK(total_len, 0)); 
        PUT(next_footer, PACK(total_len, 0));
    } 
    else if (situation_flag == 2) {
        void *pre_header = PRE_HDRP(ptr);
        int pre_block_len = GET_SIZE(pre_header);
        int cur_block_len = GET_SIZE(cur_header);

        int total_len = pre_block_len + cur_block_len;
        PUT(cur_footer , PACK(total_len, 0)); 
        PUT(pre_header, PACK(total_len, 0));
    }
    else if (situation_flag == 3){
        void *pre_header = PRE_HDRP(ptr);
        void *next_footer = NEXT_FTRP(ptr);
        
        int pre_block_len = GET_SIZE(pre_header);
        int cur_block_len = GET_SIZE(cur_header);
        int next_block_len = GET_SIZE(next_footer);

        int total_len = pre_block_len + cur_block_len + next_block_len;
        PUT(pre_header, PACK(total_len, 0));
        PUT(next_footer, PACK(total_len, 0));
    }
    else {
        printf("Error free situation!\n");
    }
   // print_memory_block();
   // mm_check();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    // printf("call realloc\n");
    if (!ptr && size > 0) {
        /* ptr == NULL */
        newptr = mm_malloc(size);
        if (!newptr) {
            return NULL;
        }
        return newptr;
    }

    if (!size) {
        /* size == 0 */
        mm_free(ptr);
        return NULL;
    }

    newptr = mm_malloc(size);
    copySize = GET_SIZE(HDRP(oldptr)) - 2 * SIZE_T_SIZE;
    if (size < copySize) {
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    // mm_check();
    return newptr;
}

/**
 * Memory checker for checking memory invariant.
 */
int mm_check(void) {
    void *start = mem_heap_lo();
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














