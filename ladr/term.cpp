#include "mystring.h"
#include "fatal.h"
#include <iostream>
#include <iomanip>
#include "ladrvglobais.h"




GlobalTerm::GlobalTerm() {
 Term_gets=0;
 Term_frees=0;
 Arg_mem=0;  
 for(int i=0; i<MAX_VNUM;i++)
    Shared_variables[i]=NULL;
 AnyConstsEnabled=true; 
 AnyConstsInited=false; /* are AnyConsts values valid? */
}

GlobalTerm::~GlobalTerm() {

}
                      
                     
void GlobalTerm::Free_Mem(void) {

for(int i=0; i<MAX_VNUM;i++)
    if (Shared_variables[i]) 
        free(Shared_variables[i]);

}

                     
TermContainer::TermContainer() {
    
}


TermContainer::~TermContainer() {
  
}



Term TermContainer::get_term(int arity) {
  /* This is a little tricky.  The pointers to the subterms are
     in an array (p->args) that is just after (contiguous with)
     the term.

     private_symbol is not initialized.
     args array is not initialized.
   */
  
  
  unsigned int j=sizeof(term);
  Term p =(Term) Memory::memNew((1+arity) * sizeof(term));  /* non-initialized Memory:: */
  p->arity = arity;
  if (arity == 0) p->args = NULL;
  else {
        Term *v = (Term *) p + sizeof(term)/sizeof(Term *); 
        p->args = v;
  }
  p->private_flags = 0;
  p->container = NULL;
  p->u.vp = NULL;
  LADR_GLOBAL_TERM.Term_gets++;
  LADR_GLOBAL_TERM.Arg_mem += arity;
  return(p);
} 


void TermContainer::free_term(Term p) {
  
  if (VARIABLE(p)) {
        return;  /* variables are never freed, because they are shared */
  }
  
  LADR_GLOBAL_TERM.Arg_mem -= p->arity;
  Memory::memFree((void *)p, (1+p->arity) * sizeof(term));
  p=NULL;
  LADR_GLOBAL_TERM.Term_frees++;
}



void TermContainer::fprint_term_mem(ostream &o, const bool heading)const {
  int n;
  
  if (heading)
      o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct term);
  o<<"term        ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_TERM.Term_gets<<setw(11)<<LADR_GLOBAL_TERM.Term_frees<<setw(11)<<LADR_GLOBAL_TERM.Term_gets - LADR_GLOBAL_TERM.Term_frees<<setw(9)<<((LADR_GLOBAL_TERM.Term_gets - LADR_GLOBAL_TERM.Term_frees) * n) / 1024<<"K"<<endl;
  o<<"term arg arrays:                                                   "<<(LADR_GLOBAL_TERM.Arg_mem * sizeof(term) / sizeof(Term *)) / 1024<<"K"<<endl<<endl;
} 


void TermContainer::p_term_mem(void) const{
  fprint_term_mem(cout, true);
}  



Term TermContainer::get_variable_term(int var_num) {
  if (var_num < 0 || var_num >= MAX_VAR)    fatal::fatal_error("get_variable_term: var_num out of range");

  if (var_num >= MAX_VNUM || LADR_GLOBAL_TERM.Shared_variables[var_num] == NULL) {
    Term t =(Term) malloc(sizeof(struct term));
    t->private_symbol = var_num;
    t->arity = 0;
    t->private_flags = 0;
    t->container = NULL;
    t->u.id = 0;
    if (var_num >= MAX_VNUM)
      return t;  /* will not be shared */
    else
      LADR_GLOBAL_TERM.Shared_variables[var_num] = t;
  }
  return LADR_GLOBAL_TERM.Shared_variables[var_num];
}  /* get_variable_term */

/*
This routine allocates and returns a term node with the same
symbol and arity as the given Term t.
*/

/* PUBLIC */
Term TermContainer::get_rigid_term_like(Term t) {
  Term t1 = get_term(ARITY(t));
  t1->private_symbol = t->private_symbol;
  return t1;
} 


/* DOCUMENTATION
This routine allocates and returns a term node with the given
symbol and arity.  If you already have a similar term node, say t,
(containing the symbol and arity you need) call get_rigid_term_like(t)
instead.
*/

/* PUBLIC */
Term TermContainer::get_rigid_term(const string &sym, int arity) {
  Term t1;
  int sn;
  if (arity > MAX_ARITY) {
    cout<<endl<<"Arity "<<arity<<" requested for symbol "<<sym<<endl;
    cerr<<endl<<"Arity "<<arity<<" requested for symbol "<<sym<<endl;
    fatal::fatal_error("get_rigid_term, arity too big");
  }
  
  t1 = get_term(arity);
  SymbolContainer S;
  sn = S.str_to_sn(sym, arity);
  if (sn >= MAX_SYM) fatal::fatal_error("get_rigid_term, too many symbols");
  t1->private_symbol = -sn;
  return t1;
}  /* get_rigid_term */



/* DOCUMENTATION
This routine can be used to allocate a term node if all you have is
the symbol ID and arity.  <I>If the arity is not correct
for the symbol ID, terrible things will happen!</I> 
<P>
If you have a similar term, use get_rigid_term_like() instead.
If you can afford the time to access the symbol table,
use sn_to_str() and get_rigid_term() instead.
*/

