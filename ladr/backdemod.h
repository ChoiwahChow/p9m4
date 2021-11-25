#ifndef TP_BACKDEMOD_H
#define TP_BACKDEMOD_H

#include "demod.h"
#include "clist.h"


class Backdemod {
					
					private:
							static void index_term_back_demod(Term, Mindex, Indexop);
							static bool rewritable_term(Term, Term, Context);
							static bool lex_rewritable(Term, Context, Term,bool lex_order_vars);
					
					public:
							static void index_clause_back_demod(Topform, Mindex, Indexop);
							static bool rewritable_clause(Topform, Topform);
							static Plist back_demod_linear(Topform, Clist,Plist);
							static Plist back_demod_indexed(Topform,int, Mindex, bool);


};


#endif
