#include "paramod.h"
#include "fatal.h"

bool  Paramodulation::Ordered_inference=false;
bool  Paramodulation::Positive_inference=false;
bool  Paramodulation::Para_from_vars=true;
bool  Paramodulation::Para_into_vars=false;
bool  Paramodulation::Para_from_small=false;
bool  Paramodulation::Check_instances=false;  /* non-oriented from lits */
int   Paramodulation::Para_instance_prunes=0;     /* counter */
int   Paramodulation::Basic_prunes=0;  



void Paramodulation::paramodulation_options(
											bool ordered_inference,
											bool check_instances,
											bool positive_inference,
											bool basic_paramodulation,
											bool para_from_vars,
											bool para_into_vars,
											bool para_from_small )
{
  Ordered_inference = ordered_inference;
  Para_from_vars = para_from_vars;
  Para_into_vars = para_into_vars;
  Para_from_small = para_from_small;
  Check_instances = check_instances;
  Positive_inference = positive_inference;
  Basic::set_basic_paramod(basic_paramodulation);
  Para_instance_prunes = 0;
  Basic_prunes = 0;
} 


int Paramodulation::para_instance_prunes() {
  return Para_instance_prunes;
}  


int Paramodulation::basic_paramodulation_prunes(void) {
  return Basic_prunes;
}

bool Paramodulation::basic_check(Term into_term) {
  
  if (Basic::basic_paramod() && Basic::nonbasic_term(into_term)) {
    Basic_prunes++;
    return false;
  }
  else
    return true;
}


Literals Paramodulation::apply_lit_para(Literals lit, Context c) {
  
  UnifyContainer U;
  if (Basic::basic_paramod())
    return LADRV_GLOBAIS_INST.Lit.new_literal(lit->sign, Basic::apply_basic(lit->atom, c));
  else
    return LADRV_GLOBAIS_INST.Lit.new_literal(lit->sign, U.apply(lit->atom, c));
}


#if 0
Term Paramodulation::apply_substitute_para(Term t, Term beta, Context from_subst, Term into, Context into_subst) {
  UnifyContainer U;
  if (Basic::basic_paramod())
    return Basic::apply_basic_substitute(t, beta, from_subst, into, into_subst);
  else
    return U.apply_substitute(t, beta, from_subst, into, into_subst);
}  /* apply_substitute_para */
#endif



Topform Paramodulation::paramodulate(Literals from_lit, int from_side, Context from_subst,Topform into_clause, Ilist into_pos, Context into_subst) {
  TopformContainer TF;
  
  UnifyContainer U;
  Topform from_clause =(Topform) from_lit->atom->container;
  Topform p = TF.get_topform();
  Term beta = ARG(from_lit->atom, from_side == 0 ? 1 : 0);
  Literals into_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(into_clause->literals, into_pos->i);
  Literals lit;
  for (lit = from_clause->literals; lit != NULL; lit = lit->next) {
    if (lit != from_lit)
      p->literals = LADRV_GLOBAIS_INST.Lit.append_literal(p->literals,apply_lit_para(lit, from_subst));
  }
  for (lit = into_clause->literals; lit != NULL; lit = lit->next) {
    if (lit != into_lit)
      p->literals = LADRV_GLOBAIS_INST.Lit.append_literal(p->literals,apply_lit_para(lit, into_subst));
    else
      p->literals = LADRV_GLOBAIS_INST.Lit.append_literal(p->literals, LADRV_GLOBAIS_INST.Lit.new_literal(lit->sign,  U.apply_substitute2(lit->atom, beta,from_subst, into_pos->next, into_subst)));
  }
  TF.inherit_attributes(from_clause, from_subst, into_clause, into_subst, p);
  TF.upward_clause_links(p);
  return p;
}


bool Paramodulation::para_from_right(Term atom) {
  /* Assume atom is an eq_atom. */
  
  Parautil Pu;
  if (Para_from_small)
    return true;
  if (Pu.oriented_eq(atom))
    return false;
  else if (Pu.renamable_flip_eq(atom) &&  LADRV_GLOBAIS_INST.Lit.unit_clause(((Topform) atom->container)->literals))
    return false;
  else
    return true;
}  


