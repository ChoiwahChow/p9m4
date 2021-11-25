#include "ivy.h"
#include "fatal.h"
#include "mystring.h"

												  
string Ivy::Dict[DICT_SIZE][2]={ {"0",  "zero_for_ivy"},
							     {"1",  "one_for_ivy"},
								 {"'",  "quote_for_ivy"},
								 {"\\", "backslash_for_ivy"},
								 {"@",  "at_for_ivy"},
								 {"^",  "meet_for_ivy"}
								 };
								 
string Ivy::dict_lookup(string key) {
  int i;
  for (i = 0; i < DICT_SIZE; i++) {
    if (myString::str_ident(Dict[i][0], key)) return Dict[i][1];
  }
  return myString::null_string();
}	


void Ivy::ivy_term_trans(Term t) {
  SymbolContainer S;
  if (VARIABLE(t))
    return;
  else {
    int i;
    string s = dict_lookup(S.sn_to_str(SYMNUM(t)));
    if (s!=myString::null_string()) t->private_symbol = -S.str_to_sn(s, ARITY(t));
    for (i = 0; i < ARITY(t); i++)   ivy_term_trans(ARG(t,i));
  }
} 	

void Ivy::ivy_clause_trans(Topform c) {
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next) {
    ivy_term_trans(lit->atom);
  }
} 
		

void Ivy::sb_ivy_write_term(String_buf sb, Term t) {
  StrbufContainer SB;
  SymbolContainer S;
  SB.set_string_buf(sb);
  if (VARIABLE(t)) {
    SB.sb_append("v");
    SB.sb_append_int(VARNUM(t));
  }
  else {
    int i;
    SB.sb_append("(");
    SB.sb_append(S.sn_to_str(SYMNUM(t)));
    for (i = 0; i < ARITY(t); i++) {
      SB.sb_append(" ");
      sb_ivy_write_term(SB.get_string_buf(), ARG(t,i));
    }
    SB.sb_append(")");
  }
  sb=SB.get_string_buf();
}		

void Ivy::sb_ivy_write_pair(String_buf sb, Term pair) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  Term v = ARG(pair,0);
  Term t = ARG(pair,1);
  SB.sb_append("(");
  sb_ivy_write_term(SB.get_string_buf(),v);
  SB.sb_append(" . ");
  sb_ivy_write_term(SB.get_string_buf(), t);
  SB.sb_append(")");
  sb=SB.get_string_buf();
} 

void Ivy::sb_ivy_write_pairs(String_buf sb, Plist pairs) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  Plist p;
  SB.sb_append("(");
  for (p = pairs; p; p = p->next) {
    sb_ivy_write_pair(SB.get_string_buf(), (Term) p->v);
    if (p->next)
      SB.sb_append(" ");	
  }
  SB.sb_append(")");
  sb=SB.get_string_buf();
} 


void Ivy::sb_ivy_write_position(String_buf sb, Ilist position) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  Ilist p;
  SB.sb_append("(");
  for (p = position; p; p = p->next) {
    SB.sb_append_int(p->i);
    if (p->next)
      SB.sb_append(" ");	
  }
  SB.sb_append(")");
  sb=SB.get_string_buf();
} 


void Ivy::sb_ivy_write_lit(String_buf sb, Literals lit) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  if (lit->sign == false) {
    SB.sb_append("(not ");
    sb_ivy_write_term(SB.get_string_buf(), lit->atom);
    SB.sb_append(")");
  }
  else
    sb_ivy_write_term(SB.get_string_buf(), lit->atom);
  sb=SB.get_string_buf();
} 


void Ivy::sb_ivy_write_literals(String_buf sb, Literals lits) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  if (lits == NULL)
    SB.sb_append("false");
  else if (lits->next == NULL)
    sb_ivy_write_lit(SB.get_string_buf(), lits);
  else {
    SB.sb_append("(or ");
    sb_ivy_write_lit(SB.get_string_buf(), lits);
    SB.sb_append(" ");
    sb_ivy_write_literals(SB.get_string_buf(), lits->next);
    SB.sb_append(")");
  }
  sb=SB.get_string_buf();
}


