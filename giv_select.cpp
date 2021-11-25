#include "./ladr/ladrvglobais.h"
#include "./ladr/fatal.h"
#include "./ladr/avltree.h"
#include "./ladr/clause_eval.h"
#include "./ladr/memory.h"
#include "./ladr/options.h"
#include "prover9vglobais.h"
#include "giv_select.h"
#include "semantics.h"

#include <iomanip>


void GlobalGivSelect::Free_Mem(void) {
    zap_given_selectors();
}



Giv_select GivSelect::get_giv_select(void) {
  Giv_select p = (Giv_select) Memory::memCNew(sizeof(struct giv_select));
  p->name=new string();
  PROVER9_GLOBAL_GIV_SELECT.Giv_select_gets++;
  return(p);
}					


void GivSelect::free_giv_select(Giv_select p) {
  if (p->name) delete p->name;
  Memory::memFree((void *) p, sizeof(struct giv_select));
  PROVER9_GLOBAL_GIV_SELECT.Giv_select_frees++;
}

int GivSelect::current_cycle_size(Select_state s) {
 
  int sum = 0;
  Plist p;
  for (p = s->selectors; p; p = p->next) {
    Giv_select gs =(Giv_select)p->v;
    if (AVL.avl_size(gs->idx) > 0)
      sum += gs->part;
  }
  return sum;
}



void GivSelect::init_giv_select(Plist rules) {
  Plist p;
 
  for (p = rules; p; p = p->next) {
    Term t = (Term) p->v;
    int n = 0;
    Term order_term;
    Term property_term;
    Giv_select gs;
    if (
        !T.is_term(t, "=", 2) ||
        !T.is_term(ARG(t,0), "part", 4) ||
        !CONSTANT(ARG(ARG(t,0),0)) ||
        !(T.is_constant(ARG(ARG(t,0),1), "high") || 
        T.is_constant(ARG(ARG(t,0),1), "low")) || 
        !((n = T.natural_constant_term(ARG(t,1))) > 0)
    )
    fatal::fatal_error("Given selection rule must be: part(<name>,high|low,age|wt|random,<property>)=<n>");

    order_term = ARG(ARG(t,0),2);
    property_term = ARG(ARG(t,0),3);
    gs = get_giv_select();
   
    if (T.is_constant(ARG(ARG(t,0),1), "high")) {
      P.set_head(PROVER9_GLOBAL_GIV_SELECT.High.selectors);
	  PROVER9_GLOBAL_GIV_SELECT.High.selectors = P.plist_append(gs);
      PROVER9_GLOBAL_GIV_SELECT.High.cycle_size += n;
    }
    else {
      P.set_head(PROVER9_GLOBAL_GIV_SELECT.Low.selectors);
	  PROVER9_GLOBAL_GIV_SELECT.Low.selectors  = P.plist_append(gs);
      PROVER9_GLOBAL_GIV_SELECT.Low.cycle_size += n;
    }

    *gs->name = T.term_symbol(ARG(ARG(t,0),0));
    gs->part = n;
    if (T.is_constant(order_term,"weight")) {
      gs->order =(int) GsOrder::GS_ORDER_WEIGHT;
      gs->compare = (OrderType (*) (void *, void *)) TopformContainer::cl_wt_id_compare;
    }
    else if (T.is_constant(order_term,"age")) {
      gs->order = (int) GsOrder::GS_ORDER_AGE;
      gs->compare = (OrderType (*) (void *, void *)) TopformContainer::cl_id_compare;
    }
    else if (T.is_constant(order_term,"random")) {
      gs->order =(int) GsOrder::GS_ORDER_RANDOM;
      gs->compare = (OrderType (*) (void *, void *)) TopformContainer::cl_id_compare;
    }
    else if (T.is_constant(order_term, "hint_age")) {
      gs->order = (int) GsOrder::GS_ORDER_HINT_AGE;
      gs->compare = (OrderType (*) (void *, void *)) TopformContainer::cl_hint_id_compare;
    }
    else
      fatal::fatal_error("Given selection order must be weight, age, or random.");
    
	gs->property =CEVAL.compile_clause_eval_rule(property_term);
    if (gs->property == NULL)
      fatal::fatal_error("Error in clause-property expression of given selection rule");
    else if (CEVAL.rule_contains_semantics(gs->property))
      PROVER9_GLOBAL_GIV_SELECT.Rule_needs_semantics = true;
  }
  PROVER9_GLOBAL_GIV_SELECT.High.current = PROVER9_GLOBAL_GIV_SELECT.High.selectors;
  PROVER9_GLOBAL_GIV_SELECT.Low.current = PROVER9_GLOBAL_GIV_SELECT.Low.selectors;
} 



