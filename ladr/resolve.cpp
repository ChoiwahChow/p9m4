#include "resolve.h"
#include "dollar.h"
#include "fatal.h"

bool Resolve::Ordered            = false;
bool Resolve:: Check_instances    = false;
int  Resolve::Ur_nucleus_limit   = INT_MAX;  /* limit num of clashable lits */
bool Resolve::Initial_nuclei     = false;    /* nuclei must be input clauses  */
bool Resolve::Production_mode    = false;
int  Resolve::Res_instance_prunes = 0;

//Global Class objects
Parautil Resolve::Pu;
UnifyContainer Resolve::U;
JustContainer Resolve::J;
TermContainer Resolve::T;
MindexContainer Resolve::M;
ClashContainer Resolve::C;
AttributeContainer Resolve::AT;
TopformContainer Resolve::TF;
IlistContainer Resolve::IL;


void Resolve::resolution_options(bool ordered,bool check_instances,bool initial_nuclei,int ur_nucleus_limit,bool production_mode) {
  Ordered = ordered;
  Check_instances = check_instances;
  Initial_nuclei = initial_nuclei;
  Ur_nucleus_limit = (ur_nucleus_limit == -1 ? INT_MAX : ur_nucleus_limit);
  Production_mode = production_mode;
  Res_instance_prunes = 0;

}

int Resolve::res_instance_prunes() {
  return Res_instance_prunes;
}  /* res_instance_prunes */


bool Resolve::unit_check(Literals lit) {
  
  Topform c = (Topform) lit->atom->container;
  return LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals);
}

bool Resolve::pos_hyper_sat_test(Literals lit) {
  Topform c = (Topform) lit->atom->container;
  if (LADRV_GLOBAIS_INST.Lit.positive_clause(c->literals))
    return Ordered ? Maximal::maximal_literal(c->literals, lit, (int) CheckType::FLAG_CHECK) : true;
  else   return false;
} 


bool Resolve::neg_hyper_sat_test(Literals lit) {
  Topform c = (Topform) lit->atom->container;
  if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
    return Ordered ? Maximal::maximal_literal(c->literals, lit, (int) CheckType::FLAG_CHECK) : true;
  else
    return false;
}

void Resolve::hyper_sat_atom(bool flipped, Literals slit, Term atom, int pos_or_neg,  Lindex idx, void (*proc_proc) (Topform)) {
  bool positive = (pos_or_neg == (int)Resolve_type::POS_RES);
  Context sat_subst = U.get_context();
  Context nuc_subst = U.get_context();
  Term fnd_atom;
  Mindex_pos mate_pos;
  fnd_atom = M.mindex_retrieve_first(atom,
				   positive ? idx->neg : idx->pos,
				   Querytype::UNIFY,
				   sat_subst, nuc_subst,
				   false, &mate_pos);
  while (fnd_atom) {  /* loop through found nuclei atoms */
    /* it must be in a nucleus because of the index used above */
    Topform nuc = (Topform) fnd_atom->container;
    Literals nuc_lit = C.atom_to_literal(fnd_atom);
    Clash p = NULL;
    Clash first = NULL;
    Literals nlit;
    for (nlit = nuc->literals; nlit; nlit = nlit->next) {
      p = C.append_clash(p);
      if (first == NULL) first = p;
      p->nuc_lit = nlit;
      p->nuc_subst = nuc_subst;
      if (nlit == nuc_lit) {
		p->sat_lit = slit;
		p->sat_subst = sat_subst;
		p->clashable = true;
		p->clashed = true;
		p->flipped = flipped;
      }
      else {
			if (!Production_mode)  p->clashable = positive ? !nlit->sign : nlit->sign;
			else  p->clashable = (positive && !nlit->sign && !Dollar::evaluable_predicate(SYMNUM(nlit->atom)));
	if (p->clashable) {
	  p->mate_index = positive ? idx->pos : idx->neg;
	  p->sat_subst = U.get_context();
	}
      }
    }  /* for each literal of nucleus */
    C.clash(first, positive ? pos_hyper_sat_test : neg_hyper_sat_test,Just_type::HYPER_RES_JUST,proc_proc);
    C.zap_clash(first);
    fnd_atom = M.mindex_retrieve_next(mate_pos);
  }  /* for each found nucleus atom */
  U.free_context(sat_subst);
  U.free_context(nuc_subst);
} 


