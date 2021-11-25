#include "weight.h"
#include "weight2.h"
#include "parse.h"
#include "complex.h"
#include "fatal.h"


 Plist Weight::Rules;
 double Weight::Constant_weight;
 double Weight::Sk_constant_weight;
 double Weight::Not_weight;
 double Weight::Or_weight;
 double Weight::Prop_atom_weight;
 double Weight::Variable_weight;
 double Weight::Nest_penalty;
 double Weight::Depth_penalty;
 double Weight::Var_penalty;
 double Weight::Complexity;
 bool Weight::Not_rules;  /* any rules for not_sym()? */
 bool Weight::Or_rules;   /* any rules for or_sym()? */
/* Cache the symnums */
 int Weight::Eq_sn;      /* equality */
 int Weight::Weight_sn;  /* weight function*/

 int Weight::Sum_sn;     /* integer arithmetic */
 int Weight::Prod_sn;    /* integer arithmetic */
 int Weight::Neg_sn;     /* integer arithmetic */
 int Weight::Div_sn;     /* integer arithmetic */
 int Weight::Max_sn;     /* integer arithmetic */
 int Weight::Min_sn;     /* integer arithmetic */
 int Weight::Depth_sn;   /* depth */
 int Weight::Vars_sn;    /* vars */
 int Weight::Call_sn;    /* vars */
 //int Weight::Avar_sn;    /* anonymous variable */
 
 
 
bool Weight::weight_beta_check(Term b) {
  TermContainer T;
  if (SYMNUM(b) == Sum_sn ||
      SYMNUM(b) == Prod_sn ||
      SYMNUM(b) == Div_sn ||
      SYMNUM(b) == Min_sn ||
      SYMNUM(b) == Max_sn)
    return weight_beta_check(ARG(b,0)) &&  weight_beta_check(ARG(b,1));
  else if (SYMNUM(b) == Neg_sn)
    return weight_beta_check(ARG(b,0));
  else if (SYMNUM(b) == Depth_sn)
    return true;
  else if (SYMNUM(b) == Vars_sn)
    return true;
  else if (SYMNUM(b) == Call_sn)
    return true;
  else if (SYMNUM(b) == Weight_sn)
    return true;
  else {
    double d;
    if (T.term_to_number(b, &d))
      return true;
    else {
      cout<<"weight_rule_check, right side of rule not understood"<<endl;
	  return false;
    }
  }
}
 
 
bool Weight::weight_rule_check(Term rule) {
  SymbolContainer S;
  if (!S.is_eq_symbol(SYMNUM(rule))) {
    cout<<"weight_rule_check, rule is not equality"<<endl;
	return false;
  }
  else  if (SYMNUM(ARG(rule, 0)) != Weight_sn) {
    cout<<"weight_rule_check, left side must be weight(...)"<<endl;
	return false;
  }
  else
    return weight_beta_check(ARG(rule, 1));
} 

