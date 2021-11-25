
#include "search.h"
#include "./ladr/memory.h"
#include "./ladr/options.h"
#include "./ladr/nonport.h"
#include "./ladr/clock.h"
#include "./ladr/options.h"
#include "search_structures.h"
#include "./ladr/attrib.h"
#include "./ladr/mystring.h"
#include "./ladr/compress.h"
#include "./ladr/demod.h"
#include "./ladr/flatdemod.h"
#include "./ladr/di_tree.h"
#include "./ladr/dollar.h"
#include "./ladr/ac_redun.h"
#include "./ladr/clauseid.h"
#include "./ladr/weight.h"
#include "./ladr/tlist.h"
#include "./ladr/ladrvglobais.h"
#include "./prover9vglobais.h"


#include "demodulate.h"

#include "./ladr/hints.h"

#include "white_black.h"
#include "forward_subsume.h"
#include "utilities.h"
#include "pred_elim.h"
#include "semantics.h"
#include "unfold.h"

#include <tgmath.h>
#include <iomanip>



static struct {
  // basic clause lists
  Clist sos;
  Clist usable;
  Clist demods;
  Clist hints;

  // other lists

  Plist actions;
  Plist weights;
  Plist kbo_weights;
  Plist interps;
  Plist given_selection;
  Plist keep_rules;
  Plist delete_rules;

  // auxiliary clause lists

  Clist limbo;
  Clist disabled;
  Plist empties;

  // indexing

//  Lindex clashable_idx;  // literal index for resolution rules
  bool use_clash_idx;    // GET RID OF THIS VARIABLE!!

  // basic properties of usable+sos

  bool horn, unit, equality; 
  int number_of_clauses, number_of_neg_clauses;

  // other stuff

  Plist desc_to_be_disabled;   // Descendents of these to be disabled
  Plist cac_clauses;           // Clauses that trigger back CAC check

  bool searching;      // set to true when first given is selected
  bool initialized;    // has this structure been initialized?
  double start_time;   // when was it initialized? 
  int start_ticks;     // quasi-clock that times the same for all machines

  int return_code;     // result of search
} Glob;


Prover_input Search::input;
Prover_results Search::results;    



GivSelect Search::GS;
JustContainer Search::J;
UnifyContainer Search::U;
myClock Search::CLOCK;



void GlobalSearch::Free_Mem(void) {
   myClock CLOCK;
   CLOCK.set_ClockStructure(Clocks.back_demod);
   CLOCK.zap_clock();
    
   CLOCK.set_ClockStructure(Clocks.back_subsume);
   CLOCK.zap_clock();
   
   
   CLOCK.set_ClockStructure(Clocks.back_unit_del);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.conflict);
   CLOCK.zap_clock();
   
   
   CLOCK.set_ClockStructure(Clocks.demod);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.disable);
   CLOCK.zap_clock();
   
   
   CLOCK.set_ClockStructure(Clocks.hints);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.index);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.infer);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.pick_given);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.preprocess);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.redundancy);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.semantics);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.subsume);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.unit_del);
   CLOCK.zap_clock();
   
   CLOCK.set_ClockStructure(Clocks.weigh);
   CLOCK.zap_clock();
   
 
}


int Search::degradation_count(Topform c) {
   return (int)(c -> weight) / 1000;
}



string Search::exit_string(int code) {
  string message;
  switch (exitCodes(code)) {
        case exitCodes::MAX_PROOFS_EXIT:  message = "max_proofs";  break;
        case exitCodes::FATAL_EXIT:       message = "fatal_error"; break;
        case exitCodes::SOS_EMPTY_EXIT:   message = "sos_empty";   break;
        case exitCodes::MAX_MEGS_EXIT:    message = "max_megs";    break;
        case exitCodes::MAX_SECONDS_EXIT: message = "max_seconds"; break;
        case exitCodes::MAX_GIVEN_EXIT:   message = "max_given";   break;
        case exitCodes::MAX_KEPT_EXIT:    message = "max_kept";    break;
        case exitCodes::ACTION_EXIT:      message = "action";      break;
        case exitCodes::MAX_NOHINTS_EXIT: message = "max_nohints"; break;
        case exitCodes::SIGSEGV_EXIT:     message = "SIGSEGV";     break;
        case exitCodes::SIGINT_EXIT:      message = "SIGINT";      break;
        default: message = "???";
  }
  return message;
} 


void Search::report(ostream &o, string level) {
 
    double seconds = myClock::user_seconds();

    if(&o!=&cerr) 
        cout<<endl<<"Note: Report at "<< seconds <<" , "<<myClock::get_date()<<endl;  
  
  if (myString::str_ident(level, ""))
    level = (PROVER9_GLOBAL_SEARCH.Opt ? LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->stats) : "lots");

  fprint_all_stats(o, level);
  o<<endl;
  cerr<<endl;
    
}



void Search::fatal_setjmp(void) {
  int return_code = setjmp(PROVER9_GLOBAL_SEARCH.Jump_env);
  if (return_code != 0)
    fatal::fatal_error("longjmp called outside of search");
} 

void Search::update_stats(void) {
  Di_treeContainer DI;
  PROVER9_GLOBAL_SEARCH.Stats.demod_attempts = Demod::demod_attempts() + Flatdemod::fdemod_attempts();
  PROVER9_GLOBAL_SEARCH.Stats.demod_rewrites = Demod::demod_rewrites() + Flatdemod::fdemod_rewrites();
  PROVER9_GLOBAL_SEARCH.Stats.res_instance_prunes = Resolve::res_instance_prunes();
  PROVER9_GLOBAL_SEARCH.Stats.para_instance_prunes = Paramodulation::para_instance_prunes();
  PROVER9_GLOBAL_SEARCH.Stats.basic_para_prunes = Paramodulation::basic_paramodulation_prunes();
  PROVER9_GLOBAL_SEARCH.Stats.sos_removed = 0; // control_sos_removed();
  PROVER9_GLOBAL_SEARCH.Stats.nonunit_fsub =         
  DI.nonunit_fsub_tests();
  PROVER9_GLOBAL_SEARCH.Stats.nonunit_bsub = DI.nonunit_bsub_tests();
  PROVER9_GLOBAL_SEARCH.Stats.usable_size = Glob.usable ? Glob.usable->length : 0;
  PROVER9_GLOBAL_SEARCH.Stats.sos_size = Glob.sos ? Glob.sos->length : 0;
  PROVER9_GLOBAL_SEARCH.Stats.demodulators_size = Glob.demods ? Glob.demods->length : 0;
  PROVER9_GLOBAL_SEARCH.Stats.limbo_size = Glob.limbo ? Glob.limbo->length : 0;
  PROVER9_GLOBAL_SEARCH.Stats.disabled_size = Glob.disabled ? Glob.disabled->length : 0;
  PROVER9_GLOBAL_SEARCH.Stats.hints_size = Glob.hints ? Glob.hints->length : 0;
  PROVER9_GLOBAL_SEARCH.Stats.kbyte_usage = Memory::bytes_palloced() / 1000;
}  /* update_stats */


Prover_results Search::collect_prover_results(bool xproofs) {
  Plist p;
  //JustContainer J;
  PlistContainer PROOFS, XPROOFS;
  Prover_results results = (Prover_results) calloc(1, sizeof(struct prover_results));
  PROOFS.set_head(results->proofs);
  XPROOFS.set_head(results->xproofs);
  for (p = Glob.empties; p; p = p->next) {
    Plist proof = J.get_clause_ancestors((Topform)p->v);
    Compress::uncompress_clauses(proof);
    results->proofs = PROOFS.plist_append(proof);
    if (xproofs) {
      Plist xproof = Xproofs::proof_to_xproof(proof);
      results->xproofs = XPROOFS.plist_append(xproof);
    }
  }
 
  update_stats();  /* puts package stats into Stats */
  results->stats = PROVER9_GLOBAL_SEARCH.Stats;  /* structure copy */
  results->user_seconds = myClock::user_seconds();
  results->system_seconds = myClock::system_seconds();
  results->return_code = (exitCodes) Glob.return_code;
  return results;
  
}  /* collect_prover_results */



void Search::rebuild_sos_index(void) {
  fatal::fatal_error("rebuild_sos_index not implemented for given_selection");
#if 0
  Clist_pos p;
  printf("\nNOTE: Reweighing all SOS clauses and rebuilding SOS indexes.\n");
  zap_picker_indexes();
  update_picker_ratios(Opt);  /* in case they've been changed */
  for (p = Glob.sos->first; p; p = p->next) {
    Topform c = p->c;
    clause_wt_with_adjustments(c); /* weigh the clause (wt stored in c) */
    update_pickers(c, TRUE); /* inserdemodulate_clauset (lower-level) into picker indexes */
#endif
} 



void Search::done_with_search(int return_code) {
  fprint_all_stats(cout, PROVER9_GLOBAL_SEARCH.Opt ? LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->stats) : "lots");
  /* If we need to return 0, we have to encode it as something else. */
  longjmp(PROVER9_GLOBAL_SEARCH.Jump_env, return_code == 0 ? INT_MAX : return_code);
}  


void Search::exit_if_over_limit(void) {
  /* max_megs is handled elsewhere */
  if (LADR_GLOBAL_OPTIONS.over_parm_limit(PROVER9_GLOBAL_SEARCH.Stats.kept, PROVER9_GLOBAL_SEARCH.Opt->max_kept))    done_with_search((int) exitCodes::MAX_KEPT_EXIT);
  else if (LADR_GLOBAL_OPTIONS.over_parm_limit(PROVER9_GLOBAL_SEARCH.Stats.given, PROVER9_GLOBAL_SEARCH.Opt->max_given))    done_with_search((int) exitCodes::MAX_GIVEN_EXIT);
  else if (LADR_GLOBAL_OPTIONS.at_parm_limit(myClock::user_seconds(), PROVER9_GLOBAL_SEARCH.Opt->max_seconds))  done_with_search((int) exitCodes::MAX_SECONDS_EXIT);
} 




void Search::possible_report(void) {
  Search S;
  static int Next_report, Next_report_stderr;
  int runtime;

  runtime = myClock::user_time() / 1000;

  if (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report) > 0) {
    if (Next_report == 0)
      Next_report = LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report);
    if (runtime >= Next_report) {
      S.report(cout, LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->stats));
      while (runtime >= Next_report)
	Next_report += LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report);
    }
  }

  if (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report_stderr) > 0) {
    if (Next_report_stderr == 0)
      Next_report_stderr = LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report_stderr);
    if (runtime >= Next_report_stderr) {
      S.report(cerr, "some");
      while (runtime >= Next_report_stderr)
	Next_report_stderr += LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report_stderr);
    }
  }
} 


// Check whether a clause is on the hitlist.
bool Search::on_hit_list(int cl_num) {
   static int next_cl_pos = 0;

   if (cl_num == PROVER9_GLOBAL_SEARCH.HIT_LIST[next_cl_pos])
   {
      next_cl_pos++ ;

      // skip over duplicates
      while(next_cl_pos < PROVER9_GLOBAL_SEARCH.hsize && PROVER9_GLOBAL_SEARCH.HIT_LIST[next_cl_pos] == PROVER9_GLOBAL_SEARCH.HIT_LIST[next_cl_pos-1])
      {
         cout <<"*** duplicate ignored ***"<<endl;
         next_cl_pos++ ;
      }
      return true;
   }

   else return false;
}

void Search::hint_derivation(Topform cl) {
 
   Plist proof, p;
   static int pfcount = 0;
   //JustContainer J;
 
   proof = J.get_clause_ancestors(cl);
   Compress::uncompress_clauses(proof);
 
   pfcount++ ;
 
   banner::print_separator(cout, "PROOF", true);
   cout << endl <<"% Hint derivation (Proof) " << pfcount<<":";
   Ioutil::fwrite_clause(cout, cl, (int) Clause_print_format::CL_FORM_BARE);
   cout<<endl<<"% Length of derivation is "<<J.proof_length(proof)<<"."<<endl<<endl;
   for (p = proof; p; p = p->next)   {
      /* printf("<SD> "); */     /* selected derivation */
      Ioutil::fwrite_clause(cout, (Topform) p->v, (int) Clause_print_format::CL_FORM_STD);
   }
   banner::print_separator(cout, "end of proof", true);
}


void Search::cl_process_keep(Topform c) {
  TopformContainer TF;
  ClauseidContainer CI;
  //JustContainer J;
  AttributeContainer A;
  Actions Ac;
  PROVER9_GLOBAL_SEARCH.Stats.kept++;
  if (!c->normal_vars) TF.renumber_variables(c, MAX_VARS);
  if (c->id == 0)   CI.assign_clause_id(c);  // unit conflict or input: already has ID
  
  /* *************************************************************************
   BV(2016-feb-17): print derivations of selected clauses
   ************************************************************************* */
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_derivations) && on_hit_list(c->id))  print_derivation(c);
  
  /* *************************************************************************
   BV(2017-apr-03): print derivations of matchers of hints with 
   ID < hint_derivations
   ************************************************************************* */
   if ((c -> matching_hint != NULL)
         && (c -> matching_hint -> id < LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->hint_derivations)))  hint_derivation(c);
  
  
  J.mark_parents_as_used(c);
  Maximal::mark_maximal_literals(c->literals);
  Maximal::mark_selected_literals(c->literals, LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->literal_selection));
  if (c->matching_hint != NULL)    Hints::keep_hint_matcher(c);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_clause_properties))    c->attributes = A.set_term_attribute(c->attributes, PROVER9_GLOBAL_SEARCH.Att.properties, TF.topform_properties(c));
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_kept) || LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen) ||    (!Glob.searching && LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_initial_clauses))) {
    cout<< "kept:      ";
    Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
  }
  else if ( LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_labeled) &&  (A.get_string_attribute(c->attributes, PROVER9_GLOBAL_SEARCH.Att.label, 1)!=myString::null_string())) {
    
    cout<<endl<<"kept:      ";
    Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
  }
  Ac.statistic_actions("kept", CI.clause_ids_assigned());  /* Note different stat */
}


void Search::clause_wt_with_adjustments(Topform c) {
 //myClock CLOCK;
 ClistContainer CL;
 TopformContainer TF;
  

  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.weigh);
  c->weight = Weight::clause_weight(c->literals);
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.weigh);

  if (!CL.clist_empty(Glob.hints)) {
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.hints);
    if (!c->normal_vars) TF.renumber_variables(c, MAX_VARS);
    Hints::adjust_weight_with_hints(c,
			     LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->degrade_hints),
			     LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->breadth_first_hints));
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.hints);
  }

  if (c->weight > LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->default_weight) && 
      c->weight <= LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->max_weight))
    c->weight = LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->default_weight);
} 




bool Search::restricted_denial(Topform c) {
  /* At one time we also required all clauses to be Horn. */
  return   LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->restrict_denials) &&  LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals);
}  /* restricted_denial */


bool Search::skip_black_white_tests(Topform c) {
  return (!Glob.searching ||
	  c->used ||
	  restricted_denial(c) ||
	  (c->matching_hint  != NULL && !LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->limit_hint_matchers)));
}


