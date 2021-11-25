#include "ladrvglobais.h"
#include "formula.h"
#include "memory.h"
#include "fatal.h"
#include "term.h"
#include "symbols.h"
#include "glist.h"
#include "mystring.h"
#include <iostream>
#include <iomanip>
#include <cstddef>




GlobalFormula::GlobalFormula() {
    Formula_gets=0;
    Formula_frees=0;
    Arg_mem=0; 
}

GlobalFormula::~GlobalFormula() {
}


FormulaContainer::FormulaContainer(){}
FormulaContainer::~FormulaContainer() {}


Formula FormulaContainer::get_formula(int arity) {
  Formula p =(Formula)  Memory::memCNew(sizeof(formula));
  if (arity==0)  
      p->kids=NULL;
  else  
        p->kids = (Formula *) Memory::memCNew(arity * sizeof(Formula));
  
  p->arity = arity;
  LADR_GLOBAL_FORMULA.Formula_gets++;
  LADR_GLOBAL_FORMULA.Arg_mem += arity;
  return(p);
} 

void FormulaContainer::free_formula(Formula p) {
  if (p->excess_refs != 0)
    fatal::fatal_error("free_formula: freeing shared formula");
  Memory::memFree((void *)p->kids, (p->arity) * sizeof(Formula)); //free the kids pointer
  LADR_GLOBAL_FORMULA.Arg_mem -= p->arity;
  if (p->qvar) 
      delete p->qvar;
  Memory::memFree((void *)p, sizeof(struct formula)); //free da estrutura
  LADR_GLOBAL_FORMULA.Formula_frees++;
} 



void FormulaContainer::fprint_formula_mem(ostream &o, bool heading) {
  int n;
  if (heading)
	o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct formula);
  o<<"formula     ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_FORMULA.Formula_gets;
  o<<setw(11)<<LADR_GLOBAL_FORMULA.Formula_frees;
  o<<setw(11)<<LADR_GLOBAL_FORMULA.Formula_gets - LADR_GLOBAL_FORMULA.Formula_frees;
  o<<setw(9)<<((LADR_GLOBAL_FORMULA.Formula_gets - LADR_GLOBAL_FORMULA.Formula_frees)*n)/1024<<"K"<<endl;
  
  o<<"formula arg arrays:                                        "<<setw(9)<< LADR_GLOBAL_FORMULA.Arg_mem * sizeof(void *) / 1024<<"K"<<endl<<endl;
} 

void FormulaContainer::p_formula_mem(){
  fprint_formula_mem(cout, true);
}


unsigned FormulaContainer::formula_megs(void) {
  unsigned bytes = (LADR_GLOBAL_FORMULA.Formula_gets - LADR_GLOBAL_FORMULA.Formula_frees) * sizeof(struct formula) + LADR_GLOBAL_FORMULA.Arg_mem * sizeof(void *);
  return bytes / (1024 * 1024);
}


Formula FormulaContainer::formula_get(int arity, Ftype type) {
  Formula f = get_formula(arity);
  f->type = type;
  f->qvar=NULL;
  return f;
} 


void FormulaContainer::zap_formula(Formula f) {
  TermContainer T;
  AttributeContainer A;
  if (f == NULL)   return;
  else if (f->excess_refs > 0)   f->excess_refs--;
  else {
	  if (f->type == Ftype::ATOM_FORM)  T.zap_term(f->atom);
    else {
      int i;
      for (i = 0; i < f->arity; i++) zap_formula(f->kids[i]);
    }
    if (f->attributes) A.zap_attributes(f->attributes);
    free_formula(f);
  }
}


bool FormulaContainer::logic_term(Term t) {
  TermContainer T; SymbolContainer S;
  return (T.is_term(t, S.true_sym(), 0)    ||
	  T.is_term(t, S.false_sym(), 0)       ||
	  T.is_term(t, S.not_sym(), 1)         ||
	  T.is_term(t, S.and_sym(), 2)         ||
	  T.is_term(t, S.or_sym(), 2)          ||
	  T.is_term(t, S.imp_sym(), 2)         ||  
	  T.is_term(t, S.impby_sym(), 2)       ||
	  T.is_term(t, S.iff_sym(), 2)         ||
	  T.is_term(t, S.quant_sym(), 3));
}


