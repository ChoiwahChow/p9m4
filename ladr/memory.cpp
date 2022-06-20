#include "memory.h"
#include "ladrvglobais.h"
#include <cstddef>
#include <iomanip>
#include "fatal.h"



                                

void GlobalMemory::Garbage_Collector(void) {

    
    for(int i=0; i < LADR_GLOBAL_MEMORY.Al_pointer; i++)
     if (LADR_GLOBAL_MEMORY.ALLOCED[i])
         free(LADR_GLOBAL_MEMORY.ALLOCED[i]);

}
               

GlobalMemory::GlobalMemory() {
    Al_pointer=0;
    for(int i=0; i<MAX_MEM_LISTS; i++) {
            M[i]=NULL;
    }

    for(unsigned int i=0; i<MAX_ALLOCED_LISTS; i++) {
            ALLOCED[i]=NULL;
    }        
    
    
   
    Max_megs_check=true;
    Max_megs=DEFAULT_MAX_MEGS;
    Malloc_calls=0;
    Bytes_palloced=0;
    Block=Block_pos=NULL;
    Mem_calls=0;
    Mem_calls_overflows=0;



    enabled=true;
    allocs=0;
    deallocs=0;
    usedMemory=0; 
   
    
    
}

GlobalMemory::~GlobalMemory() {
    
}



void *Memory::palloc(std::size_t n) {
    if(n==0) return NULL;
    void *chunk;
    size_t malloc_bytes=MALLOC_MEGS*1024*1024;
    
    //this is the first time palloc is called or in case that is needed another block
    if (LADR_GLOBAL_MEMORY.Block==NULL || (char *) LADR_GLOBAL_MEMORY.Block +  malloc_bytes - (char *) LADR_GLOBAL_MEMORY.Block_pos < n) { 
        
        if(n > malloc_bytes) fatal::fatal_error("Palloc, request too big-reset MALLOC_MEGS!");
        else if (LADR_GLOBAL_MEMORY.Max_megs_check && (LADR_GLOBAL_MEMORY.Malloc_calls+1) * MALLOC_MEGS >  LADR_GLOBAL_MEMORY.Max_megs) {
                if(Exit_proc) (*Exit_proc)();
                else fatal::fatal_error("Palloc, Max_megs parameter exceeded!");
             }
             else {
                    LADR_GLOBAL_MEMORY.Block_pos = LADR_GLOBAL_MEMORY.Block=malloc(malloc_bytes);
                   
                    LADR_GLOBAL_MEMORY.Malloc_calls++;
                    
                    if(LADR_GLOBAL_MEMORY.Block_pos==NULL) 
                        fatal::fatal_error("Palloc, operating system is out of memory!");
                    

                    if (LADR_GLOBAL_MEMORY.Al_pointer < MAX_ALLOCED_LISTS) {  // CC added the guard
                    	LADR_GLOBAL_MEMORY.ALLOCED[LADR_GLOBAL_MEMORY.Al_pointer]=LADR_GLOBAL_MEMORY.Block;
                    	LADR_GLOBAL_MEMORY.Al_pointer++;
                    }

                   
            }
    }
    chunk=LADR_GLOBAL_MEMORY.Block_pos;
    LADR_GLOBAL_MEMORY.Block_pos = (char *) ( LADR_GLOBAL_MEMORY.Block_pos ) +  n;
    LADR_GLOBAL_MEMORY.Bytes_palloced+=n;
    return (chunk);
}

void *Memory::memCNew(std::size_t n) {
    if (n==0) 
        return NULL;
    else {
            
            void **p=NULL;
            BUMP_MEM_CALLS;
            if(n>=MAX_MEM_LISTS) return calloc(1,n);
            else if (LADR_GLOBAL_MEMORY.M[n]==NULL) 
                    p=(void **) palloc(n);
            else {
                    p=LADR_GLOBAL_MEMORY.M[n];
                    LADR_GLOBAL_MEMORY.M[n]=(void **) (*p);
            }
            
            char *aux;
            aux= (char *)p;
            
            for (int i=0; i<n; i++) aux[i]=0;
            return p;   
           
    }
}

void *Memory::memNew(std::size_t n) {
    if (n==0) return NULL;
    else {
            void **p=NULL;
            BUMP_MEM_CALLS;
            if(n>=MAX_MEM_LISTS) return malloc(n);
            else if (LADR_GLOBAL_MEMORY.M[n]==NULL) 
                    p=(void **) palloc(n);
            else {
                    p=LADR_GLOBAL_MEMORY.M[n];
                    LADR_GLOBAL_MEMORY.M[n]=(void **) (*p);
            }
    
            return p;                                        
    }
    
}

void Memory::memFree(void *q, std::size_t n) {
    if(n==0) return;
    void **p = (void **) q;
    if(n>MAX_MEM_LISTS) free(p);
    else {
            *p=(void **) LADR_GLOBAL_MEMORY.M[n];
            LADR_GLOBAL_MEMORY.M[n]=p;
    }
     
}












void Memory::set_max_megs_proc(void (*proc) (void)) {
    Exit_proc=proc;
}


unsigned Memory::mega_mem_calls(){
    return  (LADR_GLOBAL_MEMORY.Mem_calls / 1000000) +
            ( (UINT_MAX/1000000) * LADR_GLOBAL_MEMORY.Mem_calls_overflows);
}

void Memory::set_max_megs(int megs) {
    LADR_GLOBAL_MEMORY.Max_megs= (megs==-1 ? INT_MAX :  megs);
    
}

int Memory::megs_malloced(void) {
   return LADR_GLOBAL_MEMORY.Malloc_calls * MALLOC_MEGS; 
}

long Memory::bytes_palloced(void) {
        return LADR_GLOBAL_MEMORY.Bytes_palloced;
}

int Memory::mlist_length(void **p) {
  int n;
  for (n = 0; p ; p =(void **) *p, n++);
  return n;
}  /* mlist_length */



void Memory::memory_report(std::ostream &o) {
  int i;
  o<< endl << "Memory report, " << LADR_GLOBAL_MEMORY.Malloc_calls <<" @ " << MALLOC_MEGS << " = " << LADR_GLOBAL_MEMORY.Malloc_calls * MALLOC_MEGS;
  o<< " megs (" << LADR_GLOBAL_MEMORY.Bytes_palloced / (1024 * 1024) <<" megs used)."<<endl;
  
  
  
  for (i = 0; i < MAX_MEM_LISTS; i++) {
    int n = mlist_length(LADR_GLOBAL_MEMORY.M[i]);
    if (n != 0)
        o<< "List "<<setw(3)<<i<<", length "<<setw(7)<<n<<", "<<setw(8)<<i * n * (sizeof(void *)) / 1024 <<"K"<<endl;
  }
   
}

void Memory::enable_max_megs_check(bool check) {
  LADR_GLOBAL_MEMORY.Max_megs_check = check;
}
