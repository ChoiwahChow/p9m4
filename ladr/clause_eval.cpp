#include "ladrvglobais.h"
#include "clause_eval.h"
#include "memory.h"
#include "fatal.h"
#include "just.h"





GlobalClauseEval::GlobalClauseEval() {
    Clause_eval_gets=0;
    Clause_eval_frees=0;
}

GlobalClauseEval::~GlobalClauseEval() {

    
}


ClauseEvalContainer::ClauseEvalContainer() {
	root=NULL;
}

ClauseEvalContainer::~ClauseEvalContainer() {
	root=NULL;
}

Clause_eval ClauseEvalContainer::get_clause_eval(void) {
  Clause_eval p= (Clause_eval) Memory::memCNew(sizeof(struct clause_eval));
  LADR_GLOBAL_CLAUSE_EVAL.Clause_eval_gets++;
  return(p);
}

void ClauseEvalContainer::free_clause_eval(Clause_eval p) {
  Memory::memFree((void *)p, sizeof(struct clause_eval));
  LADR_GLOBAL_CLAUSE_EVAL.Clause_eval_frees++;
}

void ClauseEvalContainer::zap_clause_eval_rule(Clause_eval p) {
  if (p->type == (int)ClauseEvalType::CL_EVAL_AND || p->type == (int)ClauseEvalType::CL_EVAL_OR) {
    zap_clause_eval_rule(p->left);
    zap_clause_eval_rule(p->right);
  }
  else if (p->type == (int)ClauseEvalType::CL_EVAL_OR)
    zap_clause_eval_rule(p->left);
  
  free_clause_eval(p);
} 



