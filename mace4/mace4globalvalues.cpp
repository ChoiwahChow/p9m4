

#include "mace4globalvalues.h"
#include "../ladr/termflag.h"
#include "../ladr/symbols.h"


Mace4GlobalValues::Mace4GlobalValues() : Relation_flag(-1), Negation_flag(-1), Eq_sn(-1), Or_sn(-1), Not_sn(-1)
{

}

void
Mace4GlobalValues::init_globals() {
  /* These flags are for ground clause (mclause) literals. */
  TermflagContainer TF;
  Relation_flag = TF.claim_term_flag();           // original: msearch.c
  Negation_flag = TF.claim_term_flag();           // original: msearch.c

  /* Cache some symbol numbers. */
  SymbolContainer sym_con;
  Eq_sn  = sym_con.str_to_sn(sym_con.eq_sym(), 2);           // original: msearch.c
  Or_sn  = sym_con.str_to_sn(sym_con.or_sym(), 2);           // original: msearch.c
  Not_sn = sym_con.str_to_sn(sym_con.not_sym(), 1);          // original: msearch.c
}

