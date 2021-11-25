#ifndef TP_FLATTERM_H
#define TP_FLATTERM_H

#include "term.h"
#include "discrim.h"


struct flatterm {
  //short         private_symbol; /* const/func/pred/var symbol ID */
  int private_symbol;  
  unsigned char arity;          /* number of auguments */
  flatterm *prev, *next, *end;
  /* The rest of the fields are for index retrieval and demodulation. */
  int size;                      /* symbol count */
  discrim *alternative;   /* subtree to try next */
  int varnum_bound_to;           /* -1 for not bound */
  bool reduced_flag;             /* fully demodulated */
};

typedef struct flatterm * Flatterm;



class GlobalFlatterm {
                        private:
                                unsigned Flatterm_gets;
                                unsigned Flatterm_frees;
                                
                        public:
                                GlobalFlatterm();
                                ~GlobalFlatterm();
                            
                            
                                friend class LadrVGlobais;
                                friend class FlattermContainer;
    
};



class FlattermContainer {
                            private:
                                        void fprint_flatterm_mem(ostream &, const bool) const;
                            public:
                                    FlattermContainer();
                                    ~FlattermContainer();
                                    Flatterm get_flatterm(void);
                                    void free_flatterm(Flatterm);
                                  
                                    void p_flatterm_mem() const;
                                    bool flatterm_ident(Flatterm, Flatterm);
                                    void zap_flatterm(Flatterm );
                                    Flatterm term_to_flatterm(Term );
                                    Term flatterm_to_term(Flatterm );
                                    Flatterm copy_flatterm(Flatterm );
                                    void print_flatterm(Flatterm );
                                    int flatterm_symbol_count(Flatterm );
                                    void p_flatterm(Flatterm );
                                    bool flat_occurs_in(Flatterm , Flatterm );
                                    I2list flat_multiset_vars(Flatterm );
                                    bool flat_variables_multisubset(Flatterm , Flatterm );
                                    int flatterm_count_without_vars(Flatterm);
                                    
                                    
                                    friend class Utilities;
                                   
};



#endif