void Resolve::hyper_satellite(Topform c, int pos_or_neg, Lindex idx, void (*proc_proc) (Topform)) {
  Literals slit;
 
  for (slit = c->literals; slit; slit = slit->next) {
    if (!Ordered || Maximal::maximal_literal(c->literals, slit, (int) CheckType::FLAG_CHECK)) {
      hyper_sat_atom(false, slit, slit->atom, pos_or_neg, idx, proc_proc);
      if (LADRV_GLOBAIS_INST.Lit.pos_eq(slit)) {
		Term flip = Pu.top_flip(slit->atom);
		hyper_sat_atom(true, slit, flip, pos_or_neg, idx, proc_proc);
		Pu.zap_top_flip(flip);
      }
    }  /* if sat is ok */
  }  /* for each literal of satellite */
} 

void Resolve::hyper_nucleus(Topform c, int pos_or_neg, Lindex idx, void (*proc_proc) (Topform)){
  

  bool positive = (pos_or_neg ==(int) Resolve_type::POS_RES);
  Clash p = NULL;
  Clash first = NULL;
  Literals lit;
  Context nuc_subst = U.get_context();
  for (lit = c->literals; lit; lit = lit->next) {
    p = C.append_clash(p);
    if (first == NULL)
      first = p;
    if (!Production_mode)
      p->clashable = positive ? !lit->sign : lit->sign;
    else
      p->clashable = (positive && !lit->sign &&!Dollar::evaluable_predicate(SYMNUM(lit->atom)));
    p->nuc_lit = lit;
    p->nuc_subst = nuc_subst;
    if (p->clashable) {
      p->mate_index = positive ? idx->pos : idx->neg;
      p->sat_subst = U.get_context();
    }
  }
  C.clash(first,positive ? pos_hyper_sat_test : neg_hyper_sat_test,	Just_type::HYPER_RES_JUST,	proc_proc);
  U.free_context(nuc_subst);
  C.zap_clash(first);  /* This also frees satellite contexts. */
}  



