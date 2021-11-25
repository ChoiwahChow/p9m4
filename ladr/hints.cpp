#include "hints.h"
#include "mystring.h"
#include "ioutil.h"


Lindex 		Hints::Hints_idx = NULL;       /* FPA index for hints */
Clist 		Hints::Redundant_hints = NULL;  /* list of hints not indexed */
Mindex 		Hints::Back_demod_idx;        /* to index hints for back demodulation */
int 		Hints::Bsub_wt_attr;
bool 		Hints::Back_demod_hints;
bool 		Hints::Collect_labels;

int 		Hints::Hint_id_count=0;
int 		Hints::Active_hints_count=0;
int 		Hints::Redundant_hints_count=0;

void       (*Hints::Demod_proc) (Topform, int, int, bool, bool) = NULL; //initialize static function pointer


						


void Hints::Free_Mem() {

    ClistContainer CL;
    CL.clist_zap(Redundant_hints);
}

						
void Hints::init_hints(Uniftype utype,int bsub_wt_attr,bool collect_labels,bool back_demod_hints, void (*demod_proc) (Topform, int, int, bool, bool)) {
  LindexContainer L;
  ClistContainer CL;
  MindexContainer M;
  
  static int max_fpa_depth = 15;
  
  Bsub_wt_attr = bsub_wt_attr;
  Collect_labels = collect_labels;
  Back_demod_hints = back_demod_hints;
  Demod_proc = demod_proc;
  Hints_idx = L.lindex_init(Mindextype::FPA, utype, max_fpa_depth, Mindextype::FPA, utype, max_fpa_depth);
  if (Back_demod_hints)
    Back_demod_idx = M.mindex_init(Mindextype::FPA, utype, max_fpa_depth);
  Redundant_hints = CL.clist_init("redundant_hints");
} 						

void Hints::done_with_hints(void) {
  LindexContainer L;
  ClistContainer CL;
  MindexContainer M;
  if (!L.lindex_empty(Hints_idx) ||
      !CL.clist_empty(Redundant_hints))
	    cout<<"ERROR: Hints index not empty!"<<endl;
  L.lindex_destroy(Hints_idx);
  if (Back_demod_hints)
    M.mindex_destroy(Back_demod_idx);
  Hints_idx = NULL;
  CL.clist_free(Redundant_hints);
  Redundant_hints = NULL;
}  


int Hints::redundant_hints(void) {
  ClistContainer CL;
  return CL.clist_length(Redundant_hints);
} 


Topform Hints::find_equivalent_hint(Topform c, Lindex idx) {
  PlistContainer P;
  Topform equiv_hint = NULL;
  Plist subsumees = Subsume::back_subsume(c, idx);
  Plist p;
  for (p = subsumees; p && equiv_hint == NULL; p = p->next) {
    if (Subsume::subsumes((Topform)p->v, c))     equiv_hint =(Topform) p->v;
  }
  P.set_head(subsumees);
  P.zap_plist();
  return equiv_hint;
} 



Topform Hints::find_matching_hint(Topform c, Lindex idx){
  PlistContainer P;
  Topform hint = NULL;
  Plist subsumees = Subsume::back_subsume(c, idx);
  Plist p;
  bool equivalent = false;
  for (p = subsumees; p && !equivalent; p = p->next) {
    /* printf("subsumee: "); f_clause(p->v); */
    hint = (Topform) p->v;
    if (Subsume::subsumes((Topform)p->v, c))
      equivalent = true;
  }
  
  P.set_head(subsumees);
  P.zap_plist();
  return hint;
}


void Hints::index_hint(Topform c, bool doprint) {
  AttributeContainer A;
  LindexContainer L;
  ClistContainer CL;
  
  
  
  Topform h = find_equivalent_hint(c, Hints_idx);
  c->weight = 0;  /* this is used in hints degradation to count matches */
  if (h != NULL) {
    /* copy any bsub_hint_wt attrs from rundundant hint to the indexed hint */
    h->attributes = A.copy_int_attribute(c->attributes, h->attributes,Bsub_wt_attr);
    if (Collect_labels) {
      /* copy any labels from rundundant hint to the indexed hint */
      h->attributes = A.copy_string_attribute(c->attributes, h->attributes, A.label_att());
    }
    CL.clist_append(c, Redundant_hints);
    Redundant_hints_count++;
    /*
    printf("redundant hint: "); f_clause(c);
    printf("      original: "); f_clause(h);
    */
  }
  else {
    Active_hints_count++;
    Hint_id_count++;
  
    // BV(2016-jun-17): Use id assigned at input time.  Note that a back
    // demodulated hint will now keep the id of the original, which may be
    // necessary for avl_deletion for the hint_age given selection rule
    // (so the ordering function finds the clause in the avl tree).
    
    //c->id = Hint_id_count;  /* need IDs so that back_subsume() will work */
    
    L.lindex_update(Hints_idx, c, Indexop::INSERT);
    if (Back_demod_hints) {
      
        
         /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints 
       *
       * If c contains generic _AnyConst then do not index it in
       * back demodulation index.  This disables backward demodulation
       * of c.
       *
       * */
      if (MATCH_HINTS_ANYCONST) {
        TopformContainer TF;
        TermContainer T;
        I2listContainer Il;
        I2list rsyms = NULL;
        I2list fsyms = NULL;
        TF.gather_symbols_in_topform(c, &rsyms, &fsyms);
        Il.set_head(fsyms);
        I2list anyconst = Il.i2list_member(T.any_const_sn(0));
        Il.set_head(rsyms);
        Il.zap_i2list();
        Il.set_head(fsyms);
        Il.zap_i2list();
        if (anyconst) {
          return; 
        }
      }
        
        
        
        Backdemod::index_clause_back_demod(c, Back_demod_idx, Indexop::INSERT);
    
        
      
    }
    
    if (doprint) 
      Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_BARE);  
    
    
  }
} 


