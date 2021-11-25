#include "./ladr/ladrvglobais.h"
#include "./prover9vglobais.h"
#include "./ladr/fatal.h"
#include "./ladr/top_input.h"
#include "./ladr/ioutil.h"
#include "./ladr/mystring.h"
#include "actions.h"




void (*Actions::Rebuild_sos_proc) (void);
void (*Actions::Exit_proc) (int);  			/* this is called for "exit" actions */
void (*Actions::Assert_proc) (Topform);  	/* for "assert" actions */















bool Actions::changable_flag(Term t) {
  return ((T.is_term(t, "set", 1) || T.is_term(t, "clear", 1)) &&  
				myString::string_member(S.sn_to_str(SYMNUM(ARG(t,0))), PROVER9_GLOBAL_ACTIONS.Changable_flags,NN));
} 


bool Actions::changable_parm(Term t) {
  
  return (T.is_term(t, "assign", 2) && myString::string_member(S.sn_to_str(SYMNUM(ARG(t,0))),	PROVER9_GLOBAL_ACTIONS.Changable_parms,NN));
} 


bool Actions::changable_flag_rebuild(Term t) {
  
  return ((T.is_term(t, "set", 1) || T.is_term(t, "clear", 1))) &&  myString::string_member(S.sn_to_str(SYMNUM(ARG(t,0))),PROVER9_GLOBAL_ACTIONS.Changable_flags_rebuild,NN);
  
} 

  


bool Actions::changable_parm_rebuild(Term t) {
  
  return (T.is_term(t, "assign", 2) &&  
			myString::string_member(S.sn_to_str(SYMNUM(ARG(t,0))),PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild,NN));
} 


void Actions::init_action(Term t) {

  
  Term trigger, stat;
  Term rule = T.copy_term(t);
  if (!T.is_term(rule, "->", 2))
    fatal::fatal_error("init_actions, bad rule");
  trigger = ARG(rule,0);
  if (!T.is_term(trigger, S.eq_sym(), 2))
    fatal::fatal_error("init_actions, bad trigger");
  stat = ARG(trigger,0);
  
  PlistContainer GIR,GENR, KR, LEVELR;
  GIR.set_head(PROVER9_GLOBAL_ACTIONS.Given_rules);
  GENR.set_head(PROVER9_GLOBAL_ACTIONS.Generated_rules);
  KR.set_head(PROVER9_GLOBAL_ACTIONS.Kept_rules);
  LEVELR.set_head(PROVER9_GLOBAL_ACTIONS.Level_rules);
  
  if (T.is_constant(stat, "given"))
    PROVER9_GLOBAL_ACTIONS.Given_rules = GIR.plist_append(rule);
  else if (T.is_constant(stat, "generated"))
    PROVER9_GLOBAL_ACTIONS.Generated_rules = GENR.plist_append(rule);
  else if (T.is_constant(stat, "kept"))
    PROVER9_GLOBAL_ACTIONS.Kept_rules = KR.plist_append(rule);
  else if (T.is_constant(stat, "level"))
    PROVER9_GLOBAL_ACTIONS.Level_rules = LEVELR.plist_append(rule);
  else
    fatal::fatal_error("init_action, bad statistic");
} 

void Actions::init_actions(Plist rules, void (*rebuild_sos_proc)(void), void (*exit_proc)(int), void (*assert_proc)(Topform)) {
  /* rule:          trigger -> action
     Example:     given=100 -> assign(max_weight, 25)
  */
  Plist p;
  for (p = rules; p; p = p->next)
    init_action((Term)p->v);

  /* Some changes to flags or parms require rebuilding the sos index. */
  Rebuild_sos_proc = rebuild_sos_proc; 
  Exit_proc = exit_proc;
  Assert_proc = assert_proc;
}  /* init_actions */


void Actions::register_action(string stat, string val, string op, string arg1, string arg2) {
  Term action, rule;
 
  if (myString::str_ident(op, "set") || myString::str_ident(op, "clear"))   action = T.term1(op, T.term0(arg1));
  else if (myString::str_ident(op, "assign"))    action = T.term2(op, T.term0(arg1), T.term0(arg2));
  else {
    fatal::fatal_error("register_action, operation must be set, clear, or assign");
    action = NULL;  /* to satisfy the compiler */
  }
  rule = T.term2("->",
			T.term2("=",
				T.term0(stat),
				T.term0(val)),
	       action);
  
  PlistContainer GIR,GENR, KR, LEVELR;
  GIR.set_head(PROVER9_GLOBAL_ACTIONS.Given_rules);
  GENR.set_head(PROVER9_GLOBAL_ACTIONS.Generated_rules);
  KR.set_head(PROVER9_GLOBAL_ACTIONS.Kept_rules);
  LEVELR.set_head(PROVER9_GLOBAL_ACTIONS.Level_rules);
  
  if (myString::str_ident(stat, "given"))
    PROVER9_GLOBAL_ACTIONS.Given_rules = GIR.plist_append(rule);
  else if (myString::str_ident(stat, "generated"))
    PROVER9_GLOBAL_ACTIONS.Generated_rules = GENR.plist_append(rule);
  else if (myString::str_ident(stat, "kept"))
    PROVER9_GLOBAL_ACTIONS.Kept_rules = KR.plist_append(rule);
  else if (myString::str_ident(stat, "level"))
    PROVER9_GLOBAL_ACTIONS.Level_rules = LEVELR.plist_append(rule);
  else
    fatal::fatal_error("register_action, bad statistic");
  cout<<endl<<"%% action: ";
  P.fwrite_term_nl(cout, rule);
}  




