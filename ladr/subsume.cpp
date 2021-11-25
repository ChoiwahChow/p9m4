#include "clauseid.h"
#include "subsume.h"
#include "clist.h"


int Subsume::Nonunit_subsumption_tests=0;
//LiteralsContainer Subsume::L;


int Subsume::nonunit_subsumption_tests(void) {
  return Nonunit_subsumption_tests;
}




bool Subsume::subsume_literals_anyctx(Literals clit, Context subst, Topform d, Trail *trp, int *anyctx, Ilist *anytrp) {
  bool subsumed = false;
  Literals dlit;
  UnifyContainer U;
  
  if (clit == NULL)  return true;
  else {
    for (dlit = d->literals; !subsumed && dlit != NULL; dlit = dlit->next) {
      if (clit->sign == dlit->sign) {
        Trail mark = *trp;
        
        /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
        Ilist cpos = *anytrp;

        if (U.match_anyctx(clit->atom, subst, dlit->atom, trp, anyctx, anytrp)) {
          if (subsume_literals_anyctx(clit->next, subst, d, trp, anyctx, anytrp))    subsumed = true;
          else {  
            U.undo_subst_2(*trp, mark);
            *trp = mark;

            /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints 
             *
             * Restore anyctx to the state before the failed call.
             *
             * */
            IlistContainer I;
            I.set_head(*anytrp);
            while (*anytrp != cpos) {
              anyctx[(*anytrp)->i] = -1;
              *anytrp = I.ilist_pop();
            }

          }
        }
      }
    }
    return subsumed;
  }
} 


bool Subsume::subsume_literals(Literals clit, Context subst, Topform d, Trail *trp) {
  bool ret;
  IlistContainer I;
  Ilist anytrp = NULL;
  if ((MATCH_HINTS_ANYCONST == true) && (LADR_GLOBAL_TERM.AnyConstsEnabled == true)) {
    int anyctx[MAX_ANYCONSTS];
    int i;
    for (i=0; i<MAX_ANYCONSTS; i++) {
      anyctx[i] = -1;
    }
    ret = subsume_literals_anyctx(clit, subst, d, trp, anyctx, &anytrp);
    IlistContainer I(anytrp);
    I.zap_ilist();
  }
  else {
    ret = subsume_literals_anyctx(clit, subst, d, trp, NULL, &anytrp);
  }
  return ret;
    
}



/*
bool Subsume::subsume_literals(Literals clit, Context subst, Topform d, Trail *trp) {
  bool subsumed = false;
  Literals dlit;
  UnifyContainer U;	
  if (clit == NULL)    return true;
  else {
    for (dlit = d->literals; !subsumed && dlit != NULL; dlit = dlit->next) {
      if (clit->sign == dlit->sign) {
	Trail mark = *trp;
	if (U.match(clit->atom, subst, dlit->atom, trp)) {
	  if (subsume_literals(clit->next, subst, d, trp))  subsumed = true;
	  else {
	         U.undo_subst_2(*trp, mark);
	         *trp = mark;
	   }
	  }
    }
   }
   return subsumed;
  }
}

*/

bool Subsume::subsume_bt_literals(Literals clit, Context subst, Topform d, Plist *gp) {
  if (clit == NULL)   return true;
  else {
    bool subsumed = false;
	Literals dlit;
	PlistContainer P;
	BtmContainer B;
	P.set_head(*gp);
    *gp=P.plist_prepend(NULL);
    for (dlit=d->literals; !subsumed && dlit!=NULL; dlit=dlit->next) {
      if (clit->sign == dlit->sign) {
	Btm_state bt = B.match_bt_first(clit->atom, subst, dlit->atom, false);
	while (bt != NULL && !subsumed) {
	  (*gp)->v = bt;
	  if (subsume_bt_literals(clit->next, subst, d, gp)) subsumed = true;
	  else {
	    bt = B.match_bt_next(bt);
	  }
	}
      }
    }
    if (subsumed)    return true;
    else {
      *gp = P.plist_pop();
      return false;
    }
  }
}