void FormulaContainer::gather_symbols_in_formula_term(Term t, I2list *rsyms, I2list *fsyms) {
  SymbolContainer S;
  TermContainer T;
  if (logic_term(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (T.is_term(t, S.quant_sym(), 3) && i != 3)	;  /* skip quantifier and quantified variable */
      else	gather_symbols_in_formula_term(ARG(t,i), rsyms, fsyms);
    }
  }
  else {
    int i;
	I2listContainer I;
    *rsyms =I.multiset_add(*rsyms, SYMNUM(t));
    for (i = 0; i < ARITY(t); i++)
      gather_symbols_in_term(ARG(t,i), rsyms, fsyms);
  }
}


void FormulaContainer::gather_symbols_in_term(Term t, I2list *rsyms, I2list *fsyms) {
  if(!t) return;
  if  (!VARIABLE(t)) {
    TermContainer T;
	if (T.is_term(t, "if", 3)) {
      gather_symbols_in_formula_term(ARG(t,0), rsyms, fsyms);
      gather_symbols_in_term(ARG(t,1), rsyms, fsyms);
      gather_symbols_in_term(ARG(t,2), rsyms, fsyms);
    }
    else {
      int i;
	  I2listContainer I;
      *fsyms = I.multiset_add(*fsyms, SYMNUM(t));
      for (i = 0; i < ARITY(t); i++) {
	   gather_symbols_in_term(ARG(t,i), rsyms, fsyms);
      }
    }
  }
} 

void FormulaContainer::gather_symbols_in_formula(Formula f, I2list *rsyms, I2list *fsyms) {

   if (f->type == Ftype::ATOM_FORM) {
    TermContainer T;
	if (T.is_term(f->atom, "if", 3)) {
      gather_symbols_in_formula_term(ARG(f->atom,0), rsyms, fsyms);
      gather_symbols_in_formula_term(ARG(f->atom,1), rsyms, fsyms);
      gather_symbols_in_formula_term(ARG(f->atom,2), rsyms, fsyms);
    }
    else
      gather_symbols_in_formula_term(f->atom, rsyms, fsyms);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      gather_symbols_in_formula(f->kids[i], rsyms, fsyms);
  }
} 


void FormulaContainer::gather_symbols_in_formulas(Plist lst, I2list *rsyms, I2list *fsyms) {
  Plist p;
  for (p = lst; p; p = p->next)
    gather_symbols_in_formula((Formula) p->v, rsyms, fsyms);
}

Ilist FormulaContainer::function_symbols_in_formula(Formula f) {
  IlistContainer I;
  I2listContainer I2;
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  gather_symbols_in_formula(f, &rsyms, &fsyms);
  p = I.multiset_to_set(fsyms);
  I2.set_head(rsyms);
  I2.zap_i2list();
  I2.set_head(fsyms);
  I2.zap_i2list();
  return p;
} 



Ilist FormulaContainer::relation_symbols_in_formula(Formula f) {
  Ilist p;
  IlistContainer I;
  I2listContainer I2;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  gather_symbols_in_formula(f, &rsyms, &fsyms);
  p = I.multiset_to_set(rsyms);
  I2.set_head(rsyms);
  I2.zap_i2list();
  I2.set_head(fsyms);
  I2.zap_i2list();
  return p;
} 



bool FormulaContainer::relation_symbol_in_formula(int sn, Formula f) {
  IlistContainer I(relation_symbols_in_formula(f));
  bool found = I.ilist_member(sn);
  I.zap_ilist();
  return found;
} 



