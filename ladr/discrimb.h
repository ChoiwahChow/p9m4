#ifndef TP_DISCRIMB_H
#define TP_DISCRIMB_H

#include "discrim.h"
#include "index.h"
#include <iostream>
/* INTRODUCTION
Discrimination tree indexing in which variables are
distinguished in the index and are bound
as soon as possible during retrieval.
*/

#define GO        1
#define SUCCESS   4
#define FAILURE   5
#define BACKTRACK 6


struct flat2 {  /* for building a stack of states for backtracking */
  Term     t;
  flat2    *prev, *next, *last;
  Discrim  alternatives;
  int      bound;
  int      varnum;
  int      place_holder;
};

typedef struct flat2 * Flat2;


class GlobalDiscrimB {
						private:
                                unsigned Flat2_gets, Flat2_frees;
						
						
                        public:        
                                GlobalDiscrimB();
                                ~GlobalDiscrimB();
                            
                        friend class DiscrimBContainer;
                        friend class LadrVGlobais;
                        
};


class DiscrimBContainer {
	
							private:  	Flat2 root;
										Flat2 get_flat2(void);
										void free_flat2(Flat2 p);
										void print_discrim_bind_tree(ostream &, Discrim , int , int);
										Discrim discrim_bind_insert_rec(Term, Discrim);
										void discrim_bind_insert(Term, Discrim, void *);
										Discrim discrim_bind_end(Term, Discrim, Plist *);
										void discrim_bind_delete(Term, Discrim, void *);
										Flat2 check_flat2(Flat2);
										Plist discrim_bind_retrieve_leaf(Term, Discrim , Context , Flat2 *);
										
										
							
	
							public:
										DiscrimBContainer();
										~DiscrimBContainer();
										void fprint_discrimb_mem(ostream &, bool);
										void p_discrimb_mem(void);
										void check_discrim_bind_tree(Discrim, int);
										void fprint_discrim_bind_index(ostream &, Discrim);
										void p_discrim_bind_index(Discrim);
										void discrim_bind_update(Term, Discrim,  void *, Indexop);
										void *discrim_bind_retrieve_first(Term, Discrim , Context , Discrim_pos *);
										void *discrim_bind_retrieve_next(Discrim_pos);
										void discrim_bind_cancel(Discrim_pos);
										void p_flat2(Flat2);
};

#endif  /* conditional compilation of whole file */
