#ifndef TP_AC_REDUN_H
#define TP_AC_REDUN_H

#include "parautil.h"
#include "accanon.h"
#include "ladrvglobais.h"



class Ac_redun {
					
					
					private:
					
							static Ilist  C_symbols;   /* C: commutative */
							static Ilist A1_symbols;   /* A1: associative   (xy)z = x(yz) */
							static Ilist A2_symbols;   /* A2: c-associative x(yz) = y(xz) */
							static Ilist AC_symbols;   /* AC: All three */
							
							static OrderType can_compare(Term, Term, int);
							static Term canon(Term, int);
							static bool cac_redundant_atom(Term);
	
					public:
						static bool same_top(Term, Term);
						static int commutativity(Term);
						static int associativity(Term);
						static int c_associativity(Term);
						static int associativity3(Term);
						static int associativity4(Term);
						static bool cac_tautology(Literals);
						static bool cac_redundancy(Topform, bool);
};





#endif
