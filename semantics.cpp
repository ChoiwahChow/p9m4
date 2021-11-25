#include "prover9vglobais.h"
#include "semantics.h"
#include "./ladr/mystring.h"
#include "./ladr/fatal.h"





void Semantics::init_semantics(Plist interp_terms, ClockStructure eval_clock, string type, int eval_limit, int eval_var_limit){
  InterpContainer I;
  PlistContainer PCI;
  Plist p;
  
  int max_domain_size = 0;
  PCI.set_head(PROVER9_GLOBAL_SEMANTICS.Compiled_interps);
  for (p = interp_terms; p; p = p->next) {
    Interp a = I.compile_interp(Term(p->v), false);
    max_domain_size = IMAX(max_domain_size, I.interp_size(a));
    PROVER9_GLOBAL_SEMANTICS.Compiled_interps = PCI.plist_prepend(a);
  }
  PROVER9_GLOBAL_SEMANTICS.Compiled_interps = PCI.reverse_plist();
  if (myString::str_ident(type, "false_in_all")) PROVER9_GLOBAL_SEMANTICS.False_in_all = true;
  else if (myString::str_ident(type, "false_in_some"))     PROVER9_GLOBAL_SEMANTICS.False_in_all = false;
  else
    fatal::fatal_error("init_semantics, bad type");
  if (eval_var_limit == -1)   PROVER9_GLOBAL_SEMANTICS.Eval_limit = eval_limit;
  else {
    PROVER9_GLOBAL_SEMANTICS.Eval_limit = I.int_power(max_domain_size, eval_var_limit);
    cout<<"eval_limit reset to "<<PROVER9_GLOBAL_SEMANTICS.Eval_limit<<"."<<endl;
  }
  PROVER9_GLOBAL_SEMANTICS.Eval_clock=eval_clock;  
}


bool Semantics::eval_limit_ok(Interp p, int number_of_vars){
  InterpContainer I;
  if (PROVER9_GLOBAL_SEMANTICS.Eval_limit == -1)    return true;
  else {
    int evals_required = I.int_power(I.interp_size(p), number_of_vars);
    return evals_required <= PROVER9_GLOBAL_SEMANTICS.Eval_limit;
  }
}






void Semantics::eval_in_interps(Topform c) {
  TopformContainer TF;
 
  AttributeContainer A;
  InterpContainer I;
  if (PROVER9_GLOBAL_SEMANTICS.Compiled_interps == NULL) {
    /* There are no interps, so use default interp: positive literals TRUE. */
    if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
      c->semantics =(int) Semantics_type::SEMANTICS_FALSE;
    else
      c->semantics = (int) Semantics_type::SEMANTICS_TRUE;
  }
  else {

    int num_vars = LADRV_GLOBAIS_INST.Lit.number_of_variables(c->literals);

    if (!c->normal_vars)
      TF.renumber_variables(c, MAX_VARS);

    if (PROVER9_GLOBAL_SEMANTICS.False_in_all) {
      /* False_in_all:
	   SEMANTICS_FALSE: false in all interps (evaluable in all)
	   SEMANTICS_TRUE:  true in at least one interp
	   SEMANTICS_NOT_EVALUABLE: otherwise
       */
      Plist p = PROVER9_GLOBAL_SEMANTICS.Compiled_interps;
      c->semantics =(int) Semantics_type::SEMANTICS_FALSE;
      while (p && c->semantics != (int) Semantics_type::SEMANTICS_TRUE) {
		Interp x = (Interp) p->v;
		if (!eval_limit_ok(x, num_vars) || !I.evaluable_topform(c, x))
			c->semantics = (int) Semantics_type::SEMANTICS_NOT_EVALUABLE;
		else if (I.eval_literals(c->literals, x))
			c->semantics = (int) Semantics_type::SEMANTICS_TRUE;
		p = p->next;
      }
    }
      
    else {
      /* False_in_some:
	   SEMANTICS_FALSE: false in at lease one interp
	   SEMANTICS_TRUE:  true in all interps (evaluable in all)
	   SEMANTICS_NOT_EVALUABLE: otherwise
       */
      Plist p = PROVER9_GLOBAL_SEMANTICS.Compiled_interps;
      c->semantics = (int) Semantics_type::SEMANTICS_TRUE;
      while (p && c->semantics != (int) Semantics_type::SEMANTICS_FALSE) {
		Interp x =(Interp) p->v;
		if (!eval_limit_ok(x, num_vars) || !I.evaluable_topform(c, x))
		c->semantics =(int) Semantics_type::SEMANTICS_NOT_EVALUABLE;
		else if (!I.eval_literals(c->literals, x))
			c->semantics = (int) Semantics_type::SEMANTICS_FALSE;
		p = p->next;
      }
    }

    if (c->semantics == (int) Semantics_type::SEMANTICS_FALSE)
      c->attributes = A.set_string_attribute(c->attributes,A.label_att(),"false");
  }
}

void Semantics::set_semantics(Topform c) {
  if (c->semantics == (int) Semantics_type::SEMANTICS_NOT_EVALUATED) {
    PROVER9_GLOBAL_SEMANTICS.Eval_clock.clock_start();
    eval_in_interps(c);
    PROVER9_GLOBAL_SEMANTICS.Eval_clock.clock_stop();
  }
} 


void Semantics::update_semantics_new_constant(Topform c) {
  InterpContainer I;
  SymbolContainer S;
  TermContainer T;
  Term alpha = ARG(c->literals->atom, 0);
  Term beta = ARG(c->literals->atom, 1);
  Plist p;
  int n = T.biggest_variable(alpha);
  int *vals = (int *)malloc((n+1) * sizeof(int));
  int i;

  for (i = 0; i <= n; i++)
    vals[i] = 0;

  for (p = PROVER9_GLOBAL_SEMANTICS.Compiled_interps, i = 1; p; p = p->next, i++) {
    Interp x =(Interp) p->v;
    int val = I.eval_term_ground(alpha, x, vals);
    I.update_interp_with_constant(x, beta, val);
    cout<<"NOTE: updating interpretation "<<i<<": "<<S.sn_to_str(SYMNUM(beta))<<"="<<val<<endl;
  }
  free(vals);
} 