void Ivy::sb_ivy_write_just(String_buf sb, Ivyjust j, I3list map) {
  StrbufContainer SB;
  JustContainer J;
  SB.set_string_buf(sb);
  if (j->type == Just_type::INPUT_JUST) {
    SB.sb_append("(input)");
  }
  else if (j->type == Just_type::PROPOSITIONAL_JUST) {
    SB.sb_append("(propositional ");
    J.sb_append_id(SB.get_string_buf(),j->parent1, map);
    SB.sb_append(")");
  }
  else if (j->type == Just_type::NEW_SYMBOL_JUST) {
    SB.sb_append("(new_symbol ");
    J.sb_append_id(SB.get_string_buf(), j->parent1, map);
    SB.sb_append(")");
  }
  else if (j->type == Just_type::FLIP_JUST) {
    SB.sb_append("(flip ");
    J.sb_append_id(SB.get_string_buf(), j->parent1, map);
    SB.sb_append(" ");
    sb_ivy_write_position(SB.get_string_buf(), j->pos1);
    SB.sb_append(")");
  }
  else if (j->type == Just_type::INSTANCE_JUST) {
    SB.sb_append("(instantiate ");
    J.sb_append_id(SB.get_string_buf(), j->parent1, map);
    SB.sb_append(" ");
    sb_ivy_write_pairs(SB.get_string_buf(), j->pairs);
    SB.sb_append(")");
  }
  else if (j->type == Just_type::BINARY_RES_JUST ||
	   j->type == Just_type::PARA_JUST) {
    if (j->type == Just_type::BINARY_RES_JUST)
      SB.sb_append("(resolve ");
    else
      SB.sb_append("(paramod ");
    J.sb_append_id(SB.get_string_buf(), j->parent1, map);
    SB.sb_append(" ");
    sb_ivy_write_position(SB.get_string_buf(), j->pos1);
    SB.sb_append(" ");
    J.sb_append_id(SB.get_string_buf(), j->parent2, map);
    SB.sb_append(" ");
    sb_ivy_write_position(SB.get_string_buf(), j->pos2);
    SB.sb_append(")");
  }
  else
    fatal::fatal_error("sb_ivy_write_just, bad ivy justification");
 sb=SB.get_string_buf();
} 


void Ivy::sb_ivy_write_clause_jmap(String_buf sb, Topform c, I3list map) {
  TopformContainer TF;
  StrbufContainer SB;
  JustContainer J;
  SB.set_string_buf(sb);
  if (c->justification->type != Just_type::IVY_JUST) {
    cout<<"not ivy just: "; TF.fprint_clause(cout,c);
	fatal::fatal_error("sb_ivy_write_clause_jmap, not IVY_JUST");
  }

  SB.sb_append("(");                                   /* start */
  J.sb_append_id(SB.get_string_buf(), c->id, map);                         /* ID */
  SB.sb_append(" ");
  sb_ivy_write_just(SB.get_string_buf(), c->justification->u.ivy, map);  /* justification */
  SB.sb_append(" ");
  sb_ivy_write_literals(SB.get_string_buf(), c->literals);               /* literals */
  SB.sb_append(" NIL)\n");                             /* end  */
  sb=SB.get_string_buf();
}


Topform Ivy::instantiate_inference(Topform c, Context subst) {

  JustContainer J;
  UnifyContainer U;
  AttributeContainer A;
  TopformContainer TF;
  Topform d = Resolve::instantiate_clause(c, subst);
  Plist pairs = U.context_to_pairs(LADRV_GLOBAIS_INST.Lit.varnums_in_clause(c->literals), subst);
  d->justification = J.ivy_just(Just_type::INSTANCE_JUST, c->id, NULL, 0, NULL, pairs);
  TF.inherit_attributes(c, subst, NULL, NULL, d);
  TF.upward_clause_links(d);
  return d;
} 


