#ifndef TP_ACCANON_H
#define TP_ACCANON_H


#include "termflag.h"
#include "order.h"

#define MAX_ACM_ARGS 2500

class Accnon {
                        
                            private:
                                static void right_associate(Term);
                            public:
                                static void flatten(Term , Term *, int *);
                                static void ac_canonical2(Term t, int bit, OrderType(*term_compare_proc) (Term, Term));
                                static void ac_canonical(Term, int);
                                static int check_ac_canonical(Term);
};



#endif
