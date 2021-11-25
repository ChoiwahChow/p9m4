#include "ladrvglobais.h"
#include "cnf.h"
#include "fatal.h"
#include "mystring.h"
#include "formula.h"
#include <setjmp.h>  /* Yikes! */
#include <signal.h>
#include <unistd.h>
#include <iostream>







GlobalCnf::GlobalCnf() {
    Fid_call_limit=UINT_MAX;
    Fid_calls=0;
}


GlobalCnf::~GlobalCnf() {
}


bool Cnf::formula_ident(void * a, void * b) {
    FormulaContainer F;
    return F.formula_ident((Formula) a,(Formula) b);
    
}

Formula Cnf::share_formula(Formula f, Hashtab h) {
  if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM ||
      f->type == Ftype::ALL_FORM || f->type == Ftype::EXISTS_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = share_formula(f->kids[i], h);
    return f;
  }
  else {
    unsigned hashval;
    Formula g;
    if  (f->type == Ftype::NOT_FORM)
      f->kids[0] = share_formula(f->kids[0], h);

	FormulaContainer F;
    hashval = F.hash_formula(f);
    HashtabContainer H;
	H.set_head(h);
	g =(Formula) H.hash_lookup(f, hashval, (bool (*) (void *, void *)) formula_ident);
    if (g) {
      F.zap_formula(f);
      g->excess_refs++;
      return g;
    }
    else {
      H.hash_insert(f, hashval);
	  h=H.get_head();
      return f;
    }
  }
} 

Formula Cnf::consolidate_formula(Formula f) {
  HashtabContainer H;
  H.hash_init(10000);
  f = share_formula(f, H.get_head());
  /* hash_info(h); */
  H.hash_destroy();
  return f;
}


bool Cnf::formula_ident_share(Formula f, Formula g) {
  if (LADR_GLOBAL_CNF.Fid_call_limit != 0 && ++LADR_GLOBAL_CNF.Fid_calls > LADR_GLOBAL_CNF.Fid_call_limit) {
    cout <<endl<<"%% Fid call limit; jumping home"<<endl;
	longjmp(LADR_GLOBAL_CNF.Jump_env, 1);
  }

  if (f->type != g->type || f->arity != g->arity)   return false;
  else if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      if (!formula_ident_share(f->kids[i], g->kids[i]))return false;
    return true;
  }
  else if (f->type == Ftype::ALL_FORM || f->type == Ftype::EXISTS_FORM) {
    return
      myString::str_ident(*(f->qvar), *(g->qvar)) &&
      formula_ident_share(f->kids[0], g->kids[0]);
  }
  else
    return f == g;
} 

Formula Cnf::formula_copy_share(Formula f) {
  FormulaContainer F;
  if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM) {
    
	Formula g = F.formula_get(f->arity, f->type);
    int i;
    for (i = 0; i < f->arity; i++)
      g->kids[i] = formula_copy_share(f->kids[i]);
    return g;
  }
  else if (f->type == Ftype::ALL_FORM || f->type == Ftype::EXISTS_FORM) {
    Formula g = F.formula_get(1, f->type);
    g->qvar = f->qvar;
    g->kids[0] = formula_copy_share(f->kids[0]);
    return g;
  }
  else {
    f->excess_refs++;
    return f;
  }
}

bool Cnf::complementary_share(Formula a, Formula b) {
  return
    (a->type == Ftype::NOT_FORM && formula_ident_share(a->kids[0], b))
    ||
    (b->type == Ftype::NOT_FORM && formula_ident_share(a, b->kids[0]));
} 


bool Cnf::contains_complements_share(Formula f) {
  int i, j;
  for (i = 0; i < f->arity-1; i++) {
    for (j = i+1; j < f->arity; j++) {
      if (complementary_share(f->kids[i], f->kids[j]))
	return true;
    }
  }
  return false;
}

bool Cnf::prop_member_share(Formula f, Formula g) {
  int i;
  for (i = 0; i < g->arity; i++)
    if (formula_ident_share(f, g->kids[i]))
      return true;
  return false;
}

bool Cnf::prop_subset_share(Formula f, Formula g) {
  int i;
  for (i = 0; i < f->arity; i++)
    if (!prop_member_share(f->kids[i], g))
      return false;
  return true;
} 