void GivSelect::update_selectors(Topform c, bool insert) {
 
  bool matched = false;
  Plist p;
  for (p = PROVER9_GLOBAL_GIV_SELECT.High.selectors; p; p = p->next) {
    Giv_select gs =(Giv_select) p->v;
    if (CEVAL.eval_clause_in_rule(c, gs->property)) {
      matched = true;
      if (insert) {
		gs->idx = AVL.avl_insert(gs->idx, c, gs->compare);
		PROVER9_GLOBAL_GIV_SELECT.High.occurrences++;
      }
      else {
		gs->idx = AVL.avl_delete(gs->idx, c, gs->compare);
		PROVER9_GLOBAL_GIV_SELECT.High.occurrences--;
      }
    }
  }
  /* If it is high-priority, don't let it also be low priority. */
  if (!matched) {
    for (p = PROVER9_GLOBAL_GIV_SELECT.Low.selectors; p; p = p->next) {
      Giv_select gs = (Giv_select) p->v;
      if (CEVAL.eval_clause_in_rule(c, gs->property)) {
		matched = true;
	    if (insert) {
		 gs->idx = AVL.avl_insert(gs->idx, c, gs->compare);
		 PROVER9_GLOBAL_GIV_SELECT.Low.occurrences++;
	    }
	    else {
	     gs->idx = AVL.avl_delete(gs->idx, c, gs->compare);
	     PROVER9_GLOBAL_GIV_SELECT.Low.occurrences--;
	   }
      }
    }
  }
  if (!matched) {
    static bool Already_warned = false;

    if (!Already_warned) {
      cerr<<endl<<endl<<"WARNING: one or more kept clauses do not match any given_selection rules (see output)."<<endl<<endl;
	  cout<<endl<<"WARNING: the following clause does not match any given_selection rules."<<endl<<"This message will not be repeated."<<endl;
	  Ioutil::f_clause(c);
      Already_warned = true;
    }
  }
} 


void GivSelect::insert_into_sos2(Topform c, Clist sos) {
  
  if (PROVER9_GLOBAL_GIV_SELECT.Rule_needs_semantics)
    Semantics::set_semantics(c);  /* in case not yet evaluated */

  update_selectors(c, true);
  CL.clist_append(c, sos);
  PROVER9_GLOBAL_GIV_SELECT.Sos_size++;
} 


void GivSelect::remove_from_sos2(Topform c, Clist sos) {
  update_selectors(c, false);
  CL.clist_remove(c, sos);
  PROVER9_GLOBAL_GIV_SELECT.Sos_size--;
}


Giv_select GivSelect::next_selector(Select_state s) {
  if (s->selectors == NULL) return NULL;
  else {
    Plist start = s->current;
    Giv_select gs = (Giv_select) s->current->v;
    
    while (gs->idx == NULL || s->count >= gs->part) {
      s->current = s->current->next;
      if (!s->current)	s->current = s->selectors;
      gs = (Giv_select) s->current->v;
      s->count = 0;
      if (s->current == start)	break;  /* we're back to the start */
    }
    if (gs->idx == NULL) return NULL;
    else {
      s->count++;  /* for next call */
      return gs;
    }
  }
} 


bool GivSelect::givens_available(void){
  return (PROVER9_GLOBAL_GIV_SELECT.High.occurrences > 0 || PROVER9_GLOBAL_GIV_SELECT.Low.occurrences > 0);
}


