#include "actions.h"
#include "demodulate.h"
#include "forward_subsume.h"
#include "giv_select.h"
#include "index_lits.h"
#include "search.h"


#define PROVER9_GLOBAIS_INST                   PROVER9_V_GLOBAIS::prover9_v_globais
#define PROVER9_GLOBAL_ACTIONS                 PROVER9_GLOBAIS_INST.global_actions
#define PROVER9_GLOBAL_DEMODULATE              PROVER9_GLOBAIS_INST.global_demodulate
#define PROVER9_GLOBAL_FORWARD_SUBSUME         PROVER9_GLOBAIS_INST.global_forward_subsume
#define PROVER9_GLOBAL_GIV_SELECT              PROVER9_GLOBAIS_INST.global_giv_select
#define PROVER9_GLOBAL_INDEX_LITS              PROVER9_GLOBAIS_INST.global_index_lits
#define PROVER9_GLOBAL_SEARCH                  PROVER9_GLOBAIS_INST.global_search
#define PROVER9_GLOBAL_SEMANTICS               PROVER9_GLOBAIS_INST.global_semantics





class Prover9VGlobais {
                        //public properties
                        public:       
                                GlobalActions                   global_actions;
                                GlobalDemodulate                global_demodulate;
                                GlobalForwardSubsume            global_forward_subsume;
                                GlobalGivSelect                 global_giv_select;
                                GlobalIndexLits                 global_index_lits;
                                GlobalSearch                    global_search;
                                GlobalSemantics                 global_semantics;
                                
                            
                            
    
                                
                                 //public nethods             
                        public:
                                Prover9VGlobais();
                                ~Prover9VGlobais();
                                void FREE_MEM(void);
                        
};





class PROVER9_V_GLOBAIS {
public:
        static Prover9VGlobais prover9_v_globais;   //this static instance makes all of its objects constructors be called on startup
                                                    //and its destructors on exit
    
};
