#ifndef TP_CLAUSE_H
#define TP_CLAUSE_H

#include "literals.h"
#include "attrib.h"
#include "formula.h"
#include "maximal.h"






struct topform {

  /* for both clauses and formulas */

  int              id;
  struct clist_pos *containers;     /* Clists that contain the Topform */
  Attribute        attributes;
  struct just      *justification;
  double           weight;
  char             *compressed;     /* if nonNULL, a compressed form */
  topform          *matching_hint;   /* hint that matches clause, if any */

  /* for clauses only */

  Literals         literals;        /* NULL can mean the empty clause */

  /* for formulas only */

  Formula          formula;

  int   semantics;        /* evaluation in interpretations */

  /* The rest of the fields are flags.  These could be bits. */

  bool   is_formula;      /* is this really a formula? */
  char   normal_vars;     /* variables have been renumbered */
  char   used;            /* used to infer a clause that was kept */
  char   official_id;     /* Topform is in the ID table */
  char   initial;         /* existed at the start of the search */
  char   neg_compressed;  /* negative and compressed */
  char   subsumer;        /* has this clause back subsumed anything? */

};

typedef struct topform * Topform;



class GlobalTopform {
                        private:
                                    unsigned Topform_gets, Topform_frees;
                        
                        public:            
                                    GlobalTopform();
                                    ~GlobalTopform();
                                    
                                    friend class TopformContainer;
                                    friend class LadrVGlobais;
    
};

class TopformContainer {
  
                            
                            private :
                                    
                                       Topform get_topform(void);
                                       void free_topform(Topform);
                                       
                            public :            
                                        TopformContainer();
                                        ~TopformContainer();
                                        void p_topform_mem();
                                        void zap_topform(Topform);
                                        void fprint_clause(ostream &, Topform);
                                        void p_clause(Topform);
                                        Topform term_to_clause(Term); 
                                        void upward_clause_links(Topform c);
                                        Topform term_to_topform(Term, bool);
                                        Term topform_to_term(Topform);
                                        Term topform_to_term_without_attributes(Topform);
                                        void clause_set_variables(Topform, int);
                                        void renumber_variables(Topform, int);
                                        void term_renumber_variables(Term, int);
                                        Plist renum_vars_map(Topform);
                                        bool check_upward_clause_links(Topform);
                                        Topform copy_clause(Topform);
                                        Topform copy_clause_with_flags(Topform); 
                                        Topform copy_clause_with_flag(Topform, int);
                                        void inherit_attributes(Topform, Context, Topform, Context,Topform);
                                        void gather_symbols_in_topform(Topform, I2list *, I2list *);
                                        void gather_symbols_in_topforms(Plist, I2list *, I2list *);
                                        Ilist fsym_set_in_topforms(Plist);
                                        Ilist rsym_set_in_topforms(Plist);
                                        bool min_depth(Literals);
                                        bool initial_clause(Topform);
                                        bool negative_clause_possibly_compressed(Topform); //fazer depois do clauses.h
                                        Term topform_properties(Topform);
                                        void append_label_attribute(Topform, const string &);
                                        void delete_label_attribute(Topform, const string &); //deletes last label attribute
                                        void fprint_topform_mem(ostream &, bool);
                                        
                                        static OrderType cl_id_compare(Topform, Topform);
                                        // BV(2016-may-27)
                                        static OrderType cl_hint_id_compare(Topform, Topform);
                                        static OrderType cl_wt_id_compare(Topform, Topform);
                                        
                                        
                                        
                                        friend class DefinitionsContainer;
                                        friend class Clausify;
                                        friend class Parautil;
                                        friend class Random;
                                        friend class Subsume;
                                        friend class ClashContainer;
                                        friend class Resolve;
                                        friend class Paramodulation;
                                        friend class Xproofs;
                                        friend class Ivy;
                                        friend class Ioutil;
                                        friend class TopInput;
                                        friend class Utilities;
                                        
    
};
    

#endif

