
#ifndef MACE4_GLOBAL_VALUES
#define MACE4_GLOBAL_VALUES

#include "../ladr/termflag.h"

class Mace4GlobalValues {
private:
  int         Relation_flag;  /* term flag */
  int         Negation_flag;  /* term flag */

  /* Cached symbol numbers */
  int         Eq_sn;
  int         Or_sn;
  int         Not_sn;

public:
  static inline bool TERM_FLAG(Term const t, int flag) { return TP_BIT(t->private_flags, 1 << flag) != 0; } //orig ground.h
  inline bool NEGATED(Term const t) const { return TERM_FLAG(t, Negation_flag); }                          //orig ground.h
  inline bool LITERAL(Term const t) const { return TERM_FLAG(t, Relation_flag); }                          //orig ground.h
  inline bool OR_TERM(const Term t) const { return SYMNUM(t) == Or_sn; }                                   // orig ground.h ok if variable
  inline bool NOT_TERM(const Term t) const { return SYMNUM(t) == Not_sn; }                                 // orig ground.h ok if variable
  inline bool EQ_TERM(const Term t) const { return SYMNUM(t) == Eq_sn; }                                   // orig ground.h ok if variable
  inline int  get_Negation_flag(void) const { return Negation_flag; }
  inline int  get_Relation_flag(void) const { return Relation_flag; }

public:
  Mace4GlobalValues();
  void init_globals();
};


#endif

