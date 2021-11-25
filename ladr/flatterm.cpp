#include "ladrvglobais.h"
#include "flatterm.h"
#include "memory.h"
#include "symbols.h"
#include "multiset.h"


#include <iostream>
#include <iomanip>





GlobalFlatterm::GlobalFlatterm()
{
    Flatterm_gets=0;
    Flatterm_frees=0;
}

GlobalFlatterm::~GlobalFlatterm()
{
}



FlattermContainer::FlattermContainer() {
}

FlattermContainer::~FlattermContainer(){
};



Flatterm FlattermContainer::get_flatterm(void) {
  Flatterm p = (Flatterm) Memory::memNew(sizeof(struct flatterm));  /*get uninitialized Memory:: */
  LADR_GLOBAL_FLATTERM.Flatterm_gets++;
  p->prev = NULL;
  p->next = NULL;
  p->varnum_bound_to = -1;
  p->alternative = NULL;
  p->reduced_flag = false;
  p->size = 0;
  /* end, arity, private_symbol not initilized */
  return(p);
}  /* get_flatterm */


/* PUBLIC */
void FlattermContainer::free_flatterm(Flatterm p){
  Memory::memFree(p, sizeof(struct flatterm));
  LADR_GLOBAL_FLATTERM.Flatterm_frees++;
}  /* free_flatterm */




/* DOCUMENTATION
This routine prints (to FILE *fp) Memory:: usage statistics for data types
associated with the flatterm package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void FlattermContainer::fprint_flatterm_mem(ostream &o, const bool heading) const {
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct flatterm);
  o<<"flatterm      ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_FLATTERM.Flatterm_gets;
  o<<setw(11)<<LADR_GLOBAL_FLATTERM.Flatterm_frees;
  o<<setw(11)<<LADR_GLOBAL_FLATTERM.Flatterm_gets-LADR_GLOBAL_FLATTERM.Flatterm_frees;
  o<<setw(9)<<((LADR_GLOBAL_FLATTERM.Flatterm_gets - LADR_GLOBAL_FLATTERM.Flatterm_frees) * n) / 1024<<"K"<<endl;
}  /* fprint_flatterm_mem */


/*************
 *
 *   p_flatterm_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) Memory:: usage statistics for data types
associated with the flatterm package.
*/

/* PUBLIC */
void FlattermContainer::p_flatterm_mem() const{
  fprint_flatterm_mem(cout, true);
}  /* p_flatterm_mem */

/* PUBLIC */
bool FlattermContainer::flatterm_ident(Flatterm a, Flatterm b){
  Flatterm ai, bi;
  for (ai = a, bi = b; ai != a->end->next; ai = ai->next, bi = bi->next)
    if (ai->private_symbol != bi->private_symbol)
      return false;
  return true;
}  /* flatterm_ident */

/*************
 *
 *   zap_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void FlattermContainer::zap_flatterm(Flatterm f)
{
  Flatterm fi = f;
  
  while (fi != f->end->next) {
    Flatterm tmp = fi;
    fi = fi->next;
    free_flatterm(tmp);
  }
}  /* zap_flatterm */

/*
 *  end of Memory:: management
 */


/*************
 *
 *   term_to_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Flatterm FlattermContainer::term_to_flatterm(Term t) {
  Flatterm f = get_flatterm();
  f->private_symbol = t->private_symbol;
  ARITY(f) = ARITY(t);
  if (VARIABLE(t)) {
    f->end = f;
    f->size = 1;
    return f;
  }
  else {
    int n = 1;
    int i;
    Flatterm end = f;
    for (i = 0; i < ARITY(t); i++) {
      Flatterm arg = term_to_flatterm(ARG(t,i));
      n += arg->size;
      end->next = arg;
      arg->prev = end;
      end = arg->end;
    }
    f->end = end;
    f->size = n;
    return f;
  }
}  /* term_to_flatterm */