void Resolve::hyper_resolution(Topform c, int pos_or_neg, Lindex idx,void (*proc_proc) (Topform)){
  if (pos_or_neg == (int) Resolve_type::POS_RES ?     LADRV_GLOBAIS_INST.Lit.positive_clause(c->literals) :   LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
    hyper_satellite(c, pos_or_neg, idx, proc_proc);
  else
    hyper_nucleus(c, pos_or_neg, idx, proc_proc);
}

bool Resolve::target_check(Literals lit, int target_constraint) {
  if (target_constraint == (int) Resolve_type::ANY_RES)
    return true;
  else if (target_constraint ==(int) Resolve_type::POS_RES)
    return lit->sign;
  else if (target_constraint == (int) Resolve_type::NEG_RES)
    return !lit->sign;
  else {
    fatal::fatal_error("target_check, constraint out of range");
    return false;  /* to please the compiler */
  }
} 



void Resolve::ur_sat_atom(bool flipped, Topform c, int target_constraint, Term sat_atom, Lindex idx, void (*proc_proc) (Topform)) 
{
  /* Assume C is a unit. */
  Context sat_subst = U.get_context();
  Context nuc_subst = U.get_context();
  Term fnd_atom;
  Mindex_pos mate_pos;
  Literals slit = c->literals;

  fnd_atom = M.mindex_retrieve_first(sat_atom, (slit->sign?idx->neg:idx->pos), Querytype::UNIFY, sat_subst, nuc_subst,false, &mate_pos);
  while (fnd_atom) {
    Topform nuc = (Topform) fnd_atom->container;
    int numlits = LADRV_GLOBAIS_INST.Lit.number_of_literals(nuc->literals);
    if (numlits > 1 && numlits <= Ur_nucleus_limit &&
	(!Initial_nuclei || nuc->initial)) {
      Literals fnd_lit = C.atom_to_literal(fnd_atom);
      Literals target;
      for (target=nuc->literals; target; target=target->next) {
	if (target != fnd_lit && target_check(target, target_constraint)) {
	    
	  Clash p = NULL;
	  Clash first = NULL;
	  Literals nlit;
	  for (nlit = nuc->literals; nlit; nlit = nlit->next) {
	    p = C.append_clash(p);
	    if (first == NULL)
	      first = p;
	    p->nuc_lit = nlit;
	    p->nuc_subst = nuc_subst;
	    if (nlit == fnd_lit) {
	      p->sat_lit = slit;
	      p->sat_subst = sat_subst;
	      p->clashable = true;
	      p->clashed = true;
	      p->flipped = flipped;
	    }
	    else {
	      p->clashable = (nlit != target);
	      if (p->clashable) {
			p->mate_index = (nlit->sign ? idx->neg : idx->pos);
			p->sat_subst = U.get_context();
	      }
	    }
	  }  /* for each literal of nucleus */
	  C.clash(first, unit_check, Just_type::UR_RES_JUST, proc_proc);
	  C.zap_clash(first);
	}  
      }  /* for each target */
    }  /* if we have a nuc */
    fnd_atom = M.mindex_retrieve_next(mate_pos);
  }  /* for each mate */
  U.free_context(sat_subst);
  U.free_context(nuc_subst);
} 


void Resolve::ur_satellite(Topform c, int target_constraint, Lindex idx,void (*proc_proc) (Topform)) {

  Term atom = c->literals->atom;
  ur_sat_atom(false, c, target_constraint, atom, idx, proc_proc);
  /* if equality, try with the flip */
  if (T.eq_term(atom)) {
    Term flip = Pu.top_flip(atom);
    ur_sat_atom(true, c, target_constraint, flip, idx, proc_proc);
    Pu.zap_top_flip(flip);
  }
}


void Resolve::ur_nucleus(Topform c, int target_constraint, Lindex idx,void (*proc_proc) (Topform)) {
  

  
  if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) > Ur_nucleus_limit || (Initial_nuclei && !c->initial)) return;
  else {
    Literals target;
    for (target = c->literals; target; target = target->next) {
      if (target_check(target, target_constraint)) {
	Clash p = NULL;
	Clash first = NULL;
	Literals lit;
	Context nuc_subst =U.get_context();
	for (lit = c->literals; lit; lit = lit->next) {
	  p = C.append_clash(p);
	  if (first == NULL)   first = p;
	  p->clashable = (lit != target);
	  p->nuc_lit = lit;
	  p->nuc_subst = nuc_subst;
	  if (p->clashable) {
	    p->mate_index = (lit->sign ? idx->neg : idx->pos);
	    p->sat_subst = U.get_context();
	  }
	}
	  C.clash(first, unit_check, Just_type::UR_RES_JUST, proc_proc);
	  U.free_context(nuc_subst);
	  C.zap_clash(first);  /* This also frees satellite contexts. */
     }
    }
  }
}

void Resolve::ur_resolution(Topform c, int target_constraint, Lindex idx, void (*proc_proc) (Topform)) {
  if (LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals))
    ur_satellite(c, target_constraint, idx, proc_proc);
  else
    ur_nucleus(c, target_constraint, idx, proc_proc);
} 


void Resolve::xx_res(Literals lit, void (*proc_proc) (Topform)) {

  Term alpha = ARG(lit->atom,0);
  Term beta  = ARG(lit->atom,1);
  Context subst = U.get_context();
  Trail tr = NULL;

  if (U.unify(alpha, subst, beta, subst, &tr)) {
    Topform parent = (Topform) lit->atom->container;
    int n = LADRV_GLOBAIS_INST.Lit.literal_number(parent->literals, lit);
    Topform c = TF.get_topform();
    Literals l;
    c->justification = J.xxres_just(parent, n);
    for (l = parent->literals; l; l = l->next) {
      if (l != lit)
	    c->literals = LADRV_GLOBAIS_INST.Lit.append_literal(c->literals, C.apply_lit(l, subst));
    }
    U.undo_subst(tr);
    TF.upward_clause_links(c);
    c->attributes = AT.inheritable_att_instances(parent->attributes, subst);
    
    (*proc_proc)(c);
  }
  U.free_context(subst);
}

