#ifndef TP_COMPLEX_H
#define TP_COMPLEX_H

#include "topform.h"
#include "ladrvglobais.h"


class Complex {

				private:
						
						#ifdef SOLO
							static double complexity1(int *, int);
						#endif

						#ifndef SOLO	
							static Term complex4_devar(Term, int);
							static Term complex4_revar(Term, int);
							static void complex4_compare(Term, Term, int *);
							static bool complex4_p2(Term, Term, int *);
							static void complex4_p1(Term, Term, int *);
							static void term_to_ints(Term, int *, int *);
						#endif
							
                            static double complexity2(int *, int);
							static double complexity3(int *, int, int);
                            
						
						
				public:
						#ifndef SOLO
							static double complex4(Term);
							static double term_complexity(Term, int, int);
							static double clause_complexity(Literals, int, int);
						#endif

};


#endif