/* PUBLIC */
Term TermContainer::get_rigid_term_dangerously(int symnum, int arity) {
  Term t1 = get_term(arity);
  t1->private_symbol = -symnum;
  return t1;
}  /* get_rigid_term_dangerously */



/* DOCUMENTATION
This routine frees a term t and all of its subterms.  You should not
refer to t after calling zap_term(t).
*/

/* PUBLIC */
void TermContainer::zap_term(Term t)
{
  int i;
  if (t==NULL) return;
  for (i = 0; i < ARITY(t); i++) 
      zap_term(ARG(t,i));
  free_term(t);
  t=NULL;
 
}  /* zap_term */


bool TermContainer::term_ident(Term t1, Term t2) {
  if (t1->private_symbol != t2->private_symbol)
    return 0;
  else {
    int i;
    for (i = 0; i < ARITY(t1); i++)
      if (!term_ident(ARG(t1,i), ARG(t2,i))) return 0;
    return 1;
  }
}  /* term_ident */  



/* DOCUMENTATION
This routine copies a term.  Only the symbols and structure
are copied---any extra fields such as bits or u are
NOT copied.
*/

/* PUBLIC */
Term TermContainer::copy_term(Term t) {
  if (t == NULL)  return NULL;
  if (VARIABLE(t))  
      return  get_variable_term(VARNUM(t));
  int i;
  Term t2 = get_rigid_term_like(t);
  for (i = 0; i < ARITY(t); i++) {  
      ARG(t2,i) = copy_term(ARG(t,i));
  }  
  return t2;
}


/* PUBLIC */
bool TermContainer::ground_term(Term t) {
  if (VARIABLE(t))  return false;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      if (!ground_term(ARG(t,i))) return false;
    return true;
  }
}  /* ground_term */


/* DOCUMENTATION
This routine returns the greatest variable index of any variable int
the given term t.  If t is ground, -1 is returned.
*/

/* PUBLIC */
int TermContainer::biggest_variable(Term t){
  if (VARIABLE(t))  return VARNUM(t);
  else {
int i, max, v;
for (max = -1, i = 0; i < ARITY(t); i++) {
v = biggest_variable(ARG(t,i));
max = (v > max ? v : max);
}
    return max;
  }
}  /* biggest_variable */




/* DOCUMENTATION
Return the depth of a term.  Variables and constants have depth 0.
*/

/* PUBLIC */
int TermContainer::term_depth(Term t) {
    if (VARIABLE(t) || CONSTANT(t)) return 0;
    else {
            int i;
            int max = 0;
            for (i = 0; i < ARITY(t); i++) {
                int d = term_depth(ARG(t,i));
                max = IMAX(max,d);
            }
        return max+1;
    }
}  /* term_depth */



int TermContainer::symbol_count(Term t) {
  int i;
  int count = 0;
  for (i = 0; i < ARITY(t); i++)
    count += symbol_count(ARG(t,i));
  return count+1;
}  /* symbol_count  */



/* DOCUMENTATION
This function checks if Term t2 is identical to a subterm of Term t1,
including the case term_ident(t1,t2).  All identity checks are done
with term_ident(), so extra fields such as bits or u are not
checked.
*/

/* PUBLIC */
bool TermContainer::occurs_in(Term t1, Term t2) {
  if (term_ident(t1, t2))   return true;
  else {
    int i;
    for (i = 0; i < ARITY(t2); i++)
      if (occurs_in(t1, ARG(t2,i))) return true;
    return false;
  }
}  /* occurs_in */


//This routine prints (to FILE *fp) a term.  A newline is NOT printed.
/* PUBLIC */
void TermContainer::fprint_term(ostream &o, Term t){
  if (t == NULL)  o<<"fprint_term: NULL term\n";
  else {
        if (VARIABLE(t))  o<<"v"<<to_string(VARNUM(t));
        else {
                SymbolContainer S;
                S.fprint_sym(o, SYMNUM(t));
                if (COMPLEX(t)) {
                                    int i;
                                    o<<"(";
                                    for (i = 0; i < ARITY(t); i++) {
                                                                    fprint_term(o, ARG(t,i));
                                                                    if (i < ARITY(t)-1) o<<",";
                                    }
                                    o<<")";
                }
        }
      }
  o.flush();
}  /* fprint_term */


/* DOCUMENTATION
This (recursive) routine appends the string representation of a term to
a String_buf.  A newline is not included.
*/

/* PUBLIC */
void TermContainer::sprint_term(String_buf sb, Term t) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  if (t == NULL) cout<<"sprint_term: NULL term"<<endl;
  else {
        if (VARIABLE(t)) {
                            string s;
                            s="v"+to_string(VARNUM(t));
                            SB.sb_append(s);
        }
    else {
            SymbolContainer S;
            S.sprint_sym(sb, SYMNUM(t));
            if (COMPLEX(t)) {
                                int i;
                                SB.sb_append(string("("));
                                for (i = 0; i < ARITY(t); i++) {
                                                                sprint_term(sb, ARG(t,i));
                                                                if (i < ARITY(t)-1) SB.sb_append(string(","));
                                }
             SB.sb_append(string(")"));
            }
    }
  }
  sb=SB.get_string_buf();
}  /* sprint_term */