bool Subsume::subsumes(Topform c, Topform d) {
  UnifyContainer U;
  Context subst = U.get_context();
  Trail tr = NULL;
  bool subsumed = subsume_literals(c->literals, subst, d, &tr);
  if (subsumed) U.undo_subst(tr);
  U.free_context(subst);
  Nonunit_subsumption_tests++;
  return subsumed;
} 

bool Subsume::subsumes_bt(Topform c, Topform d) {
  UnifyContainer U;
  BtmContainer B;
  PlistContainer P;
  Context subst = U.get_context();
  Plist g = NULL;
  int rc = subsume_bt_literals(c->literals, subst, d, &g);
  if (rc) {
    /* Cancel the list (stack) of btm_states */
    while (g != NULL) {
      Btm_state bt = (Btm_state) g->v;
      B.match_bt_cancel(bt);
      P.set_head(g);
	  g = P.plist_pop();
    }
  }
  U.free_context(subst);
  return rc;
}  

Topform Subsume::forward_subsume(Topform d, Lindex idx) {
  Literals dlit;
  Topform subsumer = NULL;
  UnifyContainer U;
  ClistContainer C;
  MindexContainer M;
  LindexContainer LD;
 
  Context subst = U.get_context();
  int nd = LADRV_GLOBAIS_INST.Lit.number_of_literals(d->literals);

  /* We have to consider all literals of d, because when d is
     subsumed by c, not all literals of d have to match with
     a literal in c.  c is indexed on the first literal only.
   */

  for (dlit=d->literals; dlit!=NULL && subsumer==NULL; dlit=dlit->next) {
    Mindex mdx = dlit->sign ? idx->pos : idx->neg;
    Mindex_pos pos;
    Term catom = M.mindex_retrieve_first(dlit->atom, mdx, Querytype::GENERALIZATION, NULL, subst, false, &pos);
    bool backtrack = LD.lindex_backtrack(idx);
    while (catom != NULL && subsumer == NULL) {
      Topform c = (Topform) catom->container;
      if (LADRV_GLOBAIS_INST.Lit.atom_number(c->literals, catom) == 1) {
        int nc = LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals);
        /* If c is a unit then we already know it subsumes d; otherwise,
        * do a full subsumption check on the clauses.  (We don't let
        * a clause subsume a shorter one, because that would cause
        * factors to be deleted.)
        */
        if (nc == 1 || (nc <= nd && (backtrack
				     ? subsumes_bt(c,d)
				     : subsumes(c,d)))) {
        subsumer = c;
        M.mindex_retrieve_cancel(pos);
        }
      }
      if (subsumer == NULL)
        catom = M.mindex_retrieve_next(pos);
    }
  }
  U.free_context(subst);
  return subsumer;
} 


Plist Subsume::back_subsume(Topform c, Lindex idx) {

   int nc = LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals);

  if (nc == 0) return NULL;
  else {
    Plist subsumees = NULL;
	UnifyContainer U;
	
    Context subst = U.get_context();
    Literals clit = c->literals;

    /* We only have to consider the first literal of c, because when
       c subsumes a clause d, all literals of c have to map into d.
       All literals of d are indexed.
     */

    Mindex mdx = clit->sign ? idx->pos : idx->neg;
    Mindex_pos pos;
	MindexContainer M;
    LindexContainer LD;
    ClauseidContainer CI;
    Term datom = M.mindex_retrieve_first(clit->atom, mdx, Querytype::INSTANCE,subst, NULL, false, &pos);
    bool backtrack = LD.lindex_backtrack(idx);
    while (datom != NULL) {
      Topform d = (Topform) datom->container;
      if (d != c) {  /* in case c is already in idx */
	int nd = LADRV_GLOBAIS_INST.Lit.number_of_literals(d->literals);
	/* If c is a unit the we already know it subsumes d; otherwise,
	 * do a full subsumption check on the clauses.  (We don't let
	 * a clause subsume a shorter one.)
	 */
	if (nc == 1 || (nc <= nd && (backtrack
				     ? subsumes_bt(c, d)
				     : subsumes(c, d))))
	  subsumees = CI.insert_clause_into_plist(subsumees, d, false);
      }
      datom = M.mindex_retrieve_next(pos);
    }
    U.free_context(subst);
    return subsumees;
  }
} 