/* PUBLIC */
Term FlattermContainer::flatterm_to_term(Flatterm f){
  TermContainer T;
  if (VARIABLE(f)) return T.get_variable_term(VARNUM(f));
  else {
    Term t = T.get_rigid_term_dangerously(SYMNUM(f),ARITY(f));
    int i;
    Flatterm g = f->next;
    for (i = 0; i < ARITY(f); i++) {
      ARG(t,i) = flatterm_to_term(g);
      g = g->end->next;
    }
    return t;
  }
}  /* flatterm_to_term */



Flatterm FlattermContainer::copy_flatterm(Flatterm f){
  int i;
  int n = 1;
  Flatterm g = get_flatterm();
  Flatterm end = g;
  Flatterm arg = f->next;

  g->private_symbol = f->private_symbol;
  ARITY(g) = ARITY(f);

  for (i = 0; i < ARITY(f); i++) {
    Flatterm b = copy_flatterm(arg);
    n += b->size;
    end->next = b;
    b->prev = end;
    end = b->end;
    arg = arg->end->next;
  }
  g->end = end;
  g->size = n;
  return g;
}  /* copy_flatterm */



/* PUBLIC */
void FlattermContainer::print_flatterm(Flatterm f){
  if (VARIABLE(f)) {
    if (VARNUM(f) < 3)  cout<< char ('x' + VARNUM(f));
    else if (VARNUM(f) < 6)  cout<< char ('r' + VARNUM(f));
    else   cout<<"v" + to_string (VARNUM(f));
  }
  else if (CONSTANT(f)) {
    SymbolContainer S;
    cout<<S.sn_to_str(SYMNUM(f));
    }
  else {
    int i;
    Flatterm g = f->next;
    SymbolContainer S;
    cout<<"("<<S.sn_to_str(SYMNUM(f));
    for (i = 0; i < ARITY(f); i++) {
      print_flatterm(g);
      if (i < ARITY(f) - 1) cout<<",";
      g = g->end->next;
    }
    cout<<")";
  }
}  /* print_flatterm */


int FlattermContainer::flatterm_symbol_count(Flatterm f){
  if (VARIABLE(f)) return 1;
  else {
    int n = 1;
    int i;
    Flatterm g = f->next;
    for (i = 0; i < ARITY(f); i++) {
      n += flatterm_symbol_count(g);
      g = g->end->next;
    }
    return n;
  }
}  /* flatterm_symbol_count */


/* PUBLIC */
void FlattermContainer::p_flatterm(Flatterm f){
  print_flatterm(f);
  cout<<endl;
}





bool FlattermContainer::flat_occurs_in(Flatterm t1, Flatterm t2){
  Flatterm t2i;
  for (t2i = t2; t2i != t2->end->next; t2i = t2i->next)
    if (flatterm_ident(t1, t2i))   return true;
  return false;
}  /* flat_occurs_in */



/* PUBLIC */
I2list FlattermContainer::flat_multiset_vars(Flatterm f){
  
  I2listContainer VARS;
  
  Flatterm fi;
  for (fi = f; fi != f->end->next; fi = fi->next)
    if (VARIABLE(fi))
      VARS.multiset_add(VARNUM(fi));
  return VARS.get_head();
}  /* flat_multiset_vars */


/* PUBLIC */
bool FlattermContainer::flat_variables_multisubset(Flatterm a, Flatterm b){
  I2list a_vars = flat_multiset_vars(a);
  I2list b_vars = flat_multiset_vars(b);
  I2listContainer Ia;
  I2listContainer Ib;
  Ia.set_head(a_vars);
  Ib.set_head(b_vars);
  bool ok =Ia.i2list_multisubset(Ib);
  Ia.zap_i2list();
  Ib.zap_i2list();
  return ok;
}  /* flat_variables_multisubset */



/* PUBLIC */
int FlattermContainer::flatterm_count_without_vars(Flatterm f){
  if (VARIABLE(f))    return 0;
  else {
    int n = 1;
    int i;
    Flatterm g = f->next;
    for (i = 0; i < ARITY(f); i++) {
      n += flatterm_count_without_vars(g);
      g = g->end->next;
    }
    return n;
  }
}  /* flatterm_count_without_vars */