/* DOCUMENTATION
Convert a term to a string in standard prefix form.
The string is malloced, so call free on it when done with it.
*/

/* PUBLIC */
const string &TermContainer::term_to_string(Term t) {
  static string s;
  StrbufContainer SB;
  SB.new_string_buf(string(""));
  sprint_term(SB.get_string_buf(), t);
  s = SB.sb_to_malloc_string();
  SB.zap_string_buf();
  return s;
}  /* term_to_string */



/* DOCUMENTATION
This routine prints a term, followed by '\n' and fflush, to stdout.
If you don't want the newline, use fprint_term() instead.
If you want the term put into a string, use sprint_term() instead.
*/

/* PUBLIC */
void TermContainer::p_term(Term t) {
  fprint_term(cout, t);
  cout<<endl;
}  /* p_term */


/* DOCUMENTATION
This Boolean routine checks if all argumets of Term t are VARIABLEs.
(It is true also if t is a VARIABLE.)
*/

/* PUBLIC */
bool TermContainer::all_args_vars(Term t){
  if (VARIABLE(t)) return true;
  else {
        int i;
        for (i = 0; i < ARITY(t); i++)
            if (!VARIABLE(ARG(t,i))) return false;
        return true;
  }
}  /* all_args_vars */


/* DOCUMENTATION
Build and return a binary term with SYMNUM sn, first term a1, and
second term a2.
<P>
WARNING: if sn is not a binary symbol, bad things will happen!
*/

/* PUBLIC */
Term TermContainer::build_binary_term(int sn, Term a1, Term a2){
    Term t = get_rigid_term_dangerously(sn, 2);
    ARG(t,0) = a1;
    ARG(t,1) = a2;
    return(t);
}  /* build_binary_term */


/* DOCUMENTATION
Build and return a binary term with root str, first term a1, and
second term a2.
<p>
If you know the symnum, and you're certain it has arity 2, you
can use the faster routine build_binary_term() instead;
*/

/* PUBLIC */
Term TermContainer::build_binary_term_safe(const string &str, Term a1, Term a2) {
    SymbolContainer S;
    return build_binary_term(S.str_to_sn(str, 2), a1, a2);
}  /* build_binary_term_safe */


/* DOCUMENTATION
Build and return a unary term with SYMNUM sn and argument term a.
<P>
WARNING: if sn is not a unary symbol, bad things will happen!
*/

/* PUBLIC */
Term TermContainer::build_unary_term(int sn, Term a) {
    Term t = get_rigid_term_dangerously(sn, 1);
    ARG(t,0) = a;
    return(t);
}  /* build_unary_term */

/* DOCUMENTATION
Build and return a unary term with root str, argument a.
<p>
If you know the symnum, and you're certain it has arity 1, you
can use the faster routine build_unary_term() instead;
*/

/* PUBLIC */
Term TermContainer::build_unary_term_safe(const string &str, Term a){
    SymbolContainer S;
    return build_unary_term(S.str_to_sn(str, 1), a);
}  /* build_unary_term_safe */

/* DOCUMENTATION
In term t, replace all occurrences of Term target with <I>copies of</I>
Term replacement.  Free all of the replaced terms;
*/

/* PUBLIC */
Term TermContainer::subst_term(Term t, Term target, Term replacement){
  if (term_ident(t, target)) {
    zap_term(t);
    return copy_term(replacement);
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = subst_term(ARG(t,i), target, replacement);
    return t;
  }
}  /* subst_term */


/* DOCUMENTATION
In Term t, replace all CONSTANT terms containing SYMNUM symnum
with a variable containing VARNUM varnum.  Free the replaced constants
and return the result.
*/

/* PUBLIC */
Term TermContainer::subst_var_term(Term t, int symnum, int varnum) {
    if(CONSTANT(t) && SYMNUM(t)==symnum) {
                                            Term v=get_variable_term(varnum);
                                            zap_term(t);
                                            return v;
                                        } else {
                                                for (int i=0; i<ARITY(t); i++)
                                                    ARG(t,i) = subst_var_term(ARG(t,i), symnum, varnum);
                                                return t;
                                        }
}


/* DOCUMENTATION
This routine returns the greatest variable index in a term.
If the term is ground, -1 is returned.
*/
/* PUBLIC */
int TermContainer::greatest_variable(Term t) {
    if (VARIABLE(t)) return VARNUM(t);
    else {
            int i, max, v;
            for(max=-1,i=0; i<ARITY(t); i++) {
                v=greatest_variable(ARG(t,i));
                max= (v>max ? v : max);
            }
    return max;
    }
}


/* DOCUMENTATION
This function returns the greatest SYMNUM (of a CONSTANT or COMPLEX term)
in the given Term t.
If the term is a VARIABLE, return -1.
*/

/* PUBLIC */

int TermContainer::greatest_symnum_in_term(Term t) {
    if (VARIABLE(t)) return -1;
    else {
            int max= SYMNUM(t);
            for (int i=0; i<ARITY(t); i++) {
                int sm=greatest_symnum_in_term(ARG(t,i));
                max = (sm>max ? sm : max);
            }
            return max;
        }
}


/* DOCUMENTATION
In the given Term t, make the "container" field of t and each subterm,
except variables, point to (void *) p.
*/