Topform Ivy::renumber_inference(Topform c) {
  TopformContainer TF;
  JustContainer J;
  AttributeContainer A;
  Plist pairs;
  Topform child = TF.copy_clause(c);
  pairs = TF.renum_vars_map(child);
  if (pairs == NULL) {
    TF.zap_topform(child);
    return NULL;
  }
  else {
    child->justification = J.ivy_just(Just_type::INSTANCE_JUST, c->id, NULL, 0, NULL, pairs);
    child->attributes = A.copy_attributes(c->attributes);
    return child;
  }
}

Ilist Ivy::ivy_lit_position(int n, int number_of_lits)
{
  /* Build an Ivy-style position for a literal.  Ivy clauses are
     binary trees of ORs.  Ivy accepts any association, but the
     ones we are using are always right associated.
     To build the position, we have to know if the designated
     literal is the last, e.g.,
        Designated     If last       If not last
            1              ()                (1)
            2             (2)              (2 1)
            3           (2 2)            (2 2 1)
            4         (2 2 2)          (2 2 2 1)
   */   
  IlistContainer I;
  int i;
  Ilist pos = NULL;
  I.set_head(pos);
  if (n != number_of_lits)
    pos = I.ilist_prepend(1);
  for (i = 1; i < n; i++)
    pos = I.ilist_prepend(2);
  return pos;
}


Ilist Ivy::ivy_para_position(Ilist pos1, bool sign, int number_of_lits) {
  /* Given a LADR-style position for a term within a clause,
     build an Ivy-style position for a term within a clause.
     See ivy_lit_position.
   */   
  IlistContainer Ipos2;
  IlistContainer Ipos1_next;
  IlistContainer Iaux;
  Ilist pos2 = ivy_lit_position(pos1->i, number_of_lits);
  Ipos2.set_head(pos2); 
  if (!sign)  /* Ivy position considers sign, LADR's doesn't. */
    pos2 = Ipos2.ilist_append(1);
  
  Ipos1_next.set_head(pos1->next);	
  Ilist aux=Ipos1_next.copy_ilist();
  Iaux.set_head(aux);
  pos2 = Ipos2.ilist_cat(Iaux);
  return pos2;
}



Plist Ivy::paramod2_instances(Topform from, Ilist from_pos, Topform into, Ilist into_pos, int *next_id) {
  UnifyContainer U;

  TermContainer T;
  JustContainer J;
  Context subst_from = U.get_context();
  Context subst_into = U.get_context();
  Trail tr = NULL;
  Plist steps = NULL;  /* build sequence of inferences (backward) */
  PlistContainer P;
  bool demod_like;

  Literals from_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(from->literals, from_pos->i);
  Literals into_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(into->literals, into_pos->i);
  bool left_to_right = from_pos->next->i == 1;
  Term alpha = ARG(from_lit->atom, left_to_right ? 0 : 1);
  Term beta  = ARG(from_lit->atom, left_to_right ? 1 : 0);
  Term into_term = T.term_at_pos(into_lit->atom, into_pos->next);
  if (into_term == NULL)
    fatal::fatal_error("paramod2_instances, term does not exist");

  demod_like =
    LADRV_GLOBAIS_INST.Lit.unit_clause(from->literals) &&
    T.variables_subset(beta, alpha) &&
    U.match(alpha, subst_from, into_term, &tr);

  if (demod_like || U.unify(alpha, subst_from, into_term, subst_into, &tr)) {
    Topform from_instance, into_instance, para, para_renum;

    if (LADRV_GLOBAIS_INST.Lit.ground_clause(from->literals))
      from_instance = from;
    else {
      from_instance = instantiate_inference(from, subst_from);
      from_instance->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(from_instance);
    }

    if (demod_like || LADRV_GLOBAIS_INST.Lit.ground_clause(into->literals))
      into_instance = into;
    else {
      into_instance = instantiate_inference(into, subst_into);
      into_instance->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(into_instance);
    }

    U.undo_subst(tr);

    /* Positions in instances are same as positions in originals. */

    from_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(from_instance->literals, from_pos->i);
    into_lit = LADRV_GLOBAIS_INST.Lit.ith_literal(into_instance->literals, into_pos->i);

    para = Paramodulation::paramodulate(from_lit, left_to_right ? 0 : 1, NULL, into_instance, into_pos, NULL);
    para->justification =    
		J.ivy_just(Just_type::PARA_JUST,from_instance->id, ivy_para_position(from_pos,true,  /* sign of literal */LADRV_GLOBAIS_INST.Lit.number_of_literals(from_instance->literals)),
		         into_instance->id,ivy_para_position(into_pos, into_lit->sign, LADRV_GLOBAIS_INST.Lit.number_of_literals(into_instance->literals)),NULL);
    para->id = (*next_id)++;
    P.set_head(steps);
	steps = P.plist_prepend(para);
    para_renum = renumber_inference(para);
    if (para_renum) {
      para_renum->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(para_renum);
    }
  }
  else
    steps = NULL;
  
  U.free_context(subst_from);
  U.free_context(subst_into);
  return steps;
} 