bool Paramodulation::from_parent_test(Literals from_lit, int check) {
  
  Topform from_parent = (Topform) from_lit->atom->container;
  if (Positive_inference)
    return
      LADRV_GLOBAIS_INST.Lit.pos_eq(from_lit) &&    LADRV_GLOBAIS_INST.Lit.positive_clause(from_parent->literals) &&    (!Ordered_inference ||   Maximal::maximal_literal(from_parent->literals, from_lit, check));
  else
    return
      LADRV_GLOBAIS_INST.Lit.pos_eq(from_lit) &&   !Maximal::exists_selected_literal(from_parent->literals) &&    (!Ordered_inference ||    Maximal::maximal_literal(from_parent->literals, from_lit, check));
} 

bool Paramodulation::into_parent_test(Literals into_lit, int check) {
  
  Topform into_parent = (Topform) into_lit->atom->container;
  if (into_lit->sign) {
    /* into positive literal */
    if (Positive_inference)
      return
	LADRV_GLOBAIS_INST.Lit.positive_clause(into_parent->literals) &&
	(!Ordered_inference || Maximal::maximal_literal(into_parent->literals, into_lit, check));
    else
      return !Maximal::exists_selected_literal(into_parent->literals) && (!Ordered_inference || Maximal::maximal_literal(into_parent->literals, into_lit, check));
  }
  else {
    /* into negative literal */
    if (Positive_inference) {
      if (Maximal::exists_selected_literal(into_parent->literals))	return Maximal::selected_literal(into_lit);
      else	return (!Ordered_inference || Maximal::maximal_signed_literal(into_parent->literals,into_lit,check));
    }
    else {
      if (Maximal::exists_selected_literal(into_parent->literals))	return Maximal::selected_literal(into_lit);
      else	return (!Ordered_inference || Maximal::maximal_literal(into_parent->literals, into_lit, check));
    }
  }
}


bool Paramodulation::check_instance(Literals lit, Context subst, bool is_from_parent) {
  
  UnifyContainer U;
  TopformContainer TF;
  Topform c = (Topform) lit->atom->container;
  if (Maximal::number_of_maximal_literals(c->literals, (int) CheckType::FLAG_CHECK) == 1 || U.variable_substitution(subst))   return true;
  else {
    Literals a;
    bool ok;
    int n = LADRV_GLOBAIS_INST.Lit.literal_number(c->literals, lit);
    Topform d = Resolve::instantiate_clause(c, subst);
    Maximal::copy_selected_literal_marks(c->literals, d->literals);
    a = LADRV_GLOBAIS_INST.Lit.ith_literal(d->literals, n);

    if (is_from_parent)
      ok = from_parent_test(a, (int) CheckType::FLAG_CHECK);
    else
      ok = into_parent_test(a, (int) CheckType::FLAG_CHECK);
    TF.zap_topform(d);
    if (!ok)
      Para_instance_prunes++;
    return ok;
  }
}


bool Paramodulation::check_instances(Literals from_lit, int from_side, Context cf, Literals into_lit, Term into, Context ci) {
  if (!Check_instances)
    return true;
  else {
    return
      check_instance(from_lit, cf, true) &&   check_instance(into_lit, ci, false);
  }
} 