/* PUBLIC */
void TermContainer::upward_term_links(Term t, void *p) {
  int i;
  if (!t) return;
  if (!VARIABLE(t)) {
    t->container = p;
    for (i = 0; i < ARITY(t); i++)
      upward_term_links(ARG(t,i), p);
  }
}  /* upward_term_links */


/* DOCUMENTATION
In the given Term t, check that the "container" field of t and each subterm,
except variables, point to (void *) p.
*/

/* PUBLIC */
bool TermContainer::check_upward_term_links(Term t, void *p){
  int i;
  if (!VARIABLE(t)) {
    if (t->container != p) return false;
    for (i = 0; i < ARITY(t); i++)
      if (!check_upward_term_links(ARG(t,i), p)) return false;
  }
  return true;
}  /* check_upward_term_links */


/* DOCUMENTATION
This function returns the number of occurrences of Term target in Term t.
The checks are made with term_ident().
*/

/* PUBLIC */
int TermContainer::occurrences(Term t, Term target) {
    if (term_ident(t,target)) return 1;
    else {
            int n=0;
            for(int i=0; i<ARITY(t); i++) n+=occurrences(ARG(t,i), target);
            return n;
    }
}

/*************
 *
 *   trm_set_vars_recurse()
 *
 *   There might be another (static) copy of this routine in clause.c.
 *
 *************/
Term TermContainer::trm_set_vars_recurse(Term t, string varnames[],int max_vars) {
    if(CONSTANT(t)) {
                    SymbolContainer S;
                    string name=S.sn_to_str(SYMNUM(t));
                    if (S.variable_name(name)) {
                        int i=0;
                        while(i<max_vars && !varnames[i].empty() && varnames[i]!=name) i++;
                        if(i==max_vars) fatal::fatal_error("trm_set_vars_recurse: max_vars");
                        else {
                            if(varnames[i].empty()) varnames[i]=name;
                            free_term(t);
                            t = get_variable_term(i);
                        }
                    }
    }
    else {
            for (int i = 0; i < ARITY(t); i++)
            ARG(t,i) = trm_set_vars_recurse(ARG(t,i), varnames, max_vars);
    }
    return t;
}

/* DOCUMENTATION
This routine traverses a term and changes the constants
that should be variables, into variables.  On input, the term
should have no variables.  The new variables are numbered
0, 1, 2 ... according the the first occurrence, reading from the
left.
<P>
A fatal error occurs if there are more than max_vars variables.
<P>
<I>If you are dealing with clauses, use clause_set_variables()
instead.</I>
*/

/* PUBLIC */
void TermContainer::term_set_variables(Term t, int max_vars){

  string a[MAX_VARS];
  string *vmap;
  int i;

  if (max_vars > MAX_VARS)    vmap = new string[max_vars];
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    vmap[i].clear();

  for (i = 0; i < ARITY(t); i++)
    ARG(t,i) = trm_set_vars_recurse(ARG(t,i), vmap, max_vars);

  if (max_vars > MAX_VARS) delete [] vmap;
}  /* term_set_variables */


/* DOCUMENTATION
This routine takes a nonnegative integer and returns
a constant Term with the string representation of the
integer as the constant symbol.
*/

/* PUBLIC */
Term TermContainer::nat_to_term(int n) {
  string s;
  if (n < 0)
    fatal::fatal_error("nat_to_term: negative term");
  return get_rigid_term(myString::int_to_str(n, s, 25), 0);
}  /* nat_to_term */



/* DOCUMENTATION
This routine takes an integer and returns the Term
representation.
*/

/* PUBLIC */
Term TermContainer::int_to_term(int i) {
    string s;
    Term t;
    t=get_rigid_term(myString::int_to_str(abs(i),s,25),0);
    if(i<0) t=build_unary_term_safe(string("-"),t);
    return t;
}



//implementação
/* DOCUMENTATION
This routine takes an Bool and returns the Term
representation.
*/

/* PUBLIC */
Term TermContainer::bool_to_term(bool val) {
    SymbolContainer S;
    return get_rigid_term( val ? S.true_sym() :S.false_sym() , 0);
}  /* bool_to_term */



/* DOCUMENTATION
This routine takes a double and returns
a constant Term with the string representation of the
double as the constant symbol.
*/

/* PUBLIC */
Term TermContainer::double_to_term(double d){
  string s1;
  s1=myString::double_to_str(d,350);
  return get_rigid_term(s1, 0);
}  /* double_to_term */



/* DOCUMENTATION
This routine takes a term, and if the term represents
an nonnegative integer, that integer is returned;
otherwise, -1 is returned.
*/

/* PUBLIC */
int TermContainer::natural_constant_term(Term t){
  if (!CONSTANT(t)) return -1;
  else {
        SymbolContainer S;
        return myString::natural_string(S.sn_to_str(SYMNUM(t)));
  }
}  /* natural_constant_term */


/* DOCUMENTATION
If the given terms are in a parent-child relatioship,
return the argument position (index) of the child.
Otherwise, return -1.
*/

/* PUBLIC */
int TermContainer::arg_position(Term parent, Term child) {
  int i;
  for (i = 0; i < ARITY(parent); i++) {
    if (ARG(parent,i) == child) return i;
  }
  return -1;
}  /* arg_position */

/* DOCUMENTATION
Does term t have the the given symbol and arity?
*/