bool Cnf::prop_subsume_share(Formula f, Formula g) {
  if (FALSE_FORMULA(f))  return true;
  else if  (TRUE_FORMULA(g))  return true;
  else if (g->type == Ftype::OR_FORM) {
    if (f->type == Ftype::OR_FORM)
      return prop_subset_share(f, g);
    else
      return prop_member_share(f, g);
  }
  return formula_ident_share(f, g);
}

Formula Cnf::remove_subsumed_share(Formula f) {
  if (f->type != Ftype::AND_FORM)  return f;
  else {
    Formula h;
    int new_arity = f->arity;
    int i, j;
    for (i = 0; i < f->arity; i++) {
      for (j = i+1; j < f->arity; j++) {
		if (f->kids[i] && f->kids[j] && prop_subsume_share(f->kids[i], f->kids[j])) {
			FormulaContainer F;
			F.zap_formula(f->kids[j]);
			f->kids[j] = NULL;
			new_arity--;
		}
		else if (f->kids[i] && f->kids[j] && prop_subsume_share(f->kids[j], f->kids[i])) {
			FormulaContainer F;
			F.zap_formula(f->kids[i]);
			f->kids[i] = NULL;
			new_arity--;
		}
      }
    }
    FormulaContainer F;
	h = F.formula_get(new_arity, Ftype::AND_FORM);
    j = 0;
    for (i = 0; i < f->arity; i++) {
      if (f->kids[i]) h->kids[j++] = f->kids[i];
    }
    F.free_formula(f);
    return h;
  }
}


Formula Cnf::bbt(Formula f, int start, int end) {
  if (start == end)  return f->kids[start];
  else {
    int mid = (start + end) / 2;
	FormulaContainer F;
    Formula b = F.formula_get(2, f->type);
    b->kids[0] = bbt(f, start, mid);
    b->kids[1] = bbt(f, mid+1, end); 
    return b;
  }
} 

Formula Cnf::balanced_binary(Formula f) {
  if (f->type != Ftype::AND_FORM && f->type != Ftype::OR_FORM)  return f;
  else if (f->arity == 0)  return f;
  else {
		Formula b = bbt(f, 0, f->arity-1);
		FormulaContainer F;
		F.free_formula(f);
		return b;
  }
}

Formula Cnf::disjoin_flatten_simplify(Formula a, Formula b) {
  Formula c;
  int new_arity, i, j;
  FormulaContainer F;
  a = F.make_disjunction(a);
  b = F.make_disjunction(b);
  new_arity = a->arity + b->arity;
  for (i = 0; i < a->arity; i++) {
    for (j = 0; j < b->arity; j++) {
      if (b->kids[j] != NULL) {
	if (complementary_share(a->kids[i], b->kids[j])) {
	  FormulaContainer F;
	  F.zap_formula(a);
	  F.zap_formula(b);  /* this can handle NULL kids */
	  return F.formula_get(0, Ftype::AND_FORM);  /* TRUE formula */
	}
	else if (formula_ident_share(a->kids[i], b->kids[j])) {
	  /* Note that this makes b non-well-formed. */
	  FormulaContainer F;
	  F.zap_formula(b->kids[j]);  /* really FALSE */
	  b->kids[j] = NULL;
	  new_arity--;
	}
      }
    }
  }
 
  c = F.formula_get(new_arity, Ftype::OR_FORM);
  j = 0;
  for (i = 0; i < a->arity; i++)
    c->kids[j++] = a->kids[i];
  for (i = 0; i < b->arity; i++)
    if (b->kids[i] != NULL)
      c->kids[j++] = b->kids[i];
  F.free_formula(a);
  F.free_formula(b);
  return c;
} 





Formula Cnf::simplify_and_share(Formula f) {
  FormulaContainer F;
  if (f->type != Ftype::AND_FORM)
    return f;
  else {
    f = remove_subsumed_share(f);  /* still AND */
    if (f->arity == 1) {
      Formula g = f->kids[0];
	  F.free_formula(f);
      return g;
    }
    else if (contains_complements_share(f)) {
      F.zap_formula(f);
      return F.formula_get(0, Ftype::OR_FORM);  /* FALSE */
    }
    else
      return f;
  }
} 