Topform GivSelect::get_given_clause2(Clist sos, int num_given, Prover_options opt, string &type) {
 
  Topform giv;
  Giv_select gs = next_selector(&PROVER9_GLOBAL_GIV_SELECT.High);
  if (gs == NULL)    gs = next_selector(&PROVER9_GLOBAL_GIV_SELECT.Low);
  if (gs == NULL)    return NULL;  /* no clauses are available */
    
  if (gs->order == (int) GsOrder::GS_ORDER_RANDOM) {
    int n = AVL.avl_size(gs->idx);
    int i = (rand() % n) + 1;
    giv = (Topform) AVL.avl_nth_item(gs->idx, i);
  }
  else
    giv =(Topform) AVL.avl_smallest(gs->idx);

  type = *(gs->name);
  gs->selected += 1;
  remove_from_sos2(giv, sos);
  return giv;
} 

double GivSelect::iterations_to_selection(int part, int n,int cycle_size, int occurrences, int sos_size){
  /* This approximates the number of iterations (of given selection) until
     the n-th clause in the selector is selected.  Simplyfying assumptions:
       1. High-priority selectors are empty.
       2. Other selectors don't become empty.
       3. No clauses are inserted before the n-th clause.  (unrealistic)
   */
  double x = n * ((double) cycle_size / part);
  return x / ((double) occurrences / sos_size);
} 


double GivSelect::least_iters_to_selection(Topform c, Select_state s, Plist ignore){
  Plist p;
  
  double least = INT_MAX;  /* where is DOUBLE_MAX?? */
  for (p = s->selectors; p; p = p->next) {
    if (p != ignore) {
      Giv_select gs =(Giv_select) p->v;
      if (PROVER9_GLOBAL_GIV_SELECT.Rule_needs_semantics)
		Semantics::set_semantics(c);  /* in case not yet evaluated */

      if (CEVAL.eval_clause_in_rule(c, gs->property)) {
		int n, cycle;
		double x;
		if (gs->order == (int) GsOrder::GS_ORDER_AGE && c->id == INT_MAX)  n = AVL.avl_size(gs->idx) + 1;
		else  n = AVL.avl_place(gs->idx, c, gs->compare);
		cycle = current_cycle_size(s);
		x = iterations_to_selection(gs->part, n, cycle,s->occurrences, PROVER9_GLOBAL_GIV_SELECT.Sos_size);
		if (PROVER9_GLOBAL_GIV_SELECT.Debug)
			cout<<*gs->name<<"("<<c->weight<<")"<<",cycle="<<cycle<<",part="<<gs->part<<",place="<<n<<",size="<<AVL.avl_size(gs->idx)<<",iters"<<x;
		least = (x < least ? x : least);
      }
    }
  }
  return least;
} 



bool GivSelect::sos_keep2(Topform c, Clist sos, Prover_options opt){
  
  int keep_factor = LADR_GLOBAL_OPTIONS.parm(opt->sos_keep_factor);
  int sos_size = CL.clist_length(sos);
  int sos_limit = (LADR_GLOBAL_OPTIONS.parm(opt->sos_limit)== -1 ? INT_MAX : LADR_GLOBAL_OPTIONS.parm(opt->sos_limit));
  bool keep;
  if (sos_size < sos_limit / keep_factor)
    keep = true;
  else {
    int iters;
    c->id = INT_MAX;
    iters = least_iters_to_selection(c, &PROVER9_GLOBAL_GIV_SELECT.Low, NULL);
    if (PROVER9_GLOBAL_GIV_SELECT.Debug)
       cout<<"iters="<<iters<<", wt="<<c->weight<<endl; 
      
    if (iters < sos_limit / keep_factor)
      keep = true;
    else {
      if (c->weight < PROVER9_GLOBAL_GIV_SELECT.Low_water_keep) {
        PROVER9_GLOBAL_GIV_SELECT.Low_water_keep = c->weight;
        cout<<endl<<"Low water (kepp): wt="<<c->weight<<", iters="<<iters;
        if ( LADR_GLOBAL_OPTIONS.stringparm(opt->stats, "all") )  selector_report();
        fflush(stdout);
      }
      PROVER9_GLOBAL_GIV_SELECT.Sos_deleted++;
      keep = false;  /* delete clause */
    }
    c->id = 0;
  }
  return keep;
}