void Resolve::binary_resolvent(bool flipped, Literals l1, Context s1, Literals l2, Context s2, void (*proc_proc) (Topform)){

  Topform r = TF.get_topform();
  Topform nuc = (Topform)  l1->atom->container;
  Topform sat =  (Topform) l2->atom->container;
  Ilist j  = NULL;
  Literals l3;
  int i;
  int n = 0;

  /* Include literals in the nucleus. */
  for (l3 = nuc->literals, i=1; l3; l3 = l3->next, i++) {
    if (l3 == l1)
      n = i;  /* index of resolved literal */
    else
      r->literals = LADRV_GLOBAIS_INST.Lit.append_literal(r->literals, C.apply_lit(l3, s1));
  }
  j = IL.ilist_append(nuc->id);
  j = IL.ilist_append(n);
  
  /* Include literals in the satellite. */
  for (l3 = sat->literals, i=1; l3; l3 = l3->next, i++) {
    if (l3 == l2)
      n = i;  /* index of resolved literal */
    else
      r->literals = LADRV_GLOBAIS_INST.Lit.append_literal(r->literals, C.apply_lit(l3, s2));
  }
  
  j = IL.ilist_append(sat->id);
  j = IL.ilist_append(flipped ? -n : n);
  
  TF.inherit_attributes(nuc, s1, sat, s2, r);

  r->justification = J.resolve_just(j, Just_type::BINARY_RES_JUST);
  TF.upward_clause_links(r);
  (*proc_proc)(r);
} 


bool Resolve:: binary_parent_test(Literals lit, int res_type, int check_type) {
  
  
  Topform c =(Topform) lit->atom->container;

  if (res_type == (int)Resolve_type::POS_RES) {  /* positive resolution (one parent positive) */
    if (LADRV_GLOBAIS_INST.Lit.positive_clause(c->literals))
      return !Ordered || Maximal::maximal_literal(c->literals, lit, check_type);
    else if (lit->sign)
      return false;  /* cannot resolve on pos literal in nonpos clause */
    else if (Maximal::exists_selected_literal(c->literals))
      return Maximal::selected_literal(lit);
    else
      return !Ordered || Maximal::maximal_signed_literal(c->literals, lit, check_type); /* max neg */
  }

  else if (res_type == (int) Resolve_type::NEG_RES) {  /* negative resolution (one parent neg) */
    if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
      return !Ordered || Maximal::maximal_literal(c->literals, lit, check_type);
    else if (!lit->sign)
      return false;  /* cannot resolve on neg literal in nonneg clause */
    else  /* selection ignored for negative resolution */
      return !Ordered || Maximal::maximal_signed_literal(c->literals, lit, check_type); /* max pos */
  }

  else {  /* ANY_RES (not necessarily positive or negative resolution) */
    if (Maximal::exists_selected_literal(c->literals)) {
      if (lit->sign)
	return false;  /* if any selected lits, cannot resolve on pos lit */
      else
	return Maximal::selected_literal(lit);
    }
    else
      /* no selected literals in clause */
      return !Ordered || Maximal::maximal_literal(c->literals, lit, check_type);
  }
}

Topform Resolve::instantiate_clause(Topform c, Context subst) {

  Topform d = TF.get_topform();
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next)
    d->literals = LADRV_GLOBAIS_INST.Lit.append_literal(d->literals, C.apply_lit(lit, subst));
  for (lit = d->literals; lit; lit = lit->next)
    lit->atom->container = d;
  return d;
}


bool Resolve::check_instance(Literals lit, Context subst, int res_type) {
  
  Topform c = (Topform) lit->atom->container;
  if (Maximal::number_of_maximal_literals(c->literals, (int) CheckType::FLAG_CHECK) == 1 ||  U.variable_substitution(subst))   return true;
  else {
    Literals a;
    bool ok;
    int n = LADRV_GLOBAIS_INST.Lit.literal_number(c->literals, lit);
    Topform d = instantiate_clause(c, subst);
    Maximal::copy_selected_literal_marks(c->literals, d->literals);
    a = LADRV_GLOBAIS_INST.Lit.ith_literal(d->literals, n);

    /* Note that using binary_parent_test repeats several tests
       not having to do with maximality.  These repeated steps
       are not expensive.
    */

    ok = binary_parent_test(a, res_type, (int) CheckType::FULL_CHECK);
    TF.zap_topform(d);
    if (!ok)
      Res_instance_prunes++;
    return ok;
  }
}

