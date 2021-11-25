#include "./ladr/ladrvglobais.h"
#include "./ladr/definitions.h"
#include "./ladr/fatal.h"
#include "./ladr/banner.h"
#include "./ladr/ioutil.h"
#include "./ladr/options.h"

#include "provers.h"
#include "search.h"
#include "stdlib.h"
#include "unistd.h"
#include <signal.h>
#include <iostream>





Targ_options::Targ_options()
{
    ag.parenthesize_output=false;
    ag.auto2=false;
    ag.max_seconds=INT32_MAX;
    ag.files=false;
    ag.strings=false;
}



Targ_options::~Targ_options()
{

    
}


struct arg_options Targ_options::get_arg_options() {

    return ag;
}

void Targ_options::get_command_line_args(int argc, char *argv[]) {

        int c;
        while ((c = getopt(argc, argv,":hapxt:fs")) != EOF) {
            switch (c) {
            case 'x':
            ag.auto2 = true;
            break;
            case 'p':
            ag.parenthesize_output = true;
            break;
            case 't':
                if(optarg!=NULL)
                    ag.max_seconds = atoi(optarg);
            break;
            case 'f':  /* input files */
            ag.files = true;
            break;
            
            case 's':   
                        ag.strings=true;
                        break;
            
            case 'h':
            default:
            cout<<Help_string<<endl;
            exit(1);
            break;
            }
        }
        if (false) 
             for (int i = 0; i < argc; i++) cout<<"arg:"<<argv[i]<<endl;
}




void Provers::prover_sig_handler(int condition) {
  Search S;
  cout<<endl<<"Prover catching signal "<<condition<<endl;
  switch (condition) {
  case SIGSEGV:
    S.fprint_all_stats(cout, "all");
    S.exit_with_message(cout, (int) exitCodes::SIGSEGV_EXIT);
    break;
  case SIGINT:
    S.fprint_all_stats(cout, "all");
    S.exit_with_message(cout, (int) exitCodes::SIGINT_EXIT);
    break;
  case SIGUSR1:
    S.report(cout, "");
    S.report(cerr, "");
    break;
  default: fatal::fatal_error("prover_sig_handler, unknown signal");
  }
} 


void Provers::process_command_line_args_1(struct arg_options command_opt, Prover_options prover_opt) {
   if (command_opt.auto2) {
    cout<<endl<<"%% From the command line: set(auto2)."<<endl;
    LADR_GLOBAL_OPTIONS.set_flag(prover_opt->auto2, true);
  }

  if (command_opt.parenthesize_output) {
    ParseContainer P;
    cout<<endl<<"%% From the command line: parenthesize output."<<endl;
    P.parenthesize(true);  /* tell the parsing/printing package */
  }

} 

void Provers::max_megs_exit(void) {
  Search S;  
  S.fprint_all_stats(cout, "all");
  S.exit_with_message(cout, (int) exitCodes::MAX_MEGS_EXIT);
}


void Provers::process_command_line_args_2(struct arg_options command_opt, Prover_options prover_opt) {
  if (command_opt.max_seconds != INT_MAX) {
    int n = command_opt.max_seconds;
    int id = prover_opt->max_seconds;
    cout<<endl<<"%% From the command line: assign("<<LADR_GLOBAL_OPTIONS.parm_id_to_str(id)<<","<<n<<")"<<endl;
    LADR_GLOBAL_OPTIONS.assign_parm(id, n, true);
  }

  Memory::set_max_megs(LADR_GLOBAL_OPTIONS.parm(prover_opt->max_megs));
  Memory::set_max_megs_proc(max_megs_exit);

} 

