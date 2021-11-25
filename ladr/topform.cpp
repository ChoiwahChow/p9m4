#include "ladrvglobais.h"
#include "topform.h"
#include "fatal.h"
#include "memory.h"
#include "mystring.h"
#include "just.h"
#include <iostream>
#include <iomanip>




GlobalTopform::GlobalTopform() {
    Topform_gets=0;
    Topform_frees=0;
}

GlobalTopform::~GlobalTopform() {
}



TopformContainer::TopformContainer() {

    
}

TopformContainer::~TopformContainer() {

    
}


Topform TopformContainer::get_topform() {
  Topform p = (Topform) Memory::memCNew(sizeof(struct topform));
  LADR_GLOBAL_TOPFORM.Topform_gets++;
  return(p);
}

void TopformContainer::free_topform(Topform p) {
  Memory::memFree((void *)p, sizeof(struct topform));
  LADR_GLOBAL_TOPFORM.Topform_frees++;
} 

void TopformContainer::fprint_topform_mem(ostream &o, bool heading) {
  int n;
  if (heading)
      o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct topform);
  o<<"topoform      ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_TOPFORM.Topform_gets;
  o<<setw(11)<<LADR_GLOBAL_TOPFORM.Topform_frees;
  o<<setw(11)<<LADR_GLOBAL_TOPFORM.Topform_gets-LADR_GLOBAL_TOPFORM.Topform_frees;
  o<<setw(9)<<(( LADR_GLOBAL_TOPFORM.Topform_gets-LADR_GLOBAL_TOPFORM.Topform_frees)*n )/1024<<"K"<<endl;
} 


void TopformContainer::p_topform_mem() {
  fprint_topform_mem(cout, true);
} 


void TopformContainer::zap_topform(Topform tf) {
  
  FormulaContainer F;
  AttributeContainer A;
  JustContainer J;
  LADRV_GLOBAIS_INST.Lit.zap_literals(tf->literals);
  F.zap_formula(tf->formula);
  A.zap_attributes(tf->attributes);
  free_topform(tf);
}

void TopformContainer::fprint_clause(ostream &o, Topform c) {
  SymbolContainer S;
  TermContainer T;
  if (c == NULL)  o<<"fprint_clause: NULL clause"<<endl;
  else {
    Literals lit;

    if (c->id > 0) o<<c->id;

    if (c->literals == NULL)  o<<S.false_sym();
    else {
      for (lit = c->literals; lit != NULL; lit = lit->next) {
        if (!lit->sign) o<<S.not_sym();
        T.fprint_term(o, lit->atom);
        #if 0
            if (maximal_literal_check(lit))
            fprintf(fp, "[max]");
        #endif
            if (lit->next != NULL) o<<" "<<S.or_sym()<<" ";
	  
      }
    }
    o<<endl;
  }
} 

void TopformContainer::p_clause(Topform c) {
  fprint_clause(cout, c);
} 


void TopformContainer::upward_clause_links(Topform c) {
  Literals lit;
  TermContainer T;
  for (lit = c->literals; lit != NULL; lit = lit->next)
    T.upward_term_links(lit->atom, c);
} 

Topform TopformContainer::term_to_clause(Term t) {
  TermContainer T;
  SymbolContainer S;
  AttributeContainer A;
  
  Topform c=get_topform();
  
  Term t_start;

  if (T.is_term(t, S.attrib_sym(), 2)) {
    c->attributes = A.term_to_attributes(ARG(t,1), S.attrib_sym());
    t_start = ARG(t,0);
  }
  else
    t_start = t;

  c->literals = LADRV_GLOBAIS_INST.Lit.term_to_literals(t_start, NULL);
  upward_clause_links(c);
  return(c);
} 


Topform TopformContainer::term_to_topform(Term t, bool is_formula) {
  SymbolContainer S;
  TermContainer T;
  AttributeContainer A;
  FormulaContainer F;

  Topform c = get_topform();
  Term t_start;

  if (T.is_term(t, S.attrib_sym(), 2)) {
    c->attributes = A.term_to_attributes(ARG(t,1), S.attrib_sym());
    t_start = ARG(t,0);
  }
  else
    t_start = t;

  c->is_formula = is_formula;
  if (is_formula)
    c->formula = F.term_to_formula(t_start);
  else {
    c->literals = LADRV_GLOBAIS_INST.Lit.term_to_literals(t_start, NULL);
    upward_clause_links(c);
  }
  return(c);
} 



