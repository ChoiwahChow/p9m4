#ifndef _TOP_INPUT_H
#define _TOP_INPUT_H


#include "just.h"



enum class unknown_actions { IGNORE_UNKNOWN,NOTE_UNKNOWN,WARN_UNKNOWN,KILL_UNKNOWN};

enum class Top_input_type { 
								TERMS, 
								FORMULAS
							};

/* What shall we do if we read an unknown flag or parameter? */

enum class Unknown_type	{
							IGNORE_UNKNOWN,
							NOTE_UNKNOWN,
							WARN_UNKNOWN,
							KILL_UNKNOWN
};


typedef struct readlist * Readlist;

struct readlist {
  string *name;      /* name, as it appears in the input file */
  int type;        /* FORMULAS, TERMS */
  bool auxiliary;
  Plist *p;        /* *pointer* to the Plist */
  Readlist next;
};


class TopInput {
        
        private:  
                    Readlist Input_lists;
					Plist Lex_function_list;
					Plist Lex_predicate_list;
					Plist Skolem_list;  /* temp store of user-declared skolems */
					/*
						* Memory:: management
					*/
					unsigned Readlist_gets, Readlist_frees;
					string Program_name;         
                    
                    
                    
                    
                    Readlist get_readlist();
					void free_readlist(Readlist p);
					
					
					void fatal_input_error(ostream &, string , Term);
					bool condition_is_true(Term);
					void process_op2(ostream &, Term , int, Term, Term);
					void execute_unknown_action(ostream &, int , Term, string);
					void process_symbol_list(Term, string, Plist);
					Readlist readlist_member(Readlist, string name, int);
					Readlist readlist_member_wild(Readlist, int);
					void input_symbols(int, bool, Ilist *, Ilist *);
					void symbol_check_and_declare(void);
					void check_formula_attributes(Formula);
                    void top_inputprocess_op2(ostream &, Term, int, Term, Term);
                    void destroy_Input_lists();
					
					
    
        public:            
                    TopInput();
                    ~TopInput();
                    void init_standard_ladr();
                    void set_program_name(string);
					void fprint_top_input_mem(ostream &, bool);
					void p_top_input_mem();
					void process_op(Term, bool, ostream &);
					void process_redeclare(Term, bool, ostream &);
					void flag_handler(ostream &, Term, bool, int );
					void parm_handler(ostream &, Term, bool, int );
					void accept_list(string, int , bool , Plist *);
					void read_from_file(istream &, ostream &, bool , int );
					void read_all_input(int , char *[], ostream &, bool , int );
					Plist process_input_formulas(Plist , bool );
					Plist process_demod_formulas(Plist , bool );
					Plist process_goal_formulas(Plist , bool );
					Term read_commands(istream &, ostream &, bool , int );
					Plist embed_formulas_in_topforms(Plist , bool );
                    void read_from_string_buf(String_buf, ostream &, bool, int);
                
                    void Free_Mem(void);
                    

                    friend class LadrVGlobais;
    
};



#endif