void Paramodulation::para_into(	Literals from_lit, int from_side, Context cf, Ilist from_pos,
								Topform into_clause, Literals into_lit, Term into, Context ci,
								Ilist into_pos,
								bool skip_top,
								void (*proc_proc) (Topform))
{
  
  IlistContainer I,Iaux;
  UnifyContainer U;
  JustContainer J;
  if ((!VARIABLE(into) | Para_into_vars) && basic_check(into)) {
    int i;
    if (COMPLEX(into)) {
	  I.set_head(into_pos); 
      Ilist last = I.ilist_last();
      Ilist novo = Iaux.get_ilist();
      last->next = novo;
      novo->i = 0;
      for (i = 0; i < ARITY(into); i++) {
		novo->i += 1;
		para_into(from_lit, from_side, cf, from_pos,
		  into_clause, into_lit, ARG(into,i), ci, into_pos,
		  false,
		  proc_proc);
      }
      Iaux.free_ilist(novo);
      last->next = NULL;
    }
    if (!skip_top) {
      Trail tr = NULL;
      Term alpha = ARG(from_lit->atom, from_side);
      if (U.unify(alpha, cf, into, ci, &tr)) {
			if (check_instances(from_lit, from_side, cf, into_lit, into, ci)) {
				IlistContainer Cpy1,Cpy2;
				Topform p = paramodulate(from_lit, from_side, cf, into_clause, into_pos, ci);
				Cpy1.set_head(from_pos);
				Cpy2.set_head(into_pos);
				p->justification = J.para_just(Just_type::PARA_JUST, (Topform) from_lit->atom->container, Cpy1.copy_ilist(),  into_clause,   Cpy2.copy_ilist());
				(*proc_proc)(p);
			}
			U.undo_subst(tr);
      }
    }
  }
}

void Paramodulation::para_into_lit(	Literals from_lit, int from_side, Context cf,
									Literals into_lit, Context ci,
									bool check_top,
									void (*proc_proc) (Topform))
{
  IlistContainer I1,I2;
  
  Term alpha = ARG(from_lit->atom, from_side);
  if (!VARIABLE(alpha) || Para_from_vars) {
    /* Position vectors are constructed FORWARD. */
    Ilist from_pos = I1.ilist_prepend(0);
	from_pos = I1.ilist_prepend(0);
	
    Ilist into_pos = I2.ilist_prepend(0);
	into_pos = I2.ilist_prepend(0);
	
    Term into_atom = into_lit->atom;
    Term from_atom = from_lit->atom;
    int i;
    Topform from_clause = (Topform) from_atom->container;
    Topform into_clause = (Topform) into_atom->container;
    bool positive_equality = LADRV_GLOBAIS_INST.Lit.pos_eq(into_lit);

    from_pos->i = LADRV_GLOBAIS_INST.Lit.literal_number(from_clause->literals, from_lit);
    from_pos->next->i = from_side+1;  /* arg of from_lit, counts from 1 */
    into_pos->i = LADRV_GLOBAIS_INST.Lit.literal_number(into_clause->literals, into_lit);
    for (i = 0; i < ARITY(into_atom); i++) {
      bool skip_top = (check_top && positive_equality &&  ( i == 0 || (i == 1 && para_from_right(into_lit->atom) ) ) );
      into_pos->next->i += 1;  /* increment arg number */
      para_into(from_lit, from_side, cf, from_pos, into_clause, into_lit, ARG(into_atom,i), ci, into_pos,skip_top, proc_proc);
    }
    I1.zap_ilist();
    I2.zap_ilist();
  }
}



/* PUBLIC */
void Paramodulation::para_from_into(Topform from, Context cf,  Topform into, Context ci, bool check_top, void (*proc_proc) (Topform)) {
  
  Literals from_lit;
  Literals into_lit;
  
  if (Maximal::exists_selected_literal(from->literals))    return;  /* cannot para from clause with selected literals */
  else {
    for (from_lit = from->literals; from_lit; from_lit = from_lit->next) {
      if (from_parent_test(from_lit, (int) CheckType::FLAG_CHECK)) {
        for (into_lit = into->literals; into_lit; into_lit = into_lit->next) {
            if (into_parent_test(into_lit, (int) CheckType::FLAG_CHECK)) {
                para_into_lit(from_lit,0,cf,into_lit,ci,check_top,proc_proc);  /* from L */
                if (para_from_right(from_lit->atom))   para_into_lit(from_lit,1,cf,into_lit,ci,check_top,proc_proc); /* from R */
            }
	   }
     }
   }
  }
} 


