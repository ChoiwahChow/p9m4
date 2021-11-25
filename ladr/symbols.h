#ifndef TP_SYMBOLS_H
#define TP_SYMBOLS_H

#include "strbuf.h"
#include "glist.h"
#include <string>

/* maximum number of chars in string part of symbol (includes '\0') */

#define MAX_NAME      51



/* parse/print properties of symbols */

enum class ParseType {
            NOTHING_SPECIAL,
            INFIX,         /* xfx */
            INFIX_LEFT ,   /* yfx */
            INFIX_RIGHT,   /* xfy */
            PREFIX_PAREN,  /* fx  */
            PREFIX,        /* fy  */
            POSTFIX_PAREN, /* xf  */
            POSTFIX        /* yf  */
};


#define MIN_PRECEDENCE      1
#define MAX_PRECEDENCE    999



/* Function/relation properties of symbols */

enum class Symbol_Type { 
            UNSPECIFIED_SYMBOL,
            FUNCTION_SYMBOL,
            PREDICATE_SYMBOL
};


/* Unification properties of symbols */

enum class Unify_Theory { 
                            EMPTY_THEORY,
                            COMMUTE,        
                            ASSOC_COMMUTE
};

/* LRPO status */
enum class Lrpo_Status { 
            LRPO_LR_STATUS,
            LRPO_MULTISET_STATUS
};



/* Variable style */

enum class  Variable_Style { 
    
            STANDARD_STYLE,      /* x,y,z,... */
            PROLOG_STYLE,        /* A,B,C,... */
            INTEGER_STYLE        /* 0,1,2,... */
};



/* Private definitions and types*/

struct symbol {
  int           symnum;           /* unique identifier */
  string        *name;            /* the print symbol */
  int           arity;            /* 0 for constants */
  Symbol_Type   type;             /* function, relation, unspecified */
  ParseType     parse_type;       /* infix, prefix, etc. */
  int           parse_prec;       /* precedence for parsing/printing */
  Unify_Theory  unif_theory;      /* e.g., associative-commutative */
  int           occurrences;      /* how often it occurs somewhere */
  int           lex_val;          /* precedence for term orderings */
  int           kb_weight;        /* for Knuth-Bendix ordering */
  Lrpo_Status   lrpo_status;      /* for LRPO, LPO, RPO */
  bool          skolem;
  bool          unfold;
  bool          auxiliary;        /* not part of theory, e.g., in hints only */
  /* IF YOU ADD MORE FIELDS, MAKE SURE TO INITIALIZE THEM ! */
};



typedef struct symbol * Symbol;

#define SYM_TAB_SIZE  5


class GlobalSymbol {
                        private:
                            
                                   string True_sym;  
                                   string False_sym;
                                   string And_sym;
                                   string Or_sym;
                                   string Not_sym;
                                   string Iff_sym;
                                   string Imp_sym;
                                   string Impby_sym;
                                   string All_sym;
                                   string Exists_sym;
                                   string Quant_sym;
                                   string Attrib_sym;
                                   string Eq_sym;
                                   string Neq_sym;
                                   string Null_sym;
                                   
                                   
                                   int Assoc_comm_symbols;  /* number declared */
                                   unsigned Symbol_count;
                                   int Comm_symbols;  
                                   int Eq_symnum;  
                                   int Or_symnum;
                                   int Not_symnum;
                                   Variable_Style Var_style;
                                   bool Zero_wt_kb;
                                   string Skolem_constant_prefix;
                                   string Skolem_function_prefix;
                                   
                                   int Next_skolem_constant;      /* counter for c1, c2, ... */
                                   int Next_skolem_function;      /* counter for f1, f2, ... */
                                   bool Skolem_check;  /* make sure Skolem symbols are unique */
                                   unsigned Mark_for_new_symbols;
                                   
                                   IlistContainer Preliminary_prec_func;
                                   IlistContainer Preliminary_prec_pred;
                                   
                                   
                                   //Estas duas Plists são listas para conter os pointer para os síbolos
                                   //Por cada símbolo criado, o seu endereço é inserido em cada uma destas Plist
                                   //Não esquecer que depois para libertar o símbolo basta limpar numa das listas pois o símbolo é o mesmo
                                   Plist By_id[SYM_TAB_SIZE];      //for access by symnum
                                   Plist By_sym[SYM_TAB_SIZE];     //for access by string/arity
                                   

                                   
                public:                            
                                    GlobalSymbol();
                                    ~GlobalSymbol();
                                    
                                    void Free_Mem(void);
                                   
                                   friend class SymbolContainer;
                                   friend class LadrVGlobais;
                                   
};


class SymbolContainer {
                    
                    private:
                               
                                
                    public:
                              
                                SymbolContainer();
                                ~SymbolContainer();
                              
                                const string & true_sym(void);
                                const string & false_sym(void);
                                const string & and_sym(void);
                                const string & or_sym(void);
                                const string & not_sym(void);
                                const string & iff_sym(void);
                                const string & imp_sym(void);
                                const string & impby_sym(void);
                                const string & all_sym(void);
                                const string & exists_sym(void);
                                const string & quant_sym(void);
                                const string & attrib_sym(void);
                                const string & eq_sym(void);
                                const string & neq_sym(void);
                                
