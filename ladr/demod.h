#ifndef TP_DEMOD_H
#define TP_DEMOD_H

#include "parautil.h"
#include "mindex.h"
#include "ladrvglobais.h"

enum class Demod_Type
						{ 
							NOT_DEMODULATOR, 
							ORIENTED, 
							LEX_DEP_LR, 
							LEX_DEP_RL, 
							LEX_DEP_BOTH 
						};


class Demod {
	
				private:
					static int Demod_attempts;
					static int Demod_rewrites;
					
					static Term demod(Term, Mindex , int , Ilist *, bool);
					static Term contract_bt(Term, Mindex, int, Topform *);
					static Term demod_bt(Term, Mindex, int, int,Ilist *);
					static Term demod1_recurse(Term, Term, Topform, int, Ilist *, bool);
					static Term part_recurse(Term, Term, Topform, int, int , int *, Ilist *);
				
				
				public:
				
					static int demodulator_type(Topform, int, bool);
					static void idx_demodulator(Topform, int , Indexop, Mindex );
					static int demod_attempts();
					static int demod_rewrites();
					static Term demodulate(Term,Mindex, Ilist *, bool);
					static void demod1(Topform, Topform, int ,Ilist *,Ilist *,bool);
					static void particular_demod(Topform, Topform,int, int,Ilist *, Ilist *);
};


#endif
