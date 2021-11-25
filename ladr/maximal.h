#ifndef TP_MAXIMAL_H
#define TP_MAXIMAL_H

#include "literals.h"
#include "termorder.h"



enum class CheckType {  /* how to check for maximal literals */
  FLAG_CHECK=0,
  FULL_CHECK=1
};


class Maximal {
					
						private:
                            	static	int Maximal_flag;		  /* termflag to mark maximal literals */
                                static	int Maximal_signed_flag;  /* to mark maximal literals within sign */
                                static 	int Selected_flag;        /* to mark selected literals */
                                static  TermflagContainer TF;
                                static  SymbolContainer S;
                            
								static bool greater_literals(Literals, Literals);
						public:
								static void init_maximal(void);
								static bool max_lit_test(Literals, Literals);
								static bool max_signed_lit_test(Literals, Literals);
								static void mark_maximal_literals(Literals);
								static bool maximal_literal(Literals, Literals, int);
								static bool maximal_signed_literal(Literals, Literals, int);
								static int  number_of_maximal_literals(Literals, int);
								static void mark_selected_literal(Literals);
								static void mark_selected_literals(Literals, string selection);
								static bool selected_literal(Literals);
								static bool exists_selected_literal(Literals);
								static void copy_selected_literal_marks(Literals, Literals);
};



#endif
