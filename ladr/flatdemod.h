
#ifndef TP_FLATDEMOD_H
#define TP_FLATDEMOD_H

#include "parautil.h"
#include "mindex.h"

#define MAYBE_UNBIND(f,c)  if (f->varnum_bound_to >= 0) { c->terms[f->varnum_bound_to] = NULL; f->varnum_bound_to = -1; }

enum class Flatdemod_mode
						{ 
							_GO, 
							_BACKTRACK 
						};

class Flatdemod {
	
					private:
								static int Fdemod_attempts;
								static int Fdemod_rewrites;
								static Flatterm fdemod(Flatterm, Discrim, Context,int *,int,int *,int *, I3list *, bool);
								
								#if 0
								void maybe_unbind(Flatterm, Context);
								#endif
							
	
					public:
							static Flatterm fapply_demod(Term, Context);
							static Plist discrim_flat_retrieve_leaf(Flatterm, Discrim, Context, Flatterm *);
							static void *discrim_flat_retrieve_first(Flatterm, Discrim,Context, Discrim_pos *);
							static void *discrim_flat_retrieve_next(Discrim_pos);
							static void discrim_flat_cancel(Discrim_pos);
							static Term fdemodulate(Term, Discrim, int *, int *, int *, I3list *, bool lex_order_vars);
							static int fdemod_attempts();
							static int fdemod_rewrites();
							static void fdemod_clause(Topform, Mindex,int *, int *i, bool lex_order_vars);								
};


#endif
