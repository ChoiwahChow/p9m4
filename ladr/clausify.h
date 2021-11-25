#ifndef TP_CLAUSIFY_H
#define TP_CLAUSIFY_H

#include "topform.h"
#include "cnf.h"
#include "ladrvglobais.h"


class Clausify {
					
					private:
							Term vars_to_names(Term);
							Formula literal_to_formula(Literals);
					
					public:
							Literals formula_to_literal(Formula);
							Literals formula_to_literals(Formula);
							Topform formula_to_clause(Formula);
							Plist formula_to_clauses(Formula);
							Plist clausify_formula(Formula);
							Formula clause_to_formula(Topform);
		
};

#endif