Formula Cnf::distribute_top(Formula h) {
  FormulaContainer F;
  Formula f = h->kids[0];
  Formula g = h->kids[1];
  int arity, i, j, k;
  Formula a;
  F.free_formula(h);
  
  /* If not conjunctions, make them so. */
  f = F.make_conjunction(f);
  g = F.make_conjunction(g);

  /* printf("DT: %5d x %5d\n", f->arity, g->arity); fflush(stdout); */

  arity = f->arity * g->arity;

  a = F.formula_get(arity, Ftype::AND_FORM);
  k = 0;
  for (i = 0; i < f->arity; i++) {
    for (j = 0; j < g->arity; j++) {
      Formula fi = formula_copy_share(f->kids[i]);
      Formula gj = formula_copy_share(g->kids[j]);
      a->kids[k++] = disjoin_flatten_simplify(fi, gj);
    }
  }
  F.zap_formula(f);
  F.zap_formula(g);
  a = simplify_and_share(a);
  return a;
} 

Formula Cnf::distribute(Formula f) {
  if (f->type != Ftype::OR_FORM || f->arity == 0)
    return f;
  else {
    if (f->arity != 2)
      fatal::fatal_error("distribute: not binary");
    f->kids[0] = distribute(f->kids[0]);
    f->kids[1] = distribute(f->kids[1]);
    f = distribute_top(f);
    return f;
  }
} 


Formula Cnf::cnf(Formula f) {
  if (f->type != Ftype::AND_FORM && f->type != Ftype::OR_FORM)  return f;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = cnf(f->kids[i]);
    
    if (f->type == Ftype::AND_FORM) {
      FormulaContainer F;
      f = F.flatten_top(f);
      f = simplify_and_share(f);
      return f;
    }
    else {  /* OR_FORM */
      FormulaContainer F;
	  f = F.dual(remove_subsumed_share(F.dual(f)));
      f = balanced_binary(f);  /* make the top OR-tree binary */
      f = distribute(f);
      return f;
    }
  }
}  /* cnf */


Formula Cnf::dnf(Formula f) {
    FormulaContainer F;
    return F.dual(cnf(F.dual(f)));
}




Formula Cnf::skolem(Formula f, Ilist uvars) {
IlistContainer I;
TermContainer T;
SymbolContainer S;
FormulaContainer F;

I.set_head(uvars);
if (f->type == Ftype::ATOM_FORM || f->type == Ftype::NOT_FORM)  return f;
  else if (f->type == Ftype::ALL_FORM) {
		
		Term var = T.get_rigid_term(*(f->qvar), 0);
		Ilist uvars_plus;
		
		if (I.ilist_member(SYMNUM(var))) {
			/* We are in the scope of another variable with this name, so
			* rename this variable.
			*/
			
			int sn = S.gen_new_symbol("x", 0, uvars);
			Term newvar = T.get_rigid_term(S.sn_to_str(sn), 0);
			F.subst_free_var(f->kids[0], var, newvar);
			*(f->qvar) = S.sn_to_str(sn);
			T.free_term(var);
			var = newvar;
		}

		uvars_plus = I.ilist_prepend(SYMNUM(var));
		f->kids[0] = skolem(f->kids[0], uvars_plus);
		T.free_term(var);
		I.free_ilist(uvars_plus);  /* frees first node only; uvars still good */
		return f;
  }
  else if (f->type == Ftype::EXISTS_FORM) {
    Formula g;
    int n = I.ilist_count();
    int sn = S.next_skolem_symbol(n);
    Term sk = T.get_rigid_term(S.sn_to_str(sn), n);
    Term evar = T.get_rigid_term(*(f->qvar), 0);
    Ilist p;
    int i;  /* uvars is backward */

    for (p = uvars, i = n-1; p; p = p->next, i--)
      ARG(sk,i) = T.get_rigid_term(S.sn_to_str(p->i), 0);
      
    F.subst_free_var(f->kids[0], evar, sk);

    T.zap_term(sk);
    T.zap_term(evar);

    g = skolem(f->kids[0], uvars);
    F.free_formula(f);
    return g;
  }
  else if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++) {
      f->kids[i] = skolem(f->kids[i], uvars);
    }
    return f;
  }
  else {
    /* Not in NNF!  Let the caller beware! */
    return f;
  }
}

Formula Cnf::skolemize(Formula f) {
  f = skolem(f, NULL);
  return f;
}

