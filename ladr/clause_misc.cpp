#include "clauseid.h"
#include "clause_misc.h"


Clist ClauseMisc::clist_copy(Clist a, bool assign_ids) {
  ClistContainer C;
  TopformContainer TF;
  JustContainer J;
  AttributeContainer A;
  ClauseidContainer CI;
  Clist b = C.clist_init(*a->name);
  Clist_pos p;
  for(p = a->first; p; p = p->next) {
    Topform c = TF.copy_clause(p->c);
    c->justification = J.copy_justification(p->c->justification);
    c->attributes = A.copy_attributes(p->c->attributes);
    if (assign_ids)
      CI.assign_clause_id(c);
    C.clist_append(c, b);
  }
  return b;
} 


Clist ClauseMisc::copy_clauses_to_clist(Plist clauses, string name, bool assign_ids) {
  
  ClistContainer C;
  TopformContainer TF;
  JustContainer J;
  AttributeContainer AT;
  ClauseidContainer CI;
  
  Clist b = C.clist_init(name);
  Plist p;
  
  for (p = clauses; p; p = p->next) {
    Topform x = (Topform) p->v;
    Topform c = TF.copy_clause(x);
    c->justification = J.copy_justification(x->justification);
    c->attributes = AT.copy_attributes(x->attributes);
    if (assign_ids)
      CI.assign_clause_id(c);
    C.clist_append(c, b);
  }
  return b;
}


Clist ClauseMisc::move_clauses_to_clist(Plist clauses, string name, bool assign_ids) {
  ClistContainer C;	
  ClauseidContainer CI;
  
  Clist b = C.clist_init(name);
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (assign_ids && c->id == 0)    CI.assign_clause_id(c);
    C.clist_append(c, b);
  }
  return b;
}

Plist ClauseMisc::input_clauses(Plist a) {
  if (a == NULL)  return NULL;
  else {
    Plist c = input_clauses(a->next);
    PlistContainer P;
	JustContainer J;
	P.set_head(c);
	if (J.has_input_just((Topform)a->v)) 
	  return P.plist_prepend(a->v);
	
    else
      return c;
  }
} 

void ClauseMisc::delete_clause(Topform c) {
  JustContainer J;
  TopformContainer TF;
  ClauseidContainer CI;
  J.zap_just(c->justification);  /* ok if NULL */
  CI.unassign_clause_id(c);       /* ok if ID is not "official" */
  TF.zap_topform(c);               /* zaps attributes */
}

void ClauseMisc::delete_clist(Clist l) {
  Clist_pos p;
  Topform c;
  ClistContainer C;
  p = l->first;
  while (p) {
    c = p->c;
    p = p->next;
	C.clist_remove(c,l);
    if (c->containers == NULL)
      delete_clause(c);
  }
  C.clist_free(l);
}

Topform ClauseMisc::copy_clause_ija(Topform c) {
  TopformContainer TF;
  JustContainer J;
  AttributeContainer AT;
  Topform d = TF.copy_clause_with_flags(c);
  d->id = c->id;
  d->justification = J.copy_justification(c->justification);
  d->attributes = AT.copy_attributes(c->attributes);
  return d;
} 


Plist ClauseMisc::copy_clauses_ija(Plist p) {
  Plist a;
  PlistContainer P;
  for (a = p; a; a = a->next) {
    Topform old =(Topform) a->v;
    Topform novo = copy_clause_ija(old);
	P.plist_prepend(novo);  /* build it backward */
  }
  return P.reverse_plist();
} 

void ClauseMisc::delete_clauses(Plist p) {
  if (p != NULL) {
    delete_clauses(p->next);
    delete_clause((Topform)p->v);
    PlistContainer P;
	P.free_plist(p);
  }
}

void ClauseMisc::make_clause_basic(Topform c) {
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next)
    Basic::clear_all_nonbasic_marks(lit->atom);
} 
