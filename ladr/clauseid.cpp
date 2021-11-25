#include "ladrvglobais.h"
#include "clauseid.h"
#include "fatal.h"
#include "glist.h"
#include <iostream>



GlobalClauseId::GlobalClauseId() {
    
    Topform_id_count=0;
    
    for (int i=0;i<CLAUSE_ID_TAB_SIZE;i++)
        LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i]=NULL;
    
}


GlobalClauseId::~GlobalClauseId() {

    
}


void GlobalClauseId::destroy_topform_id_tab() {
    PlistContainer P;
    TopformContainer TF;
    Topform tf;
    Plist aux;
    
    for (int i=0;i<CLAUSE_ID_TAB_SIZE;i++) {
        P.set_head(LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i]); 

        for (aux=P.get_head(); aux; aux=aux->next) {
                tf=(Topform) aux->v;
                TF.zap_topform(tf);
        }
        P.zap_plist();
    }
}


int ClauseidContainer::next_clause_id() {
     return ++LADR_GLOBAL_CLAUSE_ID.Topform_id_count;
}


int ClauseidContainer::clause_ids_assigned(void) {
  return LADR_GLOBAL_CLAUSE_ID.Topform_id_count;
}


void ClauseidContainer::assign_clause_id(Topform c) {
  int i;
  TopformContainer T;  
  if (c->id > 0) {
    T.p_clause(c);
    fatal::fatal_error("assign_clause_id, clause already has ID.");
  }
  c->id = next_clause_id();
  i = c->id % CLAUSE_ID_TAB_SIZE;
  LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i] = insert_clause_into_plist(LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i], c, true);
  c->official_id = 1;
}


Plist ClauseidContainer::insert_clause_into_plist(Plist p, Topform c, bool increasing) {
  Plist prev, curr, novo;
  PlistContainer P;
  prev = NULL;
  curr = p;
  while (curr != NULL && (increasing ? ((Topform) curr->v)->id < c->id : ((Topform) curr->v)->id > c->id)) {
    prev = curr;
    curr = curr->next;
  }
  if (curr == NULL || ((Topform) curr->v)->id != c->id) {
    novo = P.get_plist();
    novo->v = c;
    novo->next = curr;
    if (prev != NULL)
      prev->next = novo;
    else
      p = novo;
  }
  return p;
}

void ClauseidContainer::unassign_clause_id(Topform c) {
  TopformContainer T;
  PlistContainer P;
  if (c->official_id) {
    int i;
    Plist p1, p2;
    i = c->id % CLAUSE_ID_TAB_SIZE;
    for (p2 = NULL, p1 = LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i]; p1 && ((Topform) p1->v)->id < c->id;p2 = p1, p1 = p1->next);
    if (p1 == NULL || ((Topform) p1->v)->id != c->id) {
        T.p_clause(c);
        fatal::fatal_error("unassign_clause_id, cannot find clause.");
    }
    if (p2)  p2->next = p1->next;
    else  LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i] = p1->next;
    c->id = 0;
   P.free_plist(p1);
   c->official_id = 0;
  }
}


Topform ClauseidContainer::find_clause_by_id(int id) {
  int i;
  Plist p1;

  i = id % CLAUSE_ID_TAB_SIZE;
  for (p1 = LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i]; p1 && ((Topform) p1->v)->id < id;  p1 = p1->next);
  if (p1 != NULL && ((Topform) p1->v)->id == id)  return (Topform) (p1->v);
  else  return(NULL);
} 

void ClauseidContainer::fprint_clause_id_tab(ostream &o) {
  int i;
  Plist p;
  TopformContainer T;
  o<<endl<<"ID clause table:"<<endl;
  
  
  for (i=0; i<CLAUSE_ID_TAB_SIZE; i++)
    for (p = LADR_GLOBAL_CLAUSE_ID.Topform_id_tab[i]; p; p = p->next)
      T.fprint_clause(o, (Topform) p->v);
}


void ClauseidContainer::p_clause_id_tab() {
  fprint_clause_id_tab(cout);
}


bool ClauseidContainer::clause_plist_member(Plist p, Topform c,bool increasing) {
  Plist curr = p;
  while (curr != NULL && (increasing ? ((Topform) curr->v)->id < c->id : ((Topform) curr->v)->id > c->id)) {
    curr = curr->next;
  }
  return (curr != NULL && ((Topform) curr->v)->id == c->id);
} 
