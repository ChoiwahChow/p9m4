
#include <iomanip>
#include "../ladr/glist.h"
#include "../ladr/memory.h"
#include "../ladr/parse.h"

#include "ground.h"

Ground::Ground(int ds, const Term* t, const Symbol_data* s, const Cell c, struct mace_stats* m, const arithmetic* a, const Mace4GlobalValues* gv) :
  Domain_size(ds), Domain(t), Cells(c), Sn_to_mace_sn(s), Mstats(m), Mclause_gets(0), Mclause_frees(0), Mclause_use(0), Mclause_high(0),
  Arith(a), Mace4_gv(gv)
{
}

Mclause
Ground::get_mclause(int numlits) {
  /* This is a little tricky.  The pointers to the literals are
     in an array (p->lits) that is just after (contiguous with) the mclause.
  */
  Mclause p = static_cast<Mclause>(Memory::memNew((PTRS_MCLAUSE + numlits)*BYTES_POINTER));
  p->numlits = numlits;
  if (numlits == 0)
    p->lits = nullptr;  /* not sure if this can happen */
  else {
    void **v = (void **)p;
    p->lits = (Term*)(v + PTRS_MCLAUSE);  /* just after the struct */
  }
  /* initialization */
  p->subsumed = false;
  p->u.active = -1;
  p->next = nullptr;

  Mclause_gets++;
  Mclause_use += (PTRS_MCLAUSE + numlits) * BYTES_POINTER;
  Mclause_high = IMAX(Mclause_use, Mclause_high);

  return (p);
}

void
Ground::free_mclause(Mclause p)
{
  Mclause_frees++;
  Mclause_use -= (PTRS_MCLAUSE + p->numlits) * BYTES_POINTER;
  Memory::memFree(p, (PTRS_MCLAUSE + p->numlits)*BYTES_POINTER);
}

void
Ground::fprint_mclause_mem(std::ostream& fp, bool heading)
{
  if (heading)
    fp << "  type (bytes each)        gets      frees     in use      bytes\n";

  int n = sizeof(struct mclause);
  fp << "mclause (" << std::setw(4) << std::setprecision(4) << n << ")      " << std::setw(11) << Mclause_gets << std::setw(11)
     << Mclause_frees << std::setw(11) << Mclause_gets - Mclause_frees << std::setw(9) << std::setprecision(1) << Mclause_use / 1024.0
     << " K (" << std::setprecision(1) << Mclause_high / 1024.0 << " K high)\n";
}

void
Ground::zap_mterm(Term t)
{
  // Do not free variable terms (that is, domain elements).
  if (!VARIABLE(t)) {
    TermContainer term_con;
    for (int i = 0; i < ARITY(t); i++)
      zap_mterm(ARG(t,i));
    term_con.free_term(t);
  }
}

void
Ground::zap_mclause(Mclause c)
{
  for (int i = 0; i < c->numlits; i++)
    zap_mterm(LIT(c,i));
  free_mclause(c);
}

int
Ground::lit_position(Mclause parent, Term child)
{
  for (int i = 0; i < parent->numlits; i++) {
    if (LIT(parent,i) == child)
      return i;
  }
  return -1;
}

void
Ground::set_parent_pointers(Term t)
{
  // Make each term, except for variables, point to its parent.
  if (!VARIABLE(t)) {
    for (int i = 0; i < ARITY(t); i++) {
      if (VARIABLE(ARG(t,i)))
        ARG(t,i)->container = nullptr;
      else
        ARG(t,i)->container = t;
      set_parent_pointers(ARG(t,i));
    }
  }
}

bool
Ground::eterm(Term t, int* pid)
{
  /*   Originally in propagate.c
   *   Check if a term is evaluable.  If so, set the id.
   *   For example, f(k,j,i) gives k*n*n + j*n + i + base.
   *   If the term is not evaluable, set the id to -1.
   */

  *pid = -1;  /* We must return -1 if the term is not evaluable. */
  if (!Arith->arith_evaluable(t))
    return false;
  else {
    int mult = 1;
    int id = Sn_to_mace_sn[SYMNUM(t)]->base;
    for (int i = ARITY(t)-1; i >= 0; i--) {
      if (!VARIABLE(ARG(t,i)))
        return false;
      else
        id += VARNUM(ARG(t,i)) * mult;
      mult *= Domain_size;
    }
    *pid = id;
    return true;
  }
}

