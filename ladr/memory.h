#ifndef _MEMORY_H
#define _MEMORY_H

#include <iostream>
#include <vector>



static void (*Exit_proc) (void);

using namespace std;




#define MAX_MEM_LISTS       500 
#define DEFAULT_MAX_MEGS    8000
#define MALLOC_MEGS         20


#define BUMP_MEM_CALLS  {LADR_GLOBAL_MEMORY.Mem_calls++; if(LADR_GLOBAL_MEMORY.Mem_calls==0) LADR_GLOBAL_MEMORY.Mem_calls_overflows++;}


class GlobalMemory {

        

            private:        

                                unsigned Al_pointer;
                                void *ALLOCED[MAX_MEM_LISTS];
  
                
                                void **M[MAX_MEM_LISTS]; //memory_pool
                                bool Max_megs_check; //memory_pool
                                int Max_megs; //memory_pool
                                int Malloc_calls; //memory_pool
                                long Bytes_palloced; //memory_pool
                                void *Block;//memory_pool
                                void *Block_pos;//memory_pool
                                unsigned Mem_calls; //memory_pool
                                unsigned Mem_calls_overflows; //memory_pool
                                
                               
                                
                                
                                bool                enabled; 
                                unsigned long       usedMemory; //quantidade de bytes alocados
                                unsigned long       allocs; //número de allocs
                                unsigned long       deallocs;
                               
                                void (*Exit_proc) (void);
                                
                                
                                
            public:
                                GlobalMemory();
                                ~GlobalMemory();
                                
                                void Garbage_Collector(void);
                                
                                friend class Memory;
                                friend class LadrVGlobais;
};




class Memory {

              private:
                            static void *palloc(std::size_t); //memory_pool
                            static int mlist_length(void **);
                            

               public:
                            static void * memNew(std::size_t);//old
                            static void * memCNew(std::size_t);//old
                            static void  memFree(void *,std::size_t);//old
                            
                          
                            
                           
                            
                            static void set_max_megs_proc(void (*proc) (void));
                            static void memory_report(ostream &);
                            static int megs_malloced(void);
                            static void set_max_megs(int);
                            static long bytes_palloced(void);
                            static unsigned mega_mem_calls(void);
                            static void enable_max_megs_check(bool);
                           
                            
                            
                           
                            
                        
                            

};




#endif