Term TopformContainer::topform_to_term(Topform tf) {
  FormulaContainer F;
  SymbolContainer S;

  TermContainer T;
  AttributeContainer A;
  Term t;

  if (tf->is_formula)  t = F.formula_to_term(tf->formula);
  else {
    if (tf->literals == NULL)  t = T.get_rigid_term(S.false_sym(), 0);
    else t = LADRV_GLOBAIS_INST.Lit.literals_to_term(tf->literals);
  }
  if (tf->attributes == NULL)  return t;
  else return T.build_binary_term(S.str_to_sn(S.attrib_sym(), 2),t, A. attributes_to_term(tf->attributes, S.attrib_sym()));
}


Term TopformContainer::topform_to_term_without_attributes(Topform tf) {
  TermContainer T;
  FormulaContainer F;
  SymbolContainer S;
 
  Term t;
  if (tf->is_formula)  t = F.formula_to_term(tf->formula);
  else {
    if (tf->literals == NULL)
      t = T.get_rigid_term(S.false_sym(), 0);
    else
      t = LADRV_GLOBAIS_INST.Lit.literals_to_term(tf->literals);
  }
  return t;
}  



void TopformContainer::clause_set_variables(Topform c, int max_vars) {
  string a[MAX_VARS];//MAX_VAR string names
  string *vmap = nullptr;
  TermContainer T;
  AttributeContainer A;

  PlistContainer P;
  int i;
  Literals lit;

  if (max_vars > MAX_VARS) vmap=new string[max_vars];
  else vmap=a;

  for (i = 0; i < max_vars; i++)  vmap[i] = myString::null_string();

  for (lit = c->literals; lit != NULL; lit = lit->next)
    lit->atom = T.set_vars_recurse(lit->atom, vmap, max_vars);

  /* Now do any answer literals (with the same vmap). */
  if (c->attributes) {
    Plist clause_vars = LADRV_GLOBAIS_INST.Lit.vars_in_clause(c->literals);
    Plist attr_vars;
    A.set_vars_attributes(c->attributes, vmap, max_vars);
    attr_vars = A.vars_in_attributes(c->attributes);
    /* Make sure that answer vars also occur in ordinary literals. */
    P.set_head(clause_vars);
    if (!P.plist_subset(attr_vars)) { //if attr_vars are subset of clause_vars
      Plist p;
      printf("Variables in answers must also occur ordinary literals:\n");
      p_clause(c);
      for (p = attr_vars; p; p = p->next) {
        P.set_head(clause_vars);  
        if (!P.plist_member(p->v)) {
            Term t = (Term) p->v;
            cout<<"Answer variable not in ordinary literal:"<<vmap[VARNUM(t)]<<endl;
        }
      }
      
      fatal::fatal_error("clause_set_variables, answer variable not in literal");
    }
    P.set_head(clause_vars);
    P.zap_plist();
    P.set_head(attr_vars);
    P.zap_plist();
  }
  
  if (max_vars > MAX_VARS) 
        delete [] vmap;
  

  c->normal_vars = true;
}  




void TopformContainer::renumber_variables(Topform c, int max_vars) {
  int a[MAX_VARS], *vmap=nullptr;
  int i;
  Literals lit;
  TermContainer T;  
  AttributeContainer A;
  
  if (max_vars > MAX_VARS)
    vmap = new int[max_vars];
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)    a[i] = -1;


  for (lit = c->literals; lit != NULL; lit = lit->next) {
    /* There's a special case in which the atom can be null. */
    if (lit->atom)
      lit->atom = T.renum_vars_recurse(lit->atom, vmap, max_vars);
  }

  /* Now do any inheritable attributes (with the same vmap). */
  A.renumber_vars_attributes(c->attributes, vmap, max_vars);
  if (max_vars > MAX_VARS) delete[] vmap;
  

  c->normal_vars = true;
}  /* renumber_variables */

void TopformContainer::term_renumber_variables(Term t, int max_vars) {
  int a[MAX_VARS], *vmap=nullptr;
  int i;
  TermContainer T;
  
  if (max_vars > MAX_VARS)
    vmap = new int [max_vars];
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    a[i] = -1;

  t = T.renum_vars_recurse(t, vmap, max_vars);
  
  if (max_vars > MAX_VARS) delete [] vmap;
    
} 


