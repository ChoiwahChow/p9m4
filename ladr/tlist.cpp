#include "tlist.h"

void Tlist::zap_tlist(Plist p){
  TermContainer t;
  PlistContainer P;
  if (p != NULL) {
    zap_tlist(p->next);
    t.zap_term((Term)p->v);
    p->v=NULL;
    P.free_plist(p);
    p=NULL;
  }
}  /* zap_tlist */

/* DOCUMENTATION
Remove a term from a Plist.  Term_ident() is used, and the
term in the list is zapped.
*/

/* PUBLIC */
Plist Tlist::tlist_remove(Term t, Plist p){
  TermContainer T;
  PlistContainer P;
  if (p == NULL) return NULL;
  else {
    p->next = tlist_remove(t, p->next);
    if (T.term_ident(t, (Term)p->v)) {
      Plist next = p->next;
      T.zap_term((Term)p->v);
      P.free_plist(p);
      return next;
    }
    else return p;
  }
}


Plist Tlist::tlist_union(Plist a, Plist b) {
  TermContainer T;
  PlistContainer P;
  if (a == NULL)
    return b;
  else if (T.tlist_member((Term)a->v, b)) {
    Plist c = tlist_union(a->next,b );
    T.zap_term((Term)a->v);
    P.free_plist(a);
    return c;
  }
  else {
    a->next = tlist_union(a->next, b);
    return a;
  }
}



Ilist Tlist::constants_in_term(Term t, Ilist p)
{
  
  if (VARIABLE(t))  return p;
  else 
  if (CONSTANT(t)) {
    IlistContainer I;
    I.set_head(p);
    if (I.ilist_member(SYMNUM(t)))     return p;
    else  return I.ilist_prepend(SYMNUM(t));
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      p = constants_in_term(ARG(t, i), p);
    return p;
  }
}

Plist Tlist::tlist_copy(Plist p) {
 TermContainer T;
 PlistContainer P;
 if (p == NULL)    return NULL;
  else {
    Term t =(Term) p->v;
    Plist novo = P.get_plist();
    novo->v = T.copy_term(t);
    novo->next = tlist_copy(p->next);
    return novo;
  }
}  /* tlist_copy */


