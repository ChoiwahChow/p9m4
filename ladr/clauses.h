#ifndef TP_CLAUSES_H
#define TP_CLAUSES_H

#include "topform.h"
#include "ladrvglobais.h"

class ClausesContainer {
					
					private:
					
					public:
							Topform clause_member_plist(Plist, Topform);
							Plist intersect_clauses(Plist, Plist);
							double max_clause_weight(Plist);
							int max_clause_symbol_count(Plist);
							Plist nonneg_clauses(Plist);
							bool all_clauses_horn(Plist);
							bool all_clauses_unit(Plist);
							bool all_clauses_positive(Plist);
							int neg_nonunit_clauses(Plist);
							int negative_clauses(Plist);
							int most_literals(Plist);
							bool pos_equality_in_clauses(Plist);
							bool equality_in_clauses(Plist);
                            // BV(2016-nov-27): integer version of max_clause_weight, mods weights
                            // by 500 to ignore hint degradation
                            int imax_clause_weight(Plist);
					
};


#endif