Plist TopformContainer::renum_vars_map(Topform c)
{
  int a[MAX_VARS];
  int i;
  Literals lit;
  TermContainer T;  
  ListtermContainer LT;
  AttributeContainer A;
  PlistContainer P;
  for (i = 0; i < MAX_VARS; i++)
    a[i] = -1;

  for (lit = c->literals; lit != NULL; lit = lit->next)
    lit->atom = T.renum_vars_recurse(lit->atom, a, MAX_VARS);

  /* Now renumber any inheritable attributes (with the same vmap). */

  A.renumber_vars_attributes(c->attributes, a, MAX_VARS);

  /* Build the list of pairs. */
  
  for (i = 0; i < MAX_VARS; i++) {
    /* a[i] -> i */
    if (a[i] != -1 && a[i] != i) {
      Term v1 = T.get_variable_term(a[i]);
      Term v2 = T.get_variable_term(i);
      Term pair = LT.listterm_cons(v1, v2);
      P.plist_append(pair);
    }
  }
  c->normal_vars = true;
  return P.get_head();
} 

bool TopformContainer::check_upward_clause_links(Topform c) {
  Literals lit;
  TermContainer T;
  for (lit = c->literals; lit != NULL; lit = lit->next) {
    if (!T.check_upward_term_links(lit->atom, c))
      return false;
  }
  return true;
} 

Topform TopformContainer::copy_clause(Topform c) {

  Topform c2 = get_topform();
  c2->literals = LADRV_GLOBAIS_INST.Lit.copy_literals(c->literals);
  upward_clause_links(c2);
  return c2;
}

Topform TopformContainer::copy_clause_with_flags(Topform c) {
  Topform c2 = get_topform();

  c2->literals = LADRV_GLOBAIS_INST.Lit.copy_literals_with_flags(c->literals);
  upward_clause_links(c2);
  return c2;
}


Topform TopformContainer::copy_clause_with_flag(Topform c, int flag) {
 
  Topform c2 = get_topform();
  c2->literals = LADRV_GLOBAIS_INST.Lit.copy_literals_with_flag(c->literals, flag);
  upward_clause_links(c2);
  return c2;
}

void TopformContainer::inherit_attributes(Topform par1, Context s1, Topform par2, Context s2,Topform child) {
  AttributeContainer A;
  Attribute a1 = par1 ? A.inheritable_att_instances(par1->attributes, s1) : NULL;
  Attribute a2 = par2 ? A.inheritable_att_instances(par2->attributes, s2) : NULL;
  child->attributes = A.cat_att(child->attributes, A.cat_att(a1, a2));
}  

void TopformContainer::gather_symbols_in_topform(Topform c, I2list *rsyms, I2list *fsyms) {
  FormulaContainer F;
  
  if (c->is_formula)   F.gather_symbols_in_formula(c->formula, rsyms, fsyms);
  else {
    Literals lit;
    for (lit = c->literals; lit; lit = lit->next)  F.gather_symbols_in_formula_term(lit->atom, rsyms, fsyms);
  }
}  /* gather_symbols_in_topform */


void TopformContainer::gather_symbols_in_topforms(Plist lst, I2list *rsyms, I2list *fsyms) {
  Plist p;
  for (p = lst; p; p = p->next)
    gather_symbols_in_topform(Topform (p->v), rsyms, fsyms);
}


Ilist TopformContainer::fsym_set_in_topforms(Plist lst) {
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  
  IlistContainer I;
  I2listContainer I2;
  
  gather_symbols_in_topforms(lst, &rsyms, &fsyms);
  p = I.multiset_to_set(fsyms);
  I2.set_head(rsyms);
  I2.zap_i2list();
  I2.set_head(fsyms);
  I2.zap_i2list();
  return p;
} 


Ilist TopformContainer::rsym_set_in_topforms(Plist lst) {
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  IlistContainer I;
  I2listContainer I2;
  gather_symbols_in_topforms(lst, &rsyms, &fsyms);
  p = I.multiset_to_set(rsyms);
  I2.set_head(rsyms);
  I2.zap_i2list();
  I2.set_head(fsyms);
  I2.zap_i2list();
  return p;
} 


bool TopformContainer::min_depth(Literals lit) {
  Topform c =(Topform) lit->atom->container;
  TermContainer T;
  int d = T.term_depth(lit->atom);
  Literals l;
  for (l = c->literals; l; l = l->next) {
    if (T.term_depth(l->atom) < d)
      return false;
  }
  return true;
}