Formula FormulaContainer::term_to_formula(Term t) {
  TermContainer T;
  SymbolContainer S;
  AttributeContainer A;
  Formula f = NULL;
  Ftype type;
  Attribute attributes = NULL;

  
  
  if (T.is_term(t, S.attrib_sym(), 2)) {
    attributes = A.term_to_attributes(ARG(t,1), S.attrib_sym());
    t = ARG(t,0);
  }


  
  if (T.is_term(t, S.quant_sym(), 3)) {
    /* example: $quantified(all,x,p) */
    Term quant = ARG(t,0);
    Term var = ARG(t,1);
    Ftype qtype = (T.is_term(quant, S.all_sym(), 0) ? Ftype::ALL_FORM : Ftype::EXISTS_FORM);
    f = formula_get(1, qtype);
    f->kids[0] = term_to_formula(ARG(t,2));
    f->qvar = new string(S.sn_to_str(SYMNUM(var)));
  }
  else {
    if (T.is_term(t, S.true_sym(), 0))
      type = Ftype::AND_FORM;
    else if (T.is_term(t, S.false_sym(), 0))
      type = Ftype::OR_FORM;
    else if (T.is_term(t, S.not_sym(), 1))
      type = Ftype::NOT_FORM;
    else if (T.is_term(t, S.and_sym(), 2))
      type = Ftype::AND_FORM;
    else if (T.is_term(t, S.or_sym(), 2))
      type = Ftype::OR_FORM;
    else if (T.is_term(t, S.iff_sym(), 2))
      type = Ftype::IFF_FORM;
    else if (T.is_term(t, S.imp_sym(), 2))
      type = Ftype::IMP_FORM;
    else if (T.is_term(t, S.impby_sym(), 2))
      type = Ftype::IMPBY_FORM;
    else
      type = Ftype::ATOM_FORM;

    if (type == Ftype::ATOM_FORM) {
      f = formula_get(0, Ftype::ATOM_FORM);
      f->atom = T.copy_term(t);
    }
    else if (type == Ftype::NOT_FORM) {
      f = formula_get(1, Ftype::NOT_FORM);
      f->kids[0] = term_to_formula(ARG(t,0));
    }
    else if (ARITY(t) == 0) {
      f = formula_get(0, type);
    }
    else {
      f = formula_get(2, type);
      f->kids[0] = term_to_formula(ARG(t,0));
      f->kids[1] = term_to_formula(ARG(t,1));
    }
  }
  f = flatten_top(f);
  f->attributes = attributes;
  return f;
} 




Term FormulaContainer::formula_to_term(Formula f) {
  TermContainer T;
  SymbolContainer S;
  Term t = NULL;

  switch (f->type) {
  case Ftype::ATOM_FORM:
    t = T.copy_term(f->atom);
    break;
  case Ftype::NOT_FORM:
    t = T.get_rigid_term(S.not_sym(), 1);
    ARG(t,0) = formula_to_term(f->kids[0]);
    break;
  case Ftype::IFF_FORM:
    t = T.get_rigid_term(S.iff_sym(), 2);
    ARG(t,0) = formula_to_term(f->kids[0]);
    ARG(t,1) = formula_to_term(f->kids[1]);
    break;
  case Ftype::IMP_FORM:
    t = T.get_rigid_term(S.imp_sym(), 2);
    ARG(t,0) = formula_to_term(f->kids[0]);
    ARG(t,1) = formula_to_term(f->kids[1]);
    break;
  case Ftype::IMPBY_FORM:
    t = T.get_rigid_term(S.impby_sym(), 2);
    ARG(t,0) = formula_to_term(f->kids[0]);
    ARG(t,1) = formula_to_term(f->kids[1]);
    break;
  case Ftype::AND_FORM:
  case Ftype::OR_FORM:
    if (f->arity == 0)
      t = T.get_rigid_term(f->type == Ftype::AND_FORM ? S.true_sym() : S.false_sym(), 0);
  else {
      int i = f->arity-1;
      t = formula_to_term(f->kids[i]);
      for (i--; i >= 0; i--) {
		Term t1 = T.get_rigid_term(f->type == Ftype::AND_FORM ? S.and_sym() : S.or_sym(), 2);
		ARG(t1,0) = formula_to_term(f->kids[i]);
		ARG(t1,1) = t;
		t = t1;
      }
    }
    break;
  case Ftype::ALL_FORM:
  case Ftype::EXISTS_FORM:
    {
      /* transform to: $quantified(all,x,f) */
      t = T.get_rigid_term(S.quant_sym(), 3);
      ARG(t,0) = T.get_rigid_term(f->type == Ftype::ALL_FORM ? S.all_sym() : S.exists_sym(), 0);
      ARG(t,1) = T.get_rigid_term(*(f->qvar), 0);
      ARG(t,2) = formula_to_term(f->kids[0]);
    }      
    break;
  }

  if (f->attributes) {
    AttributeContainer At;
	t = T.build_binary_term(S.str_to_sn(S.attrib_sym(), 2),t,At.attributes_to_term(f->attributes, S.attrib_sym()));
 }
  return t;
}



