#include <cstdlib>
#include "memorypool.h"
#include "fatal.h"






MemoryPool::MemoryPool()
{
for (int i=0; i<MAX_MEM_LISTS; i++)  M[i]=NULL;
    Max_megs_check=true;
    Max_megs=DEFAULT_MAX_MEGS;
    Malloc_calls=0;
    Bytes_palloced=0;
    Block=NULL;
    Block_pos=NULL;
    Mem_calls=0;
    Mem_calls_overflows=0;
}

MemoryPool::~MemoryPool()
{

    
}


void * MemoryPool::palloc(std::size_t n) {
if (n==0) return NULL;
void *chunk;
size_t malloc_bytes = MALLOC_MEGS*1024*1024;

if (Block==NULL || (char *) Block + malloc_bytes - (char *) Block_pos < n) { //first time or need another block of Memory::
    if (n > malloc_bytes)  fatal::fatal_error("palloc, request too big;reset MALLOC_MEGS"); //the request block size is too big
        else if (Max_megs_check && (Malloc_calls+1)*MALLOC_MEGS > Max_megs) fatal:: fatal_error("palloc, Max_megs parameter exceeded"); //cannot request more megs due to max_megs validation
        else { //everything is ok do the request
                Block_pos = Block = malloc(malloc_bytes);
                Malloc_calls++;
                if (Block_pos == NULL) fatal::fatal_error("palloc, operating system is out of Memory::");
        }
    }
    chunk = Block_pos;
    Block_pos = (char*) Block_pos + n;
    Bytes_palloced += n;
    return(chunk);
}


void *MemoryPool::memCNew(std::size_t n) {
    if (n == 0) return NULL;
    else {
            void **p = NULL;
            BUMP_MEM_CALLS;
            if (n >= MAX_MEM_LISTS) return calloc(1, n);
            else if (M[n] == NULL)  p =(void **) palloc(n);
            else {   /* the first pointer is used for the avail list */
                    p = M[n];
                    M[n] = (void **) (*p);
            }
            for (int i = 0; i < n; i++)   p[i] = 0;
            return p;
    }
}




void *MemoryPool::memNew(std::size_t n) {
    if (n == 0) return NULL;
    else {
            void **p = NULL;
            BUMP_MEM_CALLS;
            if (n >= MAX_MEM_LISTS)  return calloc(1, n);
            else 
                if (M[n] == NULL)  p =(void **) palloc(n);
                else {   /* the first pointer is used for the avail list */
                    p = M[n];
                    M[n] = (void **) (*p);
                }
            return p;
    }
 
}



/* PUBLIC */
void MemoryPool::memFree(void *q, size_t n) {
  if (n == 0) return;  /* do nothing */
  /* put it on the appropriate avail list */
  void **p = (void **)q;
  if (n >= MAX_MEM_LISTS) free(p);
  else { /* the first pointer is used for the avail list */
       *p = (void **) M[n];
       M[n] = p;
    }
}  /* free_mem */