Ilist Cnf::unique_qvars(Formula f, Ilist vars) {
  TermContainer T;
  FormulaContainer F;
  IlistContainer I;
  SymbolContainer S;
  if (f->type == Ftype::ATOM_FORM)  return vars;
  else if (F.quant_form(f)) {
		Term var = T.get_rigid_term(*(f->qvar), 0);
		I.set_head(vars);
		if (I.ilist_member(SYMNUM(var))) {
			/* Rename this variable. */
			int sn = S.gen_new_symbol("x", 0, vars);
			Term newvar = T.get_rigid_term(S.sn_to_str(sn), 0);
			F.subst_free_var(f->kids[0], var, newvar);
			*(f->qvar) = S.sn_to_str(sn);
			T.free_term(var);
			var = newvar;
    }
	vars = I.ilist_prepend(SYMNUM(var));
    T.free_term(var); //--carlos
    return unique_qvars(f->kids[0], vars);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      vars = unique_qvars(f->kids[i], vars);
    return vars;
  }
} 


Formula Cnf::unique_quantified_vars(Formula f) {
  
  Ilist uvars = unique_qvars(f, NULL);
  IlistContainer I(uvars);
  I.zap_ilist();
  return f;
} 


void Cnf::mark_free_vars_formula(Formula f, string varname, int varnum) {
  FormulaContainer F;
  TermContainer T;
  SymbolContainer S;
  if (f->type == Ftype::ATOM_FORM)
    f->atom = T.subst_var_term(f->atom, S.str_to_sn(varname, 0), varnum);
  else if (F.quant_form(f) && myString::str_ident(*(f->qvar), varname)) return;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      mark_free_vars_formula(f->kids[i], varname, varnum);
  }
}

Formula Cnf::remove_uni_quant(Formula f, int *varnum_ptr) {
  FormulaContainer F;
  if (f->type == Ftype::AND_FORM || f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = remove_uni_quant(f->kids[i], varnum_ptr);
    return f;
  }
  else if (f->type == Ftype::ALL_FORM) {
    Formula g = f->kids[0];
    mark_free_vars_formula(g, *(f->qvar), *varnum_ptr);
    *varnum_ptr += 1;
    F.free_formula(f);
    return remove_uni_quant(g, varnum_ptr);
  }
  else {
    /* If not ATOM_FORM, something's probably wrong,
     * but let the caller beware!
     */
    return f;
  }
}


Formula Cnf::remove_universal_quantifiers(Formula f){
  int varnum = 0;
  return remove_uni_quant(f, &varnum);
} 



Formula Cnf::clausify_prepare(Formula f) {
  FormulaContainer F;
  F.formula_canon_eq(f);
  f = F.nnf(f);
  f = unique_quantified_vars(f);
  f = skolemize(f);
  f = remove_universal_quantifiers(f);
  f = F.formula_flatten(f);

  f =consolidate_formula(f);  /* causes sharing of some subformulas */
#if 0
  cout<<"%% CNF translation, nnf_size="F.formula_size(f)<<" "<<endl;
#endif
  f = cnf(f);
#if 0
  o<<"cnf_size="<<F.formula_size(f)<<" , cnf_clauses="<<f->type == Ftype::AND_FORM ? f->arity : 1<<" "<<endl;
#endif   
  return f;
}

Formula Cnf::ms_free_vars(Formula f) {
  /* f is ALL_FORM, kids are rms, kids not AND_FORM */
  FormulaContainer F;
  Formula child = f->kids[0];
  if (child->type != Ftype::OR_FORM) {
    if (F.free_variable(*(f->qvar), child))
      return f;
    else {
      F.free_formula(f);
      return child;
    }
  }
  else {
    
    PlistContainer NOTFREE;
	PlistContainer FREE;
	
	int free_count = 0;       /* size of free */
    int notfree_count = 0;    /* size of notfree */
    int i;
    for (i = child->arity-1; i >= 0; i--) {
      if (!F.free_variable(*(f->qvar), child->kids[i])) {
		NOTFREE.plist_prepend(child->kids[i]);
		notfree_count++;
      }
      else {
			FREE.plist_prepend(child->kids[i]);
			free_count++;
      }
    }
    if (notfree_count == 0)
      return f;      /* all children have qvar free */
    else if (free_count == 0) {
      F.free_formula(f);
      return child;  /* no child has qvar free */
    }
    else {
      Formula or_free = F.formula_get(free_count , Ftype::OR_FORM);
      Formula or_top = F.formula_get(notfree_count + 1 , Ftype::OR_FORM);
      Plist p;
      for (p = FREE.get_head(), i = 0; p; p = p->next, i++) or_free->kids[i] = (Formula)p->v;
  	  for (p = NOTFREE.get_head(), i = 0; p; p = p->next, i++)	or_top->kids[i] = (Formula)p->v;
      or_top->kids[i] = f;
      f->kids[0] = or_free;
      F.free_formula(child);
      return or_top;
    }
  }
} 



