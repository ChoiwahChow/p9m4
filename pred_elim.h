#ifndef TP_PRED_ELIM_H
#define TP_PRED_ELIM_H

#include "./ladr/subsume.h"
#include "./ladr/resolve.h"
#include "./ladr/clauses.h"
#include "./ladr/clause_misc.h"
#include "./ladr/ioutil.h"

class PredElim {

						private:
									int rsym_occurrences(int,Topform );
									bool arg_check(int,Topform);
									bool eliminable_relation(int, Plist, bool);
									Plist resolve_on_symbol(int, Topform, Topform);
									Plist gen_given(int, Topform, Plist);
									bool subsumed_by_member(Topform, Plist);
									Plist incorporate_new_clauses(Plist, Plist, bool);
									Plist elim_relation(int, Plist, Clist, bool);
                                    Ilist eliminable_relations(Plist, bool);
									
						public:
								   void predicate_elimination(Clist,Clist,bool);	


};


#endif
