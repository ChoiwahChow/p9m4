#ifndef TP_BASIC_H
#define TP_BASIC_H

#include "term.h"
#include "unify.h"


class GlobalBasic {
                    private:
                            bool Basic_paramodulation;
                            int Nonbasic_flag;
                            
                            
                    public:
                            GlobalBasic();
                            ~GlobalBasic();
                        
                            
                            friend class Basic;
                            friend class LadrVGlobais;
                            
                            
};


//All static.....there are no properties, only metohds
class Basic {

                            private://none
                    
                            public:
                            static void init_basic_paramod(void);
                            static void set_basic_paramod(bool);
                            static bool basic_paramod(void);
                            static void mark_term_nonbasic(Term);
                            static void mark_all_nonbasic(Term);
                            static bool nonbasic_term(Term);
                            static bool basic_term(Term);
                            static int nonbasic_flag(void);
                            static Term apply_basic(Term, Context);
                            static Term apply_basic_substitute(Term, Term, Context , Term , Context);
                            static void clear_all_nonbasic_marks(Term);
                            static void p_term_basic(Term);


};


#endif
