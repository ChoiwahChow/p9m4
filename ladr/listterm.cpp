

#include "listterm.h"
#include "ladrvglobais.h"


ListtermContainer::ListtermContainer() {

}

ListtermContainer::~ListtermContainer() {

}

/* DOCUMENTATION
Allocate and return an empty listterm.
*/

/* PUBLIC */
Term ListtermContainer::get_nil_term() {
    TermContainer T;
    return T.get_rigid_term(NIL_SYM, 0);
}  /* get_nil_term */



/* DOCUMENTATION
This routine returns the "cons" of two terms.
The two terms are not copied.
*/

/* PUBLIC */
Term ListtermContainer::listterm_cons(Term t1, Term t2) {
  TermContainer T;
  Term c = T.get_rigid_term(CONS_SYM, 2);
  ARG(c,0) = t1;
  ARG(c,1) = t2;
  return c;
}  /* listterm_cons */


/* DOCUMENTATION
This function checks if a term is a "cons", that is,
arity 2 with the official "cons" symbol.
*/

/* PUBLIC */
bool ListtermContainer::cons_term(Term t) {
    SymbolContainer S;
    return S.is_symbol(SYMNUM(t), CONS_SYM, 2);
}  /* cons_term */


/* DOCUMENTATION
This function checks if a term is a "nil", that is,
arity 0 with the official "nil" symbol.
*/


/* PUBLIC */
bool ListtermContainer::nil_term(Term t) {
    SymbolContainer S;
    return S.is_symbol(SYMNUM(t), NIL_SYM, 0);
}  /* nil_term */




/* DOCUMENTATION
This function checks if a term is a proper listterm,
that is, a nil_term(), or a cons_term() whose tail
is a proper_listterm().
*/

/* PUBLIC */
bool ListtermContainer::proper_listterm(Term t) {
  if (nil_term(t))  return true;
  else if (cons_term(t))  return proper_listterm(ARG(t,1));
  else return false;
}  /* proper_listterm */


/* DOCUMENTATION
This routine appends an element to a listterm.  The resulting
listterm is returned.  Neither the list nor the element is copied.
You should not refer to the argument "list" after calling this
routine---a good way to call it is like this:
<PRE>list = listterm_append(list, element)</PRE>
<P>
If "list" is not a proper_listterm(), the result will be
well-formed, but it might not be what you expect.
*/

/* PUBLIC */
Term ListtermContainer::listterm_append(Term list, Term element){
  if (!cons_term(list)) return listterm_cons(element, list);
  else {
        ARG(list,1) = listterm_append(ARG(list,1), element);
        return list;
    }
}  /* listterm_append */



/* DOCUMENTATION
This function returns the length of a listterm.
*/

/* PUBLIC */
int ListtermContainer::listterm_length(Term t){
  if (!cons_term(t)) return 0;
  else {
        return 1 + listterm_length(ARG(t,1));
  }
}  /* listterm_length */

/* DOCUMENTATION
Return the i-th member, counting from 1, of a listterm.
If there are less than i members, return NULL.
*/

/* PUBLIC */
Term ListtermContainer::listterm_i(Term lst, int i) {
  if (!cons_term(lst)) return NULL;
  else if (i == 1) return ARG(lst,0);
  else  return listterm_i(ARG(lst,1), i-1);
}  /* listterm_i */




/* DOCUMENTATION
This function checks if Term t is a member of a listterm (Term lst).
*/

/* PUBLIC */
bool ListtermContainer::listterm_member(Term t, Term lst) {
  if (cons_term(lst)) {
    TermContainer T; 
    if (T.term_ident(t, ARG(lst,0))) return true;
    else return listterm_member(t, ARG(lst,1));
  }
  else return false;
}  /* listterm_member */

/* DOCUMENTATION
Given a proper listterm (e.g, [a,b,c]), return a Plist
of the members.  The members are not copied.
*/

/* PUBLIC */
Plist ListtermContainer::listterm_to_tlist(Term t) {
  if (!proper_listterm(t)) return NULL;
  else {
        PlistContainer P;
        while (cons_term(t)) {
            P.plist_append(ARG(t, 0));
            t = ARG(t,1);
        }
    return P.get_head();
  }
}  /* listterm_to_tlist */


/* DOCUMENTATION
Free a list structure, but do not free its members.
*/

/* PUBLIC */
void ListtermContainer::listterm_zap(Term t) {
  TermContainer T;
  if (!cons_term(t)) T.zap_term(t);
  else {
        listterm_zap(ARG(t,1));
        T.free_term(t);
  }
}  /* listterm_zap */



Term ListtermContainer::rev2(Term t, Term done) {
  if (!cons_term(t))  return done;
  else return rev2(ARG(t,1), listterm_cons(ARG(t,0), done));
}  /* rev2 */



/* DOCUMENTATION
Reverse a listterm.  A new list structure is created, but
the members are not copied.  The old list structure is freed.
*/

/* PUBLIC */
Term ListtermContainer::listterm_reverse(Term t) {
  Term reversed = rev2(t, get_nil_term());
  listterm_zap(t);
  return reversed;
}  /* listterm_reverse */