void Weight::init_weight(	 Plist rules,
							 double variable_weight,
							 double constant_weight,
							 double not_weight,
							 double or_weight,
							 double sk_constant_weight,
							 double prop_atom_weight,
							 double nest_penalty,
							 double depth_penalty,
							 double var_penalty,
							 double complexity)
{
  Plist p;
  SymbolContainer S;
  Variable_weight = variable_weight;
  Constant_weight = constant_weight;
  Not_weight = not_weight;
  Or_weight = or_weight;
  Prop_atom_weight = prop_atom_weight;
  Sk_constant_weight = sk_constant_weight;
  Nest_penalty = nest_penalty;
  Depth_penalty = depth_penalty;
  Var_penalty = var_penalty;
  Complexity = complexity;

  /* Cache symbol numbers. */

  Weight_sn  = 	S.str_to_sn("weight", 1);
  Eq_sn  = 		S.str_to_sn(S.eq_sym(), 2);

  Sum_sn  = 	S.str_to_sn("+", 2);
  Prod_sn = 	S.str_to_sn("*", 2);
  Div_sn  = 	S.str_to_sn("/", 2);
  Max_sn  = 	S.str_to_sn("max", 2);
  Min_sn  = 	S.str_to_sn("min", 2);
  Depth_sn  = 	S.str_to_sn("depth", 1);
  Vars_sn  = 	S.str_to_sn("vars", 1);
  Call_sn  = 	S.str_to_sn("call", 2);
  Neg_sn  = 	S.str_to_sn("-", 1);
 // Avar_sn = 	S.str_to_sn("_", 0);

  /* Process the rules. */

  Rules = NULL;
  TermContainer T;
  for (p = rules; p; p = p->next) {
    
    Term rule = T.copy_term((Term)p->v);
    if (!weight_rule_check(rule)) {
    
	  T.p_term(rule);
      fatal::fatal_error("init_weight, bad rule");
    }
    else {
      T.term_set_variables(rule, MAX_VARS);
      PlistContainer P;
      P.set_head(Rules);
      Rules = P.plist_append(rule);
      if (T.is_term(ARG(ARG(rule,0),0), S.not_sym(), 1)) Not_rules = true;
      if (T.is_term(ARG(ARG(rule,0),0), S.or_sym(), 2)) Or_rules = true;
    }
  }
} 

int Weight::apply_depth(Term t, Context subst) {
  TermContainer T;
  if (VARIABLE(t))
    return T.term_depth(subst->terms[VARNUM(t)]);
  else if (CONSTANT(t))
    return 0;
  else {
    int depth = 0;
    int i;
    for (i = 0; i < ARITY(t); i++) {
      int d = apply_depth(ARG(t,i), subst);
      depth = IMAX(d, depth);
    }
    return depth + 1;
  }
}

double Weight::weight_calc(Term b, Context subst) {
  /* Apply a rule.  Term b is the right side of the rule. 
     The substitution matches the left side of the rule with the
     term being weighed.

     This routine is recursive, applying arithmetic rules to the
     top of b as much as possible.  When a non-arithmetic expression
     is encountered, the substition is applied to it and then
     it is weighed as usual.
   */
  TermContainer T;
  UnifyContainer U;
  if (VARIABLE(b)) {
    fatal::fatal_error("weight_calc, variable in rule");
    return 0;  /* to please the compiler */
  }
  else if (SYMNUM(b) == Weight_sn) {
    
	Term b_prime = U.apply(ARG(b,0), subst);
    Context subst2 = U.get_context();
    double wt = weight(b_prime, subst2);
    U.free_context(subst2);
    T.zap_term(b_prime);
    return wt;
  }
  else if (SYMNUM(b) == Sum_sn)
    return
      weight_calc(ARG(b,0), subst) +
      weight_calc(ARG(b,1), subst);
  else if (SYMNUM(b) == Prod_sn)
    return
      weight_calc(ARG(b,0), subst) *
      weight_calc(ARG(b,1), subst);
  else if (SYMNUM(b) == Div_sn)
    return
      weight_calc(ARG(b,0), subst) /
      weight_calc(ARG(b,1), subst);
  else if (SYMNUM(b) == Max_sn) {
    int w1 = weight_calc(ARG(b,0), subst);
    int w2 = weight_calc(ARG(b,1), subst);
    return IMAX(w1,w2);
  }
  else if (SYMNUM(b) == Min_sn) {
    int w1 = weight_calc(ARG(b,0), subst);
    int w2 = weight_calc(ARG(b,1), subst);
    return IMIN(w1,w2);
  }
  else if (SYMNUM(b) == Neg_sn) {
    return -weight_calc(ARG(b,0), subst);
  }
  else if (SYMNUM(b) == Depth_sn)
    return apply_depth(ARG(b,0), subst);
  else if (SYMNUM(b) == Vars_sn) {
   
	Term b_prime = U.apply(ARG(b,0), subst);
    int n = T.number_of_vars_in_term(b_prime);
    T.zap_term(b_prime);
    return n;
  }
  else if (SYMNUM(b) == Call_sn) {
   
	string prog = T.term_symbol(ARG(b,0));
    Term b_prime = U.apply(ARG(b,1), subst);
    double x = Weight2::call_weight(prog, b_prime);
    T.zap_term(b_prime);
    return x;
  }
  else {
    double wt;
    if (T.term_to_number(b, &wt))
      return wt;
    else {
      fatal::fatal_error("weight_calc, bad rule");
      return 0;  /* to please the compiler */
    }
  }
} 


