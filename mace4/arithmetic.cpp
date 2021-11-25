
#include "../ladr/accanon.h"
#include "../ladr/fatal.h"
#include "../ladr/parse.h"
#include "../ladr/symbols.h"
#include "../ladr/term.h"
#include "../ladr/termorder.h"
#include "../ladr/topform.h"

#include "arithmetic.h"
#include "ground.h"

arithmetic::arithmetic() : Mace4_gv(nullptr), Domain_size(0), Domain(nullptr), Arith_sn_size(0),
  Arith_op_sn(nullptr), Arith_rel_sn(nullptr), Sum_sn(0), Prod_sn(0), Neg_sn(0), Div_sn(0), Mod_sn(0), Min_sn(0), Max_sn(0),
  Abs_sn(0), Domain_size_sn(0), Le_sn(0), Lt_sn(0), Ge_sn(0), Gt_sn(0), Eq_sn(0)
{
}

arithmetic::~arithmetic() {
  delete Arith_op_sn;
  delete Arith_rel_sn;
}

void
arithmetic::init_arithmetic(Term* domain, int domain_size, const Mace4GlobalValues& mace4_gv) {
  Mace4_gv = &mace4_gv;
  Domain = domain;
  Domain_size = domain_size;

  SymbolContainer sym_con;
  Arith_sn_size = sym_con.greatest_symnum() + 1;
  Arith_op_sn = new bool[Arith_sn_size];
  Arith_rel_sn = new bool[Arith_sn_size];
  std::fill(Arith_op_sn, Arith_op_sn+Arith_sn_size, false);
  std::fill(Arith_rel_sn, Arith_rel_sn+Arith_sn_size, false);

  Arith_op_sn[Sum_sn = sym_con.str_to_sn("+", 2)] = true;
  Arith_op_sn[Prod_sn = sym_con.str_to_sn("*", 2)] = true;
  Arith_op_sn[Neg_sn = sym_con.str_to_sn("-", 1)] = true;
  Arith_op_sn[Div_sn = sym_con.str_to_sn("/", 2)] = true;
  Arith_op_sn[Mod_sn = sym_con.str_to_sn("mod", 2)] = true;
  Arith_op_sn[Min_sn = sym_con.str_to_sn("min", 2)] = true;
  Arith_op_sn[Max_sn = sym_con.str_to_sn("max", 2)] = true;
  Arith_op_sn[Abs_sn = sym_con.str_to_sn("abs", 1)] = true;
  Arith_op_sn[Domain_size_sn = sym_con.str_to_sn("domain_size", 0)] = true;

  Arith_rel_sn[Le_sn = sym_con.str_to_sn("<=", 2)] = true;
  Arith_rel_sn[Lt_sn = sym_con.str_to_sn("<", 2)] = true;
  Arith_rel_sn[Ge_sn = sym_con.str_to_sn(">=", 2)] = true;
  Arith_rel_sn[Gt_sn = sym_con.str_to_sn(">", 2)] = true;
  Arith_rel_sn[Eq_sn = sym_con.str_to_sn("=", 2)] = true;

  sym_con.set_assoc_comm("+", true);
  sym_con.set_assoc_comm("*", true);

  ParseContainer  parse_con;
  parse_con.declare_parse_type("+",    490, ParseType::INFIX_RIGHT);
  parse_con.declare_parse_type("*",    470, ParseType::INFIX_RIGHT);
  parse_con.declare_parse_type("/",    460, ParseType::INFIX);
  parse_con.declare_parse_type("mod",  460, ParseType::INFIX);
}

/*
In C, the % (remainder) operation is not defined for negative operands.
Also there is a distinction between the "remainder" and "modulo" operations
for negative operands:

  A   B   A/B   A rem B  A mod B

 14   5    2       4        4
-14   5   -2      -4        1
 14  -5   -2       4       -1
-14  -5    2      -4       -4
*/

