#ifndef TP_TERMFLAG_H
#define TP_TERMFLAG_H

#include "term.h"

#define TERM_BITS (sizeof(FLAGS_TYPE) * CHAR_BIT)
#define SET_BIT(bits, flag)    (bits = (bits) | (flag))
#define CLEAR_BIT(bits, flag)  (bits = (bits) & ~(flag))
#define TP_BIT(bits, flag)     (bits & flag)



class GlobalTermflag {
                        private:
                                int bits_in_use[TERM_BITS];  //ir ao initVGlobais e iniciar este array
                                GlobalTermflag();
                                ~GlobalTermflag();
                                friend class TermflagContainer;
                                friend class LadrVGlobais;

};


class TermflagContainer {
                            public:
                                    int claim_term_flag(void);
                                    void release_term_flag(const int);
                                    void term_flag_set(const Term, const int);
                                    void term_flag_clear(const Term, const int);
                                    bool term_flag(const Term, const int);
                                    int  term_flags(void)const;
                                    Term copy_term_with_flags(const Term);
                                    Term copy_term_with_flag(const Term t, const int flag);
                                    void term_flag_clear_recursively(const Term, const int);
};


#endif
