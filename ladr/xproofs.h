#ifndef TP_XPROOFS_H
#define TP_XPROOFS_H

#include "clauses.h"
#include "clause_misc.h"
#include "paramod.h"
#include "subsume.h"

class Xproofs {
	
				private:
						static Topform xx_res2(Topform, int);
						static void xx_simp2(Topform, int);
						static Topform factor(Topform,int, int);
						static void merge1(Topform, int);
				
	
				public:
					static void check_parents_and_uplinks_in_proof(Plist);
					static Topform proof_id_to_clause(Plist, int);
					static int greatest_id_in_proof(Plist);
					static Plist expand_proof(Plist, I3list *);
					static void renumber_proof(Plist, int);
					static Plist copy_and_renumber_proof(Plist, int);
					static Plist proof_to_xproof(Plist);
				
	
	
};


#endif