Topform GivSelect::worst_clause_of_priority_group(Select_state ss) {
  
  Topform worst = NULL; /* worst clause (with most iterations_to_selection)  */
  double max = 0.0;     /* iterations_to_selection for current worst clause  */
  Plist p;
  for (p = ss->selectors; p; p = p->next) {
    Giv_select gs = (Giv_select) p->v;
    if (gs->idx) {
      Topform c = (Topform) AVL.avl_largest(gs->idx);
      double x = iterations_to_selection(gs->part, AVL.avl_size(gs->idx),
					 current_cycle_size(ss),
					 ss->occurrences,
					 PROVER9_GLOBAL_GIV_SELECT.Sos_size);

      /* If that clause occurs in other selectors,
         find the lowest iterations_to_selection. */

      double y = least_iters_to_selection(c, ss, p);  /* ignore p */

      double least = (x < y ? x : y);

      if (least > max) {
		max = least;
		worst = c;
      }
    }
  }
  return worst;
}


Topform GivSelect::worst_clause(void){
  Topform worst = worst_clause_of_priority_group(&PROVER9_GLOBAL_GIV_SELECT.Low);
  if (worst == NULL) {
    worst = worst_clause_of_priority_group(&PROVER9_GLOBAL_GIV_SELECT.High);
    if (worst) cout << endl << "WARNING: worst clause (id="<<worst->id<<" , wt="<<worst->weight<<") has high priority."<<endl;
  }
  return worst;
}



void GivSelect::sos_displace2(void (*disable_proc) (Topform)) {
  Topform worst = worst_clause();
  if (worst == NULL) {
    selector_report();
    fatal::fatal_error("sos_displace2, cannot find worst clause");
  }
  else {
    if (worst->weight < PROVER9_GLOBAL_GIV_SELECT.Low_water_displace) {
      PROVER9_GLOBAL_GIV_SELECT.Low_water_displace = worst->weight;
      cout<<endl<<"Low Water (displace): id="<<worst->id<<" , wt="<<worst->weight<<endl;
    }
    PROVER9_GLOBAL_GIV_SELECT.Sos_displaced++;
    disable_proc(worst);
  }
} 



void GivSelect::selector_report(void) {
  Plist p;
  
  banner::print_separator(cout, "SELECTOR REPORT", true);
  
  cout<<"Sos_deleted="<<PROVER9_GLOBAL_GIV_SELECT.Sos_deleted<<", Sos_displaced="<<PROVER9_GLOBAL_GIV_SELECT.Sos_displaced<<", Sos_size="<<PROVER9_GLOBAL_GIV_SELECT.Sos_size<<endl;
  
  cout<<setw(14)<<"SELECTOR"<<setw(14)<<"PART"<<setw(14)<<"PRIORITY"<<setw(10)<<"ORDER"<<setw(12)<<"SIZE"<<setw(14)<<"SELECTED"<<endl;
  
  for (p = PROVER9_GLOBAL_GIV_SELECT.High.selectors; p; p = p->next) {
    Giv_select gs = Giv_select (p->v);
    string s1, s2;
	s1 = "high";
    switch (gs->order) {
    case (int) GsOrder::GS_ORDER_WEIGHT:    s2 = "weight";  break;
    case (int) GsOrder::GS_ORDER_AGE:       s2 = "age";     break;
    case (int) GsOrder::GS_ORDER_RANDOM:    s2 = "random";  break;
    case (int) GsOrder::GS_ORDER_HINT_AGE:  s2 = "hint_age";break;
    default: s2 = "???"; break;
    }
    cout<<setw(14)<<*gs->name<<setw(14)<<gs->part<<setw(14)<<s1<<setw(10)<<s2<<setw(12)<<AVL.avl_size(gs->idx)<<setw(14)<<gs->selected<<endl;
  }	
  
  for (p = PROVER9_GLOBAL_GIV_SELECT.Low.selectors; p; p = p->next) {
    Giv_select gs = (Giv_select) p->v;
    string s1, s2;
    s1 = "low";
    switch (gs->order) {
    case (int) GsOrder::GS_ORDER_WEIGHT:    s2 = "weight"; break;
    case (int) GsOrder::GS_ORDER_AGE:       s2 = "age"; break;
    case (int) GsOrder::GS_ORDER_RANDOM:    s2 = "random"; break;
    case (int) GsOrder::GS_ORDER_HINT_AGE:  s2 = "hint_age";break;             
    default: s2 = "???"; break;
    }
    cout<<setw(14)<<*gs->name<<setw(14)<<gs->part<<setw(14)<<s1<<setw(10)<<s2<<setw(12)<<AVL.avl_size(gs->idx)<<setw(14)<<gs->selected<<endl;
  }
  banner::print_separator(cout, "end of selector report", false);  
  cout<<endl;
} 