int
arithmetic::modulo(int a, int b)
{
  if (b == 0)
    return INT_MAX;
  else if (b > 0) {
    if (a >= 0)
      return a % b;                  /* a >= 0, b > 0 */
    else
      return -(abs(a) % b) + b;      /* a <  0, b > 0 */
  }
  else {
    if (a >= 0)
      return (a % abs(b)) + b;       /* a >= 0, b < 0 */
    else
      return -(abs(a) % abs(b));     /* a <  0, b < 0 */
  }
}

bool
arithmetic::arith_op_sn(int i) const
{
  if (i >= Arith_sn_size)
    return false;
  else
    return Arith_op_sn[i];
}

bool
arithmetic::arith_rel_sn(int i) const
{
  if (i >= Arith_sn_size)
    return false;
  else
    return Arith_rel_sn[i];
}

bool
arithmetic::arith_evaluable(Term t) const
{
  if (t == nullptr || VARIABLE(t))
    return false;

  int i = SYMNUM(t);
  if (i >= Arith_sn_size)
    return true;
  return !(Arith_op_sn[i] || Arith_rel_sn[i]);
}

bool
arithmetic::arith_op_term(Term t) const
{
  return !VARIABLE(t) && arith_op_sn(SYMNUM(t));
}

bool
arithmetic::arith_rel_term(Term t) const
{
  return !VARIABLE(t) && arith_rel_sn(SYMNUM(t));
}

bool
arithmetic::arith_term(Term t) const
{
  if (VARIABLE(t))
    return true;
  else
    return arith_op_term(t) || arith_rel_term(t);
}

bool
arithmetic::arith_quasi_evaluable(Term t) const
{
  if (!arith_term(t))
    return false;
  else if (VARIABLE(t))
    return true;
  else {
    for (int i = 0; i < ARITY(t); i++)
      if (!arith_quasi_evaluable(ARG(t,i)))
        return false;
    return true;
  }
}

int
arithmetic::arith_evaluate(Term t, bool* evaluated) const
{
  if (!arith_term(t)) {
    *evaluated = false;
    return 0;
  }

  if (VARIABLE(t))
    return VARNUM(t);
  else {
    int sn = SYMNUM(t);

    if (sn == Div_sn || sn == Mod_sn) {
      int d = arith_evaluate(ARG(t,1), evaluated);
      if (d == 0) {
        *evaluated = false;
        return 0;
      }
      else if (sn == Div_sn)
        return arith_evaluate(ARG(t,0), evaluated) / d;
      else
        return modulo(arith_evaluate(ARG(t,0), evaluated), d);
    }

    else if (sn == Sum_sn)
      return arith_evaluate(ARG(t,0), evaluated) + arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Prod_sn)
      return arith_evaluate(ARG(t,0), evaluated) * arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Neg_sn)
      return -arith_evaluate(ARG(t,0), evaluated);
    else if (sn == Abs_sn)
      return abs(arith_evaluate(ARG(t,0), evaluated));
    else if (sn == Domain_size_sn)
      return Domain_size;
    else if (sn == Min_sn) {
      int a0 = arith_evaluate(ARG(t,0), evaluated);
      int a1 = arith_evaluate(ARG(t,1), evaluated);
      return IMIN(a0,a1);
    }
    else if (sn == Max_sn) {
      int a0 = arith_evaluate(ARG(t,0), evaluated);
      int a1 = arith_evaluate(ARG(t,1), evaluated);
      return IMAX(a0,a1);
    }
    else if (sn == Lt_sn)
      return arith_evaluate(ARG(t,0), evaluated) <  arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Le_sn)
      return arith_evaluate(ARG(t,0), evaluated) <= arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Gt_sn)
      return arith_evaluate(ARG(t,0), evaluated) >  arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Ge_sn)
      return arith_evaluate(ARG(t,0), evaluated) >= arith_evaluate(ARG(t,1), evaluated);
    else if (sn == Eq_sn)
      return arith_evaluate(ARG(t,0), evaluated) == arith_evaluate(ARG(t,1), evaluated);
    else {
      fatal::fatal_error("arith_evaluate, operation not handled");
      return INT_MIN;
    }
  }
}