/* PUBLIC */
bool TermContainer::is_term(Term t, const string &str, int arity) {
    SymbolContainer S;
    return (t != NULL) && (S.is_symbol(SYMNUM(t), str, arity));
}  /* is_term */


/* PUBLIC */
bool TermContainer::is_constant(Term t, const string &str) {
return is_term(t,str,0);
}

/* DOCUMENTATION
Return the print string associated with the given nonvariable term.
If the term is a variable, return NULL.
*/

/* PUBLIC */
const string &TermContainer::term_symbol(Term t) {
    SymbolContainer S;
    return VARIABLE(t) ? myString::null_string() : S.sn_to_str(SYMNUM(t)); //string() é a empty string ou NULL string
}  /* term_symbol */


/* DOCUMENTATION
Given a term, see if it represents an integer.
If so, set *result to the integer and return TRUE.
If not, return FALSE.
<P>
The term representation of a negative integer is
the function symbol "-" applied to a nonnegative integer.
*/



/* PUBLIC */
bool TermContainer::term_to_int(Term t, int *result) {
    SymbolContainer S;
    if(CONSTANT(t)) {
            return (myString::str_to_int(S.sn_to_str(SYMNUM(t)) , result) ? true: false);
    }
    else if (is_term(t,string ("-"),1)) {
            if (!CONSTANT(ARG(t,0))) return false;
            else {
                    if(myString::str_to_int(S.sn_to_str(SYMNUM(ARG(t,0))),result)) {
                            *result=-(*result);
                            return true;
                    }
            else return false;
            }
   }
   else return false;
}/* term_to_int */


/* DOCUMENTATION
Given a term, see if it represents a double.
If so, set *result to the double and return TRUE.
If not, return FALSE.
*/

/* PUBLIC */
bool TermContainer::term_to_double(Term t, double *result) {
  
  if (CONSTANT(t))  {
                        SymbolContainer S;
                        return myString::str_to_double(S.sn_to_str(SYMNUM(t)), result);
  }    
  else return false;
}  /* term_to_double */


/* DOCUMENTATION
Given a term, see if it represents an integer or a double.
If so, set *result (a double) to the number and return TRUE.
If not, return FALSE.
*/

/* PUBLIC */
bool TermContainer::term_to_number(Term t, double *result) {
  int i;
  if (term_to_int(t, &i)) {
    *result = (double) i;
    return true;
  }
  else if (term_to_double(t, result)) return true;
  else return false;
}  /* term_to_number */


/* DOCUMENTATION
*/

/* PUBLIC */
bool TermContainer::true_term(Term t) {
    SymbolContainer S;
    return is_term(t, S.true_sym(),0);
}  /* true_term */

/* DOCUMENTATION
*/

/* PUBLIC */
bool TermContainer::false_term(Term t) {
    SymbolContainer S;
    return is_term(t, S.false_sym(),0);
}


/* DOCUMENTATION
Given a term, see if it represents an Boolean value.
If so, set *result to the value and return TRUE.
If not, return FALSE.
*/

/* PUBLIC */
bool TermContainer::term_to_bool(Term t, bool *result){
  if (true_term(t)) {
    *result = true;
    return true;
  }
  else if (false_term(t)) {
    *result = false;
    return true;
  }
  else return false;
}  /* term_to_bool */



I2list TermContainer::symbols_in_term(Term t, I2list g) {
    if(VARIABLE(t)) {
                        I2listContainer G;
                        G.set_head(g);
                        g = G.multiset_add(SYMNUM(t));
                        for (int i=0; i<ARITY(t); i++)
                            g=symbols_in_term(ARG(t,i),g);
    }
    return g;
}


/* PUBLIC */
Ilist TermContainer::fsym_set_in_term(Term t) {
  I2listContainer a;
  Ilist aux;
  a.set_head(symbols_in_term(t, NULL));
  
  IlistContainer b;
  aux=b.multiset_to_set(a.get_head());
  b.set_head(aux);
  a.zap_i2list();
  return b.get_head();
}  /* fsym_set_in_term */


/* DOCUMENTATION
This routine collects the multiset of nonvariable symbols in a term.
An Ilist of symbol IDs (symnums) is returned
*/

/* PUBLIC */
I2list symbols_in_term(Term t, I2list g) {
  if (!VARIABLE(t)) {
    int i;
    I2listContainer G;
    g=G.multiset_add(g,SYMNUM(t));
    for (i = 0; i < ARITY(t); i++)
      g = symbols_in_term(ARG(t,i), g);
  }
  return g;
}  /* symbols_in_term */



/* DOCUMENTATION
This routine renumbers the variables of a term.  It is assumed
that vmap has been filled with -1 on the initial call and that
the size of vmap is at least max_vars.
<P>
This returns a Term instead of being void, in case the
given term is itself a variable.  (Recall that variables
may be shared, so we can't just change a variable's index.
*/

/* PUBLIC */
Term TermContainer::renum_vars_recurse(Term t, int vmap[], int max_vars) {
  if (VARIABLE(t)) {
                        int i = 0;
                        while (i < max_vars && vmap[i] != -1 && vmap[i] != VARNUM(t))
                        i++;
                        if (i == max_vars) fatal::fatal_error("renum_vars_recurse: too many variables");
                        if (vmap[i] == -1)  vmap[i] = VARNUM(t);
                        free_term(t);
                        return get_variable_term(i);
  }
  else {
            int i;
            for (i = 0; i < ARITY(t); i++)
                    ARG(t,i) = renum_vars_recurse(ARG(t,i), vmap, max_vars);
            return t;
  }
}  /* renum_vars_recurse */



