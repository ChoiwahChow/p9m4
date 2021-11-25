#include "xproofs.h"
#include "demod.h"
#include "fatal.h"
#include "clash.h"

void Xproofs::check_parents_and_uplinks_in_proof(Plist proof) {
  JustContainer J;
  TopformContainer TF;
  
  IlistContainer Ip,Is;
  Ilist seen = NULL;
  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c =(Topform) p->v;
    Ilist parents = J.get_parents(c->justification, false);
    if (!TF.check_upward_clause_links(c)) {
      cout<<"bad uplinks: ";
	  TF.fprint_clause(cout, c);
	  fatal::fatal_error("check_parents_and_uplinks_in_proof, bad uplinks");
    }
    //ilist_subset(parents, seen) //se parents é subset de seen
	Ip.set_head(parents);
	Is.set_head(seen);
	//Se Ip é subset de Is
	if (!Ip.ilist_subset(Is)) {
      cout<<"seen:     "; Is.p_ilist();
	  cout<<"parents:  "; Ip.p_ilist();
      fatal::fatal_error("check_parents_and_uplinks_in_proof, parents not seen");
    }
    seen = Is.ilist_prepend(c->id);
    Ip.zap_ilist();
    
  }
  Is.zap_ilist();
} 


Topform Xproofs::xx_res2(Topform c, int n) {
  ClashContainer C; TermContainer T; UnifyContainer U;
  TopformContainer TF; AttributeContainer AT; JustContainer J;

  Literals lit = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n);
  if (lit == NULL ||
      lit->sign == true ||
      !T.eq_term(lit->atom))    return NULL;
  else {
    Context subst = U.get_context();
    Trail tr = NULL;
    Topform res;

    if (U.unify(ARG(lit->atom,0), subst, ARG(lit->atom,1), subst, &tr)) {
      Literals l2;
      res = TF.get_topform();
      for (l2 = c->literals; l2; l2 = l2->next)
	   if (l2 != lit)
				res->literals = LADRV_GLOBAIS_INST.Lit.append_literal(res->literals, C.apply_lit(l2,  subst));
      res->attributes = AT.cat_att(res->attributes, AT.inheritable_att_instances(c->attributes, subst));

      res->justification = J.xxres_just(c, n);
      TF.upward_clause_links(res);
      TF.renumber_variables(res, MAX_VARS);
      U.undo_subst(tr);
    }
    else {
      res = NULL;
    }
    U.free_context(subst);
    return res;
  }
}

void Xproofs::xx_simp2(Topform c, int n){ 
 
  TermContainer T;
  JustContainer J;
  ClistContainer C;
  TopformContainer TF;
  Literals lit = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n);
  Term a = lit->atom;

  if ((!lit->sign && T.eq_term(a) && T.term_ident(ARG(a,0), ARG(a,1))) ||
      Subsume::eq_removable_literal(c, lit)) {
    T.zap_term(lit->atom);
    lit->atom = NULL;
    c->literals = LADRV_GLOBAIS_INST.Lit.remove_null_literals(c->literals);
    c->justification = J.append_just(c->justification, J.xx_just(n));
  }
  else {
    cout<<endl<<"ERROR, literal "<<n<<" in clause cannot be removed: ";
    TF.fprint_clause(cout, c);
    fatal::fatal_error("xx_simp2, bad literal");
  }
}  

Topform Xproofs::factor(Topform c, int n1, int n2) {

  UnifyContainer U;
  TopformContainer TF;
  AttributeContainer AT;
  JustContainer J;
  ClashContainer C;
  Topform fac;
  Literals l1 = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n1);
  Literals l2 = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n2);
  Context subst = U.get_context();

  Trail tr = NULL;

  if (l1->sign == l2->sign && U.unify(l1->atom, subst, l2->atom, subst, &tr)) {
    Literals lit;
    fac = TF.get_topform();
    for (lit = c->literals; lit; lit = lit->next)
      if (lit != l2)
		fac->literals = LADRV_GLOBAIS_INST.Lit.append_literal(fac->literals, C.apply_lit(lit,  subst));
    fac->attributes = AT.cat_att(fac->attributes,AT.inheritable_att_instances(c->attributes, subst));

    fac->justification = J.factor_just(c, n1, n2);
    TF.upward_clause_links(fac);
    TF.renumber_variables(fac, MAX_VARS);
    U.undo_subst(tr);
  }
  else
    fac = NULL;
  U.free_context(subst);
  return fac;
} 