Prover_input Provers::std_prover_int_and_input(int argc, char *argv[], bool clausify, bool echo,  unknown_actions unknown_action) {
  
  DefinitionsContainer D;
  ClauseidContainer CI;
  JustContainer J;
  PlistContainer P;
  TopformContainer TF;
  Targ_options opts;
  Search S;
  
  Prover_input  pi = (Prover_input) calloc (1,sizeof(struct prover_input)); //creates a Prover_input structure....must be released
  
  opts.get_command_line_args(argc, argv);
  
 
  LADR_GLOBAL_TOP_INPUT.init_standard_ladr();  
 
  
  pi->options=S.init_prover_options();  //this function creates pi->options....must be released
  
  
  S.init_prover_attributes(); //prover attributes


  signal(SIGINT,  prover_sig_handler); 
  signal(SIGUSR1, prover_sig_handler);
  signal(SIGSEGV, prover_sig_handler);
  
 
 // Tell the top_input package what lists to accept and where to put them.

  LADR_GLOBAL_TOP_INPUT.accept_list("sos",              (int)  Top_input_type::FORMULAS, false, &(pi->sos));  //create Readlists
  LADR_GLOBAL_TOP_INPUT.accept_list("assumptions",      (int)  Top_input_type::FORMULAS, false, &(pi->sos));  // Synonym for sos
  LADR_GLOBAL_TOP_INPUT.accept_list("goals",            (int)  Top_input_type::FORMULAS, false, &(pi->goals));
  LADR_GLOBAL_TOP_INPUT.accept_list("usable",           (int)  Top_input_type::FORMULAS, false, &(pi->usable));
  LADR_GLOBAL_TOP_INPUT.accept_list("demodulators",     (int)  Top_input_type::FORMULAS, false, &(pi->demods));
  LADR_GLOBAL_TOP_INPUT.accept_list("hints",            (int)  Top_input_type::FORMULAS, true,  &(pi->hints));

  LADR_GLOBAL_TOP_INPUT.accept_list("actions",         (int)  Top_input_type::TERMS, false, &(pi->actions));
  LADR_GLOBAL_TOP_INPUT.accept_list("weights",         (int)  Top_input_type::TERMS, false, &(pi->weights));
  LADR_GLOBAL_TOP_INPUT.accept_list("kbo_weights",     (int)  Top_input_type::TERMS, false, &(pi->kbo_weights));
  LADR_GLOBAL_TOP_INPUT.accept_list("interpretations", (int)  Top_input_type::TERMS, false, &(pi->interps));
  LADR_GLOBAL_TOP_INPUT.accept_list("given_selection", (int)  Top_input_type::TERMS, false, &(pi->given_selection));
  LADR_GLOBAL_TOP_INPUT.accept_list("keep",            (int)  Top_input_type::TERMS, false, &(pi->keep_rules));
  LADR_GLOBAL_TOP_INPUT.accept_list("delete",          (int)  Top_input_type::TERMS, false, &(pi->delete_rules));
  
  LADR_GLOBAL_TOP_INPUT.accept_list("unused",          (int)  Top_input_type::FORMULAS, true, &(pi->unused));


 
  
  //So far we have a Readlist named Input_lists.
  //This Input_lists is a list of readlists.
  //Each readlist has a name and a Plist containing Tofformulas
  //This Plists of topformulas are full filt with pi->sos and so one
  //pi->sos and the other are not built yet
   if (echo)
    banner::print_separator(cout, "INPUT", true);
  
   process_command_line_args_1(opts.get_arg_options(), pi->options);  // high-level, e.g., auto2
  // Read commands such as set, clear, op, lex.
  // Read lists, filling in variables given to the accept_list calls.
  
  

   
   
   LADR_GLOBAL_TOP_INPUT.read_all_input(argc, argv, cout, echo, (int) unknown_action);
  
   

   
   //Know, pi->sos and the other lists are completed from input file.
  //So the readlists are completed too, each readlist has a pointer to the pi->....
  //Working with pi->sos and the other plists is the same as working with the rescpective input_list
  
  
   if (echo)
     banner::print_separator(cout, "end of input", true);
   
  
  
  process_command_line_args_2(opts.get_arg_options(), pi->options);  // others, which override
  if (!LADR_GLOBAL_OPTIONS.option_dependencies_state()) {
    cout<<endl<<"%% Enabling option dependencies (ignore applies only on input)."<<endl;
    LADR_GLOBAL_OPTIONS.enable_option_dependencies();
  }

 

  
   if (clausify) {
  
        Plist denials;
        
        
        
        //this will transform Formulas in Topformulas
        pi->sos    = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(pi->sos,      true); //so, we have topformulas
        pi->usable = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(pi->usable,   true);
        pi->demods = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(pi->demods,   true);
        pi->hints  = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(pi->hints,    true);
        pi->goals  = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(pi->goals,    false);
  
        //Kow all pi list of formulas are topforms, with the formula and justification and attributes, etc
        
        
     
        
        
        if (LADR_GLOBAL_OPTIONS.flag(pi->options->expand_relational_defs)) {
            Plist defs, nondefs, p;
            D.separate_definitions(pi->sos, &defs, &nondefs);
            pi->sos = NULL;
            banner::print_separator(cout, "EXPAND RELATIONAL DEFINITIONS", true);
            if (defs) {
                Plist results, rewritten, defs2;

                printf("\n%% Relational Definitions:\n");
                for (p = defs; p; p = p->next) {
                    Topform tf =(Topform) p->v;
                    CI.assign_clause_id(tf);
                    Ioutil::fwrite_clause(cout, tf, (int) Clause_print_format::CL_FORM_STD);
                }

        
            D.process_definitions(defs, &results, &defs2, &rewritten);
            if (results != NULL)
                fatal::fatal_error("Circular relational definitions");

        defs = defs2;

        if (rewritten) {
            printf("\n%% Relational Definitions, Expanded:\n");
        for (p = defs; p; p = p->next)
            if (!J.has_input_just((Topform)p->v))
                Ioutil::fwrite_clause(cout,(Topform) p->v, (int) Clause_print_format::CL_FORM_STD);
        }

        results = NULL;
        D.expand_with_definitions(nondefs, defs, &results, &rewritten);
        P.set_head(results);
        pi->sos = P.reverse_plist();

        results = NULL;
        D.expand_with_definitions(pi->usable, defs, &results, &rewritten);
        P.set_head(results);
        pi->usable = P.reverse_plist();

        results = NULL;
        D.expand_with_definitions(pi->hints, defs, &results, &rewritten);
        P.set_head(results);
        pi->hints = P.reverse_plist();

        results = NULL;
        D.expand_with_definitions(pi->goals, defs, &results, &rewritten);
        P.set_head(results);
        pi->goals = P.reverse_plist();

        for (p = defs; p; p = p->next)
            TF.append_label_attribute((Topform) p->v, "non_clause");

        if (rewritten) {
            printf("\n%% Formulas Being Expanded:\n");
            P.set_head(rewritten);
            rewritten = P.reverse_plist();
            for (p = rewritten; p; p = p->next) {
                Topform tf = (Topform) p->v;
                Ioutil::fwrite_clause(cout, tf, (int) Clause_print_format::CL_FORM_STD);
                TF.append_label_attribute(tf, "non_clause");
            }
        }
        
      }
      else {
        
        cout<<endl<<"%% No relations definitions were foud."<<endl;
        pi->sos = nondefs;
      }
      banner::print_separator(cout, "end of expand relational definitions", true);
    }

    
    
    
    banner::print_separator(cout, "PROCESS NON-CLAUSAL FORMULAS", true);
    if (echo)
      cout<<endl<<"% Formulas that are not ordinary clauses:"<<endl;
        
   
    denials=NULL;
    
        
    
    pi->sos    = LADR_GLOBAL_TOP_INPUT.process_input_formulas(pi->sos, echo);
    
    pi->usable = LADR_GLOBAL_TOP_INPUT.process_input_formulas(pi->usable, echo);

    pi->demods = LADR_GLOBAL_TOP_INPUT.process_demod_formulas(pi->demods, echo);
    
    pi->hints  = LADR_GLOBAL_TOP_INPUT.process_input_formulas(pi->hints, echo);

    denials    = LADR_GLOBAL_TOP_INPUT.process_goal_formulas(pi->goals, echo); 
    

    
    
    PlistContainer P1,P2;
    P1.set_head(pi->sos);
    P2.set_head(denials);
    pi->sos = P1.plist_cat(P2);
   
    
    banner::print_separator(cout, "end of process non-clausal formulas", true);
  }

 
  return pi;
}