Formula Cnf::miniscope(Formula f) {
  FormulaContainer F;
  if (f->type == Ftype::ATOM_FORM || f->type == Ftype::NOT_FORM) return f;
  if (f->type == Ftype::AND_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = miniscope(f->kids[i]);
    f = F.flatten_top(f);
    f = simplify_and_share(f);
    return f;
  }
  else if (f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = miniscope(f->kids[i]);
    f = F.flatten_top(f);
    f = F.dual(remove_subsumed_share(F.dual(f)));
    f = balanced_binary(f);  /* make the top OR-tree binary */
    f = distribute(f);
    return f;
  }
  else if (f->type == Ftype::EXISTS_FORM) {
    f = F.dual(f);
    f = miniscope(f);
    f = F.dual(f);
    return f;
  }
  else if (f->type == Ftype::ALL_FORM) {

    f->kids[0] = miniscope(f->kids[0]);
    
    if (f->kids[0]->type == Ftype::AND_FORM) {
      /* distribute all to children */
      int i;
      Formula _and = f->kids[0];
      F.free_formula(f);  /* shallow */
      for (i = 0; i < _and->arity; i++) {
		Formula g = F.get_quant_form(Ftype::ALL_FORM, *(f->qvar), _and->kids[i]);
		g = ms_free_vars(g);
		_and->kids[i] = g;
      }
      return _and;  /* need to simplify first? */
    }
    else {
      f = ms_free_vars(f);
      return f;
    }
  }  /* ALL */
  else {
    fatal::fatal_error("miniscope: formula not in nnf");
    return NULL;  /* to please the complier */
  }
}



Formula Cnf::miniscope_formula(Formula f, unsigned mega_fid_call_limit){
  int return_code;
  if (mega_fid_call_limit <= 0)
    LADR_GLOBAL_CNF.Fid_call_limit = 0;  /* no limit */
  else {
    LADR_GLOBAL_CNF.Fid_call_limit = mega_fid_call_limit * 1000000;
    LADR_GLOBAL_CNF.Fid_calls = 0;
  }

  return_code = setjmp(LADR_GLOBAL_CNF.Jump_env);
  if (return_code != 0) {
    /* We just landed from longjmp(), because the limit was exceeded.
       (I'd like to reclaim the formula Memory::, but that would take some
       thought, because the partly transformed formula is not well formed.)
    */
    LADR_GLOBAL_CNF.Fid_call_limit = 0;  /* no limit */
    return NULL;
  }
  else {
    /* ordinary execution */
    FormulaContainer F;
	Formula f2 = NULL;

    F.formula_canon_eq(f);  /* canonicalize (naively) eqs for maximum sharing */
    f = F.nnf(f);
    f = F.formula_flatten(f);
    f = consolidate_formula(f);  /* share some subformulas */
  
    f = miniscope(f);  /* do the work */

    /* return a formula without shared subformulas */

    f2 = F.formula_copy(f);
    F.zap_formula(f);

    LADR_GLOBAL_CNF.Fid_call_limit = 0;  /* no limit */
    return f2;
  }
}


int Cnf::cnf_max_clauses(Formula f) {
  if (f->type == Ftype::ATOM_FORM || f->type == Ftype::NOT_FORM)
    return 1;
  else if (f->type == Ftype::ALL_FORM || f->type == Ftype::EXISTS_FORM)
    return cnf_max_clauses(f->kids[0]);
  else if (f->type == Ftype::AND_FORM) {
    int i;
    int n = 0;
    for (i = 0; i < f->arity; i++)
      n += cnf_max_clauses(f->kids[i]);
    return n;
  }
  else if (f->type == Ftype::OR_FORM) {
    int i;
    int n = 1;
    for (i = 0; i < f->arity; i++)
      n *= cnf_max_clauses(f->kids[i]);
    return n;
  }
  else {
    fatal::fatal_error("cnf_max_clauses, formula not NNF");
    return -1;  /* won't happen */
  }
}