                                void set_operation_symbol(const string &,const string &);
                                const string & get_operation_symbol(const string &);
                                bool symbol_in_use(const string &);
                                Symbol get_symbol(void);
                                int new_symnum(void);
                                unsigned hash_sym(const string &, int);
                                unsigned hash_id(int);
                                Symbol lookup_by_id(int);
                                Symbol lookup_by_sym(const string &, int);
                                int str_to_sn(const string & , int);
                                const string& sn_to_str(int symnum); 
                                void fprint_syms(ostream &);
                                void p_syms(void);
                                void fprint_sym(ostream &, int);
                                void sprint_sym(String_buf, int);
                                void p_sym(int);
                                bool str_exists(const string &);
                                int  greatest_symnum(void); 
                                
                                bool is_symbol(int, const string &, int);
                                int sn_to_arity(int);
                                int sn_to_occurrences(int);
                                void set_unfold_symbol(int);
                                bool is_unfold_symbol(int);
                                void declare_aux_symbols(Ilist);
                                void declare_aux_symbols(IlistContainer &);
                                string parse_type_to_str(ParseType);
                                void clear_parse_type_for_all_symbols(void);
                                
                                void clear_parse_type(const string &);
                                void check_diff_type_same_prec(const string &, int , ParseType);
                                void set_parse_type(const string & , int , ParseType);
                                bool binary_parse_type(const string & , int *precedence, ParseType *type);
                                bool unary_parse_type(const string &, int *, ParseType *);
                                int  special_parse_type(const string &);
                                void set_assoc_comm(const string &, bool);
                                void set_commutative(const string &, bool);
                                bool assoc_comm_symbols(void);
                                bool comm_symbols(void);
                                bool is_assoc_comm(int);
                                bool is_commutative(int);
                                bool is_eq_symbol(int);
                                int  not_symnum(void);
                                int  or_symnum(void);
                                void declare_base_symbols(void);
                                void set_variable_style(Variable_Style);
                                Variable_Style variable_style(void);
                                bool variable_name(const string &) ;
                                void zap_symbols(void);
                                
                                void symbol_for_variable(string &, int);
                                Ilist variable_symbols(Ilist);
                                Ilist remove_variable_symbols(Ilist);
                                void set_symbol_type(int , Symbol_Type);
                                Symbol_Type get_symbol_type(int);
                                bool function_symbol(int);
                                bool relation_symbol(int);
                                bool function_or_relation_symbol(int);
                                void declare_functions_and_relations(Ilist , Ilist);
                                int  function_or_relation_sn(const string &);
                                Ilist all_function_symbols(void);
                                Ilist all_relation_symbols(void);
                                void  set_lrpo_status(int, Lrpo_Status);
                                void  all_symbols_lrpo_status(Lrpo_Status);
                                Lrpo_Status sn_to_lrpo_status(int);
                                void set_kb_weight(int, int);
                                bool zero_wt_kb(void); 
                                int  sn_to_kb_wt(int);
                                
                                void print_kbo_weights(ostream &);
                                void set_skolem(int);
                                void skolem_check(bool);
                                bool skolem_ok(const string &, int);
                                int  next_skolem_symbol(int);
                                Ilist skolem_symbols(void);
                                bool  is_skolem(int);
                                void  skolem_reset(void);
                                void  decommission_skolem_symbols(void);
                                void  set_skolem_symbols(Ilist);
                                void  set_lex_val(int, int);
                                int   sn_to_lex_val(int);
                                OrderType sym_precedence(int, int);
                                Ilist syms_with_lex_val(void);
                                bool exists_preliminary_precedence(Symbol_Type);
                                
                                Ilist current_fsym_precedence(void);
                                Ilist remove_syms_without_lex_val(Ilist);
                                Ilist sort_by_lex_val(Ilist);
                                Ilist insert_by_lex_val(Ilist, Ilist); 
                                
                                
                                static OrderType preliminary_lex_compare(Symbol, Symbol);
                                static OrderType lex_compare_base(Symbol, Symbol);
                                static OrderType lex_compare_arity_0123(Symbol, Symbol);
                                static OrderType lex_compare_arity_0213(Symbol, Symbol);
                                
                                void lex_order(Ilist fsyms, Ilist rsyms,I2list fsyms_multiset, I2list rsyms_multiset,  OrderType (*comp_proc) (Symbol, Symbol));

                                

                                
                                void add_skolems_to_preliminary_precedence(void);
                                Ilist skolem_insert(Ilist, int);
                                Ilist current_rsym_precedence(void);
                                Ilist not_in_preliminary_precedence(Ilist syms, Symbol_Type type);
                                void  print_fsym_precedence(ostream &);
                                void print_rsym_precedence(ostream &);
                                int min_lex_val(void);
                                int max_lex_val(void);
                                bool has_greatest_precedence(int);
                                void lex_insert_after_initial_constants(Ilist);
                                int fresh_symbol(const string &, int);
                                int gen_new_symbol(const string &, int, Ilist);
                                void mark_for_new_symbols(void);
                                I2list new_symbols_since_mark(void);
                                void add_new_symbols(I2list);
                                void new_constant_properties(int);
                                Ilist collect_multiples(Ilist);
                                Ilist arity_check(Ilist, Ilist); 
                                int   symbol_with_string(Ilist, const string &); 
                                void process_skolem_list(Plist, Ilist);
                                void process_lex_list(Plist, Ilist, Symbol_Type);
                                Ilist symnums_of_arity(Ilist, int);
                                void assign_greatest_precedence(int symnum);
                                void free_symbol(Symbol);
                                

};




#endif