/* DOCUMENTATION
This routine sets the variables of a term.  It is assumed
that vnames has been filled with NULL on the initial call and that
the size of vnames is at least max_vars.
<P>
This returns a Term instead of being void, in case the
given term is itself becomes a variable.
*/

/* PUBLIC */
Term TermContainer::set_vars_recurse(Term t, string vnames[], int max_vars) {
  SymbolContainer S;
  
  
  if (CONSTANT(t)) {
                   
                    string name= S.sn_to_str(SYMNUM(t));
                    if (S.variable_name(name)) { //if the term name is recognized as variable, turn it into variable
                        int i = 0;
                        while (i < max_vars && vnames[i] != myString::null_string() && vnames[i] != name) i++;
                        if (i == max_vars) fatal::fatal_error("set_vars_recurse: max_vars");
                        else {
                               if (vnames[i] == myString::null_string())   vnames[i] = name;
                               free_term(t);
                               t = get_variable_term(i); //now get a variable term
                    }
                 }
  }
  else {
        int i;
        for (i = 0; i < ARITY(t); i++) 
           ARG(t,i) = set_vars_recurse(ARG(t,i), vnames, max_vars);
            
  }
  return t;
}  /* set_vars_recurse */



/* PUBLIC */
I2list TermContainer::multiset_of_vars(Term t, I2list vars) {
        I2listContainer AUX;
        
        if (VARIABLE(t))  return AUX.multiset_add(vars, VARNUM(t));
        else {
                int i;
                for (i = 0; i < ARITY(t); i++)
                    vars = multiset_of_vars(ARG(t,i), vars);
                return vars;
  }
}  /* multiset_of_vars */


I2list TermContainer::multiset_vars(Term t){
  return multiset_of_vars(t, NULL);
}  /* multiset_vars */


/* DOCUMENTATION
See set_of_variables(t).
*/

/* PUBLIC */
Plist TermContainer::set_of_vars(Term t, Plist vars) {
  PlistContainer VARS;
    if (VARIABLE(t)) {
        VARS.set_head(vars);
        if (VARS.plist_member(t)) return vars;
        else return VARS.plist_prepend((void *)t);
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      vars = set_of_vars(ARG(t,i), vars);
    return vars;
  }
}  /* set_of_vars */


/* DOCUMENTATION
Given a Term, return the set of variables.
*/

/* PUBLIC */
Plist TermContainer::set_of_variables(Term t){
    return set_of_vars(t, NULL);
}  /* set_of_variables */


/* DOCUMENTATION
Given a Term, return the set of variables.
*/

/* PUBLIC */
int TermContainer::number_of_vars_in_term(Term t) {
  Plist p = set_of_vars(t, NULL);
  PlistContainer P;
  P.set_head(p);
  int n = P.plist_count();
  P.zap_plist();
  return n;
}  /* number_of_vars_in_term */

/* DOCUMENTATION
See set_of_ivariables(t).
*/

/* PUBLIC */
Ilist TermContainer::set_of_ivars(Term t, Ilist ivars) {
  if (VARIABLE(t)) {
        IlistContainer IVARS;
        IVARS.set_head(ivars);
        if (IVARS.ilist_member(VARNUM(t))) return ivars;
        else return IVARS.ilist_prepend(VARNUM(t));
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ivars = set_of_ivars(ARG(t,i), ivars);
    return ivars;
  }
}  /* set_of_ivars */


/* DOCUMENTATION
Given a Term, return the set of integers corresponding to its variables.
*/

/* PUBLIC */
Ilist TermContainer::set_of_ivariables(Term t)
{
  return set_of_ivars(t, NULL);
}  /* set_of_ivariables */



/* DOCUMENTATION
*/

/* PUBLIC */
bool TermContainer::variables_subset(Term t1, Term t2) {
  PlistContainer T1_VARS;
  PlistContainer T2_VARS;

  Plist t1_vars = set_of_variables(t1);
  Plist t2_vars = set_of_variables(t2);

  T1_VARS.set_head(t1_vars);
  T2_VARS.set_head(t2_vars);

  bool ok = T2_VARS.plist_subset(t1_vars); //se t1 é subset de t2

  T1_VARS.zap_plist();
  T2_VARS.zap_plist();

  return ok;
}  /* variables_subset */



/* PUBLIC */
bool TermContainer::variables_multisubset(Term a, Term b) {
#if 1
  I2listContainer I;
  I2list a_vars = multiset_vars(a);
  I2list b_vars = multiset_vars(b);
 
  bool ok = I.i2list_multisubset(a_vars, b_vars);
  
  I.set_head(a_vars);
  I.zap_i2list();
  I.set_head(b_vars);
  I.zap_i2list();
  return ok;
  
  
#else  /* old version */

  Plist a_vars = set_of_variables(a);
  Plist p;
  bool ok = true;

  for (p = a_vars; p && ok; p = p->next)
    ok = occurrences(b, p->v) >= occurrences(a, p->v);
  PlistContainer A_VARS;
  A_VARS.set_head(a_vars);
  A_VARS.zap_plist();
  return ok;

#endif
}  /* variables_multisubset */


