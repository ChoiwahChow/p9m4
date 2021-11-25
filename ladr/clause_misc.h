#ifndef TP_CLAUSE_MISC_H
#define TP_CLAUSE_MISC_H

#include "clist.h"
#include "mindex.h"
#include "just.h"
#include "basic.h"


class ClauseMisc {
					private:
					
					public:
						static Clist clist_copy(Clist, bool);
						static Clist copy_clauses_to_clist(Plist, string, bool);
						static Clist move_clauses_to_clist(Plist, string, bool);
						static Plist input_clauses(Plist);
						static void delete_clause(Topform);
						static void delete_clist(Clist);
						static Topform copy_clause_ija(Topform);
						static Plist copy_clauses_ija(Plist);
						static void delete_clauses(Plist);
						static void make_clause_basic(Topform);
};



#endif