int
arithmetic::arith_eval(Term t, bool* evaluated) const
{
  *evaluated = true;
  return arith_evaluate(t, evaluated);
}

bool
arithmetic::top_safe(Term t, int domain_size) const
{
  if (VARIABLE(t))
    return true;
  else if (CONSTANT(t)) {
    TermContainer   term_con;
    return term_con.natural_constant_term(t) < domain_size;
  }
  else if (arith_op_term(t) || arith_rel_term(t))
    return false;
  else
    return true;
}

bool
arithmetic::all_safe(Term t, int domain_size) const
{
  if (VARIABLE(t))
    return true;
  else if (!top_safe(t, domain_size))
    return false;
  else {
    for (int i = 0; i < ARITY(t); i++) {
      if (!all_safe(ARG(t,i), domain_size))
        return false;
    }
    return true;
  }
}


bool
arithmetic::all_ordinary_nodes_safe(Term t, int domain_size) const
{
  if (VARIABLE(t) || CONSTANT(t))
    return true;
  else if (arith_rel_term(t) || arith_op_term(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (!all_ordinary_nodes_safe(ARG(t,i), domain_size))
        return false;
    }
    return true;
  }
  else
    return all_safe(t, domain_size);
}


bool
arithmetic::non_arith(Term t) const
{
  if (VARIABLE(t))
    return false;
  else if (CONSTANT(t)) {
    TermContainer   term_con;
    return term_con.natural_constant_term(t) < 0;
  }
  else if (arith_rel_term(t) || arith_op_term(t))
    return false;
  else
    return true;
}


bool
arithmetic::atom_safe(Term atom, int domain_size) const
{
  if (SYMNUM(atom) == Eq_sn) {
    /* special case, because = is sometimes arith, sometimes not */
    Term a = ARG(atom,0);
    Term b = ARG(atom,1);
    TermContainer   term_con;
    if (non_arith(a) && term_con.natural_constant_term(b) >= domain_size)
      return false;
    else if (non_arith(b) && term_con.natural_constant_term(a) >= domain_size)
      return false;
    else
      return all_ordinary_nodes_safe(atom, domain_size);
  }
  else
    return all_ordinary_nodes_safe(atom, domain_size);
}

bool
arithmetic::ok_for_arithmetic(Plist clauses, int domain_size) const
{
  /* Domain elements and other integers are CONSTANTS!!! */
  for (Plist p = clauses; p; p = p->next) {
    Topform c = static_cast<Topform>(p->v);
    for (Literals lit = c->literals; lit; lit = lit->next) {
      if (!atom_safe(lit->atom, domain_size))
        return false;
    }
  }
  return true;
}

Term
arithmetic::distrib(Term t) const
{
  if (VARIABLE(t))
    return t;
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = distrib(ARG(t,i));

    if (SYMNUM(t) != Prod_sn)
      return t;
    else {
      TermContainer   term_con;
      if (SYMNUM(ARG(t,1)) == Sum_sn) {
        /* a*(b+c) */
        Term a = ARG(t,0);
        Term b = ARG(ARG(t,1),0);
        Term c = ARG(ARG(t,1),1);
        term_con.free_term(ARG(t,1));
        term_con.free_term(t);
        return term_con.build_binary_term(Sum_sn,
                                          distrib(term_con.build_binary_term(Prod_sn, a, b)),
                                          distrib(term_con.build_binary_term(Prod_sn, term_con.copy_term(a), c)));
      }
      else if (SYMNUM(ARG(t,0)) == Sum_sn) {
        /* (b+c)*a */
        Term a = ARG(t,1);
        Term b = ARG(ARG(t,0),0);
        Term c = ARG(ARG(t,0),1);
        term_con.free_term(ARG(t,0));
        term_con.free_term(t);
        return term_con.build_binary_term(Sum_sn,
                                          distrib(term_con.build_binary_term(Prod_sn, b, a)),
                                          distrib(term_con.build_binary_term(Prod_sn, c, term_con.copy_term(a))));
      }
      else
        return t;
    }
  }
}