bool Search::cl_process_delete(Topform c) {
  //myClock CLOCK;
  //GivSelect GS;
  IndexLits IL;
  
  
  // Should the clause be deleted (tautology, limits, subsumption)?

  if (LADRV_GLOBAIS_INST.Lit.true_clause(c->literals)) {  // tautology
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen))cout<<"tautology"<<endl;
    PROVER9_GLOBAL_SEARCH.Stats.subsumed++;
    return true;  // delete
  }

  clause_wt_with_adjustments(c);  // possibly sets c->matching_hint

  // White-black tests

  if (!skip_black_white_tests(c)) {
    if (WhiteBlack::white_tests(c)) {
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) cout<<"keep_rule applied."<<endl;
      PROVER9_GLOBAL_SEARCH.Stats.kept_by_rule++;
    }
    else {
      if (WhiteBlack::black_tests(c)) {
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) cout<<"delete_rule applied."<<endl;
        PROVER9_GLOBAL_SEARCH.Stats.deleted_by_rule++;
        return true;  //delete
      }
      else if (!GS.sos_keep2(c, Glob.sos, PROVER9_GLOBAL_SEARCH.Opt)) {
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) cout<<"sos_limit applied."<<endl;
        PROVER9_GLOBAL_SEARCH.Stats.sos_limit_deleted++;
        return true;  // delete
   }
  }
 }
      
  // Forward subsumption

  {
    Topform subsumer;
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.subsume);
    subsumer = IL.forward_subsumption(c);
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.subsume);
    if (subsumer != NULL && !c->used) {
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) cout<<"subsumed by "<<subsumer->id<<"."<<endl;
	  PROVER9_GLOBAL_SEARCH.Stats.subsumed++;
      return true;  // delete
    }
    else
      return false;  // keep the clause
  }
} 




void Search::handle_proof_and_maybe_exit(Topform empty_clause) {
  Term answers;
  Plist proof, p;
  ClausesContainer CL;
  PlistContainer P1,P2,Pl;
  ParseContainer P;
  ClauseidContainer CI;
  AttributeContainer A;
  Actions Ac;

  J.mark_parents_as_used(empty_clause);
  CI.assign_clause_id(empty_clause);
  proof = J.get_clause_ancestors(empty_clause);
  Compress::uncompress_clauses(proof);

  if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->reuse_denials) && Glob.horn) {
    Topform c = Utilities::first_negative_clause(proof);
    if (CL.clause_member_plist(Glob.desc_to_be_disabled, c)) {
      cout<<"%% Redundant proof: ";
      Ioutil::f_clause(empty_clause);
      return;
    }
    else {
      /* Descendants of c will be disabled when it is safe to do so. */
      P1.set_head(Glob.desc_to_be_disabled);
      Glob.desc_to_be_disabled = P1.plist_prepend(c);
    }
  }
  
  P2.set_head(Glob.empties);
  Glob.empties = P2.plist_append(empty_clause);
  PROVER9_GLOBAL_SEARCH.Stats.proofs++;

  answers = A.get_term_attributes(empty_clause->attributes, PROVER9_GLOBAL_SEARCH.Att.answer);

  /* message to stderr */

  if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
    cout<<endl;
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->bell))
      fatal::bell(cerr);
    
    cerr<<"-------- Proof "<<PROVER9_GLOBAL_SEARCH.Stats.proofs<<" -------- ";
    
    if (answers != NULL)
      P.fwrite_term_nl(cerr, answers);
    else cerr<<endl;
      
  }

  /* print proof to stdout */
  cerr<<endl;
  
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_proofs)) {
    banner::print_separator(cout, "PROOF", true);
    
    cout<<endl<<"% Proof "<< PROVER9_GLOBAL_SEARCH.Stats.proofs <<" at "<<myClock::user_seconds()<< "(+"<<myClock::system_seconds()<<") seconds";
    
    
    if (answers != NULL) {
      cout<<": ";
      P.fwrite_term(cout, answers);
    }
    cout<<"."<<endl;
    
    
    cout << "% Length of proof is "<<J.proof_length(proof)<<"."<<endl;
    cout << "% Level of proof is "<<J.clause_level(empty_clause)<<"."<<endl;
    cout << "% Maximum clause weight is "<<CL.max_clause_weight(proof)<<"."<<endl;
    cout << "% Given clauses "<<PROVER9_GLOBAL_SEARCH.Stats.given<<"."<<endl<<endl;
    
    for (p = proof; p; p = p->next) 
      Ioutil::fwrite_clause(cout, (Topform)p->v, (int) Clause_print_format::CL_FORM_STD);
      
    
    
    banner::print_separator(cout, "end of proof", true);
  }  /* print_proofs */
  else {
	 cout<<endl<<"-------- Proof "<<PROVER9_GLOBAL_SEARCH.Stats.proofs<<" at ("<<myClock::user_seconds()<<" + "<<myClock::system_seconds()<<" seconds)";
     if (answers != NULL)    P.fwrite_term_nl(cout, answers);
     else cout<<endl;
  }
  
  
  // BV(2015-jan-31): print hints matched by proof clauses
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_matched_hints))
  {
     int pmh_count = 0;
     Topform h;

     banner::print_separator(cout, "MATCHED HINTS", true);
     cout<<endl<<"formulas(hints)."<<" % Hints matched by proof clauses."<<endl;

     for (p = proof; p; p = p->next)
     {
        // BV(2016-apr-08): account for back demodulated hints
        h = ((Topform) p->v) -> matching_hint;
        if (h)
        {
           if (LADRV_GLOBAIS_INST.Lit.true_clause(h->literals))
              pmh_count++ ;
           else 
              Ioutil::fwrite_clause(cout, h , (int) Clause_print_format::CL_FORM_BARE);
        }
     }
     cout <<"%% *** Not including "<<pmh_count<<" hints that were back demodulated. ***"<<endl;
     cout<<"end_of_list."<<endl;
     banner::print_separator(cout, "end of matched hints", true);
  }
  
    // BV(2017-apr-19): print proof clauses that match a hint
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_matched_hints))
  {
     banner::print_separator(cout, "HINT MATCHERS", true);
     cout<<endl<<"formulas(hints).";
     cout<<"  %% Proof clauses that match a hint."<<endl;
     
     for (p = proof; p; p = p->next)
     {
        if (((Topform) p->v) -> matching_hint != NULL)
           Ioutil::fwrite_clause(cout, (Topform) p->v , (int) Clause_print_format::CL_FORM_BARE);
     }
     cout<<"end_of_list."<<endl;
     banner::print_separator(cout, "end of hint matchers", true);
  }


  // BV(2017-apr-19): print proof clauses that do not match any hints
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_matched_hints))
  {
     banner::print_separator(cout, "NON HINT MATCHERS", true);
     cout<<endl<<"formulas(hints).";
     cout<<"  % Proof clauses that do not match any hints."<<endl;
     for (p = proof; p; p = p->next)
     {
        if (((Topform) p->v) -> matching_hint == NULL)
           Ioutil::fwrite_clause(cout, (Topform) p->v ,(int) Clause_print_format::CL_FORM_BARE);
     }
     
     fprintf(stdout, "end_of_list.\n");
     banner::print_separator(cout, "end of non hint matchers", true);
  }
  
  cout<<endl;
  if (answers) {
      TermContainer T;
      T.zap_term(answers);
  }

  Ac.actions_in_proof(proof, &PROVER9_GLOBAL_SEARCH.Att);  /* this can exit */

  if (LADR_GLOBAL_OPTIONS.at_parm_limit(PROVER9_GLOBAL_SEARCH.Stats.proofs, PROVER9_GLOBAL_SEARCH.Opt->max_proofs)) {
      Search S;  
      S.done_with_search((int) exitCodes::MAX_PROOFS_EXIT);  /* does not return */
  }  
  Pl.set_head(proof);
  Pl.zap_plist();
} 


void Search::cl_process_conflict(Topform c, bool denial) {
   //myClock CLOCK;
   TopformContainer TF;
   IndexLits IL;
    
   if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) == 1) {
    if (!c->normal_vars)     TF.renumber_variables(c, MAX_VARS);
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.conflict);
    IL.unit_conflict(c, handle_proof_and_maybe_exit);
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.conflict);
  }
} 



void Search::cl_process_new_demod(Topform c) {
  // If the clause should be a demodulator, make it so.
  ClistContainer CL;
  Demodulate D;
  
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod)) {
    int type = Demod::demodulator_type(c,
				LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_lim),
				LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_sane));
    if (type != (int) Demod_Type::NOT_DEMODULATOR) {
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_kept)) {
	string s;
	switch(type) {
        case (int) Demod_Type::ORIENTED:     s = ""; break;
        case (int) Demod_Type::LEX_DEP_LR:   s = " (lex_dep_lr)"; break;
        case (int) Demod_Type::LEX_DEP_RL:   s = " (lex_dep_rl)"; break;
        case (int) Demod_Type::LEX_DEP_BOTH: s = " (lex_dep_both)"; break;
        default:           s = " (?)";
	}
	cout<< "    new demodulator"<<s<<": "<<c->id<<"."<<endl;
    }
      CL.clist_append(c, Glob.demods);
      D.index_demodulator(c, type, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index);
      PROVER9_GLOBAL_SEARCH.Stats.new_demodulators++;
      if (type != (int) Demod_Type::ORIENTED)  PROVER9_GLOBAL_SEARCH.Stats.new_lex_demods++;
      Hints::back_demod_hints(c, type, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_order_vars));
    }
  }
}

void Search::cl_process(Topform c) {
  // If the infer_clock is running, stop it and restart it when done.
  //myClock CLOCK;
  Actions Ac;
  IndexLits IL;
  ClistContainer CL;
  bool infer_clock_stopped = false;
  if (CLOCK.clock_running(PROVER9_GLOBAL_SEARCH.Clocks.infer)) {
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.infer);
    infer_clock_stopped = true;
  }
  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.preprocess);

  exit_if_over_limit();
  if (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report) > 0 || LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report_stderr) > 0)   possible_report();

  PROVER9_GLOBAL_SEARCH.Stats.generated++;
  Ac.statistic_actions("generated", PROVER9_GLOBAL_SEARCH.Stats.generated);
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) {
    printf("\ngenerated: ");
    cout<<endl<<"generated: ";
    Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
  }

  cl_process_simplify(c);            // all simplification

  if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) == 0)    // empty clause
    handle_proof_and_maybe_exit(c);
  else {
    // Do safe unit conflict before any deletion checks.
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->safe_unit_conflict))
      cl_process_conflict(c, false);  // marked as used if conflict

    if (cl_process_delete(c))    ClauseMisc::delete_clause(c);
    else {
      cl_process_keep(c);
      // Ordinary unit conflict.
      if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->safe_unit_conflict))	cl_process_conflict(c, false);
      cl_process_new_demod(c);
      // We insert c into the literal index now so that it will be
      // available for unit conflict and forward subsumption while
      // it's in limbo.  (It should not be back subsumed while in limbo.
      // See fatal error in limbo_process).
      IL.index_literals(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, false);
      CL.clist_append(c, Glob.limbo);
    }  // not deleted
  }  // not empty clause
  
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.preprocess);
  if (infer_clock_stopped)
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.infer);
}  // cl_process

void Search::cl_process_simplify(Topform c) {
  //myClock CLOCK;
  ClistContainer CL;
  TopformContainer TF;
  Parautil Pu;
  IndexLits IL;
  Demodulate D;
  
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->eval_rewrite)) {
    int count = 0;
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.demod);
    Dollar::rewrite_with_eval(c);
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) {
      printf("rewrites %d:     ", count);
      Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
    }
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.demod);
  }
  else if (!CL.clist_empty(Glob.demods)) {
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_order_vars)) {
      TF.renumber_variables(c, MAX_VARS);
      c->normal_vars = false;  // demodulation can make vars non-normal
    }
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.demod);
    
    int p1=LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->demod_step_limit);
    int p2=LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->demod_increase_limit);
    bool f1=!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet);
    bool f2=LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_order_vars);
    
    D.demodulate_clause(c,
                        p1,
                        p2,
                        f1,
                        f2);
    
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_gen)) {
      printf("rewrite:     ");
      cout<< "rewrite:     ";
      Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
    }
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.demod);
  }

  Pu.orient_equalities(c, true);
  Subsume::simplify_literals2(c);  // with x=x, and simplify tautologies to $T
  Resolve::merge_literals(c);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->unit_deletion)) {
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.unit_del);
    IL.unit_deletion(c);
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.unit_del);
  }

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->cac_redundancy)) {
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.redundancy);
    // If comm or assoc, make a note of it.
    // Also simplify C or AC redundant literals to $T.
    if (Ac_redun::cac_redundancy(c, !LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet))) {
      PlistContainer P;
      P.set_head(Glob.cac_clauses);
      Glob.cac_clauses = P.plist_prepend(c);
    } 
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.redundancy);
  }
} 


void Search::index_clashable(Topform c, Indexop operation) {
  //myClock CLOCK;
  LindexContainer LI;
  if (Glob.use_clash_idx) {
    CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.index);
    LI.lindex_update(PROVER9_GLOBAL_SEARCH.Glob.clashable_idx, c, operation);
    CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.index);
  }
}

void Search::disable_clause(Topform c) {
  // Assume c is in Usable, Sos, Denials, or none of those.
  // Also, c may be in Demodulators.
  //
  // Unindex c according to which lists it is on and
  // the flags that are set, remove c from the lists,
  // and append c to Disabled.  Make sure you don't
  // have a Clist_pos for c during the call, because
  // it will be freed during the call.
  //myClock CLOCK;
  ClistContainer CL;
  Demodulate D;
  IndexLits IL;
  //GivSelect GS;
  
  
  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.disable);

  // printf("disabling %d\n", c->id);

  if (CL.clist_member(c, Glob.demods)) {
    D.index_demodulator(c, Demod::demodulator_type(c,
                        LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_lim),
                        LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_sane)),
                        Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index);
                        CL.clist_remove(c, Glob.demods);
  }
  
  if (CL.clist_member(c, Glob.usable)) {
    IL.index_literals(c, Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index, false);
    D.index_back_demod(c, Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod));
    if (!restricted_denial(c))
      index_clashable(c, Indexop::DELETE);
    CL.clist_remove(c, Glob.usable);
  }
  else if (CL.clist_member(c, Glob.sos)) {
    IL.index_literals(c, Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index, false);
    D.index_back_demod(c, Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod));
    GS.remove_from_sos2(c, Glob.sos);
  }
  else if (CL.clist_member(c, Glob.limbo)) {
    IL.index_literals(c, Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index, false);
    CL.clist_remove(c, Glob.limbo);
  }

  /* printf(" (compressing %d)\n", c->id); */

  /* compress_clause(c); */

  CL.clist_append(c, Glob.disabled);
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.disable);
} 