void Xproofs::merge1(Topform c, int n) {

  TermContainer T;
  JustContainer J;
  Literals target = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n);
  Literals prev = LADRV_GLOBAIS_INST.Lit.ith_literal(c->literals, n-1);
  Literals lit = c->literals;
  bool go = true;

  while (go) {
    if (lit->sign == target->sign && T.term_ident(lit->atom, target->atom))
      go = false;
    else
      lit = lit->next;
  }
  if (lit == target)
    fatal::fatal_error("merge1, literal does not merge");
  prev->next = target->next;
  LADRV_GLOBAIS_INST.Lit.zap_literal(target);
  c->justification = J.append_just(c->justification, J.merge_just(n));
} 

Topform Xproofs::proof_id_to_clause(Plist proof, int id) {
  Plist p = proof;
  while (p && ((Topform) p->v)->id != id)    p = p->next;
  if (p == NULL)    return NULL;
  else  return (Topform) p->v;
} 

int Xproofs::greatest_id_in_proof(Plist proof) {
  if (proof == NULL)
    return INT_MIN;
  else {
    int x = greatest_id_in_proof(proof->next);
    Topform c = (Topform) proof->v;
    return (c->id > x ? c->id : x);
  }
} 



void Xproofs::renumber_proof(Plist proof, int start) {
  I2listContainer I2;
  JustContainer J;
  I2list map = NULL;         /* map old IDs to new IDs */
  int n = start;            /* for numbering the steps */
  Plist p;
  I2.set_head(map);
  for (p = proof; p; p = p->next) {
    Topform c = (Topform) p->v;
    int old_id = c->id;
    c->id = n++;
    J.map_just(c->justification, map);
    map = I2.alist_insert(old_id, c->id);
  }
  I2.zap_i2list();
  check_parents_and_uplinks_in_proof(proof);
}



/* DOCUMENTATION
Given a proof, return a more detailed proof, entirely new, leaving the
given proof unchanged.  Also returned is an I3list mapping new IDs to
pairs <old_id, n>.  The n compnent identifies the sub-steps arising
from the expansions, e.g.,  66 -> <23,4> means that step 66 in the new
proof corresponds to the 4th substep in expanding step 23 of the old proof.

Clauses in the new proof that match clauses in the old proof retain
the IDs from the old proof, and there is no entry in the map for them.
*/

