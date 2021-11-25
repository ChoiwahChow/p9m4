#ifndef TP_WEIGHT_H
#define TP_WEIGHT_H

#include "literals.h"
#include "unify.h"

class Weight {
	
				private:
						static Plist Rules;
						static double Constant_weight;
						static double Sk_constant_weight;
						static double Not_weight;
						static double Or_weight;
						static double Prop_atom_weight;
						static double Variable_weight;
						static double Nest_penalty;
						static double Depth_penalty;
						static double Var_penalty;
						static double Complexity;
						static bool Not_rules;  /* any rules for not_sym()? */
						static bool Or_rules;   /* any rules for or_sym()? */
						/* Cache the symnums */
						static int Eq_sn;      /* equality */
						static int Weight_sn;  /* weight function*/

						static int Sum_sn;     /* integer arithmetic */
						static int Prod_sn;    /* integer arithmetic */
						static int Neg_sn;     /* integer arithmetic */
						static int Div_sn;     /* integer arithmetic */
						static int Max_sn;     /* integer arithmetic */
						static int Min_sn;     /* integer arithmetic */
						static int Depth_sn;   /* depth */
						static int Vars_sn;    /* vars */
						static int Call_sn;    /* vars */
						static int Avar_sn;    /* anonymous variable */
						
						static bool weight_beta_check(Term);
						static bool weight_rule_check(Term rule);
						static int  apply_depth(Term, Context);
						static double weight_calc(Term, Context);
				public:
							static void init_weight(Plist ,
													double ,
													double ,
													double ,
													double ,
													double ,
													double ,
													double ,
													double ,
													double ,
													double 
													);
							static double weight(Term, Context);
							static double clause_weight(Literals);
				
	
	
};




#endif

