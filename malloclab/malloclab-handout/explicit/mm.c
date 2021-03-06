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
    "Q",
    /* First member's full name */
    "Rick",
    /* First member's email address */
    "q.st000rm@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Basic constants and macros from the Book */
#define WSIZE 4 /* Word and header/ footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
//size must be bigger than a DSIZE(4) or be zero(SIZE = 0);
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *) (p))
#define PUT(p, val) (*(unsigned int *) (p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *) (bp) - WSIZE)
#define FTRP(bp) ((char *) (bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *) (bp) + GET_SIZE(((char *) (bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *) (bp) - GET_SIZE(((char *) (bp) - DSIZE)))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// #define HDRP(bp) ((char*)(bp - 4))
// #define FTRP(bp) ((char*)(bp + SIZE_T_SIZE))
#define FREE 0
#define ALLOCATED 1

static char *heap_listp;
static char *free_listp;


static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

// static void *get_last_FB();
static void *get_prev_FB(char *bp);
static void *get_next_FB(char *bp);
static void dequeue_block(char *bp);
static void enqueue_block(char *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //Double word alignment so 4 WORD = 2 DWORD
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) -1)
        return -1;
    
    //TO-DO modify to contains a free_listp? or put it to here?
    PUT(heap_listp, 0);

    //Initialize the free-list-pointer to the first word
    free_listp = heap_listp;

    //Initialize 2 Block of PACK(8|1)
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + (2* WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    //change the free-list-pointer to the first block.
    // *free_listp = NEXT_BLKP(heap_listp);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if(size == 0)
        return NULL;

    asize = ALIGN(size + DSIZE);
    extendsize = MAX(asize, CHUNKSIZE);

    if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    if((bp = extend_heap(extendsize)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    if(ptr == NULL)
        return mm_malloc(size);


    // if(size_diff <= 0)
    //     return ptr;

    size_t size_diff = size - GET_SIZE(HDRP(ptr));
    void *oldptr = ptr;
    void *newptr;
    size_t copySize, new_size;

    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    // printf("CopySize = %d is it equal to %d\n", copySize, GET_SIZE(HDRP(oldptr)) - DSIZE);
    if (size < copySize)
        copySize = size;
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

//second implementation: explicit free linked list
static void *extend_heap(size_t words){
    char *bp, *last_free;
    size_t size;

    //can be replace by ALIGN(size)
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    // last_free = get_last_FB();
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

//TODO: rebuild and MOD free list
static void *coalesce(void *bp){
    //space coalesce gain?
    //printf("INFO: COALESCE of 0x%x begin.\n", bp);
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));

    //TO-DO : insert linked_list base on cases.
    if(prev_alloc && next_alloc){           /* Case 1  prev & next all allocated. */
        enqueue_block(bp);
        // return bp;
    }

    else if (prev_alloc && !next_alloc){    /* Case 2 merge with next block. */
        dequeue_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        //BUG.pointer disappear
        enqueue_block(bp);
    }
    else if (!prev_alloc && next_alloc){    /* Case 3 merge with prev block. */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else{                                 /* Case 4 merge with both block. */
        dequeue_block(NEXT_BLKP(bp));
        dequeue_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
        enqueue_block(bp);
    }

    //printf("INFO: COALESCE end.\n");
    return bp;
}

//Not first fit but best fit in o(n)
//TODO: traverse from free_listp
static void *find_fit(size_t asize){
    int *cur = GET(free_listp);
    //printf("INFO: func find_fit:original free_listp:0x%x\n", cur);
    char *ans = NULL;
    int prevsize = 0x7fffffff;
    while(cur !=NULL && GET_SIZE(HDRP(cur)) != 0){
        //unallocated and larger than asize's memory
        if(GET_ALLOC(HDRP(cur)) != FREE){
            printf("ERROR: cur block not free\n");
        }
        if(GET_SIZE(HDRP(cur)) >= asize){
            //printf("INFO: Find One block fit.\n");
            if(ans == NULL || GET_SIZE(HDRP(cur)) < prevsize){
                ans = cur;
                prevsize = GET_SIZE(HDRP(cur));
            }
        }
        // //printf("cur address : 0x%x\n", cur);
        cur = GET(cur);
    }
    return ans;
}

//Split block and place
//TO-DO: Rewrite it
static void place(void *bp, size_t asize){
    ////printf("bp address = 0x%x, asize=%d\n", (int)bp, asize);
    //printf("INFO: placing 0x%x with %d bytes.\n", bp, asize);
    int total_size = GET_SIZE(HDRP(bp));
    int prev_size = asize;
    //space header,footer lost?
    int succ_size = total_size - asize ;
    char *succ_ptr;

    if(succ_size <= DSIZE){
        succ_size = 0;
        prev_size = total_size;
    }
    dequeue_block(bp);
    PUT(HDRP(bp), PACK(prev_size, 1));
    PUT(FTRP(bp), PACK(prev_size, 1));

    //Spliting
    if(succ_size > 0){
        // printf("succ_size:%d.\n", succ_size);
        if(succ_size <= DSIZE){
            printf("succ_size too small!!!\n");
        }
        //printf("spliting %d into %d and %d\n", total_size, asize, succ_size);
        succ_ptr = NEXT_BLKP(bp);
        ////printf("succ_ptr address : %x\n", (int)succ_ptr);
        PUT(HDRP(succ_ptr), PACK(succ_size, 0));
        PUT(FTRP(succ_ptr), PACK(succ_size, 0));
        enqueue_block(succ_ptr);
    }
    //gap small than a DWORD not split

    return ;
}

static void *get_prev_FB(char *bp){
    if(GET(bp + WSIZE) == NULL)
        return NULL;
    if(GET_ALLOC(HDRP(bp)) == ALLOCATED){
        printf("ERROR: prev_Request invalid. 0x%x\n", bp);
    }
    //printf("get_prev_free_block: 0x%x\n", GET(bp + WSIZE));
    return GET(bp + WSIZE);
}

static void *get_next_FB(char *bp){
    if(GET(bp) == NULL)
        return NULL;
    if(GET_ALLOC(HDRP(bp)) == ALLOCATED){
        printf("ERROR: next_Request invalid 0x%x.\n", bp);
    }
    //printf("get_next_f_Block: 0x%x\n", GET(bp));
    return GET(bp);
}

static void enqueue_block(char *bp){
    char *cur = free_listp, *next;

    //printf("INFO: Inserting Block to free_list: 0x%x\n", bp);
    while(GET(cur) != NULL && ((unsigned int)(GET(cur)) < (unsigned int) (bp))){
        // printf("Cur address is 0x%x\t bp add = 0x%x\n", cur, bp);
        cur = GET(cur);
    }
    //cur -> pointer to previous-free-block
    //GET(cur) -> dereference of the pointer, point to the next-free-block
    PUT(bp, GET(cur));
    //Prev pointer
    //TO-DO: if cur == free_listp should it be NULL?
    if(cur != free_listp)
        PUT(bp + WSIZE, cur);
    else
        PUT(bp + WSIZE, 0);
    //Prev Block's Next pointer
    // //printf("cur address stored data is 0x%x\n", *cur);
    if(GET(cur) != NULL)
        PUT((GET(cur) + WSIZE), bp);
    PUT(cur, bp);
    //Next Block's Prev pointer
    //Try to avoid null reference
    return ;
}

static void dequeue_block(char *bp){
    char *next_free = get_next_FB(bp);
    char *prev_free = get_prev_FB(bp);
    //printf("INFO: Removing Block :0x%x out of free_list.\n\t next pointer: 0x%x, prev pointer: 0x%x\n", bp, next_free, prev_free);
    //TO-DO cases: perv_free == NULL, next_free == NULL.
    if(next_free != NULL)
        PUT(next_free + WSIZE, prev_free);
    if(prev_free == NULL){
        prev_free = free_listp;
    }
    PUT(prev_free, next_free);
    return ;
}

//TO-DO: Auto Heap Consistance Detect.