Clause_eval ClauseEvalContainer::compile_clause_eval_rule(Term t) {
  TermContainer T;
  Clause_eval p = get_clause_eval();

  if (T.is_term(t, "&", 2)) {
    p->type = (int) ClauseEvalType::CL_EVAL_AND;
    p->left  = compile_clause_eval_rule(ARG(t,0));
    if (p->left == NULL)
      return NULL;
    p->right = compile_clause_eval_rule(ARG(t,1));
    if (p->right == NULL)
      return NULL;
  }

  else if (T.is_term(t, "|", 2)) {
    p->type = (int) ClauseEvalType::CL_EVAL_OR;
    p->left  = compile_clause_eval_rule(ARG(t,0));
    if (p->left == NULL)
      return NULL;
    p->right = compile_clause_eval_rule(ARG(t,1));
    if (p->right == NULL)
      return NULL;
  }

  else if (T.is_term(t, "-", 1)) {
    p->type = (int) ClauseEvalType::CL_EVAL_NOT;
    p->left  = compile_clause_eval_rule(ARG(t,0));
    if (p->left == NULL)
      return NULL;
  }

  else if (T.is_term(t, "all",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_ALL;
  else if (T.is_term(t, "positive",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_POSITIVE;
  else if (T.is_term(t, "negative",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_NEGATIVE;
  else if (T.is_term(t, "mixed",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_MIXED;

  else if (T.is_term(t, "true",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_TRUE;
  else if (T.is_term(t, "false",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_FALSE;

  else if (T.is_term(t, "has_equality",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_HAS_EQUALITY;
  else if (T.is_term(t, "horn",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_HORN;
  else if (T.is_term(t, "definite",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_DEFINITE;
  else if (T.is_term(t, "unit",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_UNIT;
  else if (T.is_term(t, "hint",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_HINT;

  else if (T.is_term(t, "initial",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_INITIAL;
  else if (T.is_term(t, "resolvent",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_RESOLVENT;
  else if (T.is_term(t, "hyper_resolvent",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_HYPER_RESOLVENT;
  else if (T.is_term(t, "ur_resolvent",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_UR_RESOLVENT;
  else if (T.is_term(t, "factor",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_FACTOR;
  else if (T.is_term(t, "paramodulant",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_PARAMODULANT;
  else if (T.is_term(t, "back_demodulant",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_BACK_DEMODULANT;
  else if (T.is_term(t, "subsumer",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_SUBSUMER;
   // BV(2017-nov-07): max_gen_eq property
  else if (T.is_term(t, "max_gen_eq",  0))
    p->type = (int) ClauseEvalType::CL_EVAL_MAX_GEN_EQ;
  
  

  else if (T.is_term(t, "<",  2) ||
	   T.is_term(t, ">",  2) ||
	   T.is_term(t, "<=", 2) ||
	   T.is_term(t, ">=", 2) ||
	   T.is_term(t, "=",  2)) {
   
    Term a0 = ARG(t,0);
    Term a1 = ARG(t,1);
    
    
    if (T.is_term(a0, "weight",  0))
      p->type = (int) ClauseEvalType::CL_EVAL_WEIGHT;
    else if (T.is_term(a0, "variables",  0))
      p->type = (int) ClauseEvalType::CL_EVAL_VARIABLES;
    else if (T.is_term(a0, "depth",  0))
      p->type = (int) ClauseEvalType::CL_EVAL_DEPTH;
    else if (T.is_term(a0, "literals",  0))
      p->type = (int) ClauseEvalType::CL_EVAL_LITERALS;
    else if (T.is_term(a0, "level",  0))
      p->type = (int) ClauseEvalType::CL_EVAL_LEVEL;
    else if (T.is_term(a0,"hint_age",0))
      p->type=(int) ClauseEvalType::CL_EVAL_HINT_AGE;  
    
    else
      return NULL;

    if (!T.term_to_number(a1, &(p->test_val))) 
        return NULL;
    if (T.is_term(t, "<",  2))
      p->relation = OrderType::LESS_THAN;
    else if (T.is_term(t, ">",  2))
      p->relation = OrderType::GREATER_THAN;
    else if (T.is_term(t, "<=", 2))
      p->relation = OrderType::LESS_THAN_OR_SAME_AS;
    else if (T.is_term(t, ">=", 2))
      p->relation = OrderType::GREATER_THAN_OR_SAME_AS;
    else if (T.is_term(t, "=",  2))
      p->relation = OrderType::SAME_AS;
  }
  else
    return NULL;
  return p;
} 


bool ClauseEvalContainer::eval_clause_in_rule(Topform c, Clause_eval p) {

  JustContainer J;
  
  Literals lits = c->literals;

  switch (p->type) {

  case (int) ClauseEvalType::CL_EVAL_AND:
    return
      eval_clause_in_rule(c, p->left) &&
      eval_clause_in_rule(c, p->right);
  case (int) ClauseEvalType::CL_EVAL_OR:
    return
      eval_clause_in_rule(c, p->left) ||
      eval_clause_in_rule(c, p->right);
  case (int) ClauseEvalType::CL_EVAL_NOT:
    return
      !eval_clause_in_rule(c, p->left);

  case (int) ClauseEvalType::CL_EVAL_ALL:
    return true;
  case (int) ClauseEvalType::CL_EVAL_POSITIVE:
    return LADRV_GLOBAIS_INST.Lit.positive_clause(lits);
  case (int) ClauseEvalType::CL_EVAL_NEGATIVE:
    return LADRV_GLOBAIS_INST.Lit.negative_clause(lits);
  case (int) ClauseEvalType::CL_EVAL_MIXED:
    return LADRV_GLOBAIS_INST.Lit.mixed_clause(lits);

  case (int) ClauseEvalType::CL_EVAL_HINT:
    return c->matching_hint != NULL;

  case (int) ClauseEvalType::CL_EVAL_HAS_EQUALITY:
    return LADRV_GLOBAIS_INST.Lit.contains_eq(lits);
  case (int) ClauseEvalType::CL_EVAL_HORN:
    return LADRV_GLOBAIS_INST.Lit.horn_clause(lits);
  case (int) ClauseEvalType::CL_EVAL_DEFINITE:
    return LADRV_GLOBAIS_INST.Lit.definite_clause(lits);
  case (int) ClauseEvalType::CL_EVAL_UNIT:
    return LADRV_GLOBAIS_INST.Lit.unit_clause(lits);

  case (int) ClauseEvalType::CL_EVAL_INITIAL:
    return c->initial;
  case (int) ClauseEvalType::CL_EVAL_RESOLVENT:
    return J.primary_just_type(c, Just_type::BINARY_RES_JUST);
  case (int) ClauseEvalType::CL_EVAL_HYPER_RESOLVENT:
    return J.primary_just_type(c, Just_type::HYPER_RES_JUST);
  case (int) ClauseEvalType::CL_EVAL_UR_RESOLVENT:
    return J.primary_just_type(c, Just_type::UR_RES_JUST);
  case (int) ClauseEvalType::CL_EVAL_FACTOR:
    return J.primary_just_type(c, Just_type::FACTOR_JUST);
  case (int) ClauseEvalType::CL_EVAL_PARAMODULANT:
    return J.primary_just_type(c, Just_type::PARA_JUST);
  case (int) ClauseEvalType::CL_EVAL_BACK_DEMODULANT:
    return J.primary_just_type(c, Just_type::BACK_DEMOD_JUST);
  case (int) ClauseEvalType::CL_EVAL_SUBSUMER:
    return c->subsumer;

  case (int) ClauseEvalType::CL_EVAL_TRUE:
    return
      c->semantics == (int) InterpType::SEMANTICS_TRUE ||
      c->semantics == (int) InterpType::SEMANTICS_NOT_EVALUABLE;
  case (int) ClauseEvalType::CL_EVAL_FALSE:
    return c->semantics == (int) InterpType::SEMANTICS_FALSE;

  case (int) ClauseEvalType::CL_EVAL_WEIGHT:
  case (int) ClauseEvalType::CL_EVAL_VARIABLES:
  case (int) ClauseEvalType::CL_EVAL_DEPTH:
  case (int) ClauseEvalType::CL_EVAL_LITERALS:
  case (int) ClauseEvalType::CL_EVAL_HINT_AGE:    
  case (int) ClauseEvalType::CL_EVAL_LEVEL: {
    double val = 0;
    switch (p->type) {
    
        case (int) ClauseEvalType::CL_EVAL_WEIGHT:    val = c->weight;                    break;
        case (int) ClauseEvalType::CL_EVAL_VARIABLES: val = LADRV_GLOBAIS_INST.Lit.number_of_variables(lits);  break;
        case (int) ClauseEvalType::CL_EVAL_DEPTH:     val = LADRV_GLOBAIS_INST.Lit.literals_depth(lits);       break;
        case (int) ClauseEvalType::CL_EVAL_LITERALS:  val = LADRV_GLOBAIS_INST.Lit.number_of_literals(lits);   break;
        case (int) ClauseEvalType::CL_EVAL_HINT_AGE:
                                                    if(c->matching_hint!=NULL) val=c->matching_hint->id;
                                                    else val=99999;
                                                    break;
    
    case (int) ClauseEvalType::CL_EVAL_LEVEL:     val = J.clause_level(c);            break;
    }
    
    switch (p->relation) {
    case OrderType::LESS_THAN:                return val <  p->test_val;
    case OrderType::GREATER_THAN:             return val >  p->test_val;
    case OrderType::LESS_THAN_OR_SAME_AS:     return val <= p->test_val;
    case OrderType::GREATER_THAN_OR_SAME_AS:  return val >= p->test_val;
    case OrderType::SAME_AS:                  return val == p->test_val;
    default: fatal::fatal_error("eval_clause_in_rule, bad relation");
    }
  }
  default: fatal::fatal_error("eval_clause_in_rule, unknown operation");
  }  /* outer switch */
  return false;
} 


bool ClauseEvalContainer::rule_contains_semantics(Clause_eval p) {
  if (p->type == (int) ClauseEvalType::CL_EVAL_AND || p->type == (int) ClauseEvalType::CL_EVAL_OR) {
    return (rule_contains_semantics(p->left) ||
	    rule_contains_semantics(p->right));
  }
  else if (p->type == (int) ClauseEvalType::CL_EVAL_OR)
    return rule_contains_semantics(p->left);
  else
    return p->type == (int) ClauseEvalType::CL_EVAL_TRUE || p->type == (int) ClauseEvalType::CL_EVAL_FALSE;
}
