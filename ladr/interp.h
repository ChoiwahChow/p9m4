#ifndef TP_INTERP_H
#define TP_INTERP_H

#include "parse.h"
#include "topform.h"


#define ISWAP(x,y) {int t = x; x = y; y = t;}

/* We're using 1D arrays to for higher dimensions. */

#define I2(n,i,j)     ((i) * (n) + (j))
#define I3(n,i,j,k)   ((i) * (n) * (n) + (j) * (n) + (k))

#define UNDEFINED 0
#define FUNCTION  1
#define RELATION  2

#define MAX_VARS_EVAL 100







enum class Semantics_type { 
							SEMANTICS_NOT_EVALUATED,
							SEMANTICS_NOT_EVALUABLE,
							SEMANTICS_TRUE,
							SEMANTICS_FALSE
 };

 
 
 struct interp {
  Term  t;         /* the term representation (maybe NULL) */
  Term  comments;  /* list of comments (maybe NULL) */
  int   size;      /* domain size */
  /* Array of pointers to tables, one for each constant, function,
   * or predicate symbol (indexed by symbol number).  The arity and
   * print symbol can be obtained from the symbol number.
   */
  int num_tables;      /* number of tables */
  int **tables;
  int *arities;        /* arity of tables[i] */
  int *types;          /* type of tables[i]: FUNCTION or RELATION */

  int *occurrences;    /* number of occurrences of each element */
  int num_discriminators;
  int *discriminator_counts;
  int **profile;               /* [size,profile_components] */
  int num_profile_components;
  int *blocks;                 /* of identical components */
  bool incomplete;
};
 

typedef struct interp *Interp;
 
 
 class GlobalInterp {
						
						private:
								long unsigned Iso_checks;
								long unsigned Iso_perms;
								unsigned Interp_gets,Interp_frees;
                                
                                
                        public:        
                                GlobalInterp();
                                ~GlobalInterp();
                                
								
						friend class LadrVGlobais;		
						friend class InterpContainer;
						
						
 };
 
 
class InterpContainer {

				private:
						
						Interp get_interp(void);
						void free_interp(Interp);
						int *trivial_permutation(int);
						void compute_args(int *, int, int, int);
						void portable_indent(ostream &, int);
						void portable_recurse(ostream &,int, int,int *, int *, int);
						bool eval_literals_ground(Literals, Interp, int *);
						bool all_recurse(Literals, Interp, int *, int, int);
						int all_recurse2(Literals, Interp, int *, int, int);
						int eval_fterm_ground(Term, Interp, int *);
						bool eval_form(Formula, Interp, int []);
                        bool iso_interp_recurse(int *, int, int,Interp, Interp, bool);
                        OrderType compare_ints(int, int);
                        void invert_perm(int *, int *, int);
                        void copy_perm(int *, int *, int);
                        OrderType compare_permed_interps(int *, int *, int *, int *, Interp);
                        void canon_recurse(int, int *, int *, int *, int *,Interp);
				
				public:
						void fprint_interp_mem(ostream &, bool);
						void p_interp_mem();
						int int_power(int, int);
						Interp compile_interp(Term, bool);
						void transpose_binary(Term);
						void zap_interp(Interp);
						void fprint_interp_tex(ostream &, Interp);
						void fprint_interp_xml(ostream &, Interp);
						void fprint_interp_standard(ostream &, Interp);
						void fprint_interp_standard2(ostream &, Interp);
						void fprint_interp_portable(ostream &, Interp);
						void p_interp(Interp);
						void fprint_interp_cooked(ostream &, Interp);
						void fprint_interp_tabular(ostream &, Interp);
						void fprint_interp_raw(ostream &, Interp);
						int eval_term_ground(Term, Interp, int *);
						bool eval_literals(Literals, Interp);
						int eval_literals_true_instances(Literals, Interp);
						int eval_literals_false_instances(Literals, Interp);
						bool eval_formula(Formula, Interp);
						Term interp_remove_constants_recurse(Term);
						void interp_remove_constants(Term);
						Term interp_remove_others_recurse(Term, Plist);
						void interp_remove_others(Term, Plist);
						Interp copy_interp(Interp);
						Interp permute_interp(Interp, int *);
						bool ident_interp_perm(Interp, Interp, int *);
						Interp normal_interp(Interp);
						bool isomorphic_interps(Interp, Interp, bool);
						int interp_size(Interp);
						Term interp_comments(Interp);
						int *interp_table(Interp, string, int);
						long unsigned iso_checks(void);
						long unsigned iso_perms(void);
						bool evaluable_term(Term, Interp);
						bool evaluable_atom(Term, Interp);
						bool evaluable_literals(Literals, Interp);
						bool evaluable_formula(Formula, Interp);
						bool evaluable_topform(Topform, Interp);
						void update_interp_with_constant(Interp, Term, int);
						bool eval_topform(Topform, Interp);
						OrderType compare_interp(Interp, Interp);
						bool ident_interp(Interp, Interp);
						Interp canon_interp(Interp);
						void assign_discriminator_counts(Interp, Plist);
						bool same_discriminator_counts(Interp, Interp);
						void update_profile(Topform, Interp, int *);
						/* vecs[domain_element][profile_component] */
						void create_profile(Interp, Plist);
						void p_interp_profile(Interp, Plist);
						Interp normal3_interp(Interp, Plist);
						bool same_profiles(Interp, Interp);
						long unsigned perms_required(Interp);
						long unsigned factorial(int);
                        

};
 
 

#endif