void FormulaContainer::fprint_formula(ostream &o, Formula f) {
  TermContainer T;
  SymbolContainer S;
  AttributeContainer A;
  
  
  
  if(f->attributes) 
      A.p_attribute(f->attributes);
  
  
 
  if (f->type == Ftype::ATOM_FORM) {
     T.fprint_term(o, f->atom);
  }
  else if (f->type == Ftype::NOT_FORM) {
    o<<S.not_sym()<<" ";
	fprint_formula(o, f->kids[0]);
   
  }
  else if (f->type == Ftype::IFF_FORM) {
    o<<")";
    fprint_formula(o, f->kids[0]);
    o<<" "<<S.iff_sym()<<" ";
    fprint_formula(o, f->kids[1]);
    o<<")";
  }
  else if (f->type == Ftype::IMP_FORM) {
    o<<"(";
	fprint_formula(o, f->kids[0]);
    o<<" "<<S.imp_sym()<<" ";
	fprint_formula(o, f->kids[1]);
    o<<")";
  }
  else if (f->type == Ftype::IMPBY_FORM) {
    o<<"(";
	fprint_formula(o, f->kids[0]);
    o<<" "<<S.impby_sym()<<" ";
    fprint_formula(o, f->kids[1]);
	o<<")";
  }
  else if (quant_form(f)) {
    o<<"("<<( f->type==Ftype::ALL_FORM ? S.all_sym() : S.exists_sym())<<" "<<*(f->qvar);
	fprint_formula(o, f->kids[0]);
	o<<")";
  }
  else if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM) {
    if (f->arity == 0)
     o<< (f->type == Ftype::AND_FORM ? S.true_sym() : S.false_sym());
    else {
      int i;
	  o<<"(";
      for (i = 0; i < f->arity; i++) {
		fprint_formula(o, f->kids[i]);
			if (i < f->arity-1)
			 o<<" "<<(f->type == Ftype::AND_FORM ? S.and_sym() : S.or_sym())<<" ";
	   }
	   o<<")";
    }
  }
 
} 

void FormulaContainer::p_formula(Formula c) {
  if(!c) return;
  fprint_formula(cout, c);
  cout<<endl<<endl;
} 

unsigned FormulaContainer::hash_formula(Formula f) {
  TermContainer T;  
  
  if (f->type == Ftype::ATOM_FORM)  return T.hash_term(f->atom);
  else if (quant_form(f))  return ( (unsigned)f->type << 3) ^ (unsigned) f->qvar->at(0);
  else {
    unsigned x =(unsigned) f->type;
    int i;
    for (i = 0; i < f->arity; i++)
      x = (x << 3) ^ hash_formula(f->kids[i]);
    return x;
  }
}

bool FormulaContainer::formula_ident(Formula f, Formula g) {
  TermContainer T;
  if (f->type != g->type || f->arity != g->arity)   return false;
  else if (f->type == Ftype::ATOM_FORM)  return T.term_ident(f->atom, g->atom);
  else if (quant_form(f))  return (myString::str_ident(*(f->qvar),*(g->qvar)) && formula_ident(f->kids[0],g->kids[0]));
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (!formula_ident(f->kids[i], g->kids[i]))  return false;
    return true;
  }
}


Formula FormulaContainer::formula_copy(Formula f) {
  TermContainer T;
  Formula g = formula_get(f->arity, f->type);
  if (f->type == Ftype::ATOM_FORM)  g->atom =  T.copy_term(f->atom);
  else {
    int i;
    if (quant_form(f))  g->qvar = new string(*(f->qvar));
    for (i = 0; i < f->arity; i++)
      g->kids[i] = formula_copy(f->kids[i]);
  }
  return g;
}


Ftype FormulaContainer::dual_type(Ftype op) {
  switch (op) {
    case Ftype::AND_FORM: return Ftype::OR_FORM;
    case Ftype::OR_FORM: return Ftype::AND_FORM;
    case Ftype::ALL_FORM: return Ftype::EXISTS_FORM;
    case Ftype::EXISTS_FORM: return Ftype::ALL_FORM;
    default: return op;
  }
}

Formula FormulaContainer::dual(Formula f) {
  int i;
  for (i = 0; i < f->arity; i++)
    f->kids[i] = dual(f->kids[i]);
  f->type = dual_type(f->type);
  return f;
} 

