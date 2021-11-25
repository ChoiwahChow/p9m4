#include "clash.h"
#include "memory.h"
#include "attrib.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>







GlobalClash::GlobalClash() {
    Clash_gets=0;
    Clash_frees=0;
}

GlobalClash::~GlobalClash() {

    
}



ClashContainer::ClashContainer() {
	head=NULL;
}

ClashContainer::~ClashContainer() {
	head=NULL;
}

Clash ClashContainer::get_clash(void) {
  Clash p = (Clash) Memory:: memCNew(sizeof(struct clash));
  LADR_GLOBAL_CLASH.Clash_gets++;
  return(p);
} 

void ClashContainer::free_clash(Clash p) {
  Memory::memFree((void *) p, sizeof(struct clash));
  LADR_GLOBAL_CLASH.Clash_frees++;
}


void ClashContainer::fprint_clash_mem(ostream &o, bool heading) {
  int n;
  if (heading)
   o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct clash);
  o<<"clash       ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_CLASH.Clash_gets;
  o<<setw(11)<<LADR_GLOBAL_CLASH.Clash_frees;
  o<<setw(11)<<LADR_GLOBAL_CLASH.Clash_gets-LADR_GLOBAL_CLASH.Clash_frees;
  o<<setw(9)<< ( (LADR_GLOBAL_CLASH.Clash_gets-LADR_GLOBAL_CLASH.Clash_frees) *n ) /1024<<"K"<<endl;

}

void ClashContainer::p_clash_mem() {
  fprint_clash_mem(cout, true);
} 


Clash ClashContainer::append_clash(Clash p) {
  Clash q = get_clash();
  if (p != NULL)
    p->next = q;
  return q;
} 


void  ClashContainer::zap_clash(Clash p) {
  UnifyContainer U; //make ClashContainer friend of UnifyContainer class
  while (p != NULL) {
    Clash q = p;
    p = p->next;
    if (q->clashable && !q->clashed)
      U.free_context(q->sat_subst);
    free_clash(q);
  }
} 


Literals ClashContainer::atom_to_literal(Term atom) {
  Topform c = (Topform) atom->container;
  Literals lit = (c == NULL ? NULL : c->literals);
  while (lit != NULL && lit->atom != atom)
    lit = lit->next;
  return lit;
} 

Literals ClashContainer::apply_lit(Literals lit, Context c) {
  
  UnifyContainer U;
  return LADRV_GLOBAIS_INST.Lit.new_literal(lit->sign, U.apply(lit->atom, c));
}


int ClashContainer::alit_position(Topform c, Literals lit) {
  int i = 1;
  Literals l = c->literals;
  while (l != NULL && l != lit) {
    i++;
    l = l->next;
  }
  if (l == lit)
    return i;
  else
    return -1;
}

int ClashContainer::lit_position(Topform c, Literals lit) {
  int i = 1;
  Literals l = c->literals;
  while (l != NULL && l != lit) {
    i++;
    l = l->next;
  }
  if (l == lit)
    return i;
  else
    return -1;
}


