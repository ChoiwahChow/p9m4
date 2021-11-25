
#include "index_lits.h"
#include "./ladr/ladrvglobais.h"
#include "./prover9vglobais.h"
 
 



void IndexLits::init_literals_index(void) {
  LindexContainer L;
  Di_treeContainer D;
  PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx     = L.lindex_init(Mindextype::FPA,         Uniftype::ORDINARY_UNIF, 10, Mindextype::FPA,           Uniftype::ORDINARY_UNIF, 10);
  PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx  = L.lindex_init(Mindextype::FPA,         Uniftype::ORDINARY_UNIF, 10, Mindextype::FPA,           Uniftype::ORDINARY_UNIF, 10);
  PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx = L.lindex_init(Mindextype::DISCRIM_BIND, Uniftype::ORDINARY_UNIF, 10, Mindextype::DISCRIM_BIND, Uniftype::ORDINARY_UNIF, 10);
  PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx = D.init_di_tree(); //the root
}  /* init_lits_index */


void IndexLits::destroy_literals_index(void) {
  LindexContainer L;
  Di_treeContainer D;
  
  L.lindex_destroy(PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx);                       PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx = NULL;
  L.lindex_destroy(PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx);                    PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx = NULL;
  L.lindex_destroy(PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx);                   PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx = NULL;
  D.zap_di_tree(PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx, LADR_GLOBAL_FEATURES.feature_length());  PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx = NULL;
} 


void IndexLits::index_literals(Topform c, Indexop op, ClockStructure clock, bool no_fapl) {
  LindexContainer LI;
  myClock CLOCK;
  Di_treeContainer DI;
 
  bool unit = (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) == 1);
  CLOCK.clock_start(clock);
  
  if (!no_fapl || !LADRV_GLOBAIS_INST.Lit.positive_clause(c->literals))
    LI.lindex_update(unit ? PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx : PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx, c, op);
  
  if (unit)
    LI.lindex_update(PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx, c, op);
  else {
    Ilist f = LADR_GLOBAL_FEATURES.features(c->literals);
    if (op == Indexop::INSERT)  DI.di_tree_insert(f, PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx, c);
    else DI.di_tree_delete(f, PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx, c);

    IlistContainer IL(f);
    IL.zap_ilist();
  }
  CLOCK.clock_stop(clock);
}  


void IndexLits::index_denial(Topform c, Indexop op, ClockStructure clock) {
  
  LindexContainer LI;
    myClock CLOCK;
  bool unit = (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) == 1);
  CLOCK.clock_start(clock);
  LI.lindex_update(unit ? PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx :PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx, c, op);
  CLOCK.clock_stop(clock);
}



void IndexLits::unit_conflict(Topform c, void (*empty_proc) (Topform)) {
  Subsume::unit_conflict_by_index(c, PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx, empty_proc);
} 



void IndexLits::unit_deletion(Topform c) {
  Subsume::unit_delete(c, PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx);
} 



Plist IndexLits::back_unit_deletable(Topform c) {
  return Subsume::back_unit_del_by_index(c, PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx);
}  



Topform IndexLits::forward_subsumption(Topform d) {
  Di_treeContainer DI;
  Topform subsumer = Subsume::forward_subsume(d, PROVER9_GLOBAL_INDEX_LITS.Unit_discrim_idx);
  if (!subsumer)
    subsumer = DI.forward_feature_subsume(d, PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx);
  return subsumer;
} 


Plist IndexLits::back_subsumption(Topform c) {
  Di_treeContainer DI;
  Plist p1 = Subsume::back_subsume(c, PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx);
#if 0
  Plist p2 = back_subsume(c, Nonunit_fpa_idx);
#else
  Plist p2 = DI.back_feature_subsume(c, PROVER9_GLOBAL_INDEX_LITS.Nonunit_features_idx);
#endif
  PlistContainer P1, P2;
  P1.set_head(p1);
  P2.set_head(p2);
  Plist p3 = P1.plist_cat(P2);
  return p3;
} 


void IndexLits::lits_idx_report(void) {
  FpaIndexContainer F;
  cout<<"Pos unit lits index: ";
  
  F.p_fpa_density(PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx->pos->fpa);
  
  
  cout<<"Neg unit lits index: ";
  F.p_fpa_density(PROVER9_GLOBAL_INDEX_LITS.Unit_fpa_idx->neg->fpa);
  
  
  cout<<"Pos nonunit lits index: ";
  F.p_fpa_density(PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx->pos->fpa);
  
  
  cout<<"Neg nonunit lits index: ";
  F.p_fpa_density(PROVER9_GLOBAL_INDEX_LITS.Nonunit_fpa_idx->neg->fpa);
} 