void Search::back_demod(Topform demod) {
 //myClock CLOCK;
 TopformContainer TF;
 AttributeContainer A;
 //JustContainer J;
 Demodulate D;
 PlistContainer P;
 ClistContainer CL;
 
  Plist results, p, prev;

  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.back_demod);
  results = D.back_demodulatable( demod,
                                  Demod::demodulator_type(demod,
                                  LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_lim),
                                  LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_sane)),
                                  LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_order_vars));
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.back_demod);
  p = results;
  while(p != NULL) {
    Topform old =(Topform) p->v;
    if (!CL.clist_member(old, Glob.disabled)) {
      Topform novo;
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->basic_paramodulation))
        novo = TF.copy_clause_with_flag(old, Basic::nonbasic_flag());
        else novo = TF.copy_clause(old);
        PROVER9_GLOBAL_SEARCH.Stats.back_demodulated++;
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_kept))
            cout<<"        "<<demod->id<<" back demodulating "<<old->id<<"."<<endl;
            
        novo->justification = J.back_demod_just(old);
        novo->attributes = A.inheritable_att_instances(old->attributes, NULL);
        disable_clause(old);
        cl_process(novo);
    }
    prev = p;
    p = p->next;
    P.free_plist(prev);
  }
} 


void Search::back_unit_deletion(Topform unit) {

  Plist results, p, prev;
  PlistContainer P;
  //myClock CLOCK;
  AttributeContainer A;
  TopformContainer TF;
  //JustContainer J;
  ClistContainer CL;
  IndexLits IL;
  
  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.back_unit_del);
  results = IL.back_unit_deletable(unit);
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.back_unit_del);
  p = results;
  while(p != NULL) {
    Topform old = (Topform) p->v;
    if (!CL.clist_member(old, Glob.disabled)) {
      Topform novo;
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->basic_paramodulation))
        novo = TF.copy_clause_with_flag(old, Basic::nonbasic_flag());
        else	novo = TF.copy_clause(old);
      PROVER9_GLOBAL_SEARCH.Stats.back_unit_deleted++;
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_kept))
        
      cout<<"        "<<unit->id<<" back unit deleting "<<old->id<<"."<<endl;
      novo->justification = J.back_unit_deletion_just(old);
      novo->attributes = A.inheritable_att_instances(old->attributes, NULL);
      disable_clause(old);
      cl_process(novo);
    }
    prev = p;
    p = p->next;
    P.free_plist(prev);
  }
} 


void Search::back_cac_simplify(void) {
  
  
  Plist a;
  Clist_pos p;
  PlistContainer P;
  
  for (p = Glob.sos->first; p; p = p->next) {
    if (Ac_redun::cac_tautology(p->c->literals))
      P.plist_prepend(p->c);
  }
  for (p = Glob.usable->first; p; p = p->next) {
    if (Ac_redun::cac_tautology(p->c->literals))
      P.plist_prepend(p->c);
  }
  for (p = Glob.limbo->first; p; p = p->next) {
    if (Ac_redun::cac_tautology(p->c->literals))
      P.plist_prepend(p->c);
  }
  for (a = P.get_head(); a; a = a->next) {
    
    cout<<"%% back CAC tautology: ";
    Ioutil::f_clause((Topform) a->v);
    disable_clause((Topform) a->v);
  }
  
  P.zap_plist();
} 


void Search::disable_to_be_disabled(void) {
 
  ClistContainer CL;
  
  if (Glob.desc_to_be_disabled) {

    
    PlistContainer DESCENDANTS,PAUX;
    Plist p;

    CL.sort_clist_by_id(Glob.disabled);

    for (p = Glob.desc_to_be_disabled; p; p = p->next) {
      Topform c = (Topform) p->v;
      Plist x = Utilities::neg_descendants(c,Glob.usable,Glob.sos,Glob.disabled);
      DESCENDANTS.plist_cat(x);
    }
    
#if 1
    {
      int n = 0;
      cout<<endl<<"% Disable descendants (x means already disabled):"<<endl;
      for (p = DESCENDANTS.get_head(); p; p = p->next) {
        Topform d = (Topform) p->v;
        cout<<" "<<d->id<<(CL.clist_member(d, Glob.disabled) ? "x" : "");
        if (++n % 10 == 0) cout << endl;
      }
      cout<<endl;
    }
#endif

    for (p = DESCENDANTS.get_head(); p; p = p->next) {
      Topform d = (Topform) p->v;
      if (!CL.clist_member(d, Glob.disabled))	disable_clause(d);
    }

    DESCENDANTS.zap_plist();
    PAUX.set_head(Glob.desc_to_be_disabled);
    PAUX.zap_plist();
    Glob.desc_to_be_disabled = NULL;
  }
} 

void Search::limbo_process(bool pre_search) {
  
  
  
 int Dcount_c;                 // c is the next generated clause
 int Dcount_d;                 // d is the next clause subsumed by c
 int Dcount_min_sos;           // min over clauses d on list Sos
 int Dcount_min_not_sos;       // min over clauses d not on list Sos
 //GivSelect GS;
 Demodulate D;
 Parautil Pu;
 ClistContainer CL;
 PlistContainer P;
 IndexLits IL;
 //myClock CLOCK;
 //JustContainer J;
 SymbolContainer S;
  
  while (Glob.limbo->first) {
    Topform c = Glob.limbo->first->c;

    // factoring

   if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->factor))
        Resolve::binary_factors(c, cl_process);

    // Try to apply new_constant rule.

    if (!LADR_GLOBAL_OPTIONS.at_parm_limit(PROVER9_GLOBAL_SEARCH.Stats.new_constants, PROVER9_GLOBAL_SEARCH.Opt->new_constants)) {
      Topform novo = Pu.new_constant(c, INT_MAX);
      if (novo) {
        PROVER9_GLOBAL_SEARCH.Stats.new_constants++;
        if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
            cout<<endl<<"NOTE: New constant: ";
            Ioutil::f_clause(novo);
            cout<<"NOTE: New ";
            S.print_fsym_precedence(cout);
        }
	if (Glob.interps) Semantics::update_semantics_new_constant(novo);
    cl_process(novo);
    }
   }

    // fold denial (for input clauses only)

    if (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->fold_denial_max) > 1 && (J.has_input_just(c) || J.has_copy_just(c))) {
      Topform novo = Pu.fold_denial(c, LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->fold_denial_max));
      if (novo) {
        if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
        cout<<endl<<"Note: Fold denial: ";
        Ioutil::f_clause(novo);
       }
	  cl_process(novo);
      }
    }

    // Disable clauses subsumed by c (back subsumption).

    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_subsume)) {
        Plist subsumees;
        
        Dcount_c = degradation_count(c);
        Dcount_min_sos = Dcount_c;
        Dcount_min_not_sos = Dcount_c;
        
        CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.back_subsume);
        subsumees = IL.back_subsumption(c);
        if (subsumees != NULL) c->subsumer = true;
        
        while (subsumees != NULL) {
            
            Topform d = (Topform)subsumees->v;
            if (CL.clist_member(d, Glob.limbo))  // See comment in cl_process.
                fatal::fatal_error("back subsume limbo clause");
            
            PROVER9_GLOBAL_SEARCH.Stats.back_subsumed++;
            
            // when degraded hint matcher c subsumes hint matcher d 
            if (c -> matching_hint != NULL && d -> matching_hint != NULL && Dcount_c > 0) {
                Dcount_d = degradation_count(d);
                // track the minimum degradation counts across all such 
                // subsumed clauses d, distinguishing the sos/not sos cases
                if (CL.clist_member(d, Glob.sos))   {
                    if (Dcount_d < Dcount_min_sos)  Dcount_min_sos = Dcount_d;
                }
                else {
                        if (Dcount_d < Dcount_min_not_sos)   Dcount_min_not_sos = Dcount_d;
                }
            }
        
            
            
            if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_kept)) {
                if (d->matching_hint!=NULL)
                  cout << "    "<<c->id<<" back subsumes hint matcher "<<d->id<<"."<<endl;
                else
                  cout<<"    "<<c->id<<"back subsumes "<<d->id<<"."<<endl;  
            }
            disable_clause(d);
            P.set_head(subsumees);
            subsumees = P.plist_pop(subsumees);
      }
      
      
      // BV(2016-feb-02): adjust degradation of c if necessary
      //
      // Dcount_c is adjusted iff a subsumed hint matcher has a lower Dcount
      if (Dcount_min_sos < Dcount_c || Dcount_min_not_sos < Dcount_c)   {
         // original computed weight
         c -> weight = (int)(c -> weight) % 1000;
         // slight penalty if all the mins have already been given
         if (Dcount_min_sos <= Dcount_min_not_sos) {
            // new degradation
            c -> weight = c -> weight + Dcount_min_sos * 1000;
            if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_given))    cout<<" => "<<c->id<<" back subsumes a hint matcher on Sos.  Reset weight to "<<c->weight<<endl;
         }
         else {
            // new degradation plus penalty
            c -> weight = c -> weight + Dcount_min_not_sos * 1000 + 500;

           if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_given)) cout << " => " <<c->id<< " back subsumes hint matchers not on Sos.  Reset weight to "<<c->weight<<endl;
               
         }
      }
      
      
      CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.back_subsume);
    }

    // If demodulator, rewrite other clauses (back demodulation).

    if (CL.clist_member(c, Glob.demods)) {
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_kept))
	    cout<<"    starting back demodulations with "<<c->id<<"."<<endl;
      back_demod(c);  // This calls cl_process on rewritable clauses.
    }

    // If unit, use it to simplify other clauses (back unit_deletion)

    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->unit_deletion) && LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals)) {
      back_unit_deletion(c);  // This calls cl_process on rewritable clauses.
    }

    // Check if we should do back CAC simplification.
    P.set_head(Glob.cac_clauses);
    if (P.plist_member(c)) {
      back_cac_simplify();
    }

    // Remove from limbo
    CL.clist_remove(c, Glob.limbo);

    // If restricted_denial, appdend to usable, else append to sos.

    if (restricted_denial(c)) {
      // do not index_clashable!  disable_clause should not unindex_clashable!
      CL.clist_append(c, Glob.usable);
      D.index_back_demod(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod));
    }
    else {
      // Move to Sos and index to be found for back demodulation.
      if (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->sos_limit) != -1 &&	  CL.clist_length(Glob.sos) >= LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->sos_limit)) {
        GS.sos_displace2(disable_clause);
        PROVER9_GLOBAL_SEARCH.Stats.sos_displaced++;
      }
      if (pre_search) c->initial = true;
      else c->initial = false;
      GS.insert_into_sos2(c, Glob.sos);
      D.index_back_demod(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod));
    }
  }
  // Now it is safe to disable descendants of desc_to_be_disabled clauses.
  disable_to_be_disabled();
} 

void Search::infer_outside_loop(Topform c) {
  /* If simplification changes the clause, we want to do a "copy"
   inference first, so that a proof does not contain a justification
   like  [assumption,rewrite[...],...]. */
  ClistContainer CL; 
  ClauseidContainer CI;
  Topform copy = Resolve::copy_inference(c);  /* Note: c has no ID yet. */
  cl_process_simplify(copy);
  if (copy->justification->next == NULL) {
    /* Simplification does nothing, so we can just process the original. */
    ClauseMisc::delete_clause(copy);
    cl_process(c);
  }
  else {
    CI.assign_clause_id(c);
    copy->justification->u.id = c->id;
    CL.clist_append(c, Glob.disabled);
    cl_process(copy);  /* This re-simplifies, but that's ok. */
  }

  limbo_process(false);
}  



void Search::auto_process(Clist sos, Clist usable, Prover_options opt) {
  ClausesContainer CLA;
  ClistContainer CL;
  
  bool print = !LADR_GLOBAL_OPTIONS.flag(opt->quiet);
  Plist clauses;
  bool horn;

  clauses = CL.prepend_clist_to_plist(NULL, sos);
  clauses = CL.prepend_clist_to_plist(clauses, usable);

  horn  = CLA.all_clauses_horn(clauses);

  if (print)
    printf("\nAuto_process settings:");

  if (horn) {
    if (CLA.neg_nonunit_clauses(clauses) > 0) {
      if (print)
          cout<<endl<<"  %% set(unit_deletion).  %% (Horn set with negative nonunits)"<<endl;
	  LADR_GLOBAL_OPTIONS.set_flag(opt->unit_deletion, print);
    }
    else {
      if (print)
          cout<<"  (no changes)."<<endl;
        
    }
  }

  else {
    // there are nonhorn clauses
    if (print)
        cout<<endl<<"  %% set(factor). %% (non-Horn)"<<endl;
        
    LADR_GLOBAL_OPTIONS.set_flag(opt->factor, print);
    if (print)
     cout<<"  %% set(unit_deletion).  %% (non-Horn)"<<endl; 
     
    
    LADR_GLOBAL_OPTIONS.set_flag(opt->unit_deletion, print);
  }
  PlistContainer P;
  P.set_head(clauses);
  P.zap_plist();
} 




void Search::auto_inference(Clist sos, Clist usable, Prover_options opt) {
  PlistContainer P;
  ClausesContainer CLA;
  ClistContainer CL;
  bool print = !LADR_GLOBAL_OPTIONS.flag(opt->quiet);
  if (print)
    cout<<endl<<"Auto_inference settings:"<<endl;
      

  if (Glob.equality) {
    if (print)
      cout<<" % set(paramodulation).  % (positive equality literals)"<<endl;
    LADR_GLOBAL_OPTIONS.set_flag(opt->paramodulation, print);
  }

  if (!Glob.equality || !Glob.unit) {
    if (Glob.horn) {
      Plist clauses = NULL;
      clauses = CL.prepend_clist_to_plist(clauses, sos);
      clauses = CL.prepend_clist_to_plist(clauses, usable);

      if (Glob.equality) {
	if (print)
        cout<< "  % set(hyper_resolution).  % (nonunit horn with equality)"<<endl;
	  
	LADR_GLOBAL_OPTIONS.set_flag(opt->hyper_resolution, print);
	if (print)
	  cout<< "  % set(neg_ur_resolution).  % (nonunit Horn with equality)"<<endl;
        
	LADR_GLOBAL_OPTIONS.set_flag(opt->neg_ur_resolution, print);

	if (LADR_GLOBAL_OPTIONS.parm(opt->para_lit_limit) == -1) {
	  int para_lit_limit = CLA.most_literals(clauses);
	  if (print)
	    cout<<"  % assign(para_lit_limit, "<<para_lit_limit<<").  % (nonunit Horn with equality)"<<endl;
	  LADR_GLOBAL_OPTIONS.assign_parm(opt->para_lit_limit, para_lit_limit, print);
	 }
    }
    else {
	int diff = Utilities::neg_pos_depth_difference(clauses);
	if (diff > 0) {
	  if (print)
        cout<<"  % set(hyper_resolution).  %  (HNE depth_diff="<<diff<<")"<<endl;
        
	  LADR_GLOBAL_OPTIONS.set_flag(opt->hyper_resolution, print);
	}
	else {
	  if (print)
        cout<<"  % set(neg_binary_resolution).  % (HNE depth_diff="<<diff<<")"<<endl;  
	    
      LADR_GLOBAL_OPTIONS.set_flag(opt->neg_binary_resolution, print);
	  if (print)
	    cout<<"  % clear(ordered_res).  % (HNE depth_diff="<<diff<<")"<<endl;
          
	  LADR_GLOBAL_OPTIONS.clear_flag(opt->ordered_res, print);
	  if (print)
	    cout<<"  % set(ur_resolution).  % (HNE depth_diff="<<diff<<")"<<endl;
        LADR_GLOBAL_OPTIONS.set_flag(opt->ur_resolution, print);
	 }
    }
  
    P.set_head(clauses);
    P.zap_plist();
    }
    else {
      // there are nonhorn clauses
      if (print) {
        printf("  %% set(binary_resolution).  %% (non-Horn)\n");
      }
      LADR_GLOBAL_OPTIONS.set_flag(opt->binary_resolution, print);
      if (Glob.number_of_clauses < 100) {
	  if (print)
	    cout<<"  %% set(neg_ur_resolution).  %% (non-Horn, less than 100 clauses"<<endl;
      LADR_GLOBAL_OPTIONS.set_flag(opt->neg_ur_resolution, print);
      }
    }
  }
}