bool Resolve::check_instances(Literals lit1, Context subst1,Literals lit2, Context subst2,int res_type) {
  if (!Check_instances)
    return true;
  else
    return
      check_instance(lit1, subst1, res_type) && check_instance(lit2, subst2, res_type);
}

void Resolve::bin_res_lit(Topform giv, Literals lit, Term atom, int res_type, Lindex idx, void (*proc_proc) (Topform)) {
  bool flipped = (lit->atom != atom);
  Context nuc_subst = U.get_context();
  Context sat_subst = U.get_context();
  Term sat_atom;
  Mindex_pos mate_pos;
  sat_atom = M.mindex_retrieve_first(atom, lit->sign ? idx->neg : idx->pos, Querytype::UNIFY, nuc_subst, sat_subst,false, &mate_pos);
  while (sat_atom) {
    Literals slit = C.atom_to_literal(sat_atom);
    if (binary_parent_test(slit, res_type, (int) CheckType::FLAG_CHECK) &&check_instances(lit, nuc_subst, slit, sat_subst, res_type))
      binary_resolvent(flipped, lit, nuc_subst, slit, sat_subst, proc_proc);
    sat_atom = M.mindex_retrieve_next(mate_pos);
  }
  U.free_context(nuc_subst);
  U.free_context(sat_subst);
}  

void Resolve::binary_resolution(Topform c,  int res_type,  /* POS_RES, NEG_RES, ANY_RES */ Lindex idx, void (*proc_proc) (Topform)) {
  
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next) {
    if (binary_parent_test(lit, res_type, (int) CheckType::FLAG_CHECK)) {
      bin_res_lit(c, lit, lit->atom, res_type, idx, proc_proc);

      /* If equality, try for resolution with the flip. */
      if (T.eq_term(lit->atom)) {
	   Term flip = Pu.top_flip(lit->atom);
	   bin_res_lit(c, lit, flip, res_type, idx, proc_proc);
	   Pu.zap_top_flip(flip);
      }

      /* Try for resolution with x=x. */
      if (LADRV_GLOBAIS_INST.Lit.neg_eq(lit)) {
	     xx_res(lit, proc_proc);
      }
    }
  }
} 


void Resolve::binary_factors(Topform c, void (*proc_proc) (Topform)){
  
  
  Literals l1;
  int i = 1;
  Context subst = U.get_context();
  for (l1 = c->literals; l1; l1 = l1->next, i++) {
    Literals l2;
    int j = i+1;
    for (l2 = l1->next; l2; l2 = l2->next, j++) {

      Trail tr = NULL;
      if (l1->sign == l2->sign &&
	  /* maximal_literal_check ??? */
	  U.unify(l1->atom,subst,l2->atom,subst,&tr)) {
	Topform f = TF.get_topform();
	Literals l3;
	f->justification = J.factor_just(c, i, j);
	for (l3 = c->literals; l3; l3 = l3->next) {
	  if (l3 != l2)
	    f->literals = LADRV_GLOBAIS_INST.Lit.append_literal(f->literals, C.apply_lit(l3, subst));
	}
	 U.undo_subst(tr);
	 TF.upward_clause_links(f);
	 f->attributes = AT.cat_att(f->attributes,AT.inheritable_att_instances(c->attributes,subst));
	 (*proc_proc)(f);
    }
    }
  }
  U.free_context(subst);
}  /* binary_factors */

/*************
 *
 *   merge_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void Resolve::merge_literals(Topform c){

  
  Literals l3;
  int n;
  bool null_literals = false;
  for (l3 = c->literals, n = 1; l3; l3 = l3->next, n++) {
    Literals l2;
    for (l2 = c->literals; l2 != l3; l2 = l2->next) {
      if (l2->atom && l3->atom) {
	if (l3->sign == l2->sign && T.term_ident(l3->atom, l2->atom)) {
	  T.zap_term(l3->atom);
	  c->justification = J.append_just(c->justification, J.merge_just(n));
	  l3->atom = NULL;  /* remove it below */
	  null_literals = true;
	}
      }
    }
  }
  if (null_literals)
    c->literals = LADRV_GLOBAIS_INST.Lit.remove_null_literals(c->literals);
}  /* merge_literals */