Formula FormulaContainer::_and(Formula a, Formula b) {
  Formula f = formula_get(2, Ftype::AND_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
} 

Formula FormulaContainer::_or(Formula a, Formula b) {
  Formula f = formula_get(2, Ftype::OR_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
} 

Formula FormulaContainer::imp(Formula a, Formula b) {
  Formula f = formula_get(2, Ftype::IMP_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
}

Formula FormulaContainer::impby(Formula a, Formula b) {
  Formula f = formula_get(2, Ftype::IMPBY_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
}

Formula FormulaContainer::_not(Formula a) {
  Formula f = formula_get(1, Ftype::NOT_FORM);
  f->kids[0] = a;
  return f;
} 

Formula FormulaContainer::negate(Formula a) {
  return _not(a);
}

bool FormulaContainer::quant_form(Formula f) {
  return (f->type == Ftype::ALL_FORM || f->type == Ftype::EXISTS_FORM);
} 



Formula FormulaContainer:: flatten_top(Formula f) {
  if (f->type != Ftype::AND_FORM && f->type != Ftype::OR_FORM)   return f;
  else {
    bool operate = false;
    int n = 0;  /* count new arity */
    int i;
    for (i = 0; i < f->arity; i++) {
      if (f->type != f->kids[i]->type)	n++;
      else {
			n += (f->kids[i]->arity);
			operate = true;
      }
    }
    if (!operate) return f;
    else {
      Formula g = formula_get(n, f->type);
      int i, j;
      j = 0;
      for (i = 0; i < f->arity; i++) {
			if (f->kids[i]->type != f->type) g->kids[j++] = f->kids[i];
			else {
					int k;
					for (k = 0; k < f->kids[i]->arity; k++) g->kids[j++] = f->kids[i]->kids[k];
					free_formula(f->kids[i]);
			}
      }
      free_formula(f);
      /* If the new formula has just one argument, return that argument. */
      if (g->arity == 1) {
		Formula h = g->kids[0];
		free_formula(g);
		return h;
      }
      else return g;
    }
  }
} 



Formula FormulaContainer::formula_flatten(Formula f) {
  int i;
  for (i = 0; i < f->arity; i++)
    f->kids[i] = formula_flatten(f->kids[i]);
  return flatten_top(f);
}



Formula FormulaContainer::nnf2(Formula f, Fpref pref) {
  if (f->type == Ftype::ATOM_FORM)
    return f;
  else if (quant_form(f)) {
    f->kids[0] = nnf2(f->kids[0], pref);
    return f;
  }
  else if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = nnf2(f->kids[i], pref);
    return f;
  }
  else if (f->type == Ftype::IMP_FORM) {
    Formula g = nnf2(_or(_not(f->kids[0]), f->kids[1]), pref);
    free_formula(f);
    return g;
  }
  else if (f->type == Ftype::IMPBY_FORM) {
    Formula g = nnf2(_or(f->kids[0], _not(f->kids[1])), pref);
    free_formula(f);
    return g;
  }
  else if (f->type == Ftype::IFF_FORM) {
    Formula g;
    Formula a = f->kids[0];
    Formula b = f->kids[1];
    Formula ac = formula_copy(a);
    Formula bc = formula_copy(b);

    if (pref == Fpref::CONJUNCTION)
      g = nnf2(_and(imp(a,b), impby(ac,bc)), pref);
    else
      g = nnf2(_or(_and(a,b),_and(_not(ac),_not(bc))), pref);
	     
    free_formula(f);
    return g;
  }

  /* NOT */

  else if (f->type == Ftype::NOT_FORM) {
    Formula h = f->kids[0];
    if (h->type == Ftype::ATOM_FORM)
      return f;
    else if (h->type == Ftype::NOT_FORM) {
      Formula g = nnf2(h->kids[0], pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (quant_form(h)) {
      Formula g = formula_get(1, dual_type(h->type));
      g->qvar = new string (*h->qvar);
      g->kids[0] = nnf2(_not(h->kids[0]), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == Ftype::AND_FORM || h->type == Ftype::OR_FORM) {
      Formula g = formula_get(h->arity, dual_type(h->type));
      int i;
      for (i = 0; i < h->arity; i++)
        g->kids[i] = nnf2(_not(h->kids[i]), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == Ftype::IMP_FORM) {
      Formula g = nnf2(_and(h->kids[0], _not(h->kids[1])), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == Ftype::IMPBY_FORM) {
      Formula g = nnf2(_and(_not(h->kids[0]), h->kids[1]), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == Ftype::IFF_FORM) {
      Formula g;
      Formula a = h->kids[0];
      Formula b = h->kids[1];
      Formula ac = formula_copy(a);
      Formula bc = formula_copy(b);

      if (pref == Fpref::CONJUNCTION)
	g = nnf2(_and(_or(a,b),_or(_not(ac),_not(bc))), pref);
      else
	g = nnf2(_or(_and(a,_not(b)),_and(_not(ac),bc)), pref);

      free_formula(h);
      free_formula(f);
      return g;
    }
    else
      return f;
  }  /* NOT */
  else
    return f;
} 

Formula FormulaContainer::nnf(Formula f) {
  return nnf2(f, Fpref::CONJUNCTION);
}

Formula FormulaContainer::make_conjunction(Formula f) {
  if (f->type == Ftype::AND_FORM) return f;
  else {
	Formula h = formula_get(1, Ftype::AND_FORM);
    h->kids[0] = f;
    return h;
  }
} 

Formula FormulaContainer::make_disjunction(Formula f) {
  if (f->type == Ftype::OR_FORM) return f;
  else {
    Formula h = formula_get(1, Ftype::OR_FORM);
    h->kids[0] = f;
    return h;
  }
} 


void FormulaContainer::formula_canon_eq(Formula f) {
  
   if (f->type == Ftype::ATOM_FORM) {
    TermContainer T;
	
    Term a = f->atom;
    if (T.eq_term(a)) {
      Term left = ARG(a,0);
      Term right = ARG(a,1);
      if (TermOrder::term_compare_ncv(left, right) == OrderType::LESS_THAN) {
		ARG(a,0) = right;
		ARG(a,1) = left;
      }
    }
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      formula_canon_eq(f->kids[i]);
  }
} 

int FormulaContainer::formula_size(Formula f) {
  if (f->type == Ftype::ATOM_FORM)  return 1;
  else {
    int i;
    int n = 0;
    for (i = 0; i < f->arity; i++)
      n += formula_size(f->kids[i]);
    return n+1;
  }
} 

int FormulaContainer::greatest_qvar(Formula f) {
  SymbolContainer S;
  if (quant_form(f)) {
    int sn = S.str_to_sn(*(f->qvar), 0);
    int max_sub = greatest_qvar(f->kids[0]);
    return (sn > max_sub ? sn : max_sub);
  }
  else {
    int max = -1;
    int i;
    for (i = 0; i < f->arity; i++) {
      int max_sub = greatest_qvar(f->kids[i]);
      max = (max_sub > max ? max_sub : max);
    }
    return max;
  }
} 


int FormulaContainer::greatest_symnum_in_formula(Formula f) {
  SymbolContainer S;
  TermContainer T;
  if (f->type == Ftype::ATOM_FORM) {
    return T.greatest_symnum_in_term(f->atom);
  }
  if (quant_form(f)) {
    int sn = S.str_to_sn(*(f->qvar), 0);
    int max_sub = greatest_symnum_in_formula(f->kids[0]);
    return (sn > max_sub ? sn : max_sub);
  }
  else {
    int max = -1;
    int i;
    for (i = 0; i < f->arity; i++) {
      int max_sub = greatest_symnum_in_formula(f->kids[i]);
      max = (max_sub > max ? max_sub : max);
    }
    return max;
  }
} 

void FormulaContainer::subst_free_var(Formula f, Term target, Term replacement) {
  SymbolContainer S;
  TermContainer T;
  if (f->type == Ftype::ATOM_FORM)
    f->atom = T.subst_term(f->atom, target, replacement);
  else if (quant_form(f) && myString::str_ident(S.sn_to_str(SYMNUM(target)), *(f->qvar))) {
    ; /* Do nothing, because we have a quantified variable of the same name. */
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      subst_free_var(f->kids[i], target, replacement);
  }
}


Formula FormulaContainer::elim_rebind(Formula f, Ilist uvars) {
  TermContainer T;
  IlistContainer I;
  SymbolContainer S;
  if (quant_form(f)) {
    Term var = T.get_rigid_term(*(f->qvar), 0);
    Ilist uvars_plus;
	I.set_head(uvars);
    if (I.ilist_member(SYMNUM(var))) {
      /* We are in the scope of another variable with this name, so
       * rename this variable.
       */
      int sn = S.gen_new_symbol("y", 0, uvars);
      Term newvar = T.get_rigid_term(S.sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      *(f->qvar) = S.sn_to_str(sn);
      T.free_term(var);
      var = newvar;
    }

    uvars_plus = I.ilist_prepend(SYMNUM(var));
    f->kids[0] = elim_rebind(f->kids[0], uvars_plus);
    T.free_term(var);
	I.free_ilist(uvars_plus);  /* frees first node only; uvars still good */
    return f;
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = elim_rebind(f->kids[i], uvars);
    return f;
  }
}

Formula FormulaContainer::eliminate_rebinding(Formula f) {
  f = elim_rebind(f, NULL);
  return f;
} 

Plist FormulaContainer::free_vars(Formula f, Plist vars) {
  TermContainer T;
  if (f->type == Ftype::ATOM_FORM)
    vars = T.free_vars_term(f->atom, vars);
  else if (quant_form(f)) {
    Term var = T.get_rigid_term(*(f->qvar), 0);
    Plist vars2 = free_vars(f->kids[0], NULL);
    vars2 = Tlist::tlist_remove(var, vars2);
    vars =  Tlist::tlist_union(vars, vars2);
    T.zap_term(var);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      vars = free_vars(f->kids[i], vars);
  }
  return vars;
}  /* free_vars */


bool FormulaContainer::closed_formula(Formula f) {
  Plist vars = free_vars(f, NULL);  /* deep (returns new terms) */
  bool ok = (vars == NULL);
  Tlist::zap_tlist(vars);
  return ok;
}


Formula FormulaContainer::get_quant_form(Ftype type, string qvar, Formula subformula) {
  Formula f = formula_get(1, type);
  f->qvar = new string(qvar);
  f->kids[0] = subformula;
  return f;
} 

Formula FormulaContainer::uni_close(Formula f, Plist vars) {
  SymbolContainer S;
  if (vars == NULL) return f;
  else {
    Formula g = uni_close(f, vars->next);
    Term v = (Term) vars->v;
    return get_quant_form(Ftype::ALL_FORM, S.sn_to_str(SYMNUM(v)), g);
  }
} 

Formula FormulaContainer::universal_closure(Formula f) {
  
  Plist vars = free_vars(f, NULL);  /* deep (returns new terms) */
  f = uni_close(f, vars);
  Tlist::zap_tlist(vars);
  return f;
}

bool FormulaContainer::free_var(string svar, Term tvar, Formula f) {
  SymbolContainer S;
  TermContainer T;
  if (f->type == Ftype::ATOM_FORM)
    return T.occurs_in(tvar, f->atom);
  else if (quant_form(f) && myString::str_ident(svar, *(f->qvar))) {
    return false;
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++) {
      if (free_var(svar, tvar, f->kids[i]))
	return true;
    }
    return false;
  }
}

bool FormulaContainer::free_variable(string svar, Formula f) {
  TermContainer T;
  Term tvar = T.get_rigid_term(svar, 0);
  bool free = free_var(svar, tvar, f);
  T.free_term(tvar);
  return free;
} 

Formula FormulaContainer::formulas_to_conjunction(Plist formulas) {
  PlistContainer P;
  Plist p;
  P.set_head(formulas);
  int n = P.plist_count();
  Formula f = formula_get(n, Ftype::AND_FORM);
  int i = 0;
  for (p = formulas; p; p = p->next) {
    f->kids[i++] = (Formula)p->v;
  }
  return f;
}

Formula FormulaContainer::formulas_to_disjunction(Plist formulas) {
  PlistContainer P;
  Plist p;
  P.set_head(formulas);
  int n = P.plist_count();
  Formula f = formula_get(n, Ftype::OR_FORM);
  int i = 0;
  for (p = formulas; p; p = p->next) {
    f->kids[i++] =(Formula) p->v;
  }
  return f;
}

Plist FormulaContainer::copy_plist_of_formulas(Plist formulas){
  if (formulas == NULL)
    return NULL;
  else {
    PlistContainer P;  
    Plist tail = copy_plist_of_formulas(formulas->next);
    Plist head = P.get_plist();
    head->v = formula_copy((Formula)formulas->v);
    head->next = tail;
    return head;
  }
} 



bool FormulaContainer::literal_formula(Formula f) {
  if (f->type == Ftype::ATOM_FORM)    return true;
  else if (f->type == Ftype::NOT_FORM) return f->kids[0]->type == Ftype::ATOM_FORM;
  else    return false;
}


bool FormulaContainer::clausal_formula(Formula f) {
  if (f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++) {
      if (!clausal_formula(f->kids[i]))	return false;
    }
    return true;
  }
  else return literal_formula(f);
}


void FormulaContainer::formula_set_vars_recurse(Formula f, string vnames[], int max_vars) {
  TermContainer T;
  if (f->type == Ftype::ATOM_FORM)
    f->atom = T.set_vars_recurse(f->atom, vnames, max_vars);
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      formula_set_vars_recurse(f->kids[i], vnames, max_vars);
  }
} 



void FormulaContainer::formula_set_variables(Formula f, int max_vars) {
  string *vmap;
  string a[MAX_VARS];
  int i;
  AttributeContainer AT;
  if(max_vars >MAX_VARS) vmap=new string[max_vars];  
  else vmap=a;
  
  formula_set_vars_recurse(f, vmap, max_vars);

  /* Now do any answer attributes (with the same vmap). */

  if (f->attributes) {
    AT.set_vars_attributes(f->attributes, vmap, max_vars);
#if 0
    /* Make sure that answer vars also occur in formula. */
    Plist attr_vars = vars_in_attributes(c->attributes);
    Plist formula_vars = vars_in_formula(c);
    if (!plist_subset(attr_vars, formula_vars)) {
      Plist p;
      printf("Variables in answers must also occur ordinary literals:\n");
      p_formula(c);
      for (p = attr_vars; p; p = p->next) {
	if (!plist_member(formula_vars, p->v)) {
	  Term t = p->v;
	  printf("Answer variable not in ordinary literal: %s.\n",
		 vmap[VARNUM(t)]);
	}
      }
      fatal_error("formula_set_variables, answer variable not in literal");
    }
    zap_plist(formula_vars);
    zap_plist(attr_vars);
#endif
  }
    if(max_vars >MAX_VARS)
        delete [] vmap;

}


bool FormulaContainer::positive_formula(Formula f) {
  Formula g = f;
  while (quant_form(g))  g = g->kids[0];
  if (g->type == Ftype::ATOM_FORM)  return true;
  else if (g->type != Ftype::AND_FORM)  return false;
  else {
    int i;
    for (i = 0; i < g->arity; i++)
      if (!positive_formula(g->kids[i])) return false;
    return true;
  }
}


bool FormulaContainer::formula_contains_attributes(Formula f) {
  if (f->attributes != NULL)    return true;
  else if (f->type == Ftype::ATOM_FORM)  return false;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (formula_contains_attributes(f->kids[i])) return true;
    return false;
  }
}


bool FormulaContainer::subformula_contains_attributes(Formula f) {
  if (f->type == Ftype::ATOM_FORM)   return false;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (formula_contains_attributes(f->kids[i]))	return true;
    return false;
  }
}



Ilist FormulaContainer::constants_in_formula(Formula f) {
  SymbolContainer S;
  Ilist p = function_symbols_in_formula(f);
  p = S.symnums_of_arity(p, 0);
  return p;
} 


bool FormulaContainer::relation_in_formula(Formula f, int symnum){
  if (f->type == Ftype::ATOM_FORM)   return SYMNUM(f->atom) == symnum;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (relation_in_formula(f->kids[i], symnum))	return true;
    return false;
  }
} 

void FormulaContainer::rename_all_bound_vars(Formula f){
  if (quant_form(f)) {
    TermContainer T;
    SymbolContainer S;
    Term var = T.get_rigid_term(*(f->qvar), 0);
    int sn = S.fresh_symbol("x", 0);
    Term newvar = T.get_rigid_term(S.sn_to_str(sn), 0);
    subst_free_var(f->kids[0], var, newvar);
    *(f->qvar) = S.sn_to_str(sn);
    T.free_term(var);
    T.free_term(newvar);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      rename_all_bound_vars(f->kids[i]);
  }
} 


void FormulaContainer::rename_these_bound_vars(Formula f, Ilist vars) {
  /* Rename each quantified variable in "vars" to a new symbol. */
  if (quant_form(f)) {
    TermContainer T;  
    Term var = T.get_rigid_term(*(f->qvar), 0);
    SymbolContainer S;
    IlistContainer I;
    if (I.ilist_member(vars, SYMNUM(var))) {
      /* Rename this variable. */
      int sn = S.fresh_symbol("x", 0);
      Term newvar = T.get_rigid_term(S.sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      *(f->qvar) = S.sn_to_str(sn);
      T.free_term(var);
    }
    rename_these_bound_vars(f->kids[0], vars);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      rename_these_bound_vars(f->kids[i], vars);
  }
} 
