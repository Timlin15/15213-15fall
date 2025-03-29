# Malloc lab



## Goal

 Your dynamic storage allocator will consist of the following four functions, which are declared in `mm.h`
 and defined in `mm.c`.

- `int mm_init(void)`;
- `void *mm_malloc(size_t size)`;
- `void mm_free(void *ptr)`;
- `void *mm_realloc(void *ptr, size_t size)`;

The `mm.c` file we have given you implements the simplest but still functionally correct malloc package that
 we could think of. Using this as a starting place, modify these functions (and possibly define other private
 static functions), so that they obey the following semantics:

-  `mm init`: Before calling mm malloc `mm realloc` or `mm free`, the application program (i.e.,
 the trace-driven driver program that you will use to evaluate your implementation) calls `mm init` to
  perform any necessary initializations, such as allocating the initial heap area. The return value should
  be-1 if there was a problem in performing the initialization, 0 otherwise.

- `mm malloc`: The `mm malloc` routine returns a pointer to an allocated block payload of at least
 sizebytes. The entire allocated block should lie within the heap region and should not overlap with
  any other allocated chunk.
  We will comparing your implementation to the version of malloc supplied in the standard C library
  (libc). Since the libc malloc always returns payload pointers that are aligned to 8 bytes, your
  malloc implementation should do likewise and always return 8-byte aligned pointers.

- `mm free`: The mm free routine frees the block pointed to by ptr. It returns nothing. This routine is only guaranteed to work when the passed pointer (`ptr`) was returned by an earlier call to `mm malloc`or` mm realloc` and has not yet been freed.

- `mm realloc`: The `mm realloc` routine returns a pointer to an allocated region of at least size
 bytes with the following constraints.
 -  if `ptr` is NULL,the call is equivalent to mm `malloc(size)`;
 
 -  if size is equal to zero, the call is equivalent to `mm free(ptr)`;
 
 -  if `ptr` is not NULL, it must have been returned by an earlier call to `mm malloc` or `mm realloc`.
     The call to `mm realloc` changes the size of the memory block pointed to by `ptr` (the old block) to size bytes and returns the address of the new block. Notice that the address of the new block might be the same as the old block, or it might be different, depending on your implementation, the amount of internal fragmentation in the old block, and the size of the realloc request.
      The contents of the new block are the same as those of the old `ptr` block, up to the minimum of the old and new sizes. Everything else is uninitialized. For example, if the old block is 8 bytes and the new block is 12 bytes, then the first 8 bytes of the new block are identical to the first 8 bytes of the old block and the last 4 bytes are uninitialized. Similarly, if the old block is 8 bytes and the new block is 4 bytes, then the contents of the new block are identical to the first 4 bytes of the old block.
 

These semantics match the the semantics of the corresponding libc malloc, realloc, and free routines. Type man malloc to the shell for complete documentation.

Heap Consistency Checker

 Dynamic memory allocators are notoriously tricky beasts to program correctly and efficiently. They are
 difficult to program correctly because they involve a lot of untyped pointer manipulation. You will find it
 very helpful to write a heap checker that scans the heap and checks it for consistency.
 Some examples of what a heap checker might check are:
 -  Is every block in the free list marked as free?
 -  Are there any contiguous free blocks that somehow escaped coalescing?
 -  Is every free block actually in the free list?
 -  Do the pointers in the free list point to valid free blocks?
 -  Do any allocated blocks overlap?
 -  Do the pointers in a heap block point to valid heap addresses?
 Your heap checker will consist of the function int mm check(void)in `mm.c`. It will check any invariants or consistency conditions you consider prudent. It returns a nonzero value if and only if your heap is consistent. You are not limited to the listed suggestions nor are you required to check all of them. You are encouraged to print out error messages when mm check fails. This consistency checker is for your own debugging during development. When you submit `mm.c` , make sure to remove any calls to mm check as they will slow down your throughput. Style points will be given for your mm check function. Make sure to put in comments and document what you are checking



## Data structure

### Macro

```C
#define WSIZE 4
```

This will replace the WSIZE in the C file with 4, by this approach we can simplify many complex expression with macro, improving the readability of the code

### Starter code