Topform ClashContainer::resolve(Clash first, Just_type rule) {
  TopformContainer TF; //make ClashContainer friend of class Topform
  IlistContainer I;
 
  JustContainer J;
  AttributeContainer A;
  Topform r = TF.get_topform();
  Topform nuc = (Topform) first->nuc_lit->atom->container;
  Ilist j = I.ilist_append(nuc->id);
  Clash p;
  int n;

  /* First, include literals in the nucleus. */
  for (p = first; p != NULL; p = p->next, n++) {
    if (!p->clashed)
      r->literals = LADRV_GLOBAIS_INST.Lit.append_literal(r->literals,
				   apply_lit(p->nuc_lit, p->nuc_subst));
  }

  r->attributes = A.cat_att(r->attributes,
			  A.inheritable_att_instances(nuc->attributes,
						    first->nuc_subst));

  /* Next, include literals in the satellites. */

  n = 1;  /* n-th nucleus literal, starting with 1 */
  
  for (p = first; p != NULL; p = p->next, n++) {
    if (p->clashed) {
      if (p->sat_lit == NULL) {
	/* special code for resolution with x=x */
		j = I.ilist_append(n);
		j = I.ilist_append(0);
		j = I.ilist_append(0);
      }
      else {
			Literals lit;
			Topform sat =(Topform) p->sat_lit->atom->container;
			int sat_pos = lit_position(sat, p->sat_lit);
			j = I.ilist_append(n);
			j = I.ilist_append(sat->id);
			j = I.ilist_append(p->flipped ? -sat_pos : sat_pos);
			for (lit = sat->literals; lit != NULL; lit = lit->next) {
			if (lit != p->sat_lit)
				r->literals = LADRV_GLOBAIS_INST.Lit.append_literal(r->literals,  apply_lit(lit,  p->sat_subst));
	}
	r->attributes = A.cat_att(r->attributes,
				A.inheritable_att_instances(sat->attributes,
							  p->sat_subst));
      }
    }
  }
  r->justification = J.resolve_just(j, rule);
  TF.upward_clause_links(r);
  return r;
} 

void ClashContainer::clash_recurse(Clash first,Clash p, bool (*sat_test) (Literals),Just_type rule,void (*proc_proc) (Topform)){
  if (p == NULL) {
    /* All clashable literals have been mated, so construct the resolvent. */
    TopformContainer TF;
	Topform resolvent = resolve(first, rule);
    (*proc_proc)(resolvent);
  }
  else if (!p->clashable | p->clashed)
    clash_recurse(first, p->next, sat_test, rule, proc_proc);
  else {
    MindexContainer M;
	
	Term fnd_atom;
    fnd_atom = M.mindex_retrieve_first(p->nuc_lit->atom, p->mate_index, Querytype::UNIFY, p->nuc_subst, p->sat_subst, false, &(p->mate_pos));
    while (fnd_atom != NULL) {
      Literals slit = atom_to_literal(fnd_atom);
      if ((*sat_test)(slit)) {
		p->clashed = true;
		p->flipped = false;
		p->sat_lit = slit;
		clash_recurse(first, p->next, sat_test, rule, proc_proc);
		p->clashed = false;
      }
      fnd_atom = M.mindex_retrieve_next(p->mate_pos);
    }
    /* If the literal is an equality, try flipping it. */
    TermContainer T;
    Parautil Pu;

    if (T.eq_term(p->nuc_lit->atom)) {
      Term flip = Pu.top_flip(p->nuc_lit->atom);
      fnd_atom = M.mindex_retrieve_first(flip, p->mate_index, Querytype::UNIFY,p->nuc_subst, p->sat_subst,false, &(p->mate_pos));
      while (fnd_atom != NULL) {
		Literals slit = atom_to_literal(fnd_atom);
		if ((*sat_test)(slit)) {
			p->clashed = true;
			p->flipped = true;
			p->sat_lit = slit;
			clash_recurse(first, p->next, sat_test, rule, proc_proc);
			p->clashed = false;
		}
		fnd_atom = M.mindex_retrieve_next(p->mate_pos);
      }
      
	  Pu.zap_top_flip(flip);
    }
    /* Built-in resolution with x=x. */
    if (LADRV_GLOBAIS_INST.Lit.neg_eq(p->nuc_lit)) {
      UnifyContainer U;
	  Term alpha = ARG(p->nuc_lit->atom,0);
      Term beta  = ARG(p->nuc_lit->atom,1);
      Trail tr = NULL;
      if (U.unify(alpha, p->nuc_subst, beta, p->nuc_subst, &tr)) {
		p->clashed = true;
		p->sat_lit = NULL;
		clash_recurse(first, p->next, sat_test, rule, proc_proc);
		p->clashed = false;
		U.undo_subst(tr);
      }
    }
  }
}

void ClashContainer::clash(Clash c,  bool (*sat_test) (Literals),  Just_type rule,  void (*proc_proc) (Topform)) {
  clash_recurse(c, c, sat_test, rule, proc_proc);
} 
