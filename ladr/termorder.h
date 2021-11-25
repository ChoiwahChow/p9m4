#ifndef TP_TERMORDER_H
#define TP_TERMORDER_H

#include "flatterm.h"
#include "symbols.h"

/* Term ordering method */


enum class Order_method {
                            LRPO_METHOD,
                            LPO_METHOD,
                            RPO_METHOD,
                            KBO_METHOD
};


class TermOrder {
                           
                            private:
                                    static bool flat_lrpo_multiset(Flatterm, Flatterm);
                                    static bool flat_lrpo_lex(Flatterm, Flatterm, bool);
                                    static bool lrpo_lex(Term , Term , bool);
                                    static bool flat_kbo(Flatterm, Flatterm, bool);
                                    static bool flat_lrpo(Flatterm, Flatterm,  bool);
    
                            public:
                                    static Order_method Ordering_method;
                                    static void assign_order_method(Order_method method);
                                    static OrderType term_compare_basic(Term, Term);
                                    static OrderType term_compare_ncv(Term, Term);
                                    static OrderType term_compare_vcp(Term, Term);
                                    static OrderType term_compare_vr(Term , Term);
                                    
                                    static OrderType flatterm_compare_vr(Flatterm, Flatterm);
                                    static bool lrpo_multiset(Term, Term, bool);
                                    static bool lrpo(Term , Term , bool);
                                    
                                    
                                    static void init_kbo_weights(Plist);
                                    static int kbo_weight(Term);
                                    static bool kbo(Term , Term , bool);
                                    static bool term_greater(Term , Term , bool);
                                    static OrderType term_order(Term , Term);
                             
                                    static int flat_kbo_weight(Flatterm);
                                    
                                    static bool flat_greater(Flatterm, Flatterm, bool);
                                    static bool greater_multiset_current_ordering(Term, Term);
};





#endif
