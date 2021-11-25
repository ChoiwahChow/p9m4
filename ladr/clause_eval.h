#ifndef TP_CLAUSE_EVAL_H
#define TP_CLAUSE_EVAL_H

#include "topform.h"



enum class InterpType { 
        SEMANTICS_NOT_EVALUATED,
        SEMANTICS_NOT_EVALUABLE,
        SEMANTICS_TRUE,
        SEMANTICS_FALSE
 };



enum class ClauseEvalType
	{ 
		CL_EVAL_AND,
		CL_EVAL_OR,
		CL_EVAL_NOT,
		CL_EVAL_ALL,

		CL_EVAL_LESS,
		CL_EVAL_LESS_EQUAL,
		CL_EVAL_GREATER,
		CL_EVAL_GREATER_EQUAL,
		CL_EVAL_EQUAL,

		CL_EVAL_POSITIVE,
		CL_EVAL_NEGATIVE,
		CL_EVAL_MIXED,

		CL_EVAL_HINT,

		CL_EVAL_TRUE,
		CL_EVAL_FALSE,

		CL_EVAL_HAS_EQUALITY,
		CL_EVAL_HORN,
		CL_EVAL_DEFINITE,
		CL_EVAL_UNIT,
		CL_EVAL_INITIAL,
		CL_EVAL_RESOLVENT,
		CL_EVAL_UR_RESOLVENT,
		CL_EVAL_HYPER_RESOLVENT,
		CL_EVAL_FACTOR,
		CL_EVAL_PARAMODULANT,
		CL_EVAL_BACK_DEMODULANT,
		CL_EVAL_SUBSUMER,
		CL_EVAL_WEIGHT,
		CL_EVAL_VARIABLES,
		CL_EVAL_DEPTH,
		CL_EVAL_LITERALS,
		CL_EVAL_LEVEL,
        CL_EVAL_HINT_AGE,
        // BV(2017-nov-07): max_gen_eq property
        CL_EVAL_MAX_GEN_EQ
}; 


typedef struct clause_eval *Clause_eval;

struct clause_eval {
  int          type;
  Clause_eval  left;       /* for AND, OR, NOT */
  Clause_eval  right;      /* for AND, OR */
  OrderType    relation;   /* <, >, = */
  double       test_val;        /* for comparison, e.g., weight < 3 */
};

class GlobalClauseEval {

						private:
                                unsigned Clause_eval_gets, Clause_eval_frees;

                        public:        
                                GlobalClauseEval();
                                ~GlobalClauseEval();
                                
						friend class ClauseEvalContainer;
                        friend class LadrVGlobais;
};


class ClauseEvalContainer {
	
							private: Clause_eval root;
							
									Clause_eval get_clause_eval(void);
									void free_clause_eval(Clause_eval p);
							
							public:
									ClauseEvalContainer();
									~ClauseEvalContainer();
									void zap_clause_eval_rule(Clause_eval);
									Clause_eval compile_clause_eval_rule(Term);
									bool eval_clause_in_rule(Topform, Clause_eval);
									bool rule_contains_semantics(Clause_eval);

							
	
};

#endif