void Search::init_search(void) {
  // Initialize clocks.
 //myClock CLOCK;
 //GivSelect GS;
 SymbolContainer S;
 TermOrder TO;
 Actions Ac;
 PlistContainer P;
 ClausesContainer C;
 
 CLOCK.init_clock("pick_given");
 PROVER9_GLOBAL_SEARCH.Clocks.pick_given    = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("infer");
 PROVER9_GLOBAL_SEARCH.Clocks.infer    = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("preprocess");
 PROVER9_GLOBAL_SEARCH.Clocks.preprocess    = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("demod");
 PROVER9_GLOBAL_SEARCH.Clocks.demod         = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("unit_deletion");
 PROVER9_GLOBAL_SEARCH.Clocks.unit_del      = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("redundancy");
 PROVER9_GLOBAL_SEARCH.Clocks.redundancy    = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("conflict");
 PROVER9_GLOBAL_SEARCH.Clocks.conflict      = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("weigh");
 PROVER9_GLOBAL_SEARCH.Clocks.weigh         = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("hints");
 PROVER9_GLOBAL_SEARCH.Clocks.hints         = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("subsume");
 PROVER9_GLOBAL_SEARCH.Clocks.subsume       = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("semantics");
 PROVER9_GLOBAL_SEARCH.Clocks.semantics     = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("back_subsume");
 PROVER9_GLOBAL_SEARCH.Clocks.back_subsume  = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("back_demod");
 PROVER9_GLOBAL_SEARCH.Clocks.back_demod    = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("back_unit_del");
 PROVER9_GLOBAL_SEARCH.Clocks.back_unit_del = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("index");
 PROVER9_GLOBAL_SEARCH.Clocks.index         = CLOCK.get_ClockStructure();
 
 CLOCK.init_clock("disable");
 PROVER9_GLOBAL_SEARCH.Clocks.disable       = CLOCK.get_ClockStructure();

 
 Ac.init_actions(Glob.actions,
	       rebuild_sos_index, done_with_search, infer_outside_loop);
 
 
 Weight::init_weight(Glob.weights,
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->variable_weight),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->constant_weight),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->not_weight),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->or_weight),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->sk_constant_weight),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->prop_atom_weight),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->nest_penalty),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->depth_penalty),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->var_penalty),
	      LADR_GLOBAL_OPTIONS.floatparm(PROVER9_GLOBAL_SEARCH.Opt->complexity));
    PlistContainer Paux;
    if (Glob.given_selection == NULL)
        Glob.given_selection = GS.selector_rules_from_options(PROVER9_GLOBAL_SEARCH.Opt);
    else 
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->input_sos_first)) {
            Paux.set_head(Glob.given_selection);
            Glob.given_selection = Paux.plist_prepend(GS.selector_rule_term("I", "high", "age",  "initial", INT_MAX));
  }
  
  
  GS.init_giv_select(Glob.given_selection);
  
  PlistContainer P1, P2;
  P1.set_head(WhiteBlack::delete_rules_from_options(PROVER9_GLOBAL_SEARCH.Opt));
  P2.set_head(Glob.delete_rules);
  Glob.delete_rules = P1.plist_cat(P2);
  
  WhiteBlack::init_white_black(Glob.keep_rules, Glob.delete_rules);

  // Term ordering
  cout<<endl<<"Term ordering decisions:"<<endl;
  
    if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "lpo")) {
    TO.assign_order_method(Order_method::LPO_METHOD);
    S.all_symbols_lrpo_status(Lrpo_Status::LRPO_LR_STATUS);
    S.set_lrpo_status(S.str_to_sn(S.eq_sym(), 2), Lrpo_Status::LRPO_MULTISET_STATUS);
  }
  else if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "rpo")) {
    TO.assign_order_method(Order_method::RPO_METHOD);
    S.all_symbols_lrpo_status(Lrpo_Status::LRPO_MULTISET_STATUS);
  }
  else if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "kbo")) {
    TO.assign_order_method(Order_method::KBO_METHOD);
  }

  Utilities::symbol_order(Glob.usable, Glob.sos, Glob.demods, true);

  if (Glob.kbo_weights) {
    if (!LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "kbo")) {
      LADR_GLOBAL_OPTIONS.assign_stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "kbo", true);
      cout<<"assign(order, kbo), because KB weights were given."<<endl;
      
    }
    TO.init_kbo_weights(Glob.kbo_weights);
    S.print_kbo_weights(cout);
  }
  else if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "kbo")) {
    Utilities::auto_kbo_weights(Glob.usable, Glob.sos);
    S.print_kbo_weights(cout);
  }

  if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
    S.print_rsym_precedence(cout);
    S.print_fsym_precedence(cout);
  }

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->inverse_order)) {
    if (S.exists_preliminary_precedence(Symbol_Type::FUNCTION_SYMBOL)) {  // lex command
      if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet))
        cout<<"Skipping inverse_order, because there is a function_order (lex) command."<<endl;
          
    }
    else if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "kbo")) {
      if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet))
        cout<<"Skipping inverse_order, because term ordering is KBO."<<endl;  
    }
    else {
      bool change = Utilities::inverse_order(Glob.sos);
      if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
        cout<<"After inverse_order: ";
      if (change)
	    S.print_fsym_precedence(cout);
	else
	    cout << " (no changes)."<<endl;
      }
    }
  }

  if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->eq_defs, "unfold")) {
    if (S.exists_preliminary_precedence(Symbol_Type::FUNCTION_SYMBOL))  // lex command
      cout<<"Skipping unfold_eq, bacause there is a function_order (lex) command."<<endl;
        
    else
      Unfold::unfold_eq_defs(Glob.sos, INT_MAX, 3, !LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet));
  }
  else if (LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->eq_defs, "fold")) {
    if (S.exists_preliminary_precedence(Symbol_Type::FUNCTION_SYMBOL))  // lex command
      cout<<"Skipping fold_eq, because there is a function_order (lex) command."<<endl;
        
    else {
      bool change = Unfold::fold_eq_defs(Glob.sos, LADR_GLOBAL_OPTIONS.stringparm(PROVER9_GLOBAL_SEARCH.Opt->order, "kbo"));
      if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
        cout<<"After fold_eq: ";
          
      if (change)
        S.print_fsym_precedence(cout);
	else
        cout<<" (no changes)."<<endl;
      }
    }
  }

  // Automatic inference and processing settings
  
  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->auto_inference))
        auto_inference(Glob.sos, Glob.usable, PROVER9_GLOBAL_SEARCH.Opt);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->auto_process))
        auto_process(Glob.sos, Glob.usable, PROVER9_GLOBAL_SEARCH.Opt);

  // Tell packages about options and other things.

  Resolve::resolution_options(LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->ordered_res),
		     LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->check_res_instances),
		     LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->initial_nuclei),
		     LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->ur_nucleus_limit),
		     LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->eval_rewrite));
		     
  Paramodulation::paramodulation_options(LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->ordered_para),
			 LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->check_para_instances),
			 false,
			 LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->basic_paramodulation),
			 LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->para_from_vars),
			 LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->para_into_vars),
			 LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->para_from_small));
}


void Search::basic_clause_properties(Clist sos, Clist usable) {
  
  
  PlistContainer P1,P2;
  ClistContainer CL;
  ClausesContainer C;
  
  Plist sos_temp    = CL.copy_clist_to_plist_shallow(sos);
  Plist usable_temp = CL.copy_clist_to_plist_shallow(usable);

  Glob.equality =  C.pos_equality_in_clauses(sos_temp) || C.pos_equality_in_clauses(usable_temp);
    
  Glob.unit =      C.all_clauses_unit(sos_temp) && C.all_clauses_unit(usable_temp);

  Glob.horn =      C.all_clauses_horn(sos_temp) && C.all_clauses_horn(usable_temp);

  P1.set_head(sos_temp);
  P2.set_head(usable_temp);
  Glob.number_of_clauses =  P1.plist_count() + P2.plist_count();

  Glob.number_of_neg_clauses =
    C.negative_clauses(sos_temp) + C.negative_clauses(usable_temp);

  P1.zap_plist(sos_temp);
  P2.zap_plist(usable_temp);
}  



void Search::auto_denials(Clist sos, Clist usable, Prover_options opt) {
  
  PlistContainer P1,P2;
  TermContainer T;  
  AttributeContainer A;
  ClistContainer CL;
  
  
  int changes = 0;



  cout<< endl<< "Auto_denials:"; 
  
  if (Glob.horn) {
    P1.set_head(CL.neg_clauses_in_clist(sos));
    P2.set_head(CL.neg_clauses_in_clist(usable));
    Plist neg_clauses = P1.plist_cat(P2);
    
    Plist p;
    for (p = neg_clauses; p; p = p->next) {
      Topform c = (Topform) p->v;
      string label = A.get_string_attribute(c->attributes, PROVER9_GLOBAL_SEARCH.Att.label, 1);
      Term answer = A.get_term_attribute(c->attributes, PROVER9_GLOBAL_SEARCH.Att.answer, 1);
      if (label!=myString::null_string()  && !answer) {
        Term t = T.get_rigid_term(label, 0);
        c->attributes = A.set_term_attribute(c->attributes, PROVER9_GLOBAL_SEARCH.Att.answer, t);
        cout<< (changes == 0 ? "\n" : "");
        cout<<"  % copying label "<< label<< " to answer in negative clause"<<endl;
        changes++;
      }
    }

    if (Glob.number_of_neg_clauses > 1 && LADR_GLOBAL_OPTIONS.parm(opt->max_proofs) == 1) {
        cout<<(changes == 0 ? "\n" : "");
        cout<<"  % assign(max_proofs, "<<Glob.number_of_neg_clauses<<")  %  (Horn set with more than one neg. clause)"<<endl;
        LADR_GLOBAL_OPTIONS.assign_parm(opt->max_proofs, Glob.number_of_neg_clauses, true);
        Utilities::check_constant_sharing(neg_clauses);
        changes++;
    }
    P1.set_head(neg_clauses);
    P1.zap_plist(neg_clauses);
  }

  if (changes == 0) cout<<"   ("<<(Glob.horn ? "" : "non-Horn, ")<<"no changes)."<<endl;
    
} 


Topform Search::orient_input_eq(Topform c) {
  Parautil Pu;
  //JustContainer J;
  ClistContainer CL;
  ClauseidContainer CI;
  
  
  Topform novo = Resolve::copy_inference(c);
  Pu.orient_equalities(novo, true);
  if (LADRV_GLOBAIS_INST.Lit.clause_ident(c->literals, novo->literals)) {
    ClauseMisc::delete_clause(novo);
    /* the following puts "oriented" marks on c */
    Pu.orient_equalities(c, true);
    return c;
  }
  else {
    /* Replace c with new in Usable. */
    CI.assign_clause_id(novo);
    J.mark_parents_as_used(novo);
    CL.clist_swap(c, novo);
    CL.clist_append(c, Glob.disabled);
    return novo;
  }
} 