Topform Subsume::back_subsume_one(Topform c, Lindex idx) {

  
  int nc = LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals);

  if (nc == 0)
    return NULL;
  else {
	UnifyContainer U;  
	MindexContainer M;
    LindexContainer LI;
    Context subst = U.get_context();
    Literals clit = c->literals;

    Mindex mdx = clit->sign ? idx->pos : idx->neg;
    Mindex_pos pos;

    Term datom = M.mindex_retrieve_first(clit->atom, mdx, Querytype::INSTANCE,  subst, NULL, false, &pos);
    bool backtrack = LI.lindex_backtrack(idx);
    bool found = false;
    Topform d = NULL;

    while (datom != NULL && !found) {
      d = (Topform) datom->container;
      if (d != c) {  /* in case c is already in idx */
	int nd = LADRV_GLOBAIS_INST.Lit.number_of_literals(d->literals);
	/* If c is a unit the we already know it subsumes d; otherwise,
	 * do a full subsumption check on the clauses.  (We don't let
	 * a clause subsume a shorter one.)
	 */
	if (nc == 1 || (nc <= nd && (backtrack
				     ? subsumes_bt(c, d)
				     : subsumes(c, d)))) {
	  found = true;
	  M.mindex_retrieve_cancel(pos);
	}
	else
	  datom = M.mindex_retrieve_next(pos);
      }
    }
    U.free_context(subst);
    return found ? d : NULL;
  }
} 


void Subsume::atom_conflict(bool flipped, Topform c, bool sign, Term a, Lindex idx, void (*empty_proc) (Topform)) {
  UnifyContainer U;
  MindexContainer M;

  TopformContainer TF;
  ClauseidContainer CI;
  AttributeContainer AT;
  JustContainer J;
  
  int n = 0;
  Context subst1 = U.get_context();
  Context subst2 = U.get_context();
  Mindex mdx = sign ? idx->neg : idx->pos;
  Mindex_pos pos;
  Term b = M.mindex_retrieve_first(a, mdx, Querytype::UNIFY, subst1, subst2, false, &pos);
  while (b) {
    Topform d = (Topform) b->container;
    if (LADRV_GLOBAIS_INST.Lit.number_of_literals(d->literals) == 1) {
      Topform conflictor = d;
      Topform empty = TF.get_topform();

      if (c->id == 0)
			CI.assign_clause_id(c);  /* so that justification makes sense */
      c->used = true;         /* so it won't be discarded */

      empty->justification = J.binary_res_just(c, 1, conflictor, flipped ? -1 : 1);
      TF.inherit_attributes(c, subst1, conflictor, subst2, empty);
      n++;
      (*empty_proc)(empty);
      b = M.mindex_retrieve_next(pos);
    }
    else 
      b = M.mindex_retrieve_next(pos);
  }
  U.free_context(subst1);
  U.free_context(subst2);
}

void Subsume::unit_conflict_by_index(Topform c, Lindex idx, void (*empty_proc) (Topform)) {

  if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) == 1) {
    Literals lit = c->literals;
    Term atom = lit->atom;
    atom_conflict(false, c, lit->sign, atom, idx, empty_proc);
    /* maybe try the flip */
    TermContainer T;
	Parautil Pu;
	if (T.eq_term(atom) && !Pu.renamable_flip_eq(atom)) {
      
	  Term flip = Pu.top_flip(atom);
      atom_conflict(true, c, lit->sign, flip, idx, empty_proc);
      Pu.zap_top_flip(flip);
    }
  }
} 