double Weight::weight(Term t, Context subst) {
  UnifyContainer U;
  TermContainer T;
  SymbolContainer S;
  if (VARIABLE(t))
    return Variable_weight;
  else {
    /* Look for a rule to apply. */
    Plist p;
    for (p = Rules; p; p = p->next) {
      Term rule = (Term)p->v;           /* weight(f(x)) = 3 + weight(x) */
      Term alpha = ARG(rule,0);
      Term beta  = ARG(rule,1);
      Trail tr = NULL;
      
      int anyvar_ctx[MAX_ANYVARS];
      int i;
      for (i=0; i<MAX_ANYVARS; i++) anyvar_ctx[i]=-1;
      
      if (U.match_weight(ARG(alpha,0), subst, t, &tr, anyvar_ctx)) {
	/* We found a rule.  Now calculate the weight. */
        double wt = weight_calc(beta, subst);
        U.undo_subst(tr);
        return wt;
      }
    }
    /* Nothing matches; return the default. */
    if (CONSTANT(t)) {
      if (T.skolem_term(t) && Sk_constant_weight != 1) return Sk_constant_weight;
      else if (S.relation_symbol(SYMNUM(t))) return Prop_atom_weight;
      else return Constant_weight;
    }
    else {
      /* sum of weights of subterms, plus 1 */
      double wt = 1;
      int i;
      for (i = 0; i < ARITY(t); i++) {
        double arg_wt = weight(ARG(t, i), subst);
        if (Nest_penalty != 0 && ARITY(t) <= 2 && SYMNUM(t) == SYMNUM(ARG(t,i))) wt += Nest_penalty;
        wt += arg_wt;
      }
      return wt;
    }
  }
}

double Weight::clause_weight(Literals lits) {
  double wt;
 
  TermContainer T;
  Complex C;
  
  if (!Not_rules && !Or_rules) {
    /* There are no rules for OR or NOT, so we don't need to construct a
       Term representation of the clause. */
    Literals lit;
    wt = 0;
    for (lit = lits; lit; lit = lit->next) {
      UnifyContainer U;
	  Context subst = U.get_context();
      wt += weight(lit->atom, subst);
      U.free_context(subst);
    }
    wt += (LADRV_GLOBAIS_INST.Lit.negative_literals(lits) * Not_weight);
    wt += ((LADRV_GLOBAIS_INST.Lit.number_of_literals(lits)-1) * Or_weight);
  }
  else {
    /* Build a temporary Term representation of the clause and weigh that.
       This is done in case there are weight rules for OR or NOT. */
   
	UnifyContainer U;
	Term temp = LADRV_GLOBAIS_INST.Lit.lits_to_term(lits);
    Context subst = U.get_context();
    wt = weight(temp, subst);
    U.free_context(subst);
    LADRV_GLOBAIS_INST.Lit.free_lits_to_term(temp);

    /* If there are no Not_rules, we have already added one for each not;
       so we undo that and add the correct amount.  Same for Or_rules. */
       
    if (!Not_rules)
      wt += (LADRV_GLOBAIS_INST.Lit.negative_literals(lits) * (Not_weight - 1));
    if (!Or_rules)
      wt += ((LADRV_GLOBAIS_INST.Lit.number_of_literals(lits) - 1) * (Or_weight - 1));
  }

  if (Depth_penalty != 0)
    wt += Depth_penalty * LADRV_GLOBAIS_INST.Lit.literals_depth(lits);

  if (Var_penalty != 0)
    wt += Var_penalty * LADRV_GLOBAIS_INST.Lit.number_of_variables(lits);
 
  if (Complexity != 0)
    wt += Complexity * (1 - C.clause_complexity(lits, 4, 0));

  return wt;

}
