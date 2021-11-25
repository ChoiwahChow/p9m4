#include "basic.h"
#include "unify.h"
#include "fatal.h"
#include "termflag.h"
#include "symbols.h"
#include "ladrvglobais.h"








GlobalBasic::GlobalBasic() {
    Basic_paramodulation=false;
    Nonbasic_flag=-1;
}


GlobalBasic::~GlobalBasic() {

    
}



void Basic::init_basic_paramod(void){
  if (LADR_GLOBAL_BASIC.Nonbasic_flag != -1) fatal::fatal_error("init_basic_paramod, called more than once");
  TermflagContainer TF;
  LADR_GLOBAL_BASIC.Nonbasic_flag = TF.claim_term_flag();  /* allocate a termflag */
}

void Basic::set_basic_paramod(bool flag){
    LADR_GLOBAL_BASIC.Basic_paramodulation = flag;
}

bool Basic::basic_paramod(void){
  return LADR_GLOBAL_BASIC.Basic_paramodulation;
}

void Basic::mark_term_nonbasic(Term t) {
  if (LADR_GLOBAL_BASIC.Nonbasic_flag == -1)
    fatal::fatal_error("mark_term_nonbasic: init_basic() was not called");
  TermflagContainer TF;
  TF.term_flag_set(t, LADR_GLOBAL_BASIC.Nonbasic_flag);
}


void Basic::mark_all_nonbasic(Term t){
  /* Even though variables are nonbasic, don't mark them. */
  if (!VARIABLE(t)) {
    int i;
mark_term_nonbasic(t);
    for (i = 0; i < ARITY(t); i++)
      mark_all_nonbasic(ARG(t,i));
  }
}

bool Basic::nonbasic_term(Term t) {
  TermflagContainer TF;
  return TF.term_flag(t, LADR_GLOBAL_BASIC.Nonbasic_flag);
}


bool Basic::basic_term(Term t) {
  TermflagContainer TF;
  return !TF.term_flag(t, LADR_GLOBAL_BASIC.Nonbasic_flag);
}


int Basic::nonbasic_flag(void) {
  return LADR_GLOBAL_BASIC.Nonbasic_flag;
}


Term Basic::apply_basic(Term t, Context c) {
  TermContainer T;
  Term raw = t;  /* save original (nondereferenced) term */
  DEREFERENCE(t, c)

  /* A NULL context is ok.  It happens when c is built by match. */
  /* If the context is NULL, then apply just copies the term.    */

  if (VARIABLE(t)) {
    if (!c)
      return T.get_variable_term(VARNUM(t));
    else
      return T.get_variable_term(c->multiplier * MAX_VARS + VARNUM(t));
  }
  else {  /* constant or complex term */
Term t2 = T.get_rigid_term_like(t);
int i;
for (i = 0; i < ARITY(t); i++)
  ARG(t2,i) = apply_basic(ARG(t,i), c);
/* If the raw term is a variable, the result and all of its nonvariable
   subterms are marked as nonbasic; if the dereferenced term is nonbasic,
   the rood of the result is marked as nonbasic (subterms have already
   been marked as nonbasic; recall that all nonvariable subterms of
   a nonbasic term are nonbasic).
*/
if (VARIABLE(raw))
  mark_all_nonbasic(t2);
else if (nonbasic_term(t))
  mark_term_nonbasic(t2);
return t2;
}
}  /* apply_basic */


Term Basic::apply_basic_substitute(Term t, Term beta, Context c_from,
Term into_term, Context c_into) {
  TermContainer T;
  if (t == into_term)
    return apply_basic(beta, c_from);
  else if (VARIABLE(t))
    return apply_basic(t, c_into);
  else {
    int i;
    Term t2 = T.get_rigid_term_like(t);
    if (nonbasic_term(t))
      mark_term_nonbasic(t2);
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = apply_basic_substitute(ARG(t,i), beta, c_from,
into_term, c_into);
    return t2;
  }
}  /* apply_basic_substitute */

void Basic::clear_all_nonbasic_marks(Term t){
  /* recall that variables never have nonbasic marks */
  TermflagContainer TF;
  if (!VARIABLE(t)) {
    int i;
    TF.term_flag_clear(t, LADR_GLOBAL_BASIC.Nonbasic_flag);
    for (i = 0; i < ARITY(t); i++)
      clear_all_nonbasic_marks(ARG(t,i));
  }
}  /* clear_all_nonbasic_marks */


/* PUBLIC */
void Basic::p_term_basic(Term t){
  SymbolContainer S;
  if (VARIABLE(t)) cout<<"v"<<VARNUM(t);
  else {
    if (basic_term(t)) cout << "#";
    S.fprint_sym(cout, SYMNUM(t));
    if (COMPLEX(t)) {
      int i;
      cout <<"(";
      for (i = 0; i < ARITY(t); i++) {
p_term_basic(ARG(t,i));
if (i < ARITY(t)-1) cout<<",";
      }
      cout<<"(";
    }
  }
  cout<<endl;
}  /* p_term_basic */
