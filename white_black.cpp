#include "./ladr/ladrvglobais.h"
#include "./ladr/fatal.h"
#include "./ladr/clause_eval.h"
#include "./ladr/interp.h"
#include "./ladr/options.h"

#include "white_black.h"
#include "semantics.h"
#include <limits>



Plist WhiteBlack::White_rules = NULL;
Plist WhiteBlack::Black_rules = NULL;
bool  WhiteBlack::Rule_needs_semantics = false;

void WhiteBlack::init_white_black(Plist white, Plist black){
  ClauseEvalContainer CEVAL;
  PlistContainer WR,BR;
  Plist p;
  WR.set_head(White_rules);
  BR.set_head(Black_rules);
  for (p = white; p; p = p->next) {
    Clause_eval ce = CEVAL.compile_clause_eval_rule((Term)p->v);
    if (ce == NULL)   fatal::fatal_error("Error in \"keep\" rule");
    else {
      White_rules = WR.plist_append(ce);
      if (CEVAL.rule_contains_semantics(ce))	Rule_needs_semantics = true;
    }
  }
  for (p = black; p; p = p->next) {
    Clause_eval ce = CEVAL.compile_clause_eval_rule((Term) p->v);
    if (ce == NULL)   fatal::fatal_error("Error in \"delete\" rule");
    else {
      Black_rules = BR.plist_append(ce);
      if (CEVAL.rule_contains_semantics(ce))	Rule_needs_semantics = true;
    }
  }
} 



Term new_rule_int(string property, OrderType order, int value) {
  TermContainer T;
  Term t;
  string s;
  switch(order) {
  case OrderType::LESS_THAN: s = "<";  break;
  case OrderType::LESS_THAN_OR_SAME_AS: s = "<=";  break;
  case OrderType::SAME_AS: s = "=";  break;
  case OrderType::GREATER_THAN_OR_SAME_AS: s = ">=";  break;
  case OrderType::GREATER_THAN: s = ">";  break;
  default: fatal::fatal_error("new_rule_int, bad relation");
  }
  t = T.get_rigid_term(s, 2);
  ARG(t,0) = T.get_rigid_term(property, 0);
  ARG(t,1) = T.int_to_term(value);
  return t;
} 


Term WhiteBlack::new_rule_int(string property, OrderType order, int value) {
  TermContainer T;
  Term t;
  string s;
  switch(order) {
    case OrderType::LESS_THAN: s = "<";  break;
    case OrderType::LESS_THAN_OR_SAME_AS: s = "<=";  break;
    case OrderType::SAME_AS: s = "=";  break;
    case OrderType::GREATER_THAN_OR_SAME_AS: s = ">=";  break;
    case OrderType::GREATER_THAN: s = ">";  break;
    default: fatal::fatal_error("new_rule_int, bad relation");
  }
  t = T.get_rigid_term(s, 2);
  ARG(t,0) = T.get_rigid_term(property, 0);
  ARG(t,1) = T.int_to_term(value);
  
  
  return t;
}  


Term WhiteBlack::new_rule_double(string property, OrderType order, double value){
  TermContainer T;
  Term t;
  string s;
  switch(order) {
  case OrderType::LESS_THAN: s = "<";  break;
  case OrderType::LESS_THAN_OR_SAME_AS: s = "<=";  break;
  case OrderType::SAME_AS: s = "=";  break;
  case OrderType::GREATER_THAN_OR_SAME_AS: s = ">=";  break;
  case OrderType::GREATER_THAN: s = ">";  break;
  default: fatal::fatal_error("new_rule_double, bad relation");
  }
  t = T.get_rigid_term(s, 2);
  ARG(t,0) = T.get_rigid_term(property, 0);
  ARG(t,1) = T.double_to_term(value);
  
  return t;
} 


Plist WhiteBlack::delete_rules_from_options(Prover_options opt) {
  PlistContainer P;
  Plist p = NULL;

  
  if (LADR_GLOBAL_OPTIONS.parm(opt->max_weight) != DBL_LARGE)
    p=P.plist_append((void *) new_rule_double ("weight",OrderType::GREATER_THAN, LADR_GLOBAL_OPTIONS.floatparm(opt->max_weight)));

  if (LADR_GLOBAL_OPTIONS.parm(opt->max_vars) != LADR_GLOBAL_OPTIONS.parm_default(opt->max_vars))
    p=P.plist_append(new_rule_int("variables",OrderType::GREATER_THAN,LADR_GLOBAL_OPTIONS.parm(opt->max_vars)));

  if (LADR_GLOBAL_OPTIONS.parm(opt->max_depth) != LADR_GLOBAL_OPTIONS.parm_default(opt->max_depth))
    p=P.plist_append(new_rule_int("depth",OrderType::GREATER_THAN,LADR_GLOBAL_OPTIONS.parm(opt->max_depth)));

  if (LADR_GLOBAL_OPTIONS.parm(opt->max_literals) != LADR_GLOBAL_OPTIONS.parm_default(opt->max_literals))
    p=P.plist_append(new_rule_int("literals",     OrderType::GREATER_THAN,LADR_GLOBAL_OPTIONS.parm(opt->max_literals)));
  return p;
} 


bool WhiteBlack::black_tests(Topform c) {
  Plist p;
  ClauseEvalContainer CEVAL;
  if (Rule_needs_semantics)
    Semantics::set_semantics(c);  /* in case not yet evaluated */

  for (p = Black_rules; p; p = p->next) {
    if (CEVAL.eval_clause_in_rule(c, (Clause_eval)p->v))
      return true;
  }
  return false;
}


bool WhiteBlack::white_tests(Topform c) {
  Plist p;
  ClauseEvalContainer CEVAL;
  if (Rule_needs_semantics)
    Semantics::set_semantics(c);  /* in case not yet evaluated */
  for (p = White_rules; p; p = p->next) {
    if (CEVAL.eval_clause_in_rule(c, (Clause_eval) p->v))
      return true;
  }
  return false;
}
