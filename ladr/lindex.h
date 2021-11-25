#ifndef TP_LINDEX_H
#define TP_LINDEX_H

#include "mindex.h"
#include "maximal.h"
#include "topform.h"
#include <iostream>


typedef struct lindex * Lindex;

typedef struct lindex * Lindex;

struct lindex {
  Mindex pos;   /* index for positive literals */
  Mindex neg;   /* index for negative literals */
  Lindex next;  /* for avail list */
};

class GlobalLindex {
						private:
								 unsigned Lindex_gets, Lindex_frees;
                                 
                        public:         
                                 GlobalLindex();
                                 ~GlobalLindex();
                            
						friend class LindexContainer;
                        friend class LadrVGlobais;
};


class LindexContainer {
						private:
								 Lindex lindexHead;
								 Lindex get_lindex(void);
								 void free_lindex(Lindex);
								 
								 
						
						public:	
								LindexContainer();
								~LindexContainer();
								void fprint_lindex_mem(ostream &, bool);
								void p_lindex_mem();
								Lindex lindex_init(Mindextype, Uniftype, int ,Mindextype , Uniftype , int);
								void lindex_destroy(Lindex);
								void lindex_update(Lindex, Topform, Indexop);
								void lindex_update_first(Lindex, Topform, Indexop);
								bool lindex_empty(Lindex);
								bool lindex_backtrack(Lindex);
						
};







#endif

