#ifndef TP_CLIST_H
#define TP_CLIST_H


#include "topform.h"
#include <iostream>



typedef struct clist_pos * Clist_pos;
typedef struct clist * Clist;

struct clist {
  string *name;
  Clist_pos  first, last;
  int        length;
};

struct clist_pos {
  Clist_pos  prev, next;  /* previous and next member of Clist */
  Clist_pos  nocc;        /* next member of containment list */
  Clist      list;        /* the head of the list */
  Topform     c;          /* pointer to the clause */
};


class GlobalClist {
						private:
									unsigned Clist_pos_gets, Clist_pos_frees;
									unsigned Clist_gets, Clist_frees;
                                    
                        public:           
                                    GlobalClist();
                                    ~GlobalClist();
	
						friend class ClistContainer;
                        friend class LadrVglobais;
};


class ClistContainer {
					
							private: 	Clist head; //the head of the list
										
										Clist_pos get_clist_pos(void); 	
										void free_clist_pos(Clist_pos);
										Clist get_clist(void);
										void free_clist(Clist);
										
                                       
                                        void fprint_clause_clist(ostream &, Clist);
										static OrderType compare_clause_ids(Topform, Topform);
                                        void sort_clist_by_id(Clist lst, OrderType(*comp_proc) (Topform, Topform));
										
							
							
							
							public:	 	ClistContainer();
										~ClistContainer();
									 
										void fprint_clist_mem(ostream &, bool);
										void p_clist_mem();
										Clist clist_init(string name);
										void name_clist(Clist, string);
										void clist_free(Clist);
										void clist_append(Topform, Clist);
										void clist_prepend(Topform, Clist);
										void clist_insert_before(Topform, Clist_pos);
										void clist_insert_after(Topform, Clist_pos);
										void clist_remove(Topform, Clist);
										void clist_remove_all_clauses(Clist);
										int clist_remove_all(Topform);
										int clist_member(Topform, Clist);
										void fprint_clist(ostream &, Clist);
										void p_clist(Clist);
										void clist_zap(Clist);
										void clist_check(Clist);
										void clist_append_all(Clist, Clist);
										bool clist_empty(Clist);
										int clist_length(Clist);
										int max_wt_in_clist(Clist);
										bool horn_clist(Clist);
										bool unit_clist(Clist);
										bool equality_in_clist(Clist);
										bool neg_nonunit_in_clist(Clist);
										Plist clauses_in_clist(Plist, Clist);
										void clist_swap(Topform, Topform);
										void clist_move_clauses(Clist, Clist);
										Plist move_clist_to_plist(Clist);
										Plist copy_clist_to_plist_shallow(Clist a);
										Clist plist_to_clist(Plist p, string name);
										void clist_reverse(Clist l);
										Clist_pos pos_in_clist(Clist lst, Topform c);
										void clist_append_plist(Clist lst, Plist clauses);
										Plist prepend_clist_to_plist(Plist p, Clist c);
										int clist_number_of_weight(Clist lst, int weight);
                                        void sort_clist_by_id(Clist);
                                        Plist neg_clauses_in_clist(Clist) ;
										
                                       
                                        
	
};



#endif
