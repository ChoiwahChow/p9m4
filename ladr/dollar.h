#ifndef TP_DOLLAR_H
#define TP_DOLLAR_H

#include "clist.h"
#include "ladrvglobais.h"



enum class Op_Type
{
  SUM_OP=1, 
  PROD_OP, DIV_OP, MOD_OP, MIN_OP, MAX_OP, ABS_OP,
  NEG_OP,
  LT_OP, LE_OP, GT_OP, GE_OP,      /* arithmetic comparison */
  LLT_OP, LLE_OP, LGT_OP, LGE_OP,  /* lexical comparison */
  AND_OP, OR_OP,
  AND2_OP, OR2_OP,
  IF_OP,
  ID_OP, NID_OP,                   /* for all terms, including integers */
  VAR_OP, CONST_OP, GROUND_OP
};



struct rule {
  Topform c;
  Term alpha;
  Term beta;
  Term condition;
  struct rule *next;
};


class Dollar {

				private:
					static int Symbols_size;
					static int *Op_codes;
					static struct rule **Rules;
					static int Local_evals;
					
					static Term rewrite(Term , int , I3list *); 
					static Term rewrite_top(Term , int , I3list *);
				
				public:
					static void init_dollar_eval(Clist);
					static bool evaluable_predicate(int);
					static Term dollar_eval(Term);
					static void rewrite_with_eval(Topform);

};

#endif


