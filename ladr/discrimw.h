#ifndef TP_DISCRIMW_H
#define TP_DISCRIMW_H

#include "discrim.h"
#include "index.h"

#define GO        1
#define BACKTRACK 6
#define SUCCESS   4
#define FAILURE   5


struct flat {  /* for building a stack of states for backtracking */
  Term    t;
  flat    *prev, *next, *last;
  Discrim alternatives;
  int     bound;           /* (tame) */
  int     varnum;          /* (tame) */
  int     place_holder;
  int     num_ac_args;     /* for AC symbols (wild) */
  int     num_ac_nv_args;  /* for AC symbols (wild) */
  int     commutative;     /* for commutative symbols (wild) */
  int     flip;            /* for commutative symbols (wild) */
};

typedef struct flat * Flat;


class GlobalDiscrimW {
						private:
									unsigned Flat_gets, Flat_frees;
						
                        public:		GlobalDiscrimW();	
                                    ~GlobalDiscrimW();
                            
                                    
                                    friend class DiscrimWContainer;
                                    friend class LadrVGlobais;
	
};


class DiscrimWContainer {
							private:
										Flat root;
										Flat get_flat(void);
										void free_flat(Flat);
										Flat check_flat(Flat);
										void print_discrim_wild_tree(ostream &, Discrim, int, int);
										
							public:
										DiscrimWContainer();
										~DiscrimWContainer();
                                        
										void fprint_discrimw_mem(ostream &, bool);
										void p_discrimw_mem(void);
										void fprint_discrim_wild_index(ostream &, Discrim);
										void p_discrim_wild_index(Discrim);
										void discrim_wild_update(Term, Discrim, void *, Indexop);
										void *discrim_wild_retrieve_first(Term, Discrim,Discrim_pos *);
										void *discrim_wild_retrieve_next(Discrim_pos);
										void discrim_wild_cancel(Discrim_pos);
										void p_flat(Flat);
										void flip_flat(Flat);
										int  num_ac_args(struct term *, int);
										int  num_ac_nv_args(struct term *, int);
										Discrim discrim_wild_insert_ac(Term, Discrim);
										Discrim discrim_wild_insert_rec(Term , Discrim);
										Discrim discrim_wild_end(Term , Discrim , Plist *);
										Plist discrim_wild_retrieve_leaf(Term, Discrim, Flat *);
                                        void discrim_wild_delete(Term , Discrim , void *);
                                        void discrim_wild_insert(Term , Discrim , void *);
										
};


#endif