Topform Paramodulation::para_pos(Topform from_clause, Ilist from_pos, Topform into_clause, Ilist into_pos) {
  UnifyContainer U;
  
  JustContainer J;
  TermContainer T;
  IlistContainer I1,I2;
  TopformContainer TF;
  Context cf = U.get_context();
  Context ci = U.get_context();
  Trail tr = NULL;
  Topform paramodulant;
  bool ok;

  Literals from_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(from_clause->literals, from_pos->i);
  Literals into_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(into_clause->literals, into_pos->i);
  int from_side = (from_pos->next->i == 1 ? 0 : 1);
  Term alpha = ARG(from_lit->atom, from_side);
  Term into_term = T.term_at_pos(into_lit->atom, into_pos->next);
  if (into_term == NULL)
    fatal::fatal_error("paramod2_instances, term does not exist");

  ok = U.unify(alpha, cf, into_term, ci, &tr);
  if (!ok)
    fatal::fatal_error("para_pos, terms do not unify");

  paramodulant = paramodulate(from_lit, from_side, cf,into_clause, into_pos, ci);
  I1.set_head(from_pos);
  I2.set_head(into_pos);
  paramodulant->justification = J.para_just(Just_type::PARA_JUST, from_clause, I1.copy_ilist(),  into_clause, I2.copy_ilist());
  TF.renumber_variables(paramodulant, MAX_VARS);
  U.undo_subst(tr);
  U.free_context(cf);
  U.free_context(ci);
  return paramodulant;
}


Topform Paramodulation::para_pos2(Topform from, Ilist from_pos, Topform into, Ilist into_pos) {
  UnifyContainer U;
  
  IlistContainer I1,I2;
  TopformContainer TF;
  TermContainer T;
  JustContainer J;
  
  
  Context from_subst = U.get_context();
  Context into_subst = U.get_context();
  Trail tr = NULL;
  bool ok;
  int from_side;
  Term alpha, into_term;
  Topform p;
  Term beta;
  Literals lit;

  Literals from_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(from->literals, from_pos->i);
  Literals into_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(into->literals, into_pos->i);
  from_side = (from_pos->next->i == 1 ? 0 : 1);
  alpha = ARG(from_lit->atom, from_side);
  into_term = T.term_at_pos(into_lit->atom, into_pos->next);
  if (into_term == NULL)
    fatal::fatal_error("paramod2_instances, term does not exist");

  ok = U.unify(alpha, from_subst, into_term, into_subst, &tr);
  if (!ok)
    fatal::fatal_error("para_pos2, terms do not unify");

  p = TF.get_topform();
  beta = ARG(from_lit->atom, from_side == 0 ? 1 : 0);
  for (lit = from->literals; lit; lit = lit->next) {
    if (lit != from_lit)
      p->literals = LADRV_GLOBAIS_INST.Lit.append_literal(p->literals,apply_lit_para(lit, from_subst));
  }
  for (lit = into->literals; lit; lit = lit->next) {
    if (lit != into_lit) {
      p->literals = LADRV_GLOBAIS_INST.Lit.append_literal(p->literals,apply_lit_para(lit, into_subst));
    }
    else {
      p->literals = LADRV_GLOBAIS_INST.Lit.append_literal(p->literals, LADRV_GLOBAIS_INST.Lit.new_literal(lit->sign,  U.apply_substitute2(lit->atom, beta,from_subst,into_pos->next,into_subst)));
    }
  }
  TF.inherit_attributes(from, from_subst, into, into_subst, p);
  TF.upward_clause_links(p);
  I1.set_head(from_pos);
  I2.set_head(into_pos);
  p->justification = J.para_just(Just_type::PARA_JUST,   from, I1.copy_ilist(),   into, I2.copy_ilist());
  TF.renumber_variables(p, MAX_VARS);
  U.undo_subst(tr);
  U.free_context(from_subst);
  U.free_context(into_subst);
  return p;
}