/* DOCUMENTATION
*/

/* PUBLIC */
Term TermContainer::term_at_pos(Term t, Ilist pos) {
  if (pos == NULL)  return t;
  else {
        if (pos->i > ARITY(t))  return NULL;
        else return term_at_pos(ARG(t,pos->i - 1), pos->next);
  }
}  /* term_at_pos */


Ilist TermContainer::pos_of_subterm(Term t, Term subterm){
  if (VARIABLE(t))  return NULL;
  else if (t == subterm) {
    /* We need to let the caller know that we found it,
       and we also need to return the position vector (NULL).
       The easiest way I can see to do that is to return
       a non-NULL position consisting of a "terminator"
       which will have to be removed later.
    */
    IlistContainer A;
    return A.ilist_prepend(INT_MAX);
    }
    
    else {
            int i;
            Ilist p = NULL;
            for (i = 0; i < ARITY(t) && p == NULL; i++)
                p = pos_of_subterm(ARG(t, i), subterm);
            IlistContainer B;
            B.set_head(p);
            return p ? B.ilist_prepend(i) : NULL;
  }
}  /* pos_of_subterm */


/* DOCUMENTATION
*/

/* PUBLIC */
Ilist TermContainer::position_of_subterm(Term t, Term subterm){
  Ilist pos = pos_of_subterm(t, subterm);
  IlistContainer POS;
  POS.set_head(pos);
  if (pos == NULL) return NULL;
  else {
         return POS.ilist_remove_last();
  }
}  /* position_of_subterm */


/* DOCUMENTATION
Return the number of occurrences of a symbol in a term.
*/

/* PUBLIC */
int TermContainer::symbol_occurrences(Term t, int symnum){
  if (VARIABLE(t))   return 0;
  else {
    int n = (SYMNUM(t) == symnum ? 1 : 0);
    int i;
    for (i = 0; i < ARITY(t); i++)
      n += symbol_occurrences(ARG(t,i), symnum);
    return n;
  }
}  /* symbol_occurrences */



/* DOCUMENTATION
Is the Term a nonvariable with distinct variables as arguments?
(Constants satisfy this.)
*/

/* PUBLIC */
bool TermContainer::args_distinct_vars(Term t) {
#if 1
  if (VARIABLE(t))  return false;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (!VARIABLE(ARG(t,i))) return false;
      else {
            int j;
            for (j = 0; j < i; j++)
                if (VARNUM(ARG(t,i)) == VARNUM(ARG(t,j)))   return false;
            }
    }
    return true;
  }
#else
  if (VARIABLE(t))   return false;
  else {
    int *p = calloc(ARITY(t), sizeof(int));
    int i;
    bool ok = true;
    for (i = 0; i < ARITY(t) && ok; i++) {
Term s = ARG(t,i);
if (!VARIABLE(s)) ok = false;
      else if (p[VARNUM(s)]) ok = false;
      else p[VARNUM(s)] = true;
    }
    free(p);
    return ok;
  }
#endif
}  /* args_distinct_vars */



/* DOCUMENTATION
*/

/* PUBLIC */
unsigned TermContainer::hash_term(Term t) {
  if (VARIABLE(t))  return VARNUM(t);
  else {
        int i;
        unsigned x = SYMNUM(t);
        for (i = 0; i < ARITY(t); i++)
            x = (x << 3) ^ hash_term(ARG(t,i));
        return x;
  }
}  /* hash_term */


/* PUBLIC */
bool TermContainer::skolem_term(Term t) {
  SymbolContainer S;  
  return S.is_skolem(SYMNUM(t));
}  /* skolem_term */




//implementação
/* PUBLIC */
bool TermContainer::contains_skolem_term(Term t){
  if (VARIABLE(t)) return false;
  else if (skolem_term(t))   return true;
  else {
        int i;
        for (i = 0; i < ARITY(t); i++)
        if (contains_skolem_term(ARG(t,i))) return true;
        return false;
  }
}  /* contains_skolem_term */



/* DOCUMENTATION
Build constant Term.
*/

/* PUBLIC */
Term TermContainer::term0(const string &sym) {
  return get_rigid_term(sym, 0);
}  /* term0 */


/* DOCUMENTATION
Build a unary term.  The argument Term is not copied.
*/

/* PUBLIC */
Term TermContainer::term1(const string &sym, Term arg) {
  Term t = get_rigid_term(sym, 1);
  ARG(t,0) = arg;
  return t;
}  /* term1 */


/* DOCUMENTATION
Build a binary term.  The argument Terms are not copied.
*/

/* PUBLIC */
Term TermContainer::term2(const string &sym, Term arg1, Term arg2) {
  Term t = get_rigid_term(sym, 2);
  ARG(t,0) = arg1;
  ARG(t,1) = arg2;
  return t;
}  /* term2 */

/* DOCUMENTATION
*/

/* PUBLIC */
bool TermContainer::symbol_in_term(int symnum, Term t)
{
  if (VARIABLE(t))   return false;
  else if (SYMNUM(t) == symnum)    return true;
  else {
        int i;
        for (i = 0; i < ARITY(t); i++)
            if (symbol_in_term(symnum, ARG(t,i))) return true;
        return false;
  }
}  /* symbol_in_term */


