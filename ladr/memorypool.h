#ifndef TP_MEMORY_POOL_H
#define TP_MEMORY_POOL_H

#include <iostream>

#define MALLOC_MEGS        20  /* size of blocks malloced by palloc */
#define DEFAULT_MAX_MEGS  500  /* change with set_max_megs(n) */
#define MAX_MEM_LISTS     500  /* number of lists of available nodes */


#define BUMP_MEM_CALLS {Mem_calls++; if (Mem_calls==0) Mem_calls_overflows++;}



class MemoryPool {

        private:
                void **M[MAX_MEM_LISTS]; //next free block for each size
                bool Max_megs_check;
                int Max_megs;
                int Malloc_calls;
                unsigned Bytes_palloced;
                void *Block;
                void *Block_pos;
                unsigned Mem_calls;
                unsigned Mem_calls_overflows;
                void *palloc(std::size_t);
            
                
        public:
                MemoryPool();
                ~MemoryPool();
                void *memCNew(std::size_t);
                void *memNew(std::size_t);
                void memFree(void *, std::size_t);
                
};

#endif
