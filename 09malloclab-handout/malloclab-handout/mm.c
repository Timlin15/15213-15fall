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
#include <errno.h>

#include "mm.h"

//#include <bits/fenv.h>

#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define MAX_HEAP 20*(1<<20)

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/** because size is always the multiples of 8 so the lower bits are always 0. */
#define PACK(size, alloc) ((size) | alloc)
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)


/**compute the address of next and previous blocks of bp. */
#define PREV_BLKP(bp)       ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))
#define NEXT_BLKP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)))
//#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)HDRP(bp) - WSIZE)))
//#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)HDRP(bp) - DSIZE)))
static void *extend_heap(size_t word);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);

// static char *mem_heap;
// static char *mem_brk;
// static char *mem_max_addr;
char * heap_listp;
static void place(void *bp, size_t asize);
//
// void mem_init(){
//     mem_heap = (char *)malloc(MAX_HEAP);
//     mem_brk = mem_heap;
//     mem_max_addr = (char *)(mem_heap + MAX_HEAP);
// }
//
// void *mem_sbrk(int incr){
//     char *old_brk = mem_brk;
//     if ( (incr < 0) || (mem_brk + incr > mem_max_addr) ) {
//       errno = ENOMEM;
//       fprintf(stderr, "ERROR: out of memory\n");
//       return (void *)-1;
//     }
//     mem_brk += incr;
//     return (void *)old_brk;
// }

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  //printf("mm_init()\n");
  if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
    return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 2*WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 3*WSIZE, PACK(0, 1));//epilogue
    heap_listp += 2*WSIZE;
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
          return -1;
    return 0;
}

static void *extend_heap(size_t word){
  //printf("in extend\n");
char *bp;
size_t size;

size = (word % 2) ? (word + 1) * WSIZE : word * WSIZE;
if ((long)(bp = mem_sbrk(size)) == -1)
  return NULL;

PUT(HDRP(bp), PACK(size, 0));
PUT(FTRP(bp), PACK(size, 0));
PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

return coalesce(bp);
}

static void *coalesce(void *bp){
size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
size_t size = GET_SIZE(HDRP(bp));

if (prev_alloc && next_alloc){
  return bp;
}
if (prev_alloc && !next_alloc){
  size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
}
else if (!prev_alloc && next_alloc){
  size += GET_SIZE(HDRP(PREV_BLKP(bp)));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
  bp = PREV_BLKP(bp);
}
else {
  size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
  PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
  PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
  bp = PREV_BLKP(bp);
}
return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
//    int newsize = ALIGN(size + SIZE_T_SIZE);
//    void *p = mem_sbrk(newsize);
//    if (p == (void *)-1)
//	return NULL;
//    else {
//        *(size_t *)p = size;
//        return (void *)((char *)p + SIZE_T_SIZE);
//    }
  //printf("in malloc\n");
size_t asize;
size_t extendsize;
char *bp;
if (size == 0)
  return NULL;
if (size <= DSIZE)
  asize = 2*DSIZE;
else
  asize = DSIZE * ((size + 2 * DSIZE - 1) / DSIZE);

if ((bp = find_fit(asize)) != NULL) {
  place(bp, asize);
  return bp;
}
extendsize = MAX(asize, CHUNKSIZE);
if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
  return NULL;
place(bp, asize);
return bp;
}

static void *find_fit(size_t asize){
  //printf("in find_fit\n");
  char *curr_bk = heap_listp;
  while (GET_SIZE(HDRP(curr_bk)) > 0) {
    //printf("in find fit loop\n");
    //printf("%d\n",GET_SIZE(HDRP(curr_bk)));
    if (!GET_ALLOC(HDRP(curr_bk)) && GET_SIZE(HDRP(curr_bk)) >= asize)
      return curr_bk;
    curr_bk = NEXT_BLKP(curr_bk);
  }
  return NULL;
}

static void place(void *bp, size_t asize){
  //printf("in place\n");
size_t size = GET_SIZE(HDRP(bp));
  if (size - asize >= 2 * DSIZE) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(size - asize, 0));
    PUT(FTRP(bp), PACK(size - asize, 0));
  } else {
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  //printf("in free\n");
size_t size = GET_SIZE(HDRP(ptr));
PUT(HDRP(ptr), PACK(size, 0));
PUT(FTRP(ptr), PACK(size, 0));
coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