bool Actions::apply_action(Term action) {
 
  bool rebuild = false;
  if (T.is_constant(action, "exit"))  (*Exit_proc)((int) exitCodes::ACTION_EXIT);
  else if (T.is_term(action, "set", 1) || T.is_term(action, "clear", 1)) {
    if (changable_flag(action))
      LADR_GLOBAL_TOP_INPUT.flag_handler(cout, action, true, (int)unknown_actions::KILL_UNKNOWN);
    else if (changable_flag_rebuild(action)) {
      LADR_GLOBAL_TOP_INPUT.flag_handler(cout, action, true, (int)unknown_actions::KILL_UNKNOWN);
      rebuild = true;
    }
    else
      fatal::fatal_error("apply_action, flag is not changable");
  }
  else if (T.is_term(action, "assign", 2)) {
    if (changable_parm(action))
      LADR_GLOBAL_TOP_INPUT.parm_handler(cout, action, true, (int)unknown_actions::KILL_UNKNOWN);
    else if (changable_parm_rebuild(action)) {
      LADR_GLOBAL_TOP_INPUT.parm_handler(cout, action, true, (int)unknown_actions::KILL_UNKNOWN);
      rebuild = true;
    }
    else
      fatal::fatal_error("apply_action, parm is not changable");
  }
  else if (T.is_term(action, "assert", 1)) {
   
	Topform c = TF.term_to_clause(ARG(action,0));
    TF.clause_set_variables(c, MAX_VARS);
    c->justification = J.input_just();
    TF.append_label_attribute(c, "asserted_by_action");
    cout<<endl<<"Asserting clause: ";
	Ioutil::f_clause(c);
	(*Assert_proc)(c);
  }
  else {
    ParseContainer P;  
    fatal::fatal_error("apply_action, unknown action:");
    P.fwrite_term_nl(cout, action);
  }
  return rebuild;
} 


void Actions::statistic_actions(string stat, int n) {
  Plist p = NULL;
  bool rebuild = false;
  
  if (myString::str_ident(stat, "given"))  p = PROVER9_GLOBAL_ACTIONS.Given_rules;
  else if (myString::str_ident(stat, "generated"))   p = PROVER9_GLOBAL_ACTIONS.Generated_rules;
  else if (myString::str_ident(stat, "kept"))   p = PROVER9_GLOBAL_ACTIONS.Kept_rules;
  else if (myString::str_ident(stat, "level"))  p = PROVER9_GLOBAL_ACTIONS.Level_rules;
  else
    fatal::fatal_error("statistic_actions, unknown stat");
  
  for (; p; p = p->next) {
    Term rule = (Term)p->v;
    Term trigger = ARG(rule,0);
    Term tval = ARG(trigger,1);
    Term action = ARG(rule,1);
    int ival;
    if (!T.term_to_int(tval, &ival))
      fatal::fatal_error("statistic_actions, bad integer in trigger");
    if (n == ival) {
      cout<<endl<<"%% Applying rule: ";
      P.fwrite_term_nl(cout, rule);
      if (apply_action(action))	rebuild = true;
    }
  }
  if (rebuild)
    (*Rebuild_sos_proc)();
} 

void Actions::proof_action(Topform c, int attribute_id) {
  
  
  
  bool rebuild = false;
  int i = 0;
  Term t = A.get_term_attribute(c->attributes, attribute_id, ++i);
  while (t) {
    if (T.is_term(t, "->", 2) &&
	T.is_constant(ARG(t,0), "in_proof")) {
      Term action = ARG(t, 1);
      cout<<endl<<"%% Proof action: ";
	  P.fwrite_term_nl(cout, action);
      if (apply_action(action))
	rebuild = true;
    }
    t = A.get_term_attribute(c->attributes, attribute_id, ++i);
  }
  if (rebuild)
    (*Rebuild_sos_proc)();
}  /* proof_action */


void Actions::actions_in_proof(Plist proof, Prover_attributes att) {
  Plist p;
  for (p = proof; p; p = p->next) {
    proof_action((Topform)p->v, att->action);
    proof_action((Topform)p->v, att->action2);
  }
}