void Search::index_and_process_initial_clauses(void) {
  
 
 
  
  Clist_pos p;
  Clist temp_sos;

  //GivSelect GS;
  //myClock CLOCK;
  Parautil Pu;
  ClauseidContainer CI;
  AttributeContainer A;
  //JustContainer J;
  ClistContainer CL;
  TopformContainer TF;
  TermContainer T;
  FormulaContainer F;
  LindexContainer LI;
  Demodulate D;
  IndexLits IL;
  
  // Index Usable clauses if hyper, UR, or binary-res are set.

Glob.use_clash_idx = (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->binary_resolution) ||
			LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->neg_binary_resolution) ||
			LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->pos_hyper_resolution) ||
			LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->neg_hyper_resolution) ||
			LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->pos_ur_resolution) ||
			LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->neg_ur_resolution));

  // Allocate and initialize indexes (even if they won't be used).

  IL.init_literals_index();  // fsub, bsub, fudel, budel, ucon

  D.init_demodulator_index(Mindextype::DISCRIM_BIND, Uniftype::ORDINARY_UNIF, 0);

  D.init_back_demod_index(Mindextype::FPA, Uniftype::ORDINARY_UNIF, 10);

  PROVER9_GLOBAL_SEARCH.Glob.clashable_idx = LI.lindex_init(Mindextype::FPA, Uniftype::ORDINARY_UNIF, 10, Mindextype::FPA, Uniftype::ORDINARY_UNIF, 10);

  Hints::init_hints(Uniftype::ORDINARY_UNIF, PROVER9_GLOBAL_SEARCH.Att.bsub_hint_wt, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->collect_hint_labels),  LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod_hints),  Demodulate::demodulate_clause);
  Semantics::init_semantics(Glob.interps, PROVER9_GLOBAL_SEARCH.Clocks.semantics, LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->multiple_interps), LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->eval_limit), LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->eval_var_limit));

  
  static int hint_id_number=0; //2016-may-27
  
  
  // Do Sos and Denials last, in case we PROCESS_INITIAL_SOS.

  ////////////////////////////////////////////////////////////////////////////
  // Usableinsert_into_sos2

  for (p = Glob.usable->first; p != NULL; p = p->next) {
    Topform c = (Topform) p->c;
    CI.assign_clause_id(c);
    Maximal::mark_maximal_literals(c->literals);
    Maximal::mark_selected_literals(c->literals, LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->literal_selection));
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->dont_flip_input))   Pu.orient_equalities(c, false);  // mark, but don't allow flips
    else
      c =orient_input_eq(c);  /* this replaces c if any flipping occurs */
    IL.index_literals(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, false);
    D.index_back_demod(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod));
    index_clashable(c, Indexop::INSERT);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Demodulators

  if (!CL.clist_empty(Glob.demods) && !LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->eval_rewrite)) {
    cout.flush();
    
    fatal::bell(cerr);
    cerr << endl <<"WARNING: The use of input demodulators is not well tested"<<endl<<"and discouraged. You migth need to cleat(process_initial_sos)"<<endl<<"so that sos clauses ara not rewritten and deleted."<<endl;
    fflush(stderr);
  }

  for (p = Glob.demods->first; p != NULL; p = p->next) {
    Topform c = p->c;
    CI.assign_clause_id(c);
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->eval_rewrite)) {
      if (c->is_formula) {
	/* make it into a pseudo-clause */
	c->literals = LADRV_GLOBAIS_INST.Lit.new_literal(true, F.formula_to_term(c->formula));
	TF.upward_clause_links(c);
	F.zap_formula(c->formula);
	c->formula = NULL;
	c->is_formula = false;
	TF.clause_set_variables(c, MAX_VARS);
	Pu.mark_oriented_eq(c->literals->atom);
    }
    }
    else {
      if (!LADRV_GLOBAIS_INST.Lit.pos_eq_unit(c->literals))
        fatal::fatal_error("input demodulator is not equation");
      else {
        int type;
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->dont_flip_input))        Pu.orient_equalities(c, false);  /* don't allow flips */
        else  c = orient_input_eq(c);  /* this replaces c if any flipping occurs */
	
        if (c->justification->next != NULL) {
            cout<<endl<<"NOTE: input demodulator "<<c->id<<" has been flipped."<<endl;
            cout.flush();
            cerr<<endl<<"NOTE: input demodulator "<<c->id<<" has been flipped."<<endl;
            if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->bell))	    fatal::bell(cerr);
            cerr.flush();    
        }
        type = Demod::demodulator_type(c,
				LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_lim),
				LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_sane));
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->dont_flip_input) && type != (int) Demod_Type::ORIENTED &&  !Pu.renamable_flip_eq(c->literals->atom)) {
        type =(int) Demod_Type::ORIENTED;  /* let the user beware */
        Pu.mark_oriented_eq(c->literals->atom);
        fatal::bell(cerr);
        cerr<<endl<<"WARNING: demodulator does not satisfy term order"<<endl;
        cerr.flush();
        fflush(stderr);
	  
        cerr<<endl<<"WARNING: demodultator does not satisfy term order: ";
        Ioutil::f_clause(c);
        cout.flush();
     }
     else if (type == (int) Demod_Type::NOT_DEMODULATOR) {
	  Term a = ARG(c->literals->atom,0);
	  Term b = ARG(c->literals->atom,1);
	  cout<<"bad input demodulator: ";
	  Ioutil::f_clause(c);
      if (T.term_ident(a, b))
	    fatal::fatal_error("input demodulator is instance of x=x");
	  else if (!T.variables_subset(a, b) && !T.variables_subset(b, a))
	    fatal::fatal_error("input demoulator does not have var-subset property");
	  else
	    fatal::fatal_error("input demoulator not allowed");
      }
      D.index_demodulator(c, type, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index);
     }
   }
  }

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->eval_rewrite))
    Dollar::init_dollar_eval(Glob.demods);

  ////////////////////////////////////////////////////////////////////////////
  // Hints
  
  if (Glob.hints->first) {
    for (p = Glob.hints->first; p != NULL; p = p->next) {
      Topform h = p->c;
      
      //2016-may-27
      h->id=hint_id_number;
      hint_id_number++;
      
      h->used = false;// BV(2017-nov-12): initialize for flagging by print_processed_hints
      
      // assign_clause_id(h);  // This should be optional
      Pu.orient_equalities(h, true);
      TF.renumber_variables(h, MAX_VARS);
      Hints::index_hint(h,false);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Sos

  // Move Sos to a temporary list, then process that temporary list,
  // putting the clauses back into Sos in the "correct" way, either
  // by calling cl_process() or doing it here.

  temp_sos = Glob.sos;                    // move Sos to a temporary list
  CL.name_clist(temp_sos, "temp_sos");       // not really necessary
  Glob.sos = CL.clist_init("sos");           // get a new (empty) Sos list

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->process_initial_sos)) {

    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_initial_clauses)) cout << endl;
      

    while (temp_sos->first) {
      Topform c = temp_sos->first->c;
      Topform novo;
      CL.clist_remove(c, temp_sos);
      CL.clist_append(c, Glob.disabled);
      
      novo = Resolve::copy_inference(c);  // c has no ID, so this is tricky
      cl_process_simplify(novo);
      if (novo->justification->next == NULL) {
        // No simplification occurred, so make it a clone of the parent.
        J.zap_just(novo->justification);
        novo->justification = J.copy_justification(c->justification);
        // Get all attributes, not just inheritable ones.
        A.zap_attributes(novo->attributes);
        novo->attributes = A.copy_attributes(c->attributes);
      }
      else {
        // Simplification occurs, so make it a child of the parent.
        CI.assign_clause_id(c);
        novo->justification->u.id = c->id;
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_initial_clauses)) {
        
        cout<<"           ";
        Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
	   }
      }
      cl_process(novo);  // This re-simplifies, but that's ok.
    }
    
    // This will put processed clauses back into Sos.
    limbo_process(true);  // back subsumption and back demodulation.

  }
  else {
    /* not processing initial sos */
    
    cout.flush();
    fatal::bell(cerr);
    cerr<<endl<<"WARNING: clear(process_initial_sos) is not well tested."<<endl<<"We usualy recommend against using it."<<endl;
    cerr.flush();
    
    
    /* not applying full processing to initial sos */
    while (temp_sos->first) {
      Topform c = temp_sos->first->c;
      CL.clist_remove(c, temp_sos);

      if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) == 0)
	/* in case $F is in input, or if predicate elimination finds proof */
	handle_proof_and_maybe_exit(c);
      else {
        CI.assign_clause_id(c);
        if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->dont_flip_input)) Pu.orient_equalities(c, false);
        else c = orient_input_eq(c);
        Maximal::mark_maximal_literals(c->literals);
        Maximal::mark_selected_literals(c->literals, LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->literal_selection));
        c->weight = Weight::clause_weight(c->literals);
        if (!CL.clist_empty(Glob.hints)) {
            CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.hints);
            Hints::adjust_weight_with_hints(c,
				   LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->degrade_hints),
				   LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->breadth_first_hints));
         CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.hints);
        }

	c->initial = true;
	GS.insert_into_sos2(c, Glob.sos);
	IL.index_literals(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, false);
	D.index_back_demod(c, Indexop::INSERT, PROVER9_GLOBAL_SEARCH.Clocks.index, LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod));
      }
    }
  }

  CL.clist_zap(temp_sos);  // free the temporary list

  ////////////////////////////////////////////////////////////////////////////
  // Print

  banner::print_separator(cout, "end of process initial clauses", true);

  banner::print_separator(cout, "CLAUSES FOR SEARCH", true);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_initial_clauses)) {
    
    cout<<endl<<"% Clauses after input processing:"<<endl;
    Ioutil::fwrite_clause_clist(cout,Glob.usable,  (int) Clause_print_format::CL_FORM_STD);
    Ioutil::fwrite_clause_clist(cout,Glob.sos,     (int) Clause_print_format::CL_FORM_STD);
    Ioutil::fwrite_demod_clist(cout,Glob.demods,   (int) Clause_print_format::CL_FORM_STD);
  }
  if (Glob.hints->length > 0) {
      int redundant = Hints::redundant_hints();
      cout<<endl<<"% "<< Glob.hints->length - redundant<<" hints ("<<Glob.hints->length<<" processed, "<<redundant<< " redundant)."<<endl;
    }

  banner::print_separator(cout, "end of clauses for search", true);

} 


bool Search:: inferences_to_make(void) {
    //GivSelect GS;
    return GS.givens_available();
}  


void Search::given_infer(Topform given) {
  
  //myClock CLOCK;
    
  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.infer);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->binary_resolution))
    Resolve::binary_resolution(given,(int) Resolve_type::ANY_RES,PROVER9_GLOBAL_SEARCH.Glob.clashable_idx,cl_process);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->neg_binary_resolution))
    Resolve::binary_resolution(given, (int) Resolve_type::NEG_RES,  PROVER9_GLOBAL_SEARCH.Glob.clashable_idx, cl_process);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->pos_hyper_resolution))
    Resolve::hyper_resolution(given, (int) Resolve_type::POS_RES, PROVER9_GLOBAL_SEARCH.Glob.clashable_idx, cl_process);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->neg_hyper_resolution))
    Resolve::hyper_resolution(given,(int) Resolve_type::NEG_RES, PROVER9_GLOBAL_SEARCH.Glob.clashable_idx, cl_process);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->pos_ur_resolution))
    Resolve::ur_resolution(given, (int) Resolve_type::POS_RES, PROVER9_GLOBAL_SEARCH.Glob.clashable_idx, cl_process);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->neg_ur_resolution))
    Resolve::ur_resolution(given, (int) Resolve_type::NEG_RES, PROVER9_GLOBAL_SEARCH.Glob.clashable_idx, cl_process);

  if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->paramodulation) &&
      !LADR_GLOBAL_OPTIONS.over_parm_limit(LADRV_GLOBAIS_INST.Lit.number_of_literals(given->literals),  PROVER9_GLOBAL_SEARCH.Opt->para_lit_limit)) {
    // This paramodulation does not use indexing.
        Context cf = U.get_context();
        Context ci = U.get_context();
        Clist_pos p;
        for (p = Glob.usable->first; p; p = p->next) {
            if (!restricted_denial(p->c) && !LADR_GLOBAL_OPTIONS.over_parm_limit(LADRV_GLOBAIS_INST.Lit.number_of_literals(p->c->literals),  PROVER9_GLOBAL_SEARCH.Opt->para_lit_limit)) {
                Paramodulation::para_from_into(given, cf, p->c, ci, false, cl_process);
                Paramodulation::para_from_into(p->c, cf, given, ci, true, cl_process);
            }
        }
        U.free_context(cf);
        U.free_context(ci);
  }
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.infer);
}




void Search::make_inferences(void) {
 
  Topform given_clause;
  string selection_type;
  ClistContainer CL;
  //myClock CLOCK;
  Actions Ac;
  ClauseidContainer CI;
 // GivSelect GS;
  
  CLOCK.clock_start(PROVER9_GLOBAL_SEARCH.Clocks.pick_given);
  given_clause = GS.get_given_clause2(Glob.sos,PROVER9_GLOBAL_SEARCH.Stats.given, PROVER9_GLOBAL_SEARCH.Opt, selection_type);
  
  CLOCK.clock_stop(PROVER9_GLOBAL_SEARCH.Clocks.pick_given);

  if (given_clause != NULL) {
    static int level = 0;             // NOTE: STATIC VARIABLE
    static int last_of_level = 0;     // NOTE: STATIC VARIABLE
    static int nohints_count = 0;     // NOTE: STATIC VARIABLE
    
    // hint matcher condition in this context
    bool hint_matcher =  given_clause -> matching_hint && (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->degrade_limit) == -1 
    || degradation_count(given_clause) <= LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->degrade_limit));

    // condition to reset counter
    if (given_clause -> initial || hint_matcher)
       nohints_count = 0;
    else 
       nohints_count++ ;

    if (LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->max_nohints) > 0 && nohints_count > LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->max_nohints))
    {
       cout << endl<< "*** " << LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->max_nohints) <<"givens in a row w/o an input clause or a hint matcher (max_nohints)."<<endl; 
       exit_with_message(cout, (int) exitCodes::MAX_NOHINTS_EXIT);
    }
    
    
    // Print "level" message for breadth-first; also "level" actions.

    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->breadth_first) &&
        LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->true_part) == 0 &&
        LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->false_part) == 0 &&
        LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->weight_part) == 0 &&
        LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->random_part) == 0 &&
        myString::str_ident(selection_type, "A")  && given_clause->id > last_of_level) {
            level++;
            last_of_level = CI.clause_ids_assigned();
            if (!LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
                cout <<endl <<"NOTE: Starting on level "<<level<<", last clause of level "<<level-1<<" is "<< last_of_level<<"."<<endl;
                cout.flush();
                cerr<<endl <<"NOTE: Starting on level "<<level<<", last clause of level "<<level-1<<" is "<< last_of_level<<"."<<endl;
                cerr.flush();
            }
        Ac.statistic_actions("level", level);
    }

    PROVER9_GLOBAL_SEARCH.Stats.given++;

    // Maybe disable back subsumption.

    if (LADR_GLOBAL_OPTIONS.over_parm_limit(PROVER9_GLOBAL_SEARCH.Stats.given, PROVER9_GLOBAL_SEARCH.Opt->max_given))
      done_with_search((int) exitCodes::MAX_GIVEN_EXIT);

    if (PROVER9_GLOBAL_SEARCH.Stats.given == LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->backsub_check)) {
      int ratio = (PROVER9_GLOBAL_SEARCH.Stats.back_subsumed == 0 ?  INT_MAX :  PROVER9_GLOBAL_SEARCH.Stats.kept / PROVER9_GLOBAL_SEARCH.Stats.back_subsumed);
        if (ratio > 20 )  {
            LADR_GLOBAL_OPTIONS.clear_flag(PROVER9_GLOBAL_SEARCH.Opt->back_subsume, true);
            CLOCK.set_ClockStructure(PROVER9_GLOBAL_SEARCH.Clocks.back_subsume);
            cout<<endl<<"NOTE: Back_subsumption disabled, ratio of kept to back_subsumed is "<<ratio<<" ("<< CLOCK.clock_seconds()<<" of "<<myClock::user_seconds()<<" sec)."<<endl;
            cout.flush();
      }
    }
    
    
  
   
   if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_given) || PROVER9_GLOBAL_SEARCH.Stats.given % 500 == 0)  {
     
      if (given_clause->weight == round(given_clause->weight))
        cout<<endl<<"given #"<<PROVER9_GLOBAL_SEARCH.Stats.given<<" ("<<selection_type<<",wt="<< (int) given_clause->weight<<"): ";
      else 
        cout<<endl<<"given #"<<PROVER9_GLOBAL_SEARCH.Stats.given<<" ("<<selection_type<<",wt="<<(int) given_clause->weight<<"): ";
          
     Ioutil::fwrite_clause(cout, (Topform) given_clause, (int) Clause_print_format::CL_FORM_STD);
     
    }

    Ac.statistic_actions("given", PROVER9_GLOBAL_SEARCH.Stats.given);
    

    CL.clist_append(given_clause, Glob.usable);
    index_clashable(given_clause, Indexop::INSERT);
    given_infer(given_clause);
  } else done_with_search((int) exitCodes::FATAL_EXIT);
} 


// Print a clause's derivation.  Specific to hit list.
void Search::print_derivation(Topform cl) {
  
   Plist proof, p;
   static int pfcount = 0;
   //JustContainer J;
 
   proof = J.get_clause_ancestors(cl);
   Compress::uncompress_clauses(proof);
 
   pfcount++ ;
 
   banner::print_separator(cout, "PROOF", true);
   cout << endl << "%% Derivation (Proof) " << pfcount << " (Clause #"<< cl->id<<")"<<endl;
   cout << endl <<"%% Length of derivation is "<<J.proof_length(proof)<<"."<<endl<<endl;
   for (p = proof; p; p = p->next)
   {
      /* printf("<SD> "); */     /* selected derivation */
      Ioutil::fwrite_clause(cout, (Topform) p->v, (int) Clause_print_format::CL_FORM_STD);
   }
   
   banner::print_separator(cout, "end of proof", true);
 
   /* terminate execution after last specified derivation */
   if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->derivations_only) && cl->id >= PROVER9_GLOBAL_SEARCH.HIT_LIST[PROVER9_GLOBAL_SEARCH.hsize - 1])
   {
       cout<<endl<<"*** "<<PROVER9_GLOBAL_SEARCH.hsize<<" derivations completed. Terminating execution."<<endl;
       exit(0);
   }
}