void
Ground::eterm_index_term(Term t)
{
  //Insert each eterm into the occurrence list of that cell.

  int id;
  if (VARIABLE(t))
    return;
  else if (eterm(t, &id)) {
    t->u.vp = Cells[id].occurrences;
    Cells[id].occurrences = t;
  }
  else {
    for (int i = 0; i < ARITY(t); i++) {
      eterm_index_term(ARG(t,i));
    }
  }
}

Mclause
Ground::containing_mclause(Term t)
{
  while (!Mace4_gv->LITERAL(t))
    t = static_cast<Term>(t->container);
  return static_cast<Mclause>(t->container);
}

Term
Ground::containing_mliteral(Term t)
{
  while (!Mace4_gv->LITERAL(t))
    t = static_cast<Term>(t->container);
  return t;
}

bool
Ground::member(Term x, Term t)
{
  /* This does not assume OR_TERMs are right associated. */
  TermContainer term_con;
  if (term_con.term_ident(x,t))
    return true;
  else if (!Mace4_gv->OR_TERM(t))
    return false;
  else if (member(x, ARG(t,0)))
    return true;
  else
    return member(x,ARG(t,1));
}

Term
Ground::merge(Term t)
{
  /* This assumes OR_TERMs are right associated. */
  TermContainer term_con;
  if (!Mace4_gv->OR_TERM(t))
    return t;
  else {
    ARG(t,1) = merge(ARG(t,1));
    if (!member(ARG(t,0), ARG(t,1)))
      return t;
    else {
      Term t1 = ARG(t,1);
      zap_mterm(ARG(t,0));
      term_con.free_term(t);
      return t1;
    }
  }
}

Term
Ground::simp_term(Term t)
{
  if (VARIABLE(t))
    return t;
  else {
    int id;
    for (int i = 0; i < ARITY(t); i++)
      ARG(t,i) = simp_term(ARG(t,i));
    if (eterm(t, &id) && Cells[id].value != nullptr) {
      zap_mterm(t);
      return Cells[id].value;
    }
    else
      return t;
  }
}

Term
Ground::simp_tv(Term t)
{
  TermContainer term_con;
  if (term_con.true_term(t)) {
    zap_mterm(t);
    return Domain[1];
  }
  else if (term_con.false_term(t)) {
    zap_mterm(t);
    return Domain[0];
  }
  else if (Mace4_gv->OR_TERM(t)) {
    t = merge(t);
    if (!Mace4_gv->OR_TERM(t))
      return simp_tv(t);
    else {
      for (int i = 0; i < ARITY(t); i++)
         ARG(t,i) = simp_tv(ARG(t,i));

      if (TRUE_TERM(ARG(t,0), Domain) || TRUE_TERM(ARG(t,1), Domain)) {
        zap_mterm(t);
        return Domain[1];
      }
      else if (FALSE_TERM(ARG(t,0), Domain)) {
        Term t2 = ARG(t,1);
        zap_mterm(ARG(t,0));
        term_con.free_term(t);
        return t2;
      }
      else if (FALSE_TERM(ARG(t,1), Domain)) {
        Term t2 = ARG(t,0);
        zap_mterm(ARG(t,1));
        term_con.free_term(t);
        return t2;
      }
      else
        return t;
    }
  }  /* end of OR_TERM */
  else if (Mace4_gv->NOT_TERM(t)) {
    ARG(t,0) = simp_tv(ARG(t,0));

    if (TRUE_TERM(ARG(t,0), Domain)) {
      zap_mterm(t);
      return Domain[0];
    }
    else if (FALSE_TERM(ARG(t,0), Domain)) {
      zap_mterm(t);
      return Domain[1];
    }
    else
      return t;
  }  /* end of NOT_TERM */
  else {
    /* It is an atomic formula. */
    int id;
    for (int i = 0; i < ARITY(t); i++)
      ARG(t,i) = simp_term(ARG(t,i));
    if (Arith->arith_rel_term(t)) {
      bool evaluated = false;
      int b = Arith->arith_eval(t, &evaluated);
      if (evaluated) {
        term_con.zap_term(t);
        return (b ? Domain[1] : Domain[0]);
      }
      else
        return t;
    }
    else if (eterm(t, &id) && Cells[id].value != nullptr) {
      zap_mterm(t);
      return Cells[id].value;
    }
    else if (Mace4_gv->EQ_TERM(t)) {
      /* f(4,3)=2; check if 2 has been crossed off of f(4,3) list. */
      int value;
      if (VARIABLE(ARG(t,1)) && eterm(ARG(t,0), &id))
        value = VARNUM(ARG(t,1));
      else if (VARIABLE(ARG(t,0)) && eterm(ARG(t,1), &id))
        value = VARNUM(ARG(t,0));
      else
        return t;
      if (Cells[id].possible[value] == nullptr) {
        zap_mterm(t);
        return Domain[0];
      }
      else
        return t;
    }
    else
      return t;
  }
}