/* DOCUMENTATION
If variables are ignored, are the terms identical?
*/

/* PUBLIC */
bool TermContainer::same_structure(Term a, Term b) {
  if (VARIABLE(a) || VARIABLE(b)) return VARIABLE(a) && VARIABLE(b);
  else if (SYMNUM(a) != SYMNUM(b)) return false;
  else {
    int i;
    for (i = 0; i < ARITY(a); i++)
      if (!same_structure(ARG(a,i), ARG(b,i))) return false;
    return true;
  }
}  /* same_structure */


/* PUBLIC */
Plist TermContainer::copy_plist_of_terms(Plist terms) {
  if (terms == NULL)   return NULL;
  else {
    Plist tail = copy_plist_of_terms(terms->next);
    PlistContainer HEAD;
    HEAD.plist_append((void *) NULL); //para criar um elemento
    Plist head = HEAD.get_head();
    head->v = copy_term(Term(terms->v));
    head->next = tail;
    return head;
  }
}  /* copy_plist_of_terms */


/* DOCUMENTATION
Free a Plist of terms.
*/

/* PUBLIC */
void TermContainer::zap_plist_of_terms(Plist lst) {
  Plist p = lst;
  while (p != NULL) {
    Plist p2 = p;
    p = p->next;
    zap_term(Term(p2->v));
    PlistContainer P2;
    P2.free_plist(p2);
  }
}  /* zap_plist_of_terms */


/* DOCUMENTATION
This function checks if an atom is an equality atom (positive or negative)
for the purposes of paramodulation and demodulation.
*/

/* PUBLIC */
bool TermContainer::eq_term(Term a){
  SymbolContainer S;  
  return S.is_eq_symbol(SYMNUM(a));
}  /* eq_term */



/* PUBLIC */
Plist TermContainer::plist_of_subterms(Term t) {
  PlistContainer SUBTERMS;
  int i;
  for (i = 0; i < ARITY(t); i++)
    SUBTERMS.plist_append(ARG(t,i));
  return SUBTERMS.get_head();
}  /* plist_of_subterms */



/* DOCUMENTATION
This function checks if a term is a member of a Plist.
The function term_ident(t1,t2) is used.
*/

/* PUBLIC */
bool TermContainer::tlist_member(Term t, Plist lst) {
  if (lst == NULL)  return false;
  else if (term_ident(Term(lst->v), t)) return true;
  else    return tlist_member(t, lst->next);
}  /* tlist_member */


/* DOCUMENTATION
*/

/* PUBLIC */
int TermContainer::position_of_term_in_tlist(Term t, Plist lst){
  Plist p;
  int i;
  for (p = lst, i = 1; p; p = p->next, i++)
    if (term_ident(Term(p->v), t)) return i;
  return -1;
}  /* position_of_term_in_tlist */


/* PUBLIC */
bool TermContainer::tlist_subset(Plist a, Plist b){
  if (a == NULL)  return true;
  else return tlist_member(Term(a->v), b) && tlist_subset(a->next, b);
}  /* tlist_subset */


bool TermContainer::tlist_set(Plist a) {

  if (a == NULL)    return true;
  else
    return !tlist_member(Term(a->v), a->next) && tlist_set(a->next);
}  /* tlist_set */



/* PUBLIC */
Plist TermContainer::free_vars_term(Term t, Plist vars) {
  if (VARIABLE(t))    fatal::fatal_error("free_vars_term, VARIABLE term");
  if (ARITY(t) == 0) {
    SymbolContainer S;  
    if (S.variable_name(S.sn_to_str(SYMNUM(t))) && !tlist_member(t, vars)) {
        PlistContainer VARS;
        VARS.set_head(vars);
        VARS.plist_append(copy_term(t));
        vars=VARS.get_head();
    }
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      vars = free_vars_term(ARG(t,i), vars);
  }
  return vars;
}  /* free_vars_term */


void TermContainer::init_any_consts() {
    SymbolContainer S;
    LADR_GLOBAL_TERM.AnyConsts[0] = S.str_to_sn(ANYCONST, 0);
    
    char str[64]; /* this should be enough for any reasonable 
                     MAX_ANYCONSTS, but don't use too long ANYCONST. */
    int i;
    for (i=1; i<MAX_ANYCONSTS; i++) {
      snprintf(str, 64, "%s_%d", ANYCONST, i);
      LADR_GLOBAL_TERM.AnyConsts[i] = S.str_to_sn(str, 0);
    } 
    LADR_GLOBAL_TERM.AnyConstsInited = true;
}




int TermContainer::any_const(int sn) {
  /* Initialize AnyConsts if necessary */
  if (LADR_GLOBAL_TERM.AnyConstsInited == false) {
    init_any_consts();
  }

  /* Lookup sn in AnyConsts */
  int i;
  for (i=0; i<MAX_ANYCONSTS; i++) {
    if (LADR_GLOBAL_TERM.AnyConsts[i] == sn) {
      return i;
    }
  }

  return -1;
}


int TermContainer::any_const_sn(int n)
{
  /* Initialize AnyConsts if necessary */
  if (LADR_GLOBAL_TERM.AnyConstsInited == false) {
    init_any_consts();
  }

  return LADR_GLOBAL_TERM.AnyConsts[n];
}