/* *************************************************************************
   BV(2016-feb-17): Support for printing specified clause derivations.

   The three functions,

      get_hit_list
      on_hit_list
      print_derivation

   support printing derivations of selected clauses.  If the
   print_derivation flag is set, the list is read from an external file
   named "hitlist".
   ************************************************************************* */

// Read in a list of clause numbers from file "hitlist".
void Search::get_hit_list(void) {
   FILE *hitlist_fp;

   hitlist_fp = fopen ("hitlist","r");
   
   int nexthit, loc;

   if (!hitlist_fp)
   {
      cout<<"*** ERROR: Unable to open file hitlist. ***"<<endl;
      exit(0);
   }

   cout <<endl<<"`*** Specified derivaitons: ";
   

   PROVER9_GLOBAL_SEARCH.hsize = 0;
   loc = 0;
   
   while(PROVER9_GLOBAL_SEARCH.hsize < MAX_HSIZE && fscanf(hitlist_fp, "%d", &nexthit) > 0)
   {
      loc = PROVER9_GLOBAL_SEARCH.hsize;
      while (loc > 0 && nexthit < PROVER9_GLOBAL_SEARCH.HIT_LIST[loc-1])
      {
         PROVER9_GLOBAL_SEARCH.HIT_LIST[loc] = PROVER9_GLOBAL_SEARCH.HIT_LIST[loc-1];
         loc-- ;
      }
      PROVER9_GLOBAL_SEARCH.HIT_LIST[loc] = nexthit;

      PROVER9_GLOBAL_SEARCH.hsize++ ;
   }

   for (loc = 0; loc < PROVER9_GLOBAL_SEARCH.hsize; loc++) cout << PROVER9_GLOBAL_SEARCH.HIT_LIST[loc];
   cout <<" ("<<PROVER9_GLOBAL_SEARCH.hsize<<" derivations, last clause: "<<PROVER9_GLOBAL_SEARCH.HIT_LIST[PROVER9_GLOBAL_SEARCH.hsize-1]<<")***"<<endl<<endl;
   if (PROVER9_GLOBAL_SEARCH.hsize >= MAX_HSIZE)
       cout <<"*** WARNINH: Hit list truncated to max size "<<MAX_HSIZE<<". ***"<<endl;
   fclose(hitlist_fp);
}



Prover_results Search::search(Prover_input p) {

  
  PredElim PE;
  ClistContainer CL;
  

  
  
  int return_code = setjmp(PROVER9_GLOBAL_SEARCH.Jump_env);
  if (return_code != 0) {
    // we just landed from longjmp(); fix return code and return
    banner::print_separator(cout, "end of search", true);
    Glob.return_code = (return_code == INT_MAX ? 0 : return_code);
    fatal_setjmp();  /* This makes longjmps cause a fatal_error. */
    return collect_prover_results(p->xproofs);
  }
  else  { //search for a proof
       
    PlistContainer PAUX;
    
    banner::print_separator(cout, "PROCESS INITIAL CLAUSES", true);

    PROVER9_GLOBAL_SEARCH.Opt = p->options;          // put options into a global variable
    Glob.initialized = true;   // this signifies that Glob is being used
    Glob.start_time  = myClock::user_seconds();
    Glob.start_ticks = Utilities::bogo_ticks();
    PAUX.set_head(p->sos);
    if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->sort_initial_sos) && PAUX.plist_count() <= 100)
      p->sos = PAUX.sort_plist((OrderType (*) (void*, void*)) Utilities::clause_compare_m4);

    

    
    // Move clauses and term lists into Glob; do not assign IDs to clauses.

    Glob.usable  = ClauseMisc::move_clauses_to_clist(p->usable, "usable", false);
    Glob.sos     = ClauseMisc::move_clauses_to_clist(p->sos, "sos", false);
    Glob.demods  = ClauseMisc::move_clauses_to_clist(p->demods,"demodulators",false);
    Glob.hints   = ClauseMisc::move_clauses_to_clist(p->hints, "hints", false);

    Glob.weights          = Tlist::tlist_copy(p->weights);
    Glob.kbo_weights      = Tlist::tlist_copy(p->kbo_weights);
    Glob.actions          = Tlist::tlist_copy(p->actions);
    Glob.interps          = Tlist::tlist_copy(p->interps);
    Glob.given_selection  = Tlist::tlist_copy(p->given_selection);
    Glob.keep_rules       = Tlist::tlist_copy(p->keep_rules);
    Glob.delete_rules     = Tlist::tlist_copy(p->delete_rules);
    
    Glob.limbo    = CL.clist_init("limbo");
    Glob.disabled = CL.clist_init("disabled");
    Glob.empties  = NULL;
    
     if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_initial_clauses)) {
      
      cout<<endl<<"% Clauses before input processing:"<<endl;
      Ioutil::fwrite_clause_clist(cout, Glob.usable,  (int) Clause_print_format::CL_FORM_STD);
      Ioutil::fwrite_clause_clist(cout, Glob.sos,     (int) Clause_print_format::CL_FORM_STD);
      Ioutil::fwrite_clause_clist(cout, Glob.demods,  (int) Clause_print_format::CL_FORM_STD);
     
      if (Glob.hints->length > 0)
        cout<<endl<<"% "<< Glob.hints->length<<" hints input."<<endl;
          
    }
    
    
    
      if (LADR_GLOBAL_OPTIONS.flag(p->options->predicate_elim) && CL.clist_empty(Glob.usable)) {
        banner::print_separator(cout, "PREDICATE ELIMINATION", true);
        PE.predicate_elimination(Glob.sos, Glob.disabled, !LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet));
        banner::print_separator(cout, "end predicate elimination", true);
     }
    
    
      basic_clause_properties(Glob.sos, Glob.usable);
      
      if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->auto_denials))
        auto_denials(Glob.sos, Glob.usable, PROVER9_GLOBAL_SEARCH.Opt);
    
      
      init_search();  // init clocks, ordering, auto-mode, init packages
    
      
     // BV(2016-feb-17): Read file "hitlist" of clause numbers.
     if (LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_derivations))   get_hit_list();
      
      index_and_process_initial_clauses();
      
      
      if(LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->print_processed_hints)) {
          banner::print_separator(cout, "PROCESSED HINTS", true);
          // BV(2017-nov-12):  for print_processed hints
          // old: fwrite_clause_clist(stdout, Glob.hints,  CL_FORM_BARE);
          Hints::flag_indexed_hints();       // mark "used"
          Ioutil::print_flagged_hints(cout, Glob.hints,  (int) Clause_print_format::CL_FORM_BARE);
          // fwrite_cl_clist_used(stdout, Glob.hints,  CL_FORM_BARE);
          banner::print_separator(cout, "end of processed hints", true);
          
      }

      banner::print_separator(cout, "SEARCH", true);

      

      cout<<endl<<"% Starting search at "<< myClock::user_seconds()<< " seconds."<<endl;
      cout.flush();
      Glob.start_time = myClock::user_seconds();
      Glob.searching = true;
      
     
      
      while (inferences_to_make()) {
      
        // make_inferences: each inferred clause is cl_processed, which
        // does forward demodulation and subsumption; if the clause is kept
        // it is put on the Limbo list, and it is indexed so that it can be
        // used immediately with subsequent newly inferred clauses.
        make_inferences();
        // limbo_process: this applies back subsumption, back demodulation,
        // and other operations that can disable clauses.  Limbo clauses
        // are moved to the Sos list.
        limbo_process(false);
     }
      
  
    fprint_all_stats(cout, (PROVER9_GLOBAL_SEARCH.Opt ? LADR_GLOBAL_OPTIONS.stringparm1(PROVER9_GLOBAL_SEARCH.Opt->stats) : "lots"));
    banner::print_separator(cout, "end of search", true);
    fatal_setjmp();  /* This makes longjmps cause a fatal_error. */
    Glob.return_code = (int) exitCodes::SOS_EMPTY_EXIT;
    
 
    
    
    return collect_prover_results(p->xproofs);  
      
 }

}






