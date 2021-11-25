

#ifndef MACE4_ARITHMETIC_H
#define MACE4_ARITHMETIC_H

#include "mace4globalvalues.h"

typedef class arithmetic* Arithmetic;

/*
 *  This class is a singleton, but initialized only as specified.
 *  Arith_sn_size would be zero if not initialized.
 */

class arithmetic {
private:
  int Sum_sn;
  int Prod_sn;
  int Neg_sn;
  int Div_sn;
  int Mod_sn;
  int Min_sn;
  int Max_sn;
  int Abs_sn;
  int Domain_size_sn;
  int Lt_sn;
  int Le_sn;
  int Gt_sn;
  int Ge_sn;
  int Eq_sn;

  bool* Arith_op_sn;     /* array to know if sn is arithmetic operation */
  bool* Arith_rel_sn;    /* array to know if sn is arithmetic relation */
  int   Arith_sn_size;   /* size of array */

  /*
   * The following are "env/global" data used by a number of cooperating objects (e.g. msearch, select etc)
   * to do searching for models. This object does not own any of them.  They are "pointers" passed in.
   */
  int         Domain_size;     /* the size of the search domain */
  const Term* Domain;          /* pointer to global array of terms representing (shared) domain elements  */
  const Mace4GlobalValues*  Mace4_gv;

public:
  arithmetic();
  ~arithmetic();
  arithmetic(const arithmetic&) = delete;
  arithmetic& operator=(const arithmetic&) = delete;

  void init_arithmetic(Term* Domain, int domain_size, const Mace4GlobalValues& mace4_gv);
  static int  modulo(int a, int b);
  bool arith_op_sn(int i) const;
  bool arith_rel_sn(int i) const;
  bool arith_op_term(Term t) const;
  bool arith_rel_term(Term t) const;
  bool arith_term(Term t) const;
  bool arith_quasi_evaluable(Term t) const;
  int  arith_eval(Term t, bool* evaluated) const;
  bool ok_for_arithmetic(Plist clauses, int domain_size) const;
  bool check_with_arithmetic(Plist ground_clauses) const;
  bool arith_evaluable(Term t) const;

private:
  int  arith_evaluate(Term t, bool* evaluated) const;
  bool top_safe(Term t, int domain_size) const;
  bool all_safe(Term t, int domain_size) const;
  bool all_ordinary_nodes_safe(Term t, int domain_size) const;
  bool non_arith(Term t) const;
  bool atom_safe(Term atom, int domain_size) const;
  Term distrib(Term t) const;
  Term qsimp(Term t) const;
  bool arith_rel_quasi_eval(Term atom) const;
};

#endif

