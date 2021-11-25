#include "./prover9vglobais.h"
#include "./ladr/fatal.h"



void ForwardSubsume::init_fsub_index(Mindextype mtype,Uniftype utype,int fpa_depth) {
#ifdef FEATURES
  Di_treeContainer DI;
  LindexContainer LI;
  Nonunit_index = DI.init_di_tree();
  Unit_index = LI.lindex_init(mtype, utype, fpa_depth, mtype, utype, fpa_depth);
#else
  int i;
  LindexContainer LI;
  if (NUM_INDEXES < 2)
    fatal::fatal_error("init_fsub_index: NUM_INDEXES < 2");

  for (i = 0; i < NUM_INDEXES; i++)
    PROVER9_GLOBAL_FORWARD_SUBSUME.Idx[i] = LI.lindex_init(mtype, utype, fpa_depth, mtype, utype, fpa_depth);
#endif
} 

void ForwardSubsume::fsub_destroy_index(void){
#ifdef FEATURES
  Lindexcontainer LI;
  LI.lindex_destroy(Unit_index);
  /* Nonunit_index??? */
#else
  int i;
  LindexContainer LI;
  for (i = 0; i < NUM_INDEXES; i++) {
    LI.lindex_destroy(PROVER9_GLOBAL_FORWARD_SUBSUME.Idx[i]);
    PROVER9_GLOBAL_FORWARD_SUBSUME.Idx[i] = NULL;
  }
#endif
}


void ForwardSubsume::index_fsub(Topform c, Indexop op, ClockStructure clock) {
#ifdef FEATURES
 
  Lindexcontainer LI;
  Di_treeContainer DI;
 
  if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c) == 1)
    LI.lindex_update_first(Unit_index, c, op);
  else {
    Ilist f = GlobalFeatures::features(c);
    if (op == Indexop::INSERT)
      DI.di_tree_insert(f, Nonunit_index, c);
    else
      DI.di_tree_delete(f, Nonunit_index, c);
    IlistContainer IL(f)
	IL.zap_ilist(f;
  }
#else
 
  LindexContainer LI;
  myClock CLOCK;
  int n = LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals);
  if (n >= NUM_INDEXES)
    n = NUM_INDEXES-1;
  /* Index only the first literal of c, because any subsumee
     will have to match that first literal.
   */
  CLOCK.clock_start(clock);
  LI.lindex_update_first(PROVER9_GLOBAL_FORWARD_SUBSUME.Idx[n], c, op);
  CLOCK.clock_stop(clock);
#endif
}



Topform ForwardSubsume::forward_subsumption_old(Topform d) {
#ifdef FEATURES
  Topform subsumer = Subsume::forward_subsume(d, Unit_index);
  if (!subsumer)
    subsumer = Subsume::forward_nonunit_subsume(d, Nonunit_index);
  return subsumer;
#else
  
  int nc = LADRV_GLOBAIS_INST.Lit.number_of_literals(d->literals);  /* Don't let a longer clause subsume c. */
  int i;
  for (i = 1; i < NUM_INDEXES && i <= nc; i++) {
    Topform subsumer = Subsume::forward_subsume(d, PROVER9_GLOBAL_FORWARD_SUBSUME.Idx[i]);
    if (subsumer != NULL)
      return subsumer;
  }
  return NULL;
#endif
} 

void ForwardSubsume::unit_deletion_old(Topform c) {
#ifdef FEATURES
  Subsume::unit_delete(c, Unit_index);
#else
  Subsume::unit_delete(c, PROVER9_GLOBAL_FORWARD_SUBSUME.Idx[1]);
#endif
} 