Topform Ivy::flip_inference(Topform c, int n) {
  TopformContainer TF;

  TermContainer T;
  AttributeContainer A;
  JustContainer J;
  Topform child = TF.copy_clause(c);
  Literals lit = LADRV_GLOBAIS_INST.Lit.ith_literal(child->literals, n);
  Term atom = lit->atom;
  Term t;
  if (!T.eq_term(atom))
    fatal::fatal_error("flip_inference, literal not equality");
  t = ARG(atom, 0);
  ARG(atom, 0) = ARG(atom, 1);
  ARG(atom, 1) = t;

  child->justification =
    J.ivy_just(Just_type::FLIP_JUST,c->id,ivy_lit_position(n,LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals)),0, NULL, NULL);
  child->attributes = A.copy_attributes(c->attributes);
  return child;
} 




Plist Ivy::resolve2_instances(Topform c1, int n1, Topform c2, int n2, int *next_id) {

  UnifyContainer U;
  PlistContainer P;
  TopformContainer TF;
  AttributeContainer A;
  Parautil Pu;
  JustContainer J;
  
  Literals l1 = LADRV_GLOBAIS_INST.Lit.ith_literal(c1->literals, n1);
  Literals l2 = LADRV_GLOBAIS_INST.Lit.ith_literal(c2->literals, abs(n2));
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  Context s1 = U.get_context();
  Context s2 = U.get_context();
  Trail tr = NULL;
  Term a2x;
  Plist steps = NULL;

  if (n2 < 0)
    a2x = Pu.top_flip(a2);  /* temporary flipped equality */
  else
    a2x = a2;

  if (l1->sign != l2->sign && U.unify(a1, s1, a2x, s2, &tr)) {
    Literals l1i, l2i, lit;
    Topform res, res_renum, c1i, c2i;

    /* instantiate parents if not ground */

    if (LADRV_GLOBAIS_INST.Lit.ground_clause(c1->literals))
      c1i = c1;
    else {
      c1i = instantiate_inference(c1, s1);
      c1i->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(c1i);
    }

    if (LADRV_GLOBAIS_INST.Lit.ground_clause(c2->literals))     c2i = c2;
    else {
      c2i = instantiate_inference(c2, s2);
      c2i->id = (*next_id)++;
	  P.set_head(steps);
      steps = P.plist_prepend(c2i);
    }

    if (n2 < 0) {
      c2i = flip_inference(c2i, abs(n2));
      c2i->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(c2i);
    }

    U.undo_subst(tr);
    
    l1i = LADRV_GLOBAIS_INST.Lit.ith_literal(c1i->literals, n1);
    l2i = LADRV_GLOBAIS_INST.Lit.ith_literal(c2i->literals, abs(n2));
    
    /* construct the resolvent */

    res = TF.get_topform();
    for (lit = c1i->literals; lit; lit = lit->next)
		if (lit != l1i) res->literals = LADRV_GLOBAIS_INST.Lit.append_literal(res->literals, LADRV_GLOBAIS_INST.Lit.copy_literal(lit));
	for (lit = c2i->literals; lit; lit = lit->next)
      if (lit != l2i) res->literals = LADRV_GLOBAIS_INST.Lit.append_literal(res->literals, LADRV_GLOBAIS_INST.Lit.copy_literal(lit));
    TF.inherit_attributes(c1i, NULL, c2i, NULL, res);
    TF.upward_clause_links(res);

    res->justification =
      J.ivy_just(Just_type::BINARY_RES_JUST,c1i->id,ivy_lit_position(n1,LADRV_GLOBAIS_INST.Lit.number_of_literals(c1i->literals)),
				c2i->id,ivy_lit_position(abs(n2),LADRV_GLOBAIS_INST.Lit.number_of_literals(c2i->literals)), NULL);
    res->id = (*next_id)++;
    P.set_head(steps);
	steps = P.plist_prepend(res);
    res_renum = renumber_inference(res);
    if (res_renum) {
      res_renum->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(res_renum);
    }
  }
  else
    steps = NULL;

  if (n2 < 0)
    Pu.zap_top_flip(a2x);

  U.free_context(s1);
  U.free_context(s2);

  return steps;
}  