void GlobalGivSelect::zap_given_selectors(void) {
  ClauseEvalContainer CEVAL;
  AvltreeContainer AVL;
  GivSelect GS;
  PlistContainer P;
  Plist p;
  for (p = PROVER9_GLOBAL_GIV_SELECT.High.selectors; p; p = p->next) {
    Giv_select gs = (Giv_select) p->v;
    CEVAL.zap_clause_eval_rule(gs->property);
    AVL.avl_zap(gs->idx);
    GS.free_giv_select(gs);
  }
  P.set_head(PROVER9_GLOBAL_GIV_SELECT.High.selectors);
  P.zap_plist();  /* shallow */
  for (p = PROVER9_GLOBAL_GIV_SELECT.Low.selectors; p; p = p->next) {
    Giv_select gs =(Giv_select) p->v;
    CEVAL.zap_clause_eval_rule(gs->property);
    AVL.avl_zap(gs->idx);
    if(gs->name) delete (gs->name);
  }
  P.set_head(PROVER9_GLOBAL_GIV_SELECT.Low.selectors);
  P.zap_plist();  /* shallow */
  
  //Limpar as strings com o nome destes estáticos

} 

Term GivSelect::selector_rule_term(const string &name, const string & priority,	const string &order, const string &rule, int part) {
  
  Term left =  T.get_rigid_term("part", 4);
  Term right = T.nat_to_term(part);
  ARG(left,0) = T.get_rigid_term(name, 0);
  ARG(left,1) = T.get_rigid_term(priority, 0);
  ARG(left,2) = T.get_rigid_term(order, 0);
  ARG(left,3) = T.get_rigid_term(rule, 0);
  return T.build_binary_term_safe("=", left, right);
} 


Plist GivSelect::selector_rules_from_options(Prover_options opt) {
  
  Plist p = NULL;

  if (LADR_GLOBAL_OPTIONS.flag(opt->input_sos_first)) {
    p = P.plist_append(selector_rule_term("I", "high", "age", "initial", INT_MAX));
  }

  if (LADR_GLOBAL_OPTIONS.parm(opt->hints_part) == INT_MAX) {
    p = P.plist_append(selector_rule_term("H", "high", "weight", "hint", 1));
  }
  else if (LADR_GLOBAL_OPTIONS.parm(opt->hints_part) > 0) {
    p = P.plist_append(selector_rule_term("H", "low", "weight","hint", LADR_GLOBAL_OPTIONS.parm(opt->hints_part)));
  }

  if (LADR_GLOBAL_OPTIONS.parm(opt->age_part) > 0) {
    p = P.plist_append(selector_rule_term("A", "low", "age", "all", LADR_GLOBAL_OPTIONS.parm(opt->age_part)));
  }
  if (LADR_GLOBAL_OPTIONS.parm(opt->false_part) > 0) {
    p = P.plist_append(selector_rule_term("F", "low", "weight", "false", LADR_GLOBAL_OPTIONS.parm(opt->false_part)));
  }
  if (LADR_GLOBAL_OPTIONS.parm(opt->true_part) > 0) {
    p = P.plist_append(selector_rule_term("T", "low", "weight", "true", LADR_GLOBAL_OPTIONS.parm(opt->true_part)));
  }
  if (LADR_GLOBAL_OPTIONS.parm(opt->weight_part) > 0) {
    p = P.plist_append(selector_rule_term("W", "low", "weight","all", LADR_GLOBAL_OPTIONS.parm(opt->weight_part)));
  }
  if (LADR_GLOBAL_OPTIONS.parm(opt->random_part) > 0) {
    p = P.plist_append(selector_rule_term("R", "low", "random",  "all", LADR_GLOBAL_OPTIONS.parm(opt->random_part)));
  }

  return p;
}  /* selector_rules_from_options */