Topform Resolve::copy_inference(Topform c) {
  //TopformContainer TF;
  //JustContainer J;
  //AttributeContainer AT;
  Topform novo = TF.copy_clause(c);
  /* Don't copy the justification; build a "copy" justification. */
  novo->justification = J.copy_just(c);
#if 0
  /* Copy all attributes. */
  novo->attributes = copy_attributes(c->attributes);
#else
  /* Copy inheritable attributes only. */
  novo->attributes = AT.inheritable_att_instances(c->attributes, NULL);
#endif
  return novo;
}



/* PUBLIC */
Topform Resolve::resolve2(Topform c1, int n1, Topform c2, int n2, bool renumber_vars) {

  Topform res;
  Literals l1 = LADRV_GLOBAIS_INST.Lit.ith_literal(c1->literals, n1);
  Literals l2 = LADRV_GLOBAIS_INST.Lit.ith_literal(c2->literals, abs(n2));
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  Context s1 = U.get_context();
  Context s2 = U.get_context();
  Trail tr = NULL;
  Term a2x;

  if (n2 < 0)
    a2x = Pu.top_flip(a2);
  else
    a2x = a2;

  if (l1->sign != l2->sign && U.unify(a1, s1, a2x, s2, &tr)) {
    Literals lit;
    res = TF.get_topform();
    for (lit = c1->literals; lit; lit = lit->next)
      if (lit != l1)
	res->literals = LADRV_GLOBAIS_INST.Lit.append_literal(res->literals, C.apply_lit(lit,  s1));
    for (lit = c2->literals; lit; lit = lit->next)
      if (lit != l2)
	res->literals = LADRV_GLOBAIS_INST.Lit.append_literal(res->literals, C.apply_lit(lit,  s2));

    TF.inherit_attributes(c1, s1, c2, s2, res);
    res->justification = J.binary_res_just(c1, n1, c2, n2);
    TF.upward_clause_links(res);
    if (renumber_vars)
      TF.renumber_variables(res, MAX_VARS);
    U.undo_subst(tr);
  }
  else
    res = NULL;

  if (n2 < 0)
    Pu.zap_top_flip(a2x);

  U.free_context(s1);
  U.free_context(s2);
  return res;
}  /* resolve2 */

/*************
 *
 *   resolve3()
 *
 *************/

/* DOCUMENTATION
Similar to resolve2(), but literals are given instead of integers.
*/

/* PUBLIC */
Topform Resolve::resolve3(Topform c1, Literals l1, Topform c2, Literals l2, bool renumber_vars) {
 
 return resolve2(c1, LADRV_GLOBAIS_INST.Lit.literal_number(c1->literals, l1),
		  c2, LADRV_GLOBAIS_INST.Lit.literal_number(c2->literals, l2),
		  renumber_vars);
}  /* resolve3 */

/*************
 *
 *   xx_resolve2()
 *
 *************/

/* DOCUMENTATION
Resolve, if possible, a clause with x=x.
Renumber vars, include justification, transfer inheritable
attributes, but do not assign an ID.
*/

/* PUBLIC */
Topform Resolve::xx_resolve2(Topform c, int n, bool renumber_vars) {

  Topform res;
  Literals l = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n);
  Context s = U.get_context();
  Trail tr = NULL;

  if (LADRV_GLOBAIS_INST.Lit.neg_eq(l) &&
      U.unify(ARG(l->atom,0), s,
	    ARG(l->atom,1), s, &tr)) {
    Literals lit;
    res = TF.get_topform();
    for (lit = c->literals; lit; lit = lit->next)
      if (lit != l)
	res->literals = LADRV_GLOBAIS_INST.Lit.append_literal(res->literals, C.apply_lit(lit,  s));

    res->attributes = AT.inheritable_att_instances(c->attributes, s);
    res->justification = J.xxres_just(c, n);
    TF.upward_clause_links(res);
    if (renumber_vars)
      TF.renumber_variables(res, MAX_VARS);
    U.undo_subst(tr);
  }
  else
    res = NULL;
  U.free_context(s);
  return res;
}  /* xx_resolve2 */
