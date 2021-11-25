#include "prover9vglobais.h"



Prover9VGlobais PROVER9_V_GLOBAIS::prover9_v_globais;



Prover9VGlobais::Prover9VGlobais()   {

    //lets init global actions
    PROVER9_GLOBAL_ACTIONS.Given_rules     = NULL;
    PROVER9_GLOBAL_ACTIONS.Generated_rules = NULL;
    PROVER9_GLOBAL_ACTIONS.Kept_rules      = NULL;
    PROVER9_GLOBAL_ACTIONS.Level_rules     = NULL;

    PROVER9_GLOBAL_ACTIONS.Changable_flags[NN];
    PROVER9_GLOBAL_ACTIONS.Changable_flags[0]="reuse_denials";
    PROVER9_GLOBAL_ACTIONS.Changable_flags[1]="print_gen";
    PROVER9_GLOBAL_ACTIONS.Changable_flags[2]="print_kept";
    PROVER9_GLOBAL_ACTIONS.Changable_flags[3]="print_given";
    
    PROVER9_GLOBAL_ACTIONS.Changable_parms[NN];
    PROVER9_GLOBAL_ACTIONS.Changable_parms[0]="demod_step_limit";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[1]="demod_size_limit";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[2]="new_constants";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[3]="para_lit_limit";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[4]="stats";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[5]="max_given";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[6]="max_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[7]="max_depth";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[8]="max_vars";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[9]="max_proofs";
    PROVER9_GLOBAL_ACTIONS.Changable_parms[10]="max_literals";
    
    PROVER9_GLOBAL_ACTIONS.Changable_flags_rebuild[NN];
    PROVER9_GLOBAL_ACTIONS.Changable_flags_rebuild[0]="breadth_first";
    PROVER9_GLOBAL_ACTIONS.Changable_flags_rebuild[1]="lightest_first";
    PROVER9_GLOBAL_ACTIONS.Changable_flags_rebuild[2]="breadth_first_hints";
    
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[NN];
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[0]="constant_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[1]="variable_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[2]="not_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[3]="or_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[4]="prop_atom_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[5]="nest_penalty";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[6]="depth_penalty";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[7]="skolem_penalty";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[8]="default_weight";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[9]="pick_given_ratio";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[10]="hints_part";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[11]="age_part";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[12]="weight_part";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[13]="false_part";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[14]="true_part";
    PROVER9_GLOBAL_ACTIONS.Changable_parms_rebuild[15]="random_part";
    
    PROVER9_GLOBAL_GIV_SELECT.High;
    PROVER9_GLOBAL_GIV_SELECT.Low;
    PROVER9_GLOBAL_GIV_SELECT.Rule_needs_semantics=false;
    PROVER9_GLOBAL_GIV_SELECT.Sos_size=0;
    PROVER9_GLOBAL_GIV_SELECT.Low_water_keep=INT_MAX;
    PROVER9_GLOBAL_GIV_SELECT.Low_water_displace=INT_MAX;
    PROVER9_GLOBAL_GIV_SELECT.Sos_deleted=0;
    PROVER9_GLOBAL_GIV_SELECT.Sos_displaced=0;
    PROVER9_GLOBAL_GIV_SELECT.Debug = false;
    PROVER9_GLOBAL_GIV_SELECT.Giv_select_gets=0; 
    PROVER9_GLOBAL_GIV_SELECT.Giv_select_frees=0;
    
       
    PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx=NULL;          /* unit bsub, unit conflict */
    PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx=NULL;       /* back unit del */
    PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx=NULL;      /* unit fsub, unit del */
    PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx=NULL;  /* nonunit fsub, nonunit bsub */
    
    PROVER9_GLOBAL_SEMANTICS.Compiled_interps = NULL;
    PROVER9_GLOBAL_SEMANTICS.Eval_limit = 0;
    
    PROVER9_GLOBAL_SEARCH.hsize=0;
    for (int i=0; i<MAX_HSIZE; i++) PROVER9_GLOBAL_SEARCH.HIT_LIST[i]=0;
    
    
    
}

Prover9VGlobais::~Prover9VGlobais() {

    
}

void Prover9VGlobais::FREE_MEM(void) {

    PROVER9_GLOBAL_GIV_SELECT.Free_Mem();
    PROVER9_GLOBAL_SEARCH.Free_Mem();
}