Topform Subsume::try_unit_conflict(Topform a, Topform b) {
  UnifyContainer U;
  TopformContainer TF;
  JustContainer J;
  AttributeContainer AT;
 
  Context c1 = U.get_context();
  Context c2 = U.get_context();
  Trail tr = NULL;
  Topform empty = NULL;
  if (LADRV_GLOBAIS_INST.Lit.unit_clause(a->literals) && LADRV_GLOBAIS_INST.Lit.unit_clause(b->literals) &&
      a->literals->sign != b->literals->sign &&
      U.unify(a->literals->atom, c1, b->literals->atom, c2, &tr)) {
    empty = TF.get_topform();
    empty->justification = J.binary_res_just(a, 1, b, 1);
    TF.inherit_attributes(a, c1, b, c2, empty);
    U.undo_subst(tr);
  }
  U.free_context(c1);
  U.free_context(c2);
  return empty;
}

void Subsume::unit_delete(Topform c, Lindex idx) {
  UnifyContainer U;
  MindexContainer M;
  JustContainer J;
  AttributeContainer AT;
  Parautil Pu;
  TermContainer T;

  
  Context subst = U.get_context();
  Literals l;
  int i;
  bool null_literals = false;

  for (l = c->literals, i = 1; l; l = l->next, i++) {
    Mindex mdx = l->sign ? idx->neg : idx->pos;
    Mindex_pos pos;
    Term datom = M.mindex_retrieve_first(l->atom, mdx, Querytype::GENERALIZATION,  NULL, subst, false, &pos);
    bool ok = false;
    while (datom && !ok) {
      Topform d =(Topform) datom->container;
      if (LADRV_GLOBAIS_INST.Lit.unit_clause(d->literals)) {
			ok = true;
			c->justification = J.append_just(c->justification, J.unit_del_just(d, i));
			c->attributes = AT.cat_att(c->attributes, AT.inheritable_att_instances(d->attributes, subst));
			M.mindex_retrieve_cancel(pos);
			T.zap_term(l->atom);
			l->atom = NULL;  /* remove it below */
			null_literals = true;
		}
    }
    /* If still there and equality, try flipping it. */
    if (l->atom && T.eq_term(l->atom)) {
      Term flip = Pu.top_flip(l->atom);
      Term datom = M.mindex_retrieve_first(flip, mdx, Querytype::GENERALIZATION,NULL, subst, false, &pos);
      bool ok = false;
      while (datom && !ok) {
		Topform d = (Topform) datom->container;
		if (LADRV_GLOBAIS_INST.Lit.unit_clause(d->literals)) {
			ok = true;
			M.mindex_retrieve_cancel(pos);
			c->justification = J.append_just(c->justification,J.unit_del_just(d, -i));
			c->attributes = AT.cat_att(c->attributes, AT.inheritable_att_instances(d->attributes, subst));
			T.zap_term(l->atom);
			l->atom = NULL;  /* remove it below */
			null_literals = true;
	    }
      }
      Pu.zap_top_flip(flip);
    }  /* eq_atom */
  }  /* foreach literal */
  if (null_literals) {
    c->literals = LADRV_GLOBAIS_INST.Lit.remove_null_literals(c->literals);
    c->normal_vars =false;  /* removing literals can make vars non-normal */
  }
  U.free_context(subst);
} 


Plist Subsume::back_unit_del_by_index(Topform unit, Lindex idx) {
  UnifyContainer U;
  MindexContainer M;
  ClauseidContainer CI;
  TermContainer T;
  Parautil Pu;
  Plist nonunits = NULL; 
  Context subst = U.get_context();
  Literals clit = unit->literals;

  Mindex mdx = clit->sign ? idx->neg : idx->pos;
  Mindex_pos pos;

  Term datom = M.mindex_retrieve_first(clit->atom, mdx, Querytype::INSTANCE,subst, NULL, false, &pos);

  while (datom != NULL) {
    Topform d = (Topform) datom->container;
    nonunits = CI.insert_clause_into_plist(nonunits, d, false);
    datom = M.mindex_retrieve_next(pos);
  }

  /* If equality, do the same with the flip. */

  if (T.eq_term(clit->atom)) {
    Term flip = Pu.top_flip(clit->atom);
    Term datom = M.mindex_retrieve_first(flip, mdx, Querytype::INSTANCE,
				       subst, NULL, false, &pos);
    while (datom != NULL) {
      Topform d = (Topform) datom->container;
      nonunits = CI.insert_clause_into_plist(nonunits, d, false);
      datom = M.mindex_retrieve_next(pos);
    }
    Pu.zap_top_flip(flip);
  }

  U.free_context(subst);
  return nonunits;
}



