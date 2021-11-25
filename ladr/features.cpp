

#include "features.h"
#include "ladrvglobais.h"
#include "symbols.h"
#include <iostream>





/* The following are work arrays, indexed by symnum, used for calculating
the features of a clause.  They are allocated by init_features() and
left in place throughout the process.
*/
                        

GlobalFeatures::GlobalFeatures() {
    Work_size=0;         /* size of following arrays */
    Pos_occurrences=NULL;
    Neg_occurrences=NULL;
    Pos_maxdepth=NULL;
    Neg_maxdepth=NULL;
}

GlobalFeatures::~GlobalFeatures() {

}


void GlobalFeatures::Free_Mem(void) {
    free(Pos_occurrences);
    free(Neg_occurrences);
    free(Pos_maxdepth);
    free(Neg_maxdepth);
}


void GlobalFeatures::init_features(Ilist fsyms, Ilist rsyms) {
  SymbolContainer C;
  Work_size = C.greatest_symnum() + 1;
  IlistContainer Pf, Pr,P;
  Ilist pr,pf;
  
  Pf.set_head(fsyms);
  Pr.set_head(rsyms);
  
  pf=Pf.ilist_copy();  
  pr=Pr.ilist_copy();
  
  Pf.set_head(pf);
  Pr.set_head(pr);
  
  Feature_symbols=Pf.ilist_cat(Pr);
  
  Pos_occurrences = (int *)calloc(Work_size, sizeof(int));
  Neg_occurrences = (int *)calloc(Work_size, sizeof(int));
  Pos_maxdepth    = (int *)calloc(Work_size, sizeof(int));
  Neg_maxdepth    = (int *)calloc(Work_size, sizeof(int));
  
  
}




void GlobalFeatures::fill_in_arrays(Term t, bool sign, int depth) {
  if (!VARIABLE(t)) {
    int sn = SYMNUM(t);
    int i;
    if (sn >= Work_size) {
      /* Assume it's a symbol that was added after the start of
	 the search.  If we ignore symbols in features, we may get
	 less discrimination, but all answers should be returned.
       */
      ;  
    }
    else if (sign) {
      Pos_occurrences[sn]++;
      Pos_maxdepth[sn] = IMAX(depth, Pos_maxdepth[sn]);
    }
    else {
      Neg_occurrences[sn]++;
      Neg_maxdepth[sn] = IMAX(depth, Neg_maxdepth[sn]);
    }
    for (i = 0; i < ARITY(t); i++)
      fill_in_arrays(ARG(t,i), sign, depth+1);
  }
}  /* fill_in_arrays */


Ilist GlobalFeatures::features(Literals lits) {
  Ilist f = NULL;
  Ilist p;
  IlistContainer I;
  SymbolContainer S;
  
  Literals lit;
  

  /* Build it backwards, then reverse it. */
  I.set_head(f);
  f = I.ilist_prepend(LADRV_GLOBAIS_INST.Lit.positive_literals(lits));
  f = I.ilist_prepend(LADRV_GLOBAIS_INST.Lit.negative_literals(lits));

  for (lit = lits; lit; lit = lit->next) {
    fill_in_arrays(lit->atom, lit->sign, 0);
  }

  for (p = Feature_symbols; p; p = p->next) {
    f = I.ilist_prepend(Pos_occurrences[p->i]);
    f = I.ilist_prepend(Neg_occurrences[p->i]);

    if (S.function_symbol(p->i)) {
      f = I.ilist_prepend(Pos_maxdepth[p->i]);
      f = I.ilist_prepend(Neg_maxdepth[p->i]);
    }

    Pos_occurrences[p->i] = 0;
    Neg_occurrences[p->i] = 0;
    Pos_maxdepth[p->i] = 0;
    Neg_maxdepth[p->i] = 0;
  }
  f = I.reverse_ilist();
#if 0
  printf("Features for clause "); f_clause(c);
  p_features(f);
#endif
  return f;
} 

int GlobalFeatures::feature_length(void) {
  SymbolContainer S;
  int n = 2;  /* pos lits, neg lits */
  Ilist p;
  for (p = Feature_symbols; p; p = p->next) {
    n += 2;
    if (S.function_symbol(p->i))
      n += 2;
  }
  return n;
}  /* feature_length */


bool GlobalFeatures::features_less_or_equal(Ilist c, Ilist d) {
  while (c && d && c->i <= d->i) {
    c = c->next;
    d = d->next;
  }
  return !c && !d;
}  /* features_less_or_equal */

void GlobalFeatures::p_features(Ilist f)  {
  Ilist p;
  SymbolContainer S;
  cout<<"  pos_lits="<<f->i<<", neg_lits"<<f->next->i<<endl;
  f = f->next->next;

  for (p = Feature_symbols; p; p = p->next) {
    
    cout<<"  symbol "<<S.sn_to_str(p->i)<<":";
    cout<<"pos_occ="<<f->i<<", neg_occ"<<f->next->i<<endl;
    
    f = f->next->next;
    if (S.function_symbol(p->i)) {
      cout<<"  pos_max"<<f->i<<",neg_max"<<f->next->i;
      f = f->next->next;
    }
    cout<<endl;
  }
}

