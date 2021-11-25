#ifndef TP_MULTISET_H
#define TP_MULTISET_H

#include "term.h"


class MultiSet {
    private:
        static int num_occurrences(const Term , const Term [], const int n);
        static Plist set_of_more_occurrences(const Term [], const int , const Term [], const int);
    public:
        static bool greater_multiset(const Term a1[], const int n1, const Term a2[], const int n2, bool (*comp_proc) (Term, Term, bool), const bool lex_order_vars);
};


#endif