void Subsume::simplify_literals(Topform c) {
  Literals l;
  TermContainer T;
  JustContainer J;

  int i;
  bool null_literals = false;

  for (l = c->literals, i = 1; l; l = l->next, i++) {
    Term a = l->atom;
    bool sign = l->sign;
    if ((!sign && T.eq_term(a) && T.term_ident(ARG(a,0), ARG(a,1))) ||
	(!sign && T.true_term(a)) ||
	(sign && T.false_term(a))) {

      c->justification = J.append_just(c->justification, J.xx_just(i));
      T.zap_term(l->atom);
      l->atom = NULL;
      null_literals = true;
    }
  }
  if (null_literals)
    c->literals = LADRV_GLOBAIS_INST.Lit.remove_null_literals(c->literals);
} 

bool Subsume::eq_removable_literal(Topform c, Literals lit) {
  TermContainer T;
  UnifyContainer U;
  if (lit->sign || !T.eq_term(lit->atom))
    return false;
  else {
    UnifyContainer U;
	AttributeContainer AT;
	Term alpha = ARG(lit->atom, 0);
    Term beta  = ARG(lit->atom, 1);
    Context subst = U.get_context();
    Trail tr = NULL;
    bool ok = U.unify(alpha, subst, beta, subst, &tr);
    if (ok) {
      /* Check if substitution instantiates any other literal. */
      /* Note that other literals may have atom==NULL. */
      Literals l;
      for (l = c->literals; l && ok; l = l->next) {
	if (l != lit && l->atom != NULL)
	  if (U.subst_changes_term(l->atom, subst))  ok = false;
      }
      if (ok)
		AT.instantiate_inheritable_attributes(c->attributes, subst);
      U.undo_subst(tr);
    }
    U.free_context(subst);
    return ok;
  }
} 

void Subsume::simplify_literals2(Topform c) {
  Literals l;
  TermContainer T;
 
  JustContainer J;
  TopformContainer TF;
  SymbolContainer S;
  
  int i;
  bool null_literals = false;
  bool tautological = false;

  if (!c->normal_vars)
    TF.renumber_variables(c, MAX_VARS);

  for (l = c->literals, i = 1; l && !tautological; l = l->next, i++) {
    Term a = l->atom;
    bool sign = l->sign;
    if ((!sign && T.eq_term(a) && T.term_ident(ARG(a,0), ARG(a,1))) ||
	/* (!sign && true_term(a)) || */
	/* (sign && false_term(a)) || */
	eq_removable_literal(c, l)) {
      /* literal is FALSE, so remove it */
      c->justification = J.append_just(c->justification, J.xx_just(i));
      T.zap_term(l->atom);
      l->atom = NULL;
      null_literals = true;
    }
    else if ((!sign && T.true_term(a)) ||
	     (sign && T.false_term(a))) {
			T.zap_term(l->atom);
			l->atom = NULL;
			null_literals = true;
    }
    else if ((sign && T.eq_term(a) && T.term_ident(ARG(a,0), ARG(a,1))) ||
	     (sign && T.true_term(a)) ||
	     (!sign && T.false_term(a)))
      tautological = true; 
    }

  if (null_literals) {
    c->literals = LADRV_GLOBAIS_INST.Lit.remove_null_literals(c->literals);
    c->normal_vars = 0;
    TF.renumber_variables(c, MAX_VARS);
  }

  if (tautological || LADRV_GLOBAIS_INST.Lit.tautology(c->literals)) {
  
    LADRV_GLOBAIS_INST.Lit.zap_literals(c->literals);
    c->literals = LADRV_GLOBAIS_INST.Lit.new_literal(true, T.get_rigid_term(S.true_sym(), 0));
    c->literals->atom->container = c;
    /* justification not necessary because clause will disappear??? */
  }
} 
