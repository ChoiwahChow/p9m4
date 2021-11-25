#ifndef TP_ACTIONS_H
#define TP_ACTIONS_H


#include "./ladr/term.h"
#include "./ladr/topform.h"
#include "./ladr/parse.h"
#include "./ladr/just.h"
#include "search_structures.h"


#define NN 100  /* make sure that each array is <= this! */





class GlobalActions {
                      
    
       private:         Plist Given_rules;
						Plist Generated_rules;
						Plist Kept_rules;
						Plist Level_rules;
						
                        string Changable_flags[NN];
						string Changable_parms[NN];
						string Changable_flags_rebuild[NN];
                        string Changable_parms_rebuild[NN];
                        
                        
                        friend class Prover9VGlobais;
                        friend class Actions;
    
};


class Actions {

				private:
					
                        TermContainer T;
                        SymbolContainer S;
						ParseContainer P;
                        TopformContainer TF;
                        JustContainer J;
                        AttributeContainer A;
	
                        
						bool changable_flag(Term);
						bool changable_parm(Term); 
						bool changable_flag_rebuild(Term);
						bool changable_parm_rebuild(Term ); 
						bool apply_action(Term);
						void proof_action(Topform, int);
						
						
						
						static void (*Rebuild_sos_proc) (void);
						static void (*Exit_proc) (int);  			/* this is called for "exit" actions */
						static void (*Assert_proc) (Topform);  
				
				public:
						void init_action(Term);
						void init_actions(Plist rules, void (*rebuild_sos_proc)(void), void (*exit_proc)(int),void (*assert_proc)(Topform));
						void register_action(string, string, string, string, string);
						void statistic_actions(string, int);
						void actions_in_proof(Plist,Prover_attributes);
};










#endif