Plist Ivy::factor2_instances(Topform c, int n1, int n2, int *next_id) {

  UnifyContainer U;
  PlistContainer P;
  TopformContainer TF;
  AttributeContainer AT;
  JustContainer J;
  
  Literals l1 = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n1);
  Literals l2 = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n2);
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  Context subst = U.get_context();
  Trail tr = NULL;
  Plist steps = NULL;

  if (l1->sign == l2->sign && U.unify(a1, subst, a2, subst, &tr)) {

    Literals l1i, l2i, lit;
    Topform factor, factor_renum, c_instance;

    c_instance = instantiate_inference(c, subst);
    c_instance->id = (*next_id)++;
    P.set_head(steps);
	steps = P.plist_prepend(c_instance);
    U.undo_subst(tr);

    l1i = LADRV_GLOBAIS_INST.Lit.ith_literal(c_instance->literals, n1);
    l2i = LADRV_GLOBAIS_INST.Lit.ith_literal(c_instance->literals, n2);

    /* construct the factor */

    factor = TF.get_topform();
    for (lit = c_instance->literals; lit; lit = lit->next)
      if (lit != l2i)
		factor->literals = LADRV_GLOBAIS_INST.Lit.append_literal(factor->literals, LADRV_GLOBAIS_INST.Lit.copy_literal(lit));

    TF.inherit_attributes(c_instance, NULL, NULL, NULL, factor);
    TF.upward_clause_links(factor);

    factor->justification = J.ivy_just(Just_type::PROPOSITIONAL_JUST,c_instance->id, NULL,0, NULL, NULL);
    factor->id = (*next_id)++;
    P.set_head(steps);
	steps = P.plist_prepend(factor);
    factor_renum = renumber_inference(factor);
    if (factor_renum) {
      factor_renum->id = (*next_id)++;
      P.set_head(steps);
	  steps = P.plist_prepend(factor_renum);
    }
  }
  else
    steps = NULL;

  U.free_context(subst);

  return steps;
} 



Plist Ivy::copy_proof_and_rename_symbols_for_ivy(Plist proof) {
  Plist novo = NULL;
  PlistContainer P;
  TopformContainer TF;
  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c = ClauseMisc::copy_clause_ija((Topform) p->v);
    ivy_clause_trans(c);
    novo = P.plist_prepend(c);
  }
  return P.reverse_plist();
} 