bool TopformContainer::initial_clause(Topform c) {
  return c->initial;
} 



Term TopformContainer::topform_properties(Topform c) {
  Term lst;
  Literals lit;
  ListtermContainer LT;
  
  TermContainer T;
  int i;
  Term props = LT.get_nil_term();

  /* maximal literals */

  lst = LT.get_nil_term();
  for (lit = c->literals, i = 1; lit; lit = lit->next, i++) {
    if (Maximal::maximal_literal(c->literals, lit, (int) CheckType::FLAG_CHECK))
      lst = LT.listterm_cons(T.nat_to_term(i),lst);
  }
  lst = LT.listterm_reverse(lst);
  props = LT.listterm_cons(T.build_unary_term_safe("maximal", lst), props);

  /* maximal_signed literals */

  lst = LT.get_nil_term();
  for (lit = c->literals, i = 1; lit; lit = lit->next, i++) {
    if (Maximal::maximal_signed_literal(c->literals, lit, (int) CheckType::FLAG_CHECK))
      lst = LT.listterm_cons(T.nat_to_term(i),lst);
  }
  lst = LT.listterm_reverse(lst);
  props = LT.listterm_cons(T.build_unary_term_safe("maximal_signed",lst),props);

  /* maximal_signed literals */

  lst = LT.get_nil_term();
  for (lit = c->literals, i = 1; lit; lit = lit->next, i++) {
    if (Maximal::selected_literal(lit))
      lst = LT.listterm_cons(T.nat_to_term(i),lst);
  }
  lst = LT.listterm_reverse(lst);
  props = LT.listterm_cons(T.build_unary_term_safe("selected",lst),props);

  return LT.listterm_reverse(props);

}

void TopformContainer::append_label_attribute(Topform tf, const string &s) {
    AttributeContainer A;
    int x=A.label_att();
    tf->attributes = A.set_string_attribute(tf->attributes, x, s);
} 


void TopformContainer::delete_label_attribute(Topform tf, const string &s) {
    AttributeContainer A;
    A.delete_string_attribute(tf->attributes,s);
    
}

OrderType TopformContainer::cl_id_compare(Topform c1, Topform c2) {
  if (c1->id < c2->id)     return OrderType::LESS_THAN;
  else if (c1->id > c2->id)    return OrderType::GREATER_THAN;
  else    return OrderType::SAME_AS;
} 


// BV(2016-may-27): Order clauses by matched hint ids.  Clauses that
// match the same hint are ordered by their clause id.
//
// Clauses that match the same hint will be adjacent in the ordering
// (e.g., in a hint_age queue).  Tie breaking is needed to support
// operations on the relevant index structures.
OrderType TopformContainer::cl_hint_id_compare(Topform c1, Topform c2) {

  if (c1->matching_hint != NULL && c2->matching_hint != NULL) 
  {
     if (c1->matching_hint->id < c2->matching_hint->id)   return OrderType::LESS_THAN;
     else if (c1->matching_hint->id > c2->matching_hint->id) return OrderType::GREATER_THAN;
// match same hint
     else if (c1->id < c2->id)   return OrderType::LESS_THAN;
     else if (c1->id > c2->id)   return OrderType::GREATER_THAN;
     else return OrderType::SAME_AS;
  }

  // compare a hint matcher to a non hint matcher
  else if (c1->matching_hint != NULL)   return OrderType::LESS_THAN;

  // compare a non hint matcher to a hint matcher
  else if (c2->matching_hint != NULL)   return OrderType::GREATER_THAN;

  // compare two non hint matchers
  else if (c1->id < c2->id)  return OrderType::LESS_THAN;
  else if (c1->id > c2->id)  return OrderType::GREATER_THAN;
  else  return OrderType::SAME_AS;

}  /* cl_hint_id_compare */

    



OrderType TopformContainer::cl_wt_id_compare(Topform c1, Topform c2) {
  if (c1->weight < c2->weight)    return OrderType::LESS_THAN;
  else if (c1->weight > c2->weight)     return OrderType::GREATER_THAN;
  else if (c1->id < c2->id)    return OrderType::LESS_THAN;
  else if (c1->id > c2->id)    return OrderType::GREATER_THAN;
  else    return OrderType::SAME_AS;
} 

bool TopformContainer::negative_clause_possibly_compressed(Topform c){
 
  if (c->compressed!=NULL)     return c->neg_compressed;
  else  return LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals);
} 
