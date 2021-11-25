
#ifndef MACE_PROPAGATE_H
#define MACE_PROPAGATE_H


#include "../ladr/term.h"
#include "cell.h"
#include "estack.h"
#include "ground.h"
#include "mace4globalvalues.h"
#include "mace4vglobais.h"
#include "mstate.h"
#include "mstats.h"
#include "negpropindex.h"
#include "options.h"
#include "syms.h"


class propagate {
public:
  static constexpr int MAX_MACE_VARS = 50;     // originally from ground.h

  static constexpr int ASSIGNMENT = 0;         /* f(1,2)=3      P(1,2)     ~P(1,2) */
  static constexpr int NEAR_ASSIGNMENT = 1;    /* f(1,g(2))=3   P(1,g(2))   ~P(1,g(2)) */
  static constexpr int ELIMINATION = 2;        /* f(1,2) != 3 */
  static constexpr int NEAR_ELIMINATION = 3;   /* f(1,g(2)) != 3 */

private:
  negpropindex       npi;
  Plist              Ground_clauses;

  /*
   * The following are "env/global" data used by a number of cooperating objects (e.g. msearch, select etc)
   * to do searching for models. This object does not own any of them.  They are "pointers" passed in.
   */
  const Symbol_data  Symbols;
  const Symbol_data* Sn_to_mace_sn;
  const Cell         Cells;
  const Term*        Domain;
  int                Domain_size;
  MstateContainer*   MScon;
  struct mace_stats* Mstats;
  EstackContainer*   EScon;
  const Mace4GlobalValues* Mace4_gv;
  const Mace4VGlobais*     Mace4vglobais;
  Ground*                  Grounder;

  int  nterm_id(Term t);
  bool nterm(Term t, int *ppos, int *pid);
  void negprop_elim(int id, Term beta, Mstate state);

  Term pvalues_check(Term* a, int n);
  void nterm_check_and_process(Term lit, Mstate state);

  void new_assignment(int id, Term value, Mstate state);
  void new_elimination(int id, Term beta, Mstate state);
  void negprop_assign(int id, Mstate state);
  void negprop_near_elim(int subterm_id, Term alpha, Term beta, int pos, Mstate state);
  void negprop_near_assign(int subterm_id, Term alpha, Term beta, int pos, Mstate state);
  void process_clause(Mclause c, Mstate state);
  Mclause handle_literal(Term lit, Term result, Mstate state);
  void    propagate_positive(int id, Mstate state);
  void    propagate_all(Mstate state);

  void    instances_recurse(Topform c, int* vals, int* domains, int nextvar, int nvars, Mstate stat);

public:
  propagate() = delete;
  propagate(const propagate&) = delete;
  propagate& operator=(const propagate&) = delete;
  ~propagate();

  propagate(const Symbol_data s, int ds, const Term* dn, const Cell c, const Symbol_data* Sn, struct mace_stats* m,
            MstateContainer* mc, EstackContainer* ec, Ground* g, const Mace4GlobalValues* gv, const Mace4VGlobais* mg);

  Term   decode_eterm_id(int id);
  void   propagate_negative(int type, int id, Term alpha, Term beta, int pos, Mstate state);
  Estack assign_and_propagate(int id, Term value);
  void   process_initial_clause(Mclause c, Mstate state);
  void   generate_ground_clauses(Topform c, Mstate state);
  bool   check_that_ground_clauses_are_true(void);
  bool   check_with_arithmetic(void);
};


#endif