Term
arithmetic::qsimp(Term t) const
{
  if (VARIABLE(t))
    return t;
  else {
    bool all_args_ints = true;
    for (int i = 0; i < ARITY(t); i++) {
      ARG(t,i) = qsimp(ARG(t,i));
      if (!(VARIABLE(ARG(t,i)) ||
            (SYMNUM(ARG(t,i)) == Neg_sn && VARIABLE(ARG(ARG(t,i),0)))))
        all_args_ints = false;
    }

    TermContainer   term_con;
    if (all_args_ints) {
      bool evaluated;
      int i = arith_eval(t, &evaluated);
      if (evaluated) {
          term_con.zap_term(t);
        if (i >= 0)
          return term_con.get_variable_term(i);
        else
          return term_con.build_unary_term(Neg_sn, term_con.get_variable_term(-i));
      }
      else
        return t;
    }
    else {
      if (SYMNUM(t) != Prod_sn && VARIABLE(ARG(t,0)) && VARNUM(ARG(t,0)) == 0) {
        /* 0*x to 0 */
        term_con.zap_term(t);
        return term_con.get_variable_term(0);
      }
      else if (SYMNUM(t) != Sum_sn &&
               SYMNUM(ARG(t,1)) == Neg_sn &&
               term_con.term_ident(ARG(t,0),ARG(ARG(t,1),0))) {
        /* x + -x to 0 */
        term_con.zap_term(t);
        return term_con.get_variable_term(0);
      }
      else
        return t;
    }
  }
}

bool
arithmetic::arith_rel_quasi_eval(Term atom) const
{
  /* This is an initial version for testing only. */
  if (SYMNUM(atom) == Eq_sn) {
    TermContainer   term_con;
    bool negated = Mace4_gv->NEGATED(atom);
    Term atom2 = term_con.copy_term(atom);

    atom2 = distrib(atom2);
    // printf("after distrib:  "); fwrite_term_nl(stdout, atom2);
    Accnon::ac_canonical2(atom2, -1, TermOrder::term_compare_vcp);
    // printf("after AC canon: "); fwrite_term_nl(stdout, atom2);

    ARG(atom2,0) = qsimp(ARG(atom2,0));
    ARG(atom2,1) = qsimp(ARG(atom2,1));

    // printf("after qsimp: "); fwrite_term_nl(stdout, atom2);

    bool val = false;
    if (term_con.term_ident(ARG(atom2,0), ARG(atom2,1)))
      val = negated ? false : true;

    term_con.zap_term(atom2);
    return val;
  }
  else
    return false;
}

bool
arithmetic::check_with_arithmetic(Plist ground_clauses) const
{
  for (Plist p = ground_clauses; p; p = p->next) {
    Mclause c = static_cast<Mclause>(p->v);
    if (!c->subsumed) {
      /* look for an arithmetic term and evaluate it */
      bool clause_is_true = false;
      for (int i = 0; i < c->numlits && !clause_is_true; ++i) {
        Term atom = Ground::LIT(c, i);
        if (arith_quasi_evaluable(atom)) {
          if (arith_rel_quasi_eval(atom))
            clause_is_true = true;
        }
        else if (!Ground::FALSE_TERM(atom, Domain) && !Ground::TRUE_TERM(atom, Domain)) {
          /* non-arithmetic lits should have been reduced to true or false */
          fprintf(stderr, "ERROR, model reported, but clause not true!\n");
          fprintf(stdout, "ERROR, model reported, but clause not true! ");
          Ground::p_mclause(c, *Mace4_gv);
          fatal::fatal_error("check_with_arithmetic, clause not reduced");
        }
      }
      if (!clause_is_true)
        return false;
    }
  }
  return true;
}