Plist
Ground::term_to_lits(Term t)
{
  if (!Mace4_gv->OR_TERM(t)) {
    PlistContainer p_con;
    return p_con.plist_append(t);
  }
  else {
    Plist g0 = term_to_lits(ARG(t,0));
    PlistContainer  p0;
    p0.set_head(g0);
    Plist g1 = term_to_lits(ARG(t,1));
    PlistContainer  p1;
    p1.set_head(g1);
    TermContainer term_con;
    term_con.free_term(t);  /* the OR node */
    return p0.plist_cat(p1);
  }
}

Mclause
Ground::term_to_mclause(Term t)
{
  Plist g = term_to_lits(t);
  PlistContainer p_con;
  p_con.set_head(g);
  int n = p_con.plist_count();
  int i = 0;
  Mclause c = get_mclause(n);
  c->u.active = n;
  TermflagContainer tf_con;
  TermContainer     term_con;
  for (Plist g2 = g; g2 != nullptr; g2 = g2->next) {
    Term lit = static_cast<Term>(g2->v);
    Term atom;
    if (Mace4_gv->NOT_TERM(lit)) {
      atom = ARG(lit,0);
      term_con.free_term(lit);  /* the NOT node */
      tf_con.term_flag_set(atom, Mace4_gv->get_Negation_flag());
    }
    else
      atom = lit;
    tf_con.term_flag_set(atom, Mace4_gv->get_Relation_flag());
    *LIT_l(c, i) = atom;
    i++;
  }
  p_con.zap_plist();
  return c;
}

Term
Ground::subst_domain_elements_term(Term t, int *vals)
{
  TermContainer     term_con;
  if (VARIABLE(t)) {
    Term t2 = Domain[vals[VARNUM(t)]];
    term_con.zap_term(t);
    return t2;
  }
  else {
    int i = term_con.natural_constant_term(t);
    if (i >= 0) {
      term_con.zap_term(t);
      if (i < Domain_size)
        return Domain[i];  /* domain element */
      else
        return term_con.get_variable_term(i);  /* natural number of arithmetic only */
    }
    else {
      for (int i = 0; i < ARITY(t); i++)
        ARG(t,i) = subst_domain_elements_term(ARG(t,i), vals);
      return t;
    }
  }
}

void
Ground::p_mclause(Mclause c, const Mace4GlobalValues& mace4_gv)
{
  std::cout << "numlits=" << c->numlits << ", active=" << c->u.active << ", subsumed=" << c->subsumed << ": ";

  ParseContainer pc;
  for (int i = 0; i < c->numlits; i++) {
    Term atom = LIT(c,i);
    if (!mace4_gv.NEGATED(atom))
      pc.fwrite_term(std::cout, atom);
    else {
      std::cout << "~(";
      pc.fwrite_term(std::cout, atom);
      std::cout << ")";
    }
    if (i < c->numlits-1)
      std::cout << " | ";
    else
      std::cout << ".\n";
  }
}

bool
Ground::check_that_ground_clauses_are_true(Plist Ground_clauses)
{
  bool ok = true;
  for (Plist g = Ground_clauses; g != nullptr; g = g->next) {
    Mclause c = static_cast<Mclause>(g->v);
    if (!c->subsumed) {
      std::cerr << "ERROR, model reported, but clause not true!\n";
      std::cout << "ERROR, model reported, but clause not true! ";
      p_mclause(c, *Mace4_gv);
      ok = false;
    }
  }
  return ok;
}