void Hints::unindex_hint(Topform c) {
  ClistContainer CL; 
  LindexContainer L;
  if (CL.clist_member(c, Redundant_hints)) {
    CL.clist_remove(c, Redundant_hints);
    Redundant_hints_count--;
  }
  else {
    L.lindex_update(Hints_idx, c, Indexop::DELETE);
    if (Back_demod_hints)
      Backdemod::index_clause_back_demod(c, Back_demod_idx, Indexop::DELETE);
    Active_hints_count--;
  }
} 


void Hints::adjust_weight_with_hints(Topform c,bool degrade,bool breadth_first_hints) {
  TopformContainer TF;
  AttributeContainer AT;
  
  TermContainer T;
  Parautil Pu;
  Topform hint = find_matching_hint(c, Hints_idx);
  if (hint == NULL &&
      LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals) &&
      T.eq_term(c->literals->atom) &&
      !Pu.oriented_eq(c->literals->atom)) {

    /* Try to find a hint that matches the flipped equality. */

    Term save_atom = c->literals->atom;
    c->literals->atom = Pu.top_flip(save_atom);
    hint = find_matching_hint(c, Hints_idx);
    Pu.zap_top_flip(c->literals->atom);
    c->literals->atom = save_atom;
    if (hint != NULL)
      c->attributes = AT.set_string_attribute(c->attributes, AT.label_att(), "flip_matches_hint");
  }

  if (hint != NULL) {

    int bsub_wt = AT.get_int_attribute(hint->attributes, Bsub_wt_attr, 1);

    if (bsub_wt != INT_MAX)
      c->weight = bsub_wt;
    else if (breadth_first_hints)
      c->weight = 0;

    /* If the hint has label attributes, copy them to the clause. */
    
    {
      int i = 0;
      string s = AT.get_string_attribute(hint->attributes, AT.label_att(), ++i);
      while (s!=myString::null_string()) {
        if (!AT.string_attribute_member(c->attributes, AT.label_att(), s))
		c->attributes = AT.set_string_attribute(c->attributes, AT.label_att(), s);
		s = AT.get_string_attribute(hint->attributes, AT.label_att(), ++i);
      }
    }

    /* Veroff's hint degradation strategy. */

    if (degrade) {
      /* for now, add 1000 for each previous match */
      int i;
      for (i = 0; i < hint->weight; i++) 
		c->weight = c->weight + 1000;
    }
    c->matching_hint = hint;
    /* If/when c is eventually kept, the hint will have its weight
       field incremented in case hint degradation is being used. */
  }
}


void Hints::keep_hint_matcher(Topform c)
{
  Topform hint = c->matching_hint;
  hint->weight++;
}


void Hints::flag_indexed_hints() {
   FpaIndexContainer F; 
   F.flag_fpa_clauses(cout, Hints_idx -> pos -> fpa);
}


void Hints::back_demod_hints(Topform demod, int type, bool lex_order_vars)
{
   Parautil Pu;
   TopformContainer TF;
   if (Back_demod_hints) {
    Plist rewritables = Backdemod::back_demod_indexed(demod, type, Back_demod_idx, lex_order_vars);
    Plist p;
    for (p = rewritables; p; p = p->next) {
      Topform hint =(Topform) p->v;
      /* printf("\nBEFORE: "); f_clause(hint); */
      unindex_hint(hint);
      (*Demod_proc)(hint, 1000, 1000, false, lex_order_vars);
      Pu.orient_equalities(hint, true);
      Subsume::simplify_literals2(hint);
      Resolve::merge_literals(hint);
      TF.renumber_variables(hint, MAX_VARS);

      /* printf("AFTER : "); f_clause(hint); */
      index_hint(hint, false);
      hint->weight = 0;  /* reset count of number of matches */
    }
  }
}