Prover_options Search::init_prover_options() {

  Prover_options p = (Prover_options) calloc(1, sizeof(struct prover_options));  

  p->binary_resolution      = LADR_GLOBAL_OPTIONS.init_flag("binary_resolution",      false);
  p->neg_binary_resolution  = LADR_GLOBAL_OPTIONS.init_flag("neg_binary_resolution",  false);
  p->hyper_resolution       = LADR_GLOBAL_OPTIONS.init_flag("hyper_resolution",       false);
  p->pos_hyper_resolution   = LADR_GLOBAL_OPTIONS.init_flag("pos_hyper_resolution",   false);
  p->neg_hyper_resolution   = LADR_GLOBAL_OPTIONS.init_flag("neg_hyper_resolution",   false);
  p->ur_resolution          = LADR_GLOBAL_OPTIONS.init_flag("ur_resolution",          false);
  p->pos_ur_resolution      = LADR_GLOBAL_OPTIONS.init_flag("pos_ur_resolution",      false);
  p->neg_ur_resolution      = LADR_GLOBAL_OPTIONS.init_flag("neg_ur_resolution",      false);
  p->paramodulation         = LADR_GLOBAL_OPTIONS.init_flag("paramodulation",         false);
  p->eval_rewrite           = LADR_GLOBAL_OPTIONS.init_flag("eval_rewrite",           false);

  p->ordered_res            = LADR_GLOBAL_OPTIONS.init_flag("ordered_res",            true);
  p->check_res_instances    = LADR_GLOBAL_OPTIONS.init_flag("check_res_instances",    false);
  p->ordered_para           = LADR_GLOBAL_OPTIONS.init_flag("ordered_para",           true);
  p->check_para_instances   = LADR_GLOBAL_OPTIONS.init_flag("check_para_instances",   false);
  p->para_units_only        = LADR_GLOBAL_OPTIONS.init_flag("para_units_only",        false);
  p->para_from_vars         = LADR_GLOBAL_OPTIONS.init_flag("para_from_vars",         true);
  p->para_into_vars         = LADR_GLOBAL_OPTIONS.init_flag("para_into_vars",         false);
  p->para_from_small        = LADR_GLOBAL_OPTIONS.init_flag("para_from_small",        false);
  p->basic_paramodulation   = LADR_GLOBAL_OPTIONS.init_flag("basic_paramodulation",   false);
  p->initial_nuclei         = LADR_GLOBAL_OPTIONS.init_flag("initial_nuclei",         false);

  p->process_initial_sos    = LADR_GLOBAL_OPTIONS.init_flag("process_initial_sos",     true);
  p->back_demod             = LADR_GLOBAL_OPTIONS.init_flag("back_demod",              true);
  p->lex_dep_demod          = LADR_GLOBAL_OPTIONS.init_flag("lex_dep_demod",           true);
  p->lex_dep_demod_sane     = LADR_GLOBAL_OPTIONS.init_flag("lex_dep_demod_sane",      true);
  p->safe_unit_conflict     = LADR_GLOBAL_OPTIONS.init_flag("safe_unit_conflict",     false);
  p->reuse_denials          = LADR_GLOBAL_OPTIONS.init_flag("reuse_denials",          false);
  p->back_subsume           = LADR_GLOBAL_OPTIONS.init_flag("back_subsume",            true);
  p->unit_deletion          = LADR_GLOBAL_OPTIONS.init_flag("unit_deletion",          false);
  p->factor                 = LADR_GLOBAL_OPTIONS.init_flag("factor",                 false);
  p->cac_redundancy         = LADR_GLOBAL_OPTIONS.init_flag("cac_redundancy",          true);
  p->degrade_hints          = LADR_GLOBAL_OPTIONS.init_flag("degrade_hints",           true);
  p->limit_hint_matchers    = LADR_GLOBAL_OPTIONS.init_flag("limit_hint_matchers",    false);
  p->back_demod_hints       = LADR_GLOBAL_OPTIONS.init_flag("back_demod_hints",        true);
  p->collect_hint_labels    = LADR_GLOBAL_OPTIONS.init_flag("collect_hint_labels",    false);
  p->dont_flip_input        = LADR_GLOBAL_OPTIONS.init_flag("dont_flip_input",        false);

  p->echo_input             = LADR_GLOBAL_OPTIONS.init_flag("echo_input",              true);
  p->bell                   = LADR_GLOBAL_OPTIONS.init_flag("bell",                    true);
  p->quiet                  = LADR_GLOBAL_OPTIONS.init_flag("quiet",                  false);
  p->print_initial_clauses  = LADR_GLOBAL_OPTIONS.init_flag("print_initial_clauses",   true);
  p->print_given            = LADR_GLOBAL_OPTIONS.init_flag("print_given",             true);
  p->print_gen              = LADR_GLOBAL_OPTIONS.init_flag("print_gen",              false);
  p->print_kept             = LADR_GLOBAL_OPTIONS.init_flag("print_kept",             false);
  p->print_labeled          = LADR_GLOBAL_OPTIONS.init_flag("print_labeled",          false);
  p->print_proofs           = LADR_GLOBAL_OPTIONS.init_flag("print_proofs",            true);
  
    // BV(2015-jan-31): flag(print_matched_hints) default false
  p->print_matched_hints    = LADR_GLOBAL_OPTIONS.init_flag("print_matched_hints",    false);
  
  // BV(2016-aug-25): flag(print_derivations) default false
  p->print_derivations      = LADR_GLOBAL_OPTIONS.init_flag("print_derivations",      false);
  
  // BV(2016-aug-25): flag(derivations_only) default true
  p->derivations_only       =   LADR_GLOBAL_OPTIONS.init_flag("derivations_only",      true);
  
  p->print_processed_hints  = LADR_GLOBAL_OPTIONS.init_flag("print_processed_hints",  false);

  
  
  p->default_output         = LADR_GLOBAL_OPTIONS.init_flag("default_output",          true);
  
  
  
  p->print_clause_properties= LADR_GLOBAL_OPTIONS.init_flag("print_clause_properties",false);

  p->expand_relational_defs = LADR_GLOBAL_OPTIONS.init_flag("expand_relational_defs", false);
  p->predicate_elim         = LADR_GLOBAL_OPTIONS.init_flag("predicate_elim",          true);
  p->inverse_order          = LADR_GLOBAL_OPTIONS.init_flag("inverse_order",           true);
  p->sort_initial_sos       = LADR_GLOBAL_OPTIONS.init_flag("sort_initial_sos",       false);
  p->restrict_denials       = LADR_GLOBAL_OPTIONS.init_flag("restrict_denials",       false);

  p->input_sos_first        = LADR_GLOBAL_OPTIONS.init_flag("input_sos_first",         true);
  p->breadth_first          = LADR_GLOBAL_OPTIONS.init_flag("breadth_first",          false);
  p->lightest_first         = LADR_GLOBAL_OPTIONS.init_flag("lightest_first",         false);
  p->random_given           = LADR_GLOBAL_OPTIONS.init_flag("random_given",           false);
  p->breadth_first_hints    = LADR_GLOBAL_OPTIONS.init_flag("breadth_first_hints",    false);
  p->default_parts          = LADR_GLOBAL_OPTIONS.init_flag("default_parts",           true);

  p->automatic              = LADR_GLOBAL_OPTIONS.init_flag("auto",                    true);
  p->auto_setup             = LADR_GLOBAL_OPTIONS.init_flag("auto_setup",              true);
  p->auto_limits            = LADR_GLOBAL_OPTIONS.init_flag("auto_limits",             true);
  p->auto_denials           = LADR_GLOBAL_OPTIONS.init_flag("auto_denials",            true);
  p->auto_inference         = LADR_GLOBAL_OPTIONS.init_flag("auto_inference",          true);
  p->auto_process           = LADR_GLOBAL_OPTIONS.init_flag("auto_process",            true);
  p->auto2                  = LADR_GLOBAL_OPTIONS.init_flag("auto2",                  false);
  p->raw                    = LADR_GLOBAL_OPTIONS.init_flag("raw",                    false);
  p->production             = LADR_GLOBAL_OPTIONS.init_flag("production",             false);

  p->lex_order_vars         = LADR_GLOBAL_OPTIONS.init_flag("lex_order_vars",         false);
    
  
  p->max_given =        LADR_GLOBAL_OPTIONS.init_parm("max_given",            -1,     -1,  INT_MAX);
  p->max_kept =         LADR_GLOBAL_OPTIONS.init_parm("max_kept",             -1,     -1,  INT_MAX);
  p->max_proofs =       LADR_GLOBAL_OPTIONS.init_parm("max_proofs",            1,     -1,  INT_MAX);
  p->max_megs =         LADR_GLOBAL_OPTIONS.init_parm("max_megs",            500,     -1,  INT_MAX);
  p->max_seconds =      LADR_GLOBAL_OPTIONS.init_parm("max_seconds",          -1,     -1,  INT_MAX);
  p->max_minutes =      LADR_GLOBAL_OPTIONS.init_parm("max_minutes",          -1,     -1,  INT_MAX);
  p->max_hours =        LADR_GLOBAL_OPTIONS.init_parm("max_hours",            -1,     -1,  INT_MAX);
  p->max_days =         LADR_GLOBAL_OPTIONS.init_parm("max_days",             -1,     -1,  INT_MAX);  
  p->max_nohints =      LADR_GLOBAL_OPTIONS.init_parm("max_nohints",          -1,     -1,  INT_MAX);
  p->degrade_limit =    LADR_GLOBAL_OPTIONS.init_parm("degrade_limit",         0,     -1,  INT_MAX);
  
  // BV(2017-apr-04): parm hint_derivations, default 0
  p->hint_derivations = LADR_GLOBAL_OPTIONS.init_parm("hint_derivations",      0,      0,  INT_MAX);

  
  p->new_constants =    LADR_GLOBAL_OPTIONS.init_parm("new_constants",         0,     -1,  INT_MAX);
  p->para_lit_limit =   LADR_GLOBAL_OPTIONS.init_parm("para_lit_limit",       -1,     -1,  INT_MAX);
  p->ur_nucleus_limit = LADR_GLOBAL_OPTIONS.init_parm("ur_nucleus_limit",     -1,     -1,  INT_MAX);

  p->fold_denial_max =  LADR_GLOBAL_OPTIONS.init_parm("fold_denial_max",       0,     -1,  INT_MAX);

  p->pick_given_ratio  = LADR_GLOBAL_OPTIONS.init_parm("pick_given_ratio",    -1,     -1,  INT_MAX);
  p->hints_part        = LADR_GLOBAL_OPTIONS.init_parm("hints_part",       INT_MAX,    0,  INT_MAX);
  p->age_part          = LADR_GLOBAL_OPTIONS.init_parm("age_part",             1,      0,  INT_MAX);
  p->weight_part       = LADR_GLOBAL_OPTIONS.init_parm("weight_part",          0,      0,  INT_MAX);
  p->false_part        = LADR_GLOBAL_OPTIONS.init_parm("false_part",           4,      0,  INT_MAX);
  p->true_part         = LADR_GLOBAL_OPTIONS.init_parm("true_part",            4,      0,  INT_MAX);
  p->random_part       = LADR_GLOBAL_OPTIONS.init_parm("random_part",          0,      0,  INT_MAX);
  p->random_seed       = LADR_GLOBAL_OPTIONS.init_parm("random_seed",          0,     -1,  INT_MAX);
  p->eval_limit        = LADR_GLOBAL_OPTIONS.init_parm("eval_limit",        1024,     -1,  INT_MAX);
  p->eval_var_limit    = LADR_GLOBAL_OPTIONS.init_parm("eval_var_limit",      -1,     -1,  INT_MAX);

  p->max_depth =        LADR_GLOBAL_OPTIONS.init_parm("max_depth",            -1,     -1,  INT_MAX);
  p->lex_dep_demod_lim =LADR_GLOBAL_OPTIONS.init_parm("lex_dep_demod_lim",    11,     -1,  INT_MAX);
  p->max_literals =     LADR_GLOBAL_OPTIONS.init_parm("max_literals",         -1,     -1,  INT_MAX);
  p->max_vars =         LADR_GLOBAL_OPTIONS.init_parm("max_vars",             -1,     -1,  INT_MAX);
  p->demod_step_limit = LADR_GLOBAL_OPTIONS.init_parm("demod_step_limit",   1000,     -1,  INT_MAX);
  p->demod_increase_limit = LADR_GLOBAL_OPTIONS.init_parm("demod_increase_limit",1000,-1,  INT_MAX);
  p->backsub_check    = LADR_GLOBAL_OPTIONS.init_parm("backsub_check",       500,     -1,  INT_MAX);

  
  p->constant_weight =  LADR_GLOBAL_OPTIONS.init_floatparm("constant_weight",       1.0,-DBL_LARGE, DBL_LARGE);
  p->not_weight =       LADR_GLOBAL_OPTIONS.init_floatparm("not_weight",            0.0,-DBL_LARGE, DBL_LARGE);
  p->or_weight =        LADR_GLOBAL_OPTIONS.init_floatparm("or_weight",             0.0,-DBL_LARGE, DBL_LARGE);
  p->sk_constant_weight=LADR_GLOBAL_OPTIONS.init_floatparm("sk_constant_weight",    1.0,-DBL_LARGE, DBL_LARGE);
  p->prop_atom_weight = LADR_GLOBAL_OPTIONS.init_floatparm("prop_atom_weight",      1.0,-DBL_LARGE, DBL_LARGE);
  p->nest_penalty =     LADR_GLOBAL_OPTIONS.init_floatparm("nest_penalty",          0.0,     0.0,   DBL_LARGE);
  p->depth_penalty =    LADR_GLOBAL_OPTIONS.init_floatparm("depth_penalty",         0.0,-DBL_LARGE, DBL_LARGE);
  p->var_penalty =      LADR_GLOBAL_OPTIONS.init_floatparm("var_penalty",           0.0,-DBL_LARGE, DBL_LARGE);
  p->default_weight =   LADR_GLOBAL_OPTIONS.init_floatparm("default_weight",  DBL_LARGE,-DBL_LARGE, DBL_LARGE);
  p->complexity =       LADR_GLOBAL_OPTIONS.init_floatparm("complexity",            0.0,-DBL_LARGE, DBL_LARGE);

  p->sos_limit =        LADR_GLOBAL_OPTIONS.init_parm("sos_limit",         20000,     -1,INT_MAX);
  p->sos_keep_factor =  LADR_GLOBAL_OPTIONS.init_parm("sos_keep_factor",       3,      2,10);
  p->min_sos_limit =    LADR_GLOBAL_OPTIONS.init_parm("min_sos_limit",         0,      0,INT_MAX);
  p->lrs_interval =     LADR_GLOBAL_OPTIONS.init_parm("lrs_interval",         50,      1,INT_MAX);
  p->lrs_ticks =        LADR_GLOBAL_OPTIONS.init_parm("lrs_ticks",            -1,     -1,INT_MAX);

  p->report =           LADR_GLOBAL_OPTIONS.init_parm("report",               -1,     -1,INT_MAX);
  p->report_stderr =    LADR_GLOBAL_OPTIONS.init_parm("report_stderr",        -1,     -1,INT_MAX);
  
  p->max_weight =       LADR_GLOBAL_OPTIONS.init_floatparm("max_weight",  100.0, -DBL_LARGE, DBL_LARGE);

  
  
  
  p->order = LADR_GLOBAL_OPTIONS.init_stringparm("order", 3,string("lpo"),string("rpo"),string("kbo"));

  p->eq_defs = LADR_GLOBAL_OPTIONS.init_stringparm("eq_defs", 3,string("unfold"),string("fold"),string("pass"));

  p->literal_selection = LADR_GLOBAL_OPTIONS.init_stringparm("literal_selection", 3,string("max_negative"), string("all_negative"), string("none"));

  p->stats = LADR_GLOBAL_OPTIONS.init_stringparm("stats", 4, string("lots"),string("some"),string("all"),string("none"));

  p->multiple_interps = LADR_GLOBAL_OPTIONS.init_stringparm("multiple_interps", 2,string("false_in_all"),string("false_in_some"));

  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->max_minutes, p->max_seconds,         60, true);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->max_hours,   p->max_seconds,       3600, true);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->max_days,    p->max_seconds,      86400, true);


  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->para_units_only,    true,  p->para_lit_limit,      1);
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->para_units_only,   false, p->para_lit_limit, LADR_GLOBAL_OPTIONS.getParms());

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->hyper_resolution, true,  p->pos_hyper_resolution, true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->hyper_resolution, false, p->pos_hyper_resolution, false);


  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->ur_resolution, true,  p->pos_ur_resolution, true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->ur_resolution, true,  p->neg_ur_resolution, true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->ur_resolution, false,  p->pos_ur_resolution, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->ur_resolution, false,  p->neg_ur_resolution, false);

  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lex_dep_demod, false, p->lex_dep_demod_lim, 0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lex_dep_demod,  true, p->lex_dep_demod_lim, 11);

    

  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->pick_given_ratio, p->age_part,          1, false);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->pick_given_ratio, p->weight_part,       1,  true);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->pick_given_ratio, p->false_part,        0, false);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->pick_given_ratio, p->true_part,         0, false);
  LADR_GLOBAL_OPTIONS.parm_parm_dependency(p->pick_given_ratio, p->random_part,       0, false);

  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lightest_first,    true,  p->weight_part,     1);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lightest_first,    true,  p->age_part,        0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lightest_first,    true,  p->false_part,      0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lightest_first,    true,  p->true_part,       0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->lightest_first,    true,  p->random_part,     0);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->lightest_first,   false,  p->default_parts, true);
  
  // BV(2016-aug-25): flag(derivations_only) default true
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output,   true, p->print_derivations,   false);
  
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->random_given,    true,  p->weight_part,     0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->random_given,    true,  p->age_part,        0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->random_given,    true,  p->false_part,      0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->random_given,    true,  p->true_part,       0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->random_given,    true,  p->random_part,     1);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->random_given,   false,  p->default_parts, true);

  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->breadth_first,    true,  p->age_part,        1);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->breadth_first,    true,  p->weight_part,     0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->breadth_first,    true,  p->false_part,      0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->breadth_first,    true,  p->true_part,       0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->breadth_first,    true,  p->random_part,     0);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->breadth_first,    false, p->default_parts, true);

  
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    true,  p->age_part,          1);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    true,  p->weight_part,       0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    true,  p->false_part,        4);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    true,  p->true_part,         4);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    true,  p->random_part,       0);

  
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    false,  p->age_part,         0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    false,  p->weight_part,      0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    false,  p->false_part,       0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    false,  p->true_part,        0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->default_parts,    false,  p->random_part,      0);

  

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->quiet,               false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->echo_input,           true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_initial_clauses,true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_given,          true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_proofs,         true);
  
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_matched_hints,   false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_processed_hints, false);
  
  
  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(p->default_output, true, p->stats,        string("lots"));

  
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_kept,          false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->default_output, true, p->print_gen,           false);

   

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto_setup,  true, p->predicate_elim,    true);
  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(p->auto_setup, true, p->eq_defs,    "unfold");

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto_setup,  false, p->predicate_elim,    false);
  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(p->auto_setup, false, p->eq_defs,   "pass");



  LADR_GLOBAL_OPTIONS.flag_floatparm_dependency(p->auto_limits,  true, p->max_weight,    100.0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto_limits,       true, p->sos_limit,     20000);

  LADR_GLOBAL_OPTIONS.flag_floatparm_dependency(p->auto_limits, false, p->max_weight, DBL_LARGE);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto_limits,      false, p->sos_limit,         -1);

    

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       true, p->auto_inference,     true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       true, p->auto_setup,         true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       true, p->auto_limits,        true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       true, p->auto_denials,       true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       true, p->auto_process,       true);

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       false, p->auto_inference,    false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       false, p->auto_setup,        false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       false, p->auto_limits,       false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       false, p->auto_denials,      false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->automatic,       false, p->auto_process,      false);

   

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto2, true, p->automatic,                 true);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->new_constants,            1);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->fold_denial_max,          3);
  LADR_GLOBAL_OPTIONS.flag_floatparm_dependency(p->auto2, true, p->max_weight,      200.0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->nest_penalty,             1);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->sk_constant_weight,       0);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->prop_atom_weight,         5);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto2, true, p->sort_initial_sos,       true);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->sos_limit,                -1);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->lrs_ticks,              3000);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->auto2, true, p->max_megs,                400);
  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(p->auto2, true, p->stats,          "some");
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto2, true, p->echo_input,            false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto2, true, p->quiet,                  true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto2, true, p->print_initial_clauses, false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->auto2, true, p->print_given,           false);

  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->auto2, false, p->automatic);
  
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->new_constants, LADR_GLOBAL_OPTIONS.getParms());
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->fold_denial_max,LADR_GLOBAL_OPTIONS.getParms());
  
  LADR_GLOBAL_OPTIONS.flag_floatparm_dep_default(p->auto2, false, p->max_weight, LADR_GLOBAL_OPTIONS.getFloatParms());
  
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->nest_penalty, LADR_GLOBAL_OPTIONS.getParms());
  
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->sk_constant_weight, LADR_GLOBAL_OPTIONS.getParms());
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->prop_atom_weight, LADR_GLOBAL_OPTIONS.getParms());
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->auto2, false, p->sort_initial_sos);
  
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->sos_limit,LADR_GLOBAL_OPTIONS.getParms());
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->lrs_ticks,LADR_GLOBAL_OPTIONS.getParms());
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->auto2, false, p->max_megs,LADR_GLOBAL_OPTIONS.getParms());
  
  LADR_GLOBAL_OPTIONS.flag_stringparm_dep_default(p->auto2, false, p->stats, LADR_GLOBAL_OPTIONS.getStringParms());
  
  
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->auto2, false, p->echo_input);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->auto2, false, p->quiet);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->auto2, false, p->print_initial_clauses);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->auto2, false, p->print_given);



  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->automatic,           false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->ordered_res,         false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->ordered_para,        false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->para_into_vars,      true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->para_from_small,     true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->ordered_para,        false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->back_demod,          false);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->cac_redundancy,      false);
  LADR_GLOBAL_OPTIONS.flag_parm_dependency(p->raw, true, p->backsub_check,     INT_MAX);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->raw, true, p->lightest_first,       true);
  LADR_GLOBAL_OPTIONS.flag_stringparm_dependency(p->raw, true, p->literal_selection, "none");
    
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->automatic);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->ordered_res);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->ordered_para);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->para_into_vars);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->para_from_small);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->ordered_para);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->back_demod);
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->cac_redundancy);
  LADR_GLOBAL_OPTIONS.flag_parm_dep_default(p->raw, false, p->backsub_check, LADR_GLOBAL_OPTIONS.getParms());
  LADR_GLOBAL_OPTIONS.flag_flag_dep_default(p->raw, false, p->lightest_first);
  LADR_GLOBAL_OPTIONS.flag_stringparm_dep_default(p->raw, false, p->literal_selection, LADR_GLOBAL_OPTIONS.getStringParms());

   

  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->production,   true,  p->raw,               true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->production,   true,  p->eval_rewrite,      true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->production,   true,  p->hyper_resolution,  true);
  LADR_GLOBAL_OPTIONS.flag_flag_dependency(p->production,   true,  p->back_subsume,     false);
  
  
  
  return p;
}





