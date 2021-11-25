#ifndef TP_MINDEX_H
#define TP_MINDEX_H

#include "fpa.h"
#include "discrimb.h"
#include "discrimw.h"
#include "btu.h"
#include "btm.h"
#include "unify.h"



enum class Mindextype{ 	
                        LINEAR,
						FPA,
						DISCRIM_WILD,
						DISCRIM_BIND
};


/* types of unification */

enum class Uniftype{ 	
                            ORDINARY_UNIF,
							BACKTRACK_UNIF
};

struct mindex {
  				  Mindextype index_type;
				  Uniftype   unif_type;
				  /* FPA */
				  Fpa_index  fpa;
				  /* LINEAR */
				  Plist   linear_first;
				  Plist   linear_last;
				  /* DISCRIM_WILD and DISCRIM_BIND */
				  Discrim   discrim_tree;
				  mindex     *next;  /* for avail list */
};

typedef struct mindex * Mindex;

struct mindex_pos {

  Mindex     index;
  Querytype  query_type;
  Term       query_term;
  Term       found_term;
  Context    query_subst;
  Context    found_subst;
  Trail      tr;
  Btu_state  btu_position;  /* backtrack unification */
  Btm_state  btm_position;  /* backtrack matching */
  bool       partial_match;

  /* FPA */
  Fpa_state  fpa_position;

  /* LINEAR */
  Plist   linear_position;
  
  /* DISCRIM_WILD */
  /* DISCRIM_BIND */
  Discrim_pos  discrim_position;

  mindex_pos *next;  /* for avail list */
};

typedef struct mindex_pos * Mindex_pos;


class GlobalMindex {
					 private:
								unsigned Mindex_gets, Mindex_frees;
								unsigned Mindex_pos_gets, Mindex_pos_frees;
					 
                    public: 
                                GlobalMindex();
                                ~GlobalMindex();
                            
                    friend class MindexContainer;
                     friend class LadrVGlobais;
};


class MindexContainer {
						private: 
									Mindex_pos head;
									Mindex get_mindex(void);
									void free_mindex(Mindex);
									Mindex_pos get_mindex_pos(void);
									void free_mindex_pos(Mindex_pos);
									
						public:
									MindexContainer();
									~MindexContainer();
									void fprint_mindex_mem(ostream &, bool);
									void p_mindex_mem();
									Mindex mindex_init(Mindextype, Uniftype, int);
									bool mindex_empty(Mindex);
									void mindex_free(Mindex);
									void mindex_destroy(Mindex);
									void linear_insert(Mindex, Term);
									void linear_delete(Mindex, Term);
									void linear_update(Mindex, Term, Indexop);
									void mindex_update(Mindex, Term, Indexop);
									Term mindex_retrieve_first(Term, Mindex, Querytype,Context, Context,bool partial_match, Mindex_pos *);
									Term next_candidate(Mindex_pos);
									Term retrieve_next_backtrack(Mindex_pos);
									Term mindex_retrieve_next(Mindex_pos);
									void mindex_retrieve_cancel(Mindex_pos);
									void fprint_linear_index(ostream &, Plist);
									void fprint_mindex(ostream &, Mindex);
									
						
};

#endif