Plist Ivy::expand_proof_ivy(Plist proof) {
  PlistContainer P1,P2,P3;
  Parautil Pu;
  TopformContainer TF;
  
  AttributeContainer AT;
  IlistContainer I;
  JustContainer J;
  
  Plist work_proof = copy_proof_and_rename_symbols_for_ivy(proof);
  Plist new_proof = NULL; /* build it backward, reverse at end */
  int next_id;
  Plist p;
  bool need_reflexivity_of_eq = false;
  Ilist to_be_removed = NULL;  /* clauses to be removed from proof */
  Plist final_proof = NULL;

  /* Build clause x=x, id=0, justification input; delete later if not used. */
  Topform xx = Pu.build_reflex_eq();
  xx->id = 0;
  xx->justification = J.ivy_just(Just_type::INPUT_JUST, 0, NULL, 0, NULL, NULL);
  P1.set_head(new_proof);
  new_proof = P1.plist_prepend(xx);

  /* Start numbering the new proof where the old one ends. */

  next_id = Xproofs::greatest_id_in_proof(work_proof) + 1;

  for (p = work_proof; p; p = p->next) {
    Topform c = (Topform) p->v;
    Just j = c->justification;
    Topform new_c = NULL;

    if (j->type == Just_type::BINARY_RES_JUST || j->type == Just_type::XXRES_JUST) {
      Ilist p = j->u.lst;

      int id1  = p->i;
      int lit1 = p->next->i;

      int id2  = j->type == Just_type::XXRES_JUST ? 0 : p->next->next->i;
      int lit2 = j->type == Just_type::XXRES_JUST ? 1 : p->next->next->next->i;

      Topform c1 = Xproofs::proof_id_to_clause(new_proof, id1);
      Topform c2 = Xproofs::proof_id_to_clause(new_proof, id2);

      Plist new_steps = resolve2_instances(c1, lit1, c2, lit2, &next_id);
      new_c = (Topform) new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;
	  P2.set_head(new_steps);	
      new_proof = P2.plist_cat(new_proof);

      if (j->type == Just_type::XXRES_JUST)	need_reflexivity_of_eq = true;
    }
    else if (j->type == Just_type::PARA_JUST) {
      Parajust p = j->u.para;

      Topform from = Xproofs::proof_id_to_clause(new_proof, p->from_id);
      Topform into = Xproofs::proof_id_to_clause(new_proof, p->into_id);

      Plist new_steps = paramod2_instances(from, p->from_pos, into, p->into_pos, &next_id);

      new_c = (Topform) new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;
	  P2.set_head(new_steps);	
      new_proof = P2.plist_cat(new_proof);
    }
    else if (j->type == Just_type::FACTOR_JUST) {
      Ilist p = j->u.lst;

      int id =  p->i;
      int lit1 = p->next->i;
      int lit2 = p->next->next->i;

      Topform parent = Xproofs::proof_id_to_clause(new_proof, id);

      Plist new_steps = factor2_instances(parent, lit1, lit2, &next_id);
      new_c = (Topform) new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;
      P2.set_head(new_steps);		
      new_proof = P2.plist_cat(new_proof);
    }

    else if (j->type == Just_type::COPY_JUST &&	     j->next &&     j->next->type == Just_type::FLIP_JUST) {
      int flip_lit = j->next->u.id;
      Topform parent =Xproofs:: proof_id_to_clause(new_proof, j->u.id);
      Topform child = flip_inference(parent, flip_lit);
      Topform child_renum;

      child->id = next_id++;
	  P2.set_head(new_proof);
      new_proof = P2.plist_prepend(child);

      child_renum = renumber_inference(child);

      if (child_renum) {
		P1.set_head(new_proof);
		new_proof = P1.plist_prepend(child_renum);
		child_renum->id = next_id++;
		new_c = child_renum;
      }
      else
			new_c = child;
      new_c->id = c->id;
      next_id--;
    }
    else if (j->type == Just_type::COPY_JUST &&    j->next &&    j->next->type == Just_type::XX_JUST) {

      int id1  = j->u.id;
      int lit1 = j->next->u.id;

      int id2  = 0;  /* special clause x = x */
      int lit2 = 1;  /* special clause x = x */

      Topform c1 = Xproofs::proof_id_to_clause(new_proof, id1);
      Topform c2 = Xproofs::proof_id_to_clause(new_proof, id2);

      Plist new_steps = resolve2_instances(c1, lit1, c2, lit2, &next_id);
      new_c = (Topform) new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;
	  P2.set_head(new_steps);
      new_proof = P2.plist_cat(new_proof);
      need_reflexivity_of_eq = true;
    }
    else if (j->type == Just_type::COPY_JUST && (j->next == NULL || j->next->type == Just_type::MERGE_JUST)) {
      int parent_id = j->u.id;
      new_c = TF.copy_clause(c);
      new_c->justification = J.ivy_just(Just_type::PROPOSITIONAL_JUST,parent_id, NULL,0, NULL, NULL);
      new_c->id = c->id;
      P1.set_head(new_proof);
	  new_proof = P1.plist_prepend(new_c);
    }
    else if (j->type == Just_type::INPUT_JUST ||
	     j->type == Just_type::CLAUSIFY_JUST ||
	     j->type == Just_type::EXPAND_DEF_JUST ||
	     j->type == Just_type::GOAL_JUST ||
	     j->type == Just_type::DENY_JUST) {
      new_c = TF.copy_clause(c);
      new_c->id = c->id;
      new_c->attributes = AT.copy_attributes(c->attributes);
      new_c->justification = J.ivy_just(Just_type::INPUT_JUST, 0, NULL, 0, NULL, NULL);
      P1.set_head(new_proof);
	  new_proof = P1.plist_prepend(new_c);
      if (j->type == Just_type::CLAUSIFY_JUST || j->type == Just_type::EXPAND_DEF_JUST || j->type == Just_type::DENY_JUST) {
	/* Parents of these must be removed from the
	   proof, because IVY expects ordinary clauses only. */
		Ilist parents = J.get_parents(j, true);
        IlistContainer IP;
        IP.set_head(parents);
        IlistContainer TBR;
        TBR.set_head(to_be_removed);
		to_be_removed = TBR.ilist_cat(IP);
      }
    }
    else if (j->type == Just_type::NEW_SYMBOL_JUST) {
      int parent_id = j->u.id;
      new_c = TF.copy_clause(c);
      new_c->justification = J.ivy_just(Just_type::NEW_SYMBOL_JUST, parent_id, NULL,0, NULL, NULL);
      new_c->id = c->id;
	  P1.set_head(new_proof);
      new_proof = P1.plist_prepend(new_c);
      cerr<<endl<<";; WARNING: IVY proof contains unaccepted NEW_SYMBOL justification."<<endl<<endl;
	  cerr<<endl<<";; WARNING: IVY proof contains unaccepted NEW_SYMBOL justification."<<endl<<endl;
    }
    else {
      new_c = ClauseMisc::copy_clause_ija(c);
	  P1.set_head(new_proof);
      new_proof = P1.plist_prepend(new_c);
    }

    if (!Subsume::subsumes(c, new_c) || !Subsume::subsumes(new_c, c)) {
      cout<<"old clause: "; TF.fprint_clause(cout, c);
	  cout<<"new clause: "; TF.fprint_clause(cout, new_c);
	  fatal::fatal_error("expand_proof_ivy, clauses not equivalent");
    }
  }  /* process proof step c */
  
  ClauseMisc::delete_clauses(work_proof);

  /* The following does 2 things: remove clauses and reverse proof. */

  final_proof = NULL;
  for (p = new_proof; p; p = p->next) {
    Topform c = (Topform) p->v;
    I.set_head(to_be_removed);
	if (!I.ilist_member(c->id)) { 
	  P3.set_head(final_proof);
	  final_proof = P3.plist_prepend(c);
	}
	
  }

  P1.set_head(new_proof);
  P1.zap_plist();  /* shallow */
  I.set_head(to_be_removed);
  I.zap_ilist();

  if (!need_reflexivity_of_eq) {
    /* We didn't use x=x, so delete it from the proof (it is first). */
    Plist p = final_proof;
    
    PlistContainer P;
    final_proof = final_proof->next;
    TF.zap_topform((Topform) p->v);
	P.zap_plist(p);
  }

  Xproofs::check_parents_and_uplinks_in_proof(final_proof);
  return final_proof;
} 
