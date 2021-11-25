#ifndef TP_PINDEX_H
#define TP_PINDEX_H

#include "clist.h"
#include <iostream>


#define INT_LARGE INT_MAX / 2  /* It can be doubled without overflow. */
#define IN_RANGE(i, min, max) (i < min ? min : (i > max ? max : i))

struct pair_index {
    int finished;        /* set if nothing to retrieve */
    int n;               /* number of lists */
    int i, j;            /* working pair */
    int min;             /* smallest wt of inserted clause */
    int new_min;         /* smallest inserted wt since previous retrieval */
    Clist *lists;         /* lists */
    Clist_pos *top;
    Clist_pos *curr;
    struct pair_index *next;  /* for avail list */
    };

typedef struct pair_index * Pair_index;


class GlobalPindex {
						private:
                                unsigned Pair_index_gets, Pair_index_frees;	
                                
                        public:        
                                GlobalPindex();
                                ~GlobalPindex();

						friend class PindexContainer;
                        friend class LaadrVGlobais;
};


class PindexContainer {
	
						private: 
								Pair_index pindexHead;
								Pair_index get_pair_index(void);
								void init_pair(int, int, Pair_index);
								void p_pair_index(Pair_index);
                                void free_pair_index(Pair_index); 
								
						
						
						public:
								PindexContainer();
								~PindexContainer();
								void fprint_pindex_mem(ostream &,bool);
								void p_pindex_mem();
								Pair_index init_pair_index(int);
								void zap_pair_index(Pair_index);
								int pairs_exhausted(Pair_index);
								void insert_pair_index(Topform, int , Pair_index);
								void delete_pair_index(Topform, int , Pair_index);
								void retrieve_pair(Pair_index , Topform *, Topform *);
								int pair_already_used(Topform , int,Topform, int,Pair_index);
};




#endif