void Search::fprint_prover_stats(ostream &fp, prover_stats s, string stats_level) {
  
  fp<<endl<<"Given="<<s.given<<". Generated="<<s.generated<<". Kept="<<s.kept<<". proofs="<<s.proofs<<"."<<endl;
  
  fp<<"Usable="<<s.usable_size<<". Sos="<<s.sos_size<<". Demods="<<s.demodulators_size<<". Limbo="<<s.limbo_size<<". Disabled="<<s.disabled_size<<". Hints="<<s.hints_size<<"."<<endl;
     
  
  if (myString::str_ident(stats_level, "lots") || myString::str_ident(stats_level, "all")) {

    fp<<"Kept_by_rule="<<s.kept_by_rule<<". Deleted_by_rule="<<s.deleted_by_rule<<"."<<endl;
    fp<<"Forward_subsumed="<<s.subsumed<<". Back_subsumed="<<s.back_subsumed<<"."<<endl;
    fp<<"Sos_limit_deleted="<<s.sos_limit_deleted<<". Sos_displaced="<<s.sos_displaced<<". Sos_removed="<<s.sos_removed<<"."<<endl;

    fp<<"New_demodulators="<<s.new_demodulators<<" ("<<s.new_lex_demods<<" lex), Back_demodulated="<<s.back_demodulated<<". back_unit_deleted="<<s.back_unit_deleted<<"."<<endl;
    
    fp<<"Demod_attempts="<<s.demod_attempts<<". Demod_rewrites="<<s.demod_rewrites<<"."<<endl;
    
    fp<<"Res_instance_prunes="<<s.res_instance_prunes<<". Para_instance_prunes="<<s.para_instance_prunes<<". Basic_paramod_prunes="<<s.basic_para_prunes<<"."<<endl;    
    
    fp<<"Nonunit_fsub_feature_tests="<<s.nonunit_fsub<<". ";
    
    fp<<"Nonunit_bsub_feature_tests="<<s.nonunit_bsub<<"."<<endl;
    
    
  }

  fp<<"Megabytes="<<(s.kbyte_usage /1000.0) << " MB"<<endl; //number of megabytes used
  

#if 1
  fp<< "User_CPU="<<myClock::user_seconds()<<" sec. System_CPU="<<myClock::system_seconds()<<" sec. Wall_clock="<<myClock::wallclock()<<" sec."<<endl;
  
#else  /* some debugging junk */
  {
    double user_sec = user_seconds();
    fprintf(fp,"User_CPU=%.2f, System_CPU=%.2f, Wall_clock=%u, "
	    "Mega_mem_calls/sec=%.2f, "
	    "Mega_next_calls/sec=%.2f, "
	    "Mega_sub_calls/sec=%.2f.\n",
	    user_sec, system_seconds(), wallclock(),
	    user_sec == 0 ? 0.0 : mega_mem_calls() / user_sec,
	    user_sec == 0 ? 0.0 : mega_next_calls() / user_sec,
	    user_sec == 0 ? 0.0 : mega_sub_calls() / user_sec);
  }
#endif
}


void Search::init_prover_attributes(void) {

  AttributeContainer A;
  
  PROVER9_GLOBAL_SEARCH.Att.label            = A.register_attribute("label",            Attribute_type::STRING_ATTRIBUTE);
  PROVER9_GLOBAL_SEARCH.Att.bsub_hint_wt     = A.register_attribute("bsub_hint_wt",     Attribute_type::INT_ATTRIBUTE);
  PROVER9_GLOBAL_SEARCH.Att.answer           = A.register_attribute("answer",           Attribute_type::TERM_ATTRIBUTE);
  PROVER9_GLOBAL_SEARCH.Att.properties       = A.register_attribute("props",            Attribute_type::TERM_ATTRIBUTE);

  PROVER9_GLOBAL_SEARCH.Att.action           = A.register_attribute("action",           Attribute_type::TERM_ATTRIBUTE);
  PROVER9_GLOBAL_SEARCH.Att.action2          = A.register_attribute("action2",          Attribute_type::TERM_ATTRIBUTE);

  A.declare_term_attribute_inheritable(PROVER9_GLOBAL_SEARCH.Att.answer);
  A.declare_term_attribute_inheritable(PROVER9_GLOBAL_SEARCH.Att.action2);
}  


void Search::fprint_prover_clocks(ostream &fp, prover_clocks clks) {
  //myClock CLOCK;
  
  if (myClock::clocks_enabled()) {
    
    fp<<endl;
    CLOCK.set_ClockStructure(clks.pick_given);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.infer);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure( clks.preprocess);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.demod);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.unit_del);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.redundancy);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.conflict);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.weigh);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.hints);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.subsume);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.semantics);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.back_subsume);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.back_demod);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.back_unit_del);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.index);
    CLOCK.fprint_clock(fp);
    
    CLOCK.set_ClockStructure(clks.disable);
    CLOCK.fprint_clock(fp);
  }
} 

void Search::fprint_all_stats(ostream &fp, string stats_level) {
  //GivSelect GS;
  update_stats();

  banner::print_separator(fp, "STATISTICS", true);
  fprint_prover_stats(fp, PROVER9_GLOBAL_SEARCH.Stats, stats_level);
  fprint_prover_clocks(fp, PROVER9_GLOBAL_SEARCH.Clocks);
  if (myString::str_ident(stats_level, "all")) {
   Utilities::print_memory_stats(fp);
   GS.selector_report();
    /* p_sos_dist(); */
  }
  banner::print_separator(fp, "end of statistics", true);
  fp.flush();
}

/* PUBLIC */
int Search::exit_with_message(std::ostream &o, int code)
{
  
  bool error= (code==(int) exitCodes::SIGSEGV_EXIT);

  int proofs = Glob.initialized ? PROVER9_GLOBAL_SEARCH.Stats.proofs : -1;
  
  if (proofs == -1)
    o<<endl<<"Exiting."<<endl;
  
  else if (proofs == 0)
    o<<endl<<"Exiting with failure"<<endl;
  else
      o<<endl<<"Exiting with "<< proofs << " proof"<< (proofs==1 ? "" : "s")  <<endl;  
     
  
    
  if (!PROVER9_GLOBAL_SEARCH.Opt || !LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->quiet)) {
      cerr<<endl<<"------ process "<< nonport::my_process_id()<<" exit ("<<exit_string(code)<<")"<<endl;
      
      
      
      
      if (!PROVER9_GLOBAL_SEARCH.Opt || LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->bell))
            fatal::bell(cerr);
     
  }


  
  
  
  string s="some";
  if (PROVER9_GLOBAL_SEARCH.Opt && LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->report_stderr) > 0)  report(cerr, s);
  
  o<<endl<<"Process "<<nonport::my_process_id()<<" exit("<<exit_string(code)<<") "<<myClock::get_date()<<endl;
  
  if (error) {
      LADR_GLOBAL_MEMORY.Garbage_Collector();
      exit(code);
  }    
  
  
  //call here the memory free for all classes and the garbage collector
  
  LADRV_GLOBAIS_INST.FREE_MEM();
  PROVER9_GLOBAIS_INST.FREE_MEM();
  
  Hints::Free_Mem();
  
  //some Search locals
  if (Glob.usable->name)
    delete (Glob.usable->name);
  
  if (Glob.sos->name)
    delete (Glob.sos->name);
  
  if (Glob.demods->name)
    delete (Glob.demods->name);
  
  if (Glob.hints->name)
    delete (Glob.hints->name);
  
  if (Glob.limbo->name)
    delete (Glob.limbo->name);
  
  if (Glob.disabled->name)
    delete (Glob.disabled->name);

  
  
  if (input->options)
    free(input->options);
  
  if(input)
    free(input);
  
  if(results)
    free(results);
  
  LADR_GLOBAL_MEMORY.Garbage_Collector();
  exit(code);
}  /* exit_with_message */



void Search::free_search_memory(void) {
  
  // Demodulators
  LindexContainer LI;
  IndexLits IL;
  Demodulate D;
  ClistContainer CL;
  while (Glob.demods->first) {
    Topform c = Glob.demods->first->c;
    D.index_demodulator(c, Demod::demodulator_type(c,
					  LADR_GLOBAL_OPTIONS.parm(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_lim),
					  LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->lex_dep_demod_sane)),
                      Indexop::DELETE, PROVER9_GLOBAL_SEARCH.Clocks.index);
    CL.clist_remove(c, Glob.demods);
    if (c->containers == NULL)       ClauseMisc::delete_clause(c);
  }
  CL.clist_free(Glob.demods);
  
  D.destroy_demodulation_index();

  // Lightest

  // zap_sos();

  // Usable, Sos, Limbo

  while (Glob.usable->first)
    disable_clause(Glob.usable->first->c);
  CL.clist_free(Glob.usable);
  Glob.usable = NULL;

  while (Glob.sos->first)
    disable_clause(Glob.sos->first->c);
  CL.clist_free(Glob.sos);
  Glob.sos = NULL;

  while (Glob.limbo->first)
    disable_clause(Glob.limbo->first->c);
  CL.clist_free(Glob.limbo);
  Glob.limbo = NULL;

  
  
  IL.destroy_literals_index();
  D.destroy_back_demod_index();
  LI.lindex_destroy(PROVER9_GLOBAL_SEARCH.Glob.clashable_idx);
  PROVER9_GLOBAL_SEARCH.Glob.clashable_idx = NULL;

  ClauseMisc::delete_clist(Glob.disabled);
  Glob.disabled = NULL;

  if (Glob.hints->first) {
    Clist_pos p;
    for(p = Glob.hints->first; p; p = p->next)
      Hints::unindex_hint(p->c);
    Hints::done_with_hints();
  }
  ClauseMisc::delete_clist(Glob.hints);
  Glob.hints = NULL;

}


/* PUBLIC */
void Search::prochints(Prover_input p) {
    
    
    Clist_pos ptr;
    TopformContainer TF;
    Parautil Pu;
    ClistContainer CL;
    
    // BV(2016-may-27): assign hint ids separate from clause numbers
    static int hint_id_number = 0;

    PROVER9_GLOBAL_SEARCH.Opt = p->options;          // put options into a global variable
    Glob.initialized = true;   // this signifies that Glob is being used

    // Move clauses and term lists into Glob; do not assign IDs to clauses.

    Glob.usable  = ClauseMisc::move_clauses_to_clist(p->usable, "usable", false);
    Glob.sos     = ClauseMisc::move_clauses_to_clist(p->sos, "sos", false);
    Glob.demods  = ClauseMisc::move_clauses_to_clist(p->demods,"demodulators",false);
    Glob.hints   = ClauseMisc::move_clauses_to_clist(p->hints, "hints", false);

    Glob.weights          = Tlist::tlist_copy(p->weights);
    Glob.kbo_weights      = Tlist::tlist_copy(p->kbo_weights);
    Glob.actions          = Tlist::tlist_copy(p->actions);
    Glob.interps          = Tlist::tlist_copy(p->interps);
    Glob.given_selection  = Tlist::tlist_copy(p->given_selection);
    Glob.keep_rules       = Tlist::tlist_copy(p->keep_rules);
    Glob.delete_rules     = Tlist::tlist_copy(p->delete_rules);

    // Allocate auxiliary clause lists. 

    Glob.limbo    = CL.clist_init("limbo");
    Glob.disabled = CL.clist_init("disabled");
    Glob.empties  = NULL;

    if (Glob.hints->length > 0)
	cout<<endl<<"% "<<Glob.hints->length <<" hints input."<<endl;
    init_search();  // init clocks, ordering, auto-mode, init packages
    
    // *******************************************************************
    // The remainder of the code is the prochints version of function
    // index_and_process_initial_clauses();
    // *******************************************************************

    Hints::init_hints(
                        Uniftype::ORDINARY_UNIF, PROVER9_GLOBAL_SEARCH.Att.bsub_hint_wt,
                        LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->collect_hint_labels),
                        LADR_GLOBAL_OPTIONS.flag(PROVER9_GLOBAL_SEARCH.Opt->back_demod_hints),
                        Demodulate::demodulate_clause)
    ;

    banner::print_separator(cout, "PROCESSED HINTS", true);
    cout <<endl << "formulas(hints).   %% processed input hints"<<endl;
    
  
  if (Glob.hints->first) {
    for (ptr = Glob.hints->first; ptr != NULL; ptr = ptr->next) {
      Topform h = ptr->c;

      // BV(2016-may-27): hint id is "hint_age" (input order)
      // assign_clause_id(h);  // This should be optional
      h->id = hint_id_number;
      hint_id_number++ ;

      Pu.orient_equalities(h, true);
      TF.renumber_variables(h, MAX_VARS);

      // BV(2016-aug-19): added argument doprint
      Hints::index_hint(h,true);
    }
  }
   cout << "end_of_list.   %% end of processed input hints"<<endl;
   

   /* summary stats */
   if (Glob.hints->length > 0)
   {
      int redundant = Hints::redundant_hints();
      cout <<endl << "%% "<< Glob.hints->length - redundant<< " hints (" << Glob.hints->length << " processed, "<< redundant << " redundant)."<<endl;
   }

   /* terminate the program after input hints processing */
   exit(0);
}



void Search::destroy_prover_input(Prover_input input) {
    return; //nothing to do
}