/* PUBLIC */
Plist Xproofs::expand_proof(Plist proof, I3list *pmap) {
  ClistContainer CL;
  PlistContainer P;
  TopformContainer TF;
  I3listContainer I3;
  Parautil Pu;
  JustContainer J;
 
  TermContainer T;

  Plist new_proof = NULL; /* build it backward, reverse at end */
  I3list map = NULL;     /* map new IDs to <old-id,n> for intermediate steps */
  int old_id, old_id_n;  /* for mapping new steps to old */
  int next_id;
  Plist p;



  /* Start numbering the new proof where the old one ends. */

  next_id = greatest_id_in_proof(proof) + 1;
  I3.set_head(map);  
  for (p = proof; p; p = p->next) {
    Topform c = (Topform) p->v;         /* the clause we're expanding */
    Topform current = NULL;   /* by substeps, this becomes identical to c */
    Just j;

    j = c->justification;
    old_id = c->id;
    old_id_n = 0;  /* this counts substeps of the expansion */

#ifdef DEBUG_EXPAND
    cout<<endl<<"Expanding..."; CL.fprint_clause(cout, c);
#endif

    if (j->next == NULL &&
	j->type != Just_type::HYPER_RES_JUST &&
	j->type != Just_type::UR_RES_JUST) {

      /* No expansion is necessary for this step.
	 We take a shortcut by just copying the clause.
      */

      current = ClauseMisc::copy_clause_ija(c);
	  P.set_head(new_proof);
      new_proof = P.plist_prepend(current);

      /* The next 2 steps get undone below.  They are performed here
	 so that the state is consistent with the cases in which
	 some expansion occurs.
       */
   
      map = I3.alist2_insert(next_id, old_id, old_id_n++);
      current->id = next_id++;
    }
    else {
      /* To adjust literal numbers when literals disappear. */
      int merges = 0;
      int unit_deletes = 0;
      int xx_simplify = 0;
      /* primary inference */

      if (j->type == Just_type::COPY_JUST ||
	  j->type == Just_type::BACK_DEMOD_JUST ||
	  j->type == Just_type::PROPOSITIONAL_JUST ||
	  j->type == Just_type::BACK_UNIT_DEL_JUST) {
	/* Note that we get "current" directly from the new proof.
	   This prevents an unnecessary "copy" inference.
	   This assumes there is some secondary justification.
	*/
	current = proof_id_to_clause(new_proof, j->u.id);
      }
      else if (j->type == Just_type::HYPER_RES_JUST ||
	       j->type == Just_type::UR_RES_JUST ||
	       j->type == Just_type::BINARY_RES_JUST) {
	/* c ncn ncn ncn ... (length is 3m+1) */
	Ilist p = j->u.lst;
	Topform c1 = proof_id_to_clause(proof, p->i);
	int j = 0;  /* literals resolved; subtract from nucleus position */
	p = p->next;
	while (p != NULL) {
	  Topform resolvent;
	  int n1 = p->i - j;  /* literal number in c1 */
	  int sat_id = p->next->i;
	  if (sat_id == 0)
	    resolvent = Resolve::xx_resolve2(c1, n1, true);
	  else {
	    Topform c2 = proof_id_to_clause(proof, sat_id);
	    int n2 = p->next->next->i;
	    resolvent = Resolve::resolve2(c1, n1, c2, n2, true);
	    if (resolvent == NULL) {
	      cout<<"Lit "<<n1<<" "; TF.fprint_clause(cout, c1);
	      cout<<"Lit "<<n2<<" "; TF.fprint_clause(cout, c2);
	      fatal::fatal_error("expand_step, clauses don't resolve");
	    }
	  }

	  map = I3.alist2_insert(next_id, old_id, old_id_n++);
	  resolvent->id = next_id++;
	  new_proof = P.plist_prepend(resolvent);
	  c1 = resolvent;
	  j++;
	  p = p->next->next->next;
	}
	current = c1;
      }
      else if (j->type ==Just_type::PARA_JUST) {
		Topform from = proof_id_to_clause(proof, j->u.para->from_id);
		Topform into = proof_id_to_clause(proof, j->u.para->into_id);
		Ilist from_pos = j->u.para->from_pos;
		Ilist into_pos = j->u.para->into_pos;
		current = Paramodulation::para_pos(from, from_pos, into, into_pos);  /* does just. */
		map = I3.alist2_insert(next_id, old_id, old_id_n++);
		current->id = next_id++;
		new_proof = P.plist_prepend(current);
      }
      else if (j->type == Just_type::FACTOR_JUST) {
		Ilist p = j->u.lst;
		Topform parent = proof_id_to_clause(proof, p->i);
		int lit1 = p->next->i;
		int lit2 = p->next->next->i;

		current = factor(parent, lit1, lit2);
		map = I3.alist2_insert(next_id, old_id, old_id_n++);
		current->id = next_id++;
		new_proof = P.plist_prepend(current);
      }
      else if (j->type == Just_type::XXRES_JUST) {
		Ilist p = j->u.lst;
		Topform parent = proof_id_to_clause(proof, p->i);
		int lit = p->next->i;
		current = xx_res2(parent, lit);
		map = I3.alist2_insert(next_id, old_id, old_id_n++);
		current->id = next_id++;
		new_proof = P.plist_prepend(current);
      }
      else if (j->type == Just_type::NEW_SYMBOL_JUST) {
	Topform parent = proof_id_to_clause(proof, j->u.id);
	/* Assume EQ unit with right side constant. */
	int sn = SYMNUM(ARG(c->literals->atom,1));
	current = Pu.new_constant(parent, sn);
	map = I3.alist2_insert(next_id, old_id, old_id_n++);
	current->id = next_id++;
	new_proof = P.plist_prepend(current);
      }
      else {
		cout<<"expand_step, unknown primary justification"<<endl;
		new_proof = P.plist_prepend(ClauseMisc::copy_clause_ija(c));
      }

#ifdef DEBUG_EXPAND
      printf("primary: "); fprint_clause(stdout, current);
#endif

      /* secondary inferences */

      for (j = j->next; j; j = j->next) {
	if (j->type == Just_type::DEMOD_JUST) {
	  /* list of triples: <ID, position, direction> */
	  I3list p;
	  for (p = j->u.demod; p; p = p->next) {
	    Topform demod = proof_id_to_clause(proof, p->i);
	    int position = p->j;
	    int direction = p->k;
	    Ilist from_pos, into_pos;
	    Topform work = TF.copy_clause(current);
	    map = I3.alist2_insert(next_id, old_id, old_id_n++);
	    work->id = next_id++;
	    Demod::particular_demod(work, demod, position, direction,
			     &from_pos, &into_pos);
	    work->justification = J.para_just(Just_type::PARA_JUST, demod, from_pos,  current, into_pos);
	    current = work;
	    new_proof = P.plist_prepend(current);
#ifdef DEBUG_EXPAND
		cout<<"demod: "; CL.fprint_clause(cout, current);
#endif
	  }
	}
	else if (j->type == Just_type::FLIP_JUST) {
	  Term atom;
	  int n = j->u.id;
	  Topform work = Resolve::copy_inference(current);
	  current = work;
	  map = I3.alist2_insert(next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  atom = LADRV_GLOBAIS_INST.Lit.ith_literal(current->literals, n)->atom;
	  if (!T.eq_term(atom))
	    fatal::fatal_error("expand_step, cannot flip nonequality");
	  Pu.flip_eq(atom, n);  /* updates justification */
	  new_proof = P.plist_prepend(current);
#ifdef DEBUG_EXPAND
	  cout<<"flip: "; CL.fprint_clause(cout, current);
#endif
	}
	else if (j->type == Just_type::MERGE_JUST) {
	  int n = j->u.id - merges;
	  Topform work = Resolve::copy_inference(current);
	  current = work;
	  map = I3.alist2_insert(next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  merge1(current, n);  /* updates justification */
	  new_proof = P.plist_prepend(current);
#ifdef DEBUG_EXPAND
	  cout<<"merge: ";CL.fprint_clause(cout, current);
	  
#endif
	  merges++;
	}
	else if (j->type == Just_type::UNIT_DEL_JUST) {
	  Ilist p = j->u.lst;
	  int n = p->i - unit_deletes;
	  Topform unit = proof_id_to_clause(proof, p->next->i);
	  Topform work = Resolve::resolve2(unit, 1,current, n, true);
	  if (work == NULL) {
	    cout<<"Lit "<<n<<" "; TF.fprint_clause(cout, current);
		cout<<"Lit 1 "; TF.fprint_clause(cout, unit);
	    fatal::fatal_error("expand_step, clauses don't unit_del");
	  }
	  current = work;
	  map = I3.alist2_insert(next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  new_proof = P.plist_prepend(current);
	  unit_deletes++;
#ifdef DEBUG_EXPAND
	  cout<<"unit_del: ";CL.fprint_clause(cout, current);
#endif
	}
	else if (j->type == Just_type::XX_JUST) {
	  int n = j->u.id - xx_simplify;
	  Topform work = Resolve::copy_inference(current);
	  current = work;
	  map = I3.alist2_insert(next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  xx_simp2(current,n);
	  new_proof = P.plist_prepend(current);
	  xx_simplify++;
#ifdef DEBUG_EXPAND
	  cout<<"xx_simplify: ";CL.fprint_clause(cout,current);
#endif
	}
	 else {
	  cout<<"expand_step, unknown secondary justification"<<endl;
	  new_proof = P.plist_prepend(current);
     }
    }

      TF.renumber_variables(current, MAX_VARS);

#ifdef DEBUG_EXPAND
      cout<<"secondary: ");CL.fprint_clause(cout, current);
#endif

      }

    /* Okay.  Now current should be identical to c. */
    
    if (current == c)
      fatal::fatal_error("expand_proof, current == c");
    else if (!LADRV_GLOBAIS_INST.Lit.clause_ident(current->literals, c->literals)) {
      TF.fprint_clause(cout, c);
      TF.fprint_clause(cout, current);
      fatal::fatal_error("expand step, result is not identical");
    }
    else {
      /* Now we undo the numbering of the last substep (including
	 the cases in which no expansion is done).  This is so that
	 the clauses in the expanded proof that match the clauses
	 in the original proof have the same IDs.  That is, only
	 the clauses introduced by expansion (e.g., intermedediate
	 demodulants) get new IDs.
       */
      current->id = c->id;
      next_id--;
      map = I3.alist2_remove(next_id);
#ifdef DEBUG_EXPAND
      cout<<"end: "; CL.fprint_clause(cout,current);
#endif
    }  /* expand */
  }  /* process proof step c */

  *pmap = map;  /* make available to caller */

  new_proof = P.reverse_plist();
  check_parents_and_uplinks_in_proof(new_proof);
  return new_proof;
}


Plist Xproofs::copy_and_renumber_proof(Plist proof, int start) {
  Plist workproof = ClauseMisc::copy_clauses_ija(proof);
  renumber_proof(workproof, start);
  return workproof;
} 


Plist Xproofs::proof_to_xproof(Plist proof){
  I3list map;
  Plist xproof = expand_proof(proof, &map);
  return xproof;
}
