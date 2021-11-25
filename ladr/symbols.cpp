#include "fatal.h"

#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "mystring.h"
#include "ladrvglobais.h"
#include <climits>





GlobalSymbol::GlobalSymbol() {
   
    True_sym                       ="$T";  
    False_sym                      ="$F";
    And_sym                        ="&";
    Or_sym                         ="|";
    Not_sym                        ="-";
    Iff_sym                        ="<->";
    Imp_sym                        ="->";
    Impby_sym                      ="<-";
    All_sym                        ="all";
    Exists_sym                     ="exists";
    Quant_sym                      ="$quantified";
      /* Other symbols when in Term form */
    Attrib_sym                     ="#";    /* operator for attaching attributes */
    Eq_sym                         ="=";    /* for equality inference rules */
    Neq_sym                        ="!=";   /* abbreviation for negation of Eq_sym */
    Null_sym=                      ""; 


    Assoc_comm_symbols=0;  /* number declared */
    Symbol_count=0;
    Comm_symbols=0;  
    Eq_symnum=0;  
    Or_symnum=0;
    Not_symnum=0;
    Var_style=Variable_Style::STANDARD_STYLE;
    Zero_wt_kb=false;

    Skolem_constant_prefix="c";
    Skolem_function_prefix="f";

    Next_skolem_constant=1;      /* counter for c1, c2, ... */
    Next_skolem_function=1;      /* counter for f1, f2, ... */

    Skolem_check=true;  /* make sure Skolem symbols are unique */
    Mark_for_new_symbols=0;

    
    for (int i=0; i<SYM_TAB_SIZE;i++) {
        By_id[i]=NULL;
        By_sym[i]=NULL;
    
    }  
    
    
    
}


GlobalSymbol::~GlobalSymbol() {

}

void GlobalSymbol::Free_Mem(void) {
    PlistContainer P; 
    SymbolContainer S;
   
   for (int i=0; i<SYM_TAB_SIZE;i++) {
       Plist ps;
       Symbol s;
       
       for(ps=By_sym[i]; ps; ps=ps->next) {
            s=(Symbol) ps->v;
            S.free_symbol(s);
        }
        
        P.set_head(By_sym[i]);
        P.zap_plist();
        
        
        Plist pi;
        for(pi=By_id[i]; pi; pi=pi->next) {
            s=(Symbol) pi->v;
           // S.free_symbol(s); //basta limpar o símbolo numa das listas, visto que o mesmo é inserido nas duas
        }
        
        P.set_head(By_id[i]);
        P.zap_plist();
    } 
    
    Symbol_count=0;
}











SymbolContainer::SymbolContainer(){
}

//O destrutor tem de limpar as string name criadas dinamicamente
SymbolContainer::~SymbolContainer(){
}




const string & SymbolContainer::true_sym(void) {
    return LADR_GLOBAL_SYMBOL.True_sym;
}

const string & SymbolContainer::false_sym(void) {
    return LADR_GLOBAL_SYMBOL.False_sym;
}

const string & SymbolContainer::and_sym(void) {
    return LADR_GLOBAL_SYMBOL.And_sym;
}

const string & SymbolContainer::or_sym(void){
    return LADR_GLOBAL_SYMBOL.Or_sym;
}

const string & SymbolContainer::not_sym(void){
    return LADR_GLOBAL_SYMBOL.Not_sym;
}

const string & SymbolContainer::iff_sym(void){
    return LADR_GLOBAL_SYMBOL.Iff_sym;
}

const string & SymbolContainer::imp_sym(void) {
    return LADR_GLOBAL_SYMBOL.Imp_sym;
}

const string & SymbolContainer::impby_sym(void){
    return LADR_GLOBAL_SYMBOL.Impby_sym;
}

const string & SymbolContainer::all_sym(void){
    return LADR_GLOBAL_SYMBOL.All_sym;
}

const string & SymbolContainer::exists_sym(void) {
    return LADR_GLOBAL_SYMBOL.Exists_sym;
}

const string & SymbolContainer::quant_sym(void) {
    return LADR_GLOBAL_SYMBOL.Quant_sym;
}

const string & SymbolContainer::attrib_sym(void) {
    return LADR_GLOBAL_SYMBOL.Attrib_sym;
}

const string & SymbolContainer::eq_sym(void) {
    return LADR_GLOBAL_SYMBOL.Eq_sym;
}

const string & SymbolContainer::neq_sym(void){
    return LADR_GLOBAL_SYMBOL.Neq_sym;
}


void SymbolContainer::assign_greatest_precedence(int symnum) {
  set_lex_val(symnum, max_lex_val() + 1);
}  /* assign_greatest_precedence */


void SymbolContainer::set_operation_symbol(const string &operation, const string &symbol) {
    if (     myString::str_ident(operation, "true"))                          LADR_GLOBAL_SYMBOL.True_sym = symbol;
    else if (myString::str_ident(operation, "false"))                         LADR_GLOBAL_SYMBOL.False_sym = symbol;
    else if (myString::str_ident(operation, "conjunction"))                   LADR_GLOBAL_SYMBOL.And_sym = symbol;
    else if (myString::str_ident(operation, "disjunction"))                   LADR_GLOBAL_SYMBOL.Or_sym = symbol;
    else if (myString::str_ident(operation, "negation"))                      LADR_GLOBAL_SYMBOL.Not_sym = symbol;
    else if (myString::str_ident(operation, "implication"))                   LADR_GLOBAL_SYMBOL.Imp_sym = symbol;
    else if (myString::str_ident(operation, "backward_implication"))          LADR_GLOBAL_SYMBOL.Impby_sym = symbol;
    else if (myString::str_ident(operation, "equivalence"))                   LADR_GLOBAL_SYMBOL.Iff_sym = symbol;
    else if (myString::str_ident(operation, "universal_quantification"))      LADR_GLOBAL_SYMBOL.All_sym = symbol;
    else if (myString::str_ident(operation, "existential_quantification"))    LADR_GLOBAL_SYMBOL.Exists_sym = symbol;
    else if (myString::str_ident(operation, "quantification"))                LADR_GLOBAL_SYMBOL.Quant_sym = symbol;
    else if (myString::str_ident(operation, "attribute"))                     LADR_GLOBAL_SYMBOL.Attrib_sym = symbol;
    else if (myString::str_ident(operation, "equality"))                      LADR_GLOBAL_SYMBOL.Eq_sym = symbol;
    else if (myString::str_ident(operation, "negated_equality"))              LADR_GLOBAL_SYMBOL.Neq_sym = symbol;
    else {
            cout<<"The unknown operation is "<<operation<<endl;
            fatal::fatal_error("set_operation_symbol, unknown operation");
  }
    
}

const string &SymbolContainer::get_operation_symbol(const string &operation) {
  if (     myString::str_ident(operation, "true"))                                        return LADR_GLOBAL_SYMBOL.True_sym;
  else if (myString::str_ident(operation, "false"))                                       return LADR_GLOBAL_SYMBOL.False_sym;
  else if (myString::str_ident(operation, "conjunction"))                                 return LADR_GLOBAL_SYMBOL.And_sym;
  else if (myString::str_ident(operation, "disjunction"))                                 return LADR_GLOBAL_SYMBOL.Or_sym;
  else if (myString::str_ident(operation, "negation"))                                    return LADR_GLOBAL_SYMBOL.Not_sym;
  else if (myString::str_ident(operation, "implication"))                                 return LADR_GLOBAL_SYMBOL.Imp_sym;
  else if (myString::str_ident(operation, "backward_implication"))                        return LADR_GLOBAL_SYMBOL.Impby_sym;
  else if (myString::str_ident(operation, "equivalence"))                                 return LADR_GLOBAL_SYMBOL.Iff_sym;
  else if (myString::str_ident(operation, "universal_quantification"))                    return LADR_GLOBAL_SYMBOL.All_sym;
  else if (myString::str_ident(operation, "existential_quantification"))                  return LADR_GLOBAL_SYMBOL.Exists_sym;
  else if (myString::str_ident(operation, "quantification"))                              return LADR_GLOBAL_SYMBOL.Quant_sym;
  else if (myString::str_ident(operation, "attribute"))                                   return LADR_GLOBAL_SYMBOL.Attrib_sym;
  else if (myString::str_ident(operation, "equality"))                                    return LADR_GLOBAL_SYMBOL.Eq_sym;
  else if (myString::str_ident(operation, "negated_equality"))                            return LADR_GLOBAL_SYMBOL.Neq_sym;
  else {
        cout<<"The unknown operation is "<<operation<<endl;
        fatal::fatal_error("get_operation_symbol, unknown operation");
        return LADR_GLOBAL_SYMBOL.Null_sym;
  }

}


bool SymbolContainer::symbol_in_use(const string &str){
  if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.True_sym))                               return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.False_sym))                         return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.And_sym))                           return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Or_sym))                            return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Not_sym))                           return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Imp_sym))                           return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Impby_sym))                         return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Iff_sym))                           return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.All_sym))                           return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Exists_sym))                        return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Quant_sym))                         return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Attrib_sym))                        return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Eq_sym))                            return true;
  else if (myString::str_ident(str, LADR_GLOBAL_SYMBOL.Neq_sym))                           return true;
  else return false;
}


Symbol SymbolContainer::get_symbol(void)
{
  Symbol p = (Symbol) malloc(sizeof(symbol));  
  p->name=NULL;
  p->symnum = 0;
  p->parse_type = ParseType::NOTHING_SPECIAL;
  p->parse_prec = 0;
  p->arity = -1;
  p->unif_theory = Unify_Theory::EMPTY_THEORY;
  p->occurrences = -1;
  p->lex_val = INT_MAX;
  p->lrpo_status = Lrpo_Status::LRPO_LR_STATUS;
  p->kb_weight = 1;
  p->type = Symbol_Type::UNSPECIFIED_SYMBOL;
  p->skolem = false;
  p->unfold = false;
  p->auxiliary = false;
  
  return(p);
}

void SymbolContainer::free_symbol(Symbol s) {

 if(s->name) {
            delete s->name;
            s->name=NULL;
}
  free(s);
}


int SymbolContainer::new_symnum(void) 
{
  LADR_GLOBAL_SYMBOL.Symbol_count++;
  return(LADR_GLOBAL_SYMBOL.Symbol_count);
} 

unsigned SymbolContainer::hash_sym(const string &s, int arity)
{
  unsigned x = arity;
  unsigned i=0;
  while (i<s.length()) {
    unsigned c = s.at(i);
    x = (x << 4) | c;
    i++;
  }
  return abs((int)x) % SYM_TAB_SIZE;
} 


unsigned SymbolContainer::hash_id(int id)
{
  return abs(id) % SYM_TAB_SIZE;
}


Symbol SymbolContainer::lookup_by_id(int symnum) {
  
  for (Plist p=LADR_GLOBAL_SYMBOL.By_id[hash_id(symnum)]; p ; p = p->next) {
                             Symbol s = (Symbol) p->v;
                             if (s->symnum == symnum) return s;
  }
  return NULL;
}


Symbol SymbolContainer::lookup_by_sym(const string &name, int arity)
{
 unsigned i=hash_sym(name,arity);
  for (Plist p = LADR_GLOBAL_SYMBOL.By_sym[i]; p; p = p->next) {
                Symbol s = (Symbol) p->v;
                if (s->arity == arity && myString::str_ident(*s->name, name))  return s;
  }
  return NULL;
} 



int SymbolContainer::str_to_sn(const string &str, int arity) {
    PlistContainer P;
    Symbol s = lookup_by_sym(str,arity);
    
    if(s==NULL) {
        s=get_symbol();
        s->name= new string(str);
        s->arity=arity;
        s->symnum=new_symnum();
        unsigned  hashval_id = hash_id(s->symnum);
        unsigned hashval_sym = hash_sym(str, arity);

         if (hashval_id>SYM_TAB_SIZE || hashval_sym>SYM_TAB_SIZE) {
            fatal::fatal_error("str_to_sn :  hashval_id/hashval_sym bigger than SYM_TAB_SIZE");
            return -1;
        }
        
        P.set_head(LADR_GLOBAL_SYMBOL.By_sym[hashval_sym]);
        LADR_GLOBAL_SYMBOL.By_sym[hashval_sym]=P.plist_prepend((void *)s);
    
        P.set_head(LADR_GLOBAL_SYMBOL.By_id[hashval_id]);
        LADR_GLOBAL_SYMBOL.By_id[hashval_id]=P.plist_prepend((void *)s);
    }
    return(s->symnum);    
}

void SymbolContainer::fprint_syms(ostream &o) {
    
    for (int i=0; i<SYM_TAB_SIZE; i++) {
        Plist p;
        for (p=LADR_GLOBAL_SYMBOL.By_id[i]; p ; p=p->next) {
            Symbol s = (Symbol) p->v;
            o<<s->symnum<<"  "<<*s->name<<"  "<<s->arity<<"  "<<(s->type==Symbol_Type::FUNCTION_SYMBOL? "Function":(s->type==Symbol_Type::PREDICATE_SYMBOL ? "Relation":"NOTHING"))<<"  "<<s->lex_val<<"  "<<s->kb_weight<<endl;
        }
    }
}

void SymbolContainer::p_syms(){
    fprint_syms(cout);
}

const string & SymbolContainer::sn_to_str(int symnum) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)    
      return myString::null_string();
  else 
        return (*p->name);
}

void SymbolContainer::fprint_sym(ostream &o, int symnum)
{
  o<<sn_to_str(symnum);
}


void SymbolContainer::sprint_sym(String_buf sb, int symnum) {
    StrbufContainer SB;
    SB.set_string_buf(sb);
    SB.sb_append(sn_to_str(symnum));
    sb=SB.get_string_buf();
}


void SymbolContainer::p_sym(int symnum)
{
  fprint_sym(cout, symnum);
} 


bool SymbolContainer:: str_exists(const string &str) {
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s = (Symbol) p->v;
      if (myString::str_ident(str, *(s->name))) return true;
    }
  }
  return false;
}

int SymbolContainer::greatest_symnum(void) 
{
  return LADR_GLOBAL_SYMBOL.Symbol_count;
}

bool SymbolContainer::is_symbol(int symnum, const string & str, int arity)
{
  Symbol n = lookup_by_id(symnum);
  if (n == NULL)  return false;
  else  return (n->arity == arity && myString::str_ident(*n->name, str));
}


int SymbolContainer::sn_to_arity(int symnum){
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)  return(-1);
  else return(p->arity);
} 

int SymbolContainer::sn_to_occurrences(int symnum) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL) return(-1);
  else return(p->occurrences);
}

void SymbolContainer::set_unfold_symbol(int symnum) {
  Symbol p = lookup_by_id(symnum);
  p->unfold = true;
}

bool SymbolContainer::is_unfold_symbol(int symnum){
  Symbol p = lookup_by_id(symnum);
  return p->unfold;
}

void SymbolContainer::declare_aux_symbols(Ilist syms) {
  Ilist p;
  for (p = syms; p; p = p->next) {
    Symbol s = lookup_by_id(p->i);
    s->auxiliary = true;
  }
} 

void SymbolContainer::declare_aux_symbols(IlistContainer &l) {
	declare_aux_symbols(l.get_head());
}

string SymbolContainer::parse_type_to_str(ParseType type) {
  switch (type) {
  case ParseType::INFIX_LEFT: return "infix_left";
  case ParseType::INFIX_RIGHT: return "infix_right";
  case ParseType::INFIX: return "infix";
  case ParseType::PREFIX: return "prefix";
  case ParseType::PREFIX_PAREN: return "prefix_paren";
  case ParseType::POSTFIX: return "postfix";
  case ParseType::POSTFIX_PAREN: return "postfix_paren";
  case ParseType::NOTHING_SPECIAL: return "ordinary";
  }
  return "???";
}


void SymbolContainer::clear_parse_type_for_all_symbols(void){
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s = (Symbol) p->v;
      s->parse_type = ParseType::NOTHING_SPECIAL;
      s->parse_prec = 0;
    }
  }
}

void SymbolContainer::clear_parse_type(const string &str){
  Symbol p;
  p = lookup_by_sym(str, 1);
  if (p != NULL) {
    p->parse_type = ParseType::NOTHING_SPECIAL;
    p->parse_prec = 0;
  }
  p = lookup_by_sym(str, 2);
  if (p != NULL) {
    p->parse_type = ParseType::NOTHING_SPECIAL;
    p->parse_prec = 0;
  }
} 


void SymbolContainer::check_diff_type_same_prec(const string &str, int prec, ParseType type) {
  if (type != ParseType::NOTHING_SPECIAL) {
    int i;
    for (i = 0; i < SYM_TAB_SIZE; i++) {
      Plist p;
      for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
			Symbol s = (Symbol) p->v;
			ParseType type2 = s->parse_type;
			int prec2 = s->parse_prec;
			string name = *s->name;
			if (type2 != type && prec == prec2 && !myString::str_ident(name, str)) {
					cout <<endl<<"Conflicting declarations (the first may be built in):"<<endl;
					cout <<"  op(" <<prec2 <<",  " <<parse_type_to_str(type2)<<",|"<<name<<"|"<<endl;
					cout <<"  op(" <<prec  <<",  " <<parse_type_to_str(type)<<", |"<<str<<"|"<<endl;
					fatal::fatal_error("cannot declare different parse types with same precedence (see output file)");
			}
     }
    }
  }
} 


void SymbolContainer::set_parse_type(const string &str , int precedence, ParseType type) {
  if (precedence < MIN_PRECEDENCE || precedence > MAX_PRECEDENCE) fatal::fatal_error("set_parse_type: precedence out of range");
  else {
		Symbol p = NULL;
		clear_parse_type(str);  /* in case it has parse type of diff. arity */
		check_diff_type_same_prec(str, precedence, type);
		switch (type) {
						case ParseType::INFIX_LEFT:
						case ParseType::INFIX_RIGHT:
						case ParseType::INFIX:
												p = lookup_by_id(str_to_sn(str, 2));
												p->parse_type = type;
												p->parse_prec = precedence;
                                            break;
						case ParseType::PREFIX:
						case ParseType::PREFIX_PAREN:
						case ParseType::POSTFIX:
						case ParseType::POSTFIX_PAREN:
												p = lookup_by_id(str_to_sn(str, 1));
												p->parse_type = type;
												p->parse_prec = precedence;
                                            break;
						case ParseType::NOTHING_SPECIAL:
															/* already cleared above */
                                            break;
    }
  }
}

bool SymbolContainer::binary_parse_type(const string &str , int *precedence, ParseType *type) {
  Symbol p = lookup_by_sym(str, 2);
  if (p == NULL || p->parse_type == ParseType::NOTHING_SPECIAL)  return false;
  else {
		*precedence = p->parse_prec;
		*type = p->parse_type;
		return true;
  }
} 

bool SymbolContainer::unary_parse_type(const string &str, int *precedence, ParseType *type) {
  Symbol p = lookup_by_sym(str, 1);
  if (p == NULL || p->parse_type == ParseType::NOTHING_SPECIAL)  return false;
  else {
		*precedence = p->parse_prec;
		*type = p->parse_type;
		return true;
  }
}

int SymbolContainer::special_parse_type(const string &str){
  int prec;
  ParseType type;
  if (binary_parse_type(str, &prec, &type)) return 2;
  else if (unary_parse_type(str, &prec, &type)) return 1;
  else return -1;
}

void SymbolContainer::set_assoc_comm(const string &str, bool set) {
  int sn = str_to_sn(str, 2);
  Symbol p = lookup_by_id(sn);
  if (set) {
    p->unif_theory = Unify_Theory::ASSOC_COMMUTE;
    LADR_GLOBAL_SYMBOL.Assoc_comm_symbols++;
  }
  else {
    p->unif_theory = Unify_Theory::EMPTY_THEORY;
    LADR_GLOBAL_SYMBOL.Assoc_comm_symbols--;
  }
}


void SymbolContainer::set_commutative(const string &str, bool set){
  int sn = str_to_sn(str, 2);
  Symbol p = lookup_by_id(sn);
  if (set) {
    p->unif_theory = Unify_Theory::COMMUTE;
    LADR_GLOBAL_SYMBOL.Comm_symbols++;
  }
  else {
    p->unif_theory = Unify_Theory::EMPTY_THEORY;
    LADR_GLOBAL_SYMBOL.Comm_symbols--;
  }
}

bool SymbolContainer::assoc_comm_symbols(void) {
  return LADR_GLOBAL_SYMBOL.Assoc_comm_symbols != 0;
} 

bool SymbolContainer::comm_symbols(void) {
  return LADR_GLOBAL_SYMBOL.Comm_symbols != 0;
} 


bool SymbolContainer::is_assoc_comm(int sn) {
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? 0 : p->unif_theory == Unify_Theory::ASSOC_COMMUTE);
}  

bool SymbolContainer::is_commutative(int sn) {
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? 0 : p->unif_theory == Unify_Theory::COMMUTE);
}

bool SymbolContainer:: is_eq_symbol(int symnum) {
  if (LADR_GLOBAL_SYMBOL.Eq_symnum == 0) 	LADR_GLOBAL_SYMBOL.Eq_symnum = str_to_sn(eq_sym(), 2);
  return (symnum == LADR_GLOBAL_SYMBOL.Eq_symnum ? true : false);
}

int SymbolContainer::not_symnum(void) {
  if (LADR_GLOBAL_SYMBOL.Not_symnum == 0) LADR_GLOBAL_SYMBOL.Not_symnum = str_to_sn(not_sym(), 1);
  return LADR_GLOBAL_SYMBOL.Not_symnum;
}

int SymbolContainer::or_symnum(void) {
  if (LADR_GLOBAL_SYMBOL.Or_symnum == 0) LADR_GLOBAL_SYMBOL.Or_symnum = str_to_sn(or_sym(), 2);
  return LADR_GLOBAL_SYMBOL.Or_symnum;
}

void SymbolContainer::declare_base_symbols(void){
  int sn;
  sn = str_to_sn(false_sym(), 0);
  sn = str_to_sn(true_sym(), 0);
  sn = str_to_sn("false", 0);
  sn = str_to_sn("true", 0);
}

void SymbolContainer::set_variable_style(Variable_Style style) {
  LADR_GLOBAL_SYMBOL.Var_style = style;
} 

Variable_Style SymbolContainer::variable_style(void) {
  return LADR_GLOBAL_SYMBOL.Var_style;
}



bool SymbolContainer:: variable_name(const string &s) {
  if (variable_style() == Variable_Style::PROLOG_STYLE) return (s.at(0) >= 'A' && s.at(0) <= 'Z');
  else if (variable_style() == Variable_Style::INTEGER_STYLE)  return (s.at(0) >= '0' && s.at(0) <= '9');
  else  return (s.at(0) >= 'u' && s.at(0) <= 'z');
} 



void SymbolContainer::symbol_for_variable(string &str, int varnum) {
  unsigned char c;
  str.clear();
  if (variable_style() == Variable_Style::INTEGER_STYLE)
    /* 0,1,2,3,4,5,6,7,... */
    str=to_string(varnum);
    else if (variable_style() == Variable_Style::PROLOG_STYLE) {
    /* A,B,C,D,E,F,V6,V7,V8,... */
    if (varnum < 6) {
                        c='A'+  varnum;
                        str+=c;
    }
    else
        str="V"+to_string(varnum);
    } //prolog style
    
    else {
    /* x,y,z,u,w,v5,v6,v7,v8,... */
    if (varnum < 3) {
                        c='x'+varnum;
                        str+=c;
    }
    else if (varnum == 3) str="u";
    else if (varnum == 4) str="w";
    else str="v"+to_string(varnum);
  }
}

//retorna a lista com as variáveis existentes em syms
Ilist SymbolContainer::variable_symbols(Ilist syms) 
{
  if (syms == NULL) return NULL;
  else {
        Ilist work = variable_symbols(syms->next);
        IlistContainer w;
        w.set_head(work);
        if (sn_to_arity(syms->i) == 0 && variable_name(sn_to_str(syms->i))) work = w.ilist_prepend(syms->i);
        return work;
  }
}


Ilist SymbolContainer::remove_variable_symbols(Ilist syms) {
  Ilist vars = variable_symbols(syms); //vars tem as variáveis existentes em syms
  IlistContainer Vars(vars);
  IlistContainer Syms(syms);
  
  Ilist result = Syms.ilist_subtract(Vars);  //o resultado é a Ilist resultante da subtração em Syms de Vars, ou seja os elementos de Syms que não estão em Vars
  Syms.zap_ilist();
  Vars.zap_ilist();
  return result;
}

void SymbolContainer::set_symbol_type(int symnum, Symbol_Type type) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL) fatal::fatal_error("set_symbol_type: bad symnum");
  p->type = type;
}  

Symbol_Type SymbolContainer::get_symbol_type(int symnum) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL) fatal::fatal_error("get_symbol_type: bad symnum");
  return p->type;
}


bool SymbolContainer::function_symbol(int symnum) {
  return get_symbol_type(symnum) == Symbol_Type::FUNCTION_SYMBOL;
}

bool SymbolContainer::relation_symbol(int symnum) {
  return get_symbol_type(symnum) == Symbol_Type::PREDICATE_SYMBOL;
}

bool SymbolContainer::function_or_relation_symbol(int symnum)
{
  Symbol_Type t = get_symbol_type(symnum);
  return t == Symbol_Type::PREDICATE_SYMBOL || t == Symbol_Type::FUNCTION_SYMBOL;
}



void SymbolContainer::declare_functions_and_relations(Ilist fsyms, Ilist rsyms)
{
  Ilist p;
  if (fsyms!=NULL)
      for (p = fsyms; p; p = p->next) set_symbol_type(p->i,Symbol_Type::FUNCTION_SYMBOL);
  if(rsyms!=NULL)
    for (p = rsyms; p; p = p->next) set_symbol_type(p->i,Symbol_Type::PREDICATE_SYMBOL);
}



int SymbolContainer::function_or_relation_sn(const string &str){
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s = (Symbol) p->v;
      if ((s->type == Symbol_Type::FUNCTION_SYMBOL || s->type ==Symbol_Type::PREDICATE_SYMBOL) && myString::str_ident(str, *s->name))
      return s->symnum;
    }
  }
  return -1;
}



Ilist SymbolContainer::all_function_symbols(void) {
  IlistContainer syms;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s =(Symbol) p->v;
      if (s->type == Symbol_Type::FUNCTION_SYMBOL) 
          syms.ilist_append(s->symnum);
    }
  }
  return syms.get_head();
}


Ilist SymbolContainer::all_relation_symbols(void) {
  IlistContainer syms;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s = (Symbol )p->v;
      if (s->type == Symbol_Type::PREDICATE_SYMBOL) syms.ilist_append(s->symnum);
    }
  }
  return syms.get_head();
}


void SymbolContainer::set_lrpo_status(int symnum, Lrpo_Status status)
{
  Symbol p = lookup_by_id(symnum);
  p->lrpo_status = status;
} 

void SymbolContainer::all_symbols_lrpo_status(Lrpo_Status status){
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s =(Symbol) p->v;
      s->lrpo_status = status;
    }
  }
}

Lrpo_Status SymbolContainer::sn_to_lrpo_status(int sn) {
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? Lrpo_Status(0) : p->lrpo_status);
}

void SymbolContainer::set_kb_weight(int symnum, int weight) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL) {
    fatal::fatal_error("set_kb_weight, symbol not found");
  }
  if (weight == 0) {
    if (LADR_GLOBAL_SYMBOL.Zero_wt_kb)
      fatal::fatal_error("set_kb_weight, more than one symbol of weight 0");
    else if (p->arity != 1 || p->type != Symbol_Type::FUNCTION_SYMBOL)  fatal::fatal_error("set_kb_weight, weight 0 symbols must be unary function symbols");
    else LADR_GLOBAL_SYMBOL.Zero_wt_kb = true;
  }
  p->kb_weight = weight;
} 

bool SymbolContainer::zero_wt_kb(void) {
  return LADR_GLOBAL_SYMBOL.Zero_wt_kb;
} 

int SymbolContainer::sn_to_kb_wt(int symnum) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)    return(-1);
  else  return(p->kb_weight);
} 


Ilist SymbolContainer::remove_syms_without_lex_val(Ilist syms) {
  if (syms == NULL)    return NULL;
  else {
        syms->next = remove_syms_without_lex_val(syms->next);
        if (sn_to_lex_val(syms->i) == INT_MAX) {
            Ilist rest = syms->next;
            IlistContainer Syms;
            Syms.free_ilist(syms);
            return rest;
    }
    else return syms;
  }
}


Ilist SymbolContainer::insert_by_lex_val(Ilist head, Ilist tail) {
  if (tail == NULL) {
    head->next = NULL;
    return head;
  }
  else if (sn_to_lex_val(head->i) < sn_to_lex_val(tail->i)) {
    head->next = tail;
    return head;
  }
  else {
    tail->next = insert_by_lex_val(head, tail->next);
    return tail;
  }
} 

Ilist SymbolContainer::sort_by_lex_val(Ilist p) {
  if (p == NULL) return NULL;
  else {
            return insert_by_lex_val(p, sort_by_lex_val(p->next));
  }
} 


Ilist SymbolContainer::current_fsym_precedence(void) {
  SymbolContainer S;
  Ilist syms = all_function_symbols();
  syms = remove_syms_without_lex_val(syms);
  syms = sort_by_lex_val(syms);
  return syms;
} 

void SymbolContainer::print_kbo_weights(ostream &o) {
  IlistContainer Fsyms(current_fsym_precedence());
  
  Ilist p;
  o<<"Function symbol KB weights: ";
  for (p = Fsyms.get_head(); p; p = p->next)
    o<<" "<<sn_to_str(p->i)<<"="<<sn_to_kb_wt(p->i)<<".";
  o<<endl;
  Fsyms.zap_ilist();
}


void SymbolContainer::set_skolem(int symnum) {
  Symbol p = lookup_by_id(symnum);
  p->skolem = true;
  p->type = Symbol_Type::FUNCTION_SYMBOL;
}


void SymbolContainer::skolem_check(bool flag) {
  LADR_GLOBAL_SYMBOL.Skolem_check = flag;
}

bool SymbolContainer::skolem_ok(const string &name, int arity) {
  if (!LADR_GLOBAL_SYMBOL.Skolem_check)    return true;
  else {
    Symbol s = lookup_by_sym(name, arity);
    if (s == NULL) return true;
    else return s->auxiliary;
  }
}

int SymbolContainer::next_skolem_symbol(int arity){
  string name;
  do {
    if (arity == 0) {
        name=LADR_GLOBAL_SYMBOL.Skolem_constant_prefix+to_string(LADR_GLOBAL_SYMBOL.Next_skolem_constant);
        LADR_GLOBAL_SYMBOL.Next_skolem_constant++;
    }
    else {
        name=LADR_GLOBAL_SYMBOL.Skolem_function_prefix + to_string(LADR_GLOBAL_SYMBOL.Next_skolem_function);
        LADR_GLOBAL_SYMBOL.Next_skolem_function++;
    }
  } while (!skolem_ok(name,arity));

  {
    int symnum = str_to_sn(name, arity);
    set_skolem(symnum);
    return symnum;
  }
}

Ilist SymbolContainer::skolem_symbols(void){
  IlistContainer g;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s =(Symbol) p->v;
      if (s->skolem) g.ilist_prepend(s->symnum);
    }
  }
  g.reverse_ilist();
  return g.get_head();
}

bool SymbolContainer::is_skolem(int symnum) {
  Symbol p = lookup_by_id(symnum);
  return p->skolem;
}

void SymbolContainer::skolem_reset(void){
  LADR_GLOBAL_SYMBOL.Next_skolem_constant = 1;
  LADR_GLOBAL_SYMBOL.Next_skolem_function = 1;
}

void SymbolContainer::decommission_skolem_symbols(void){
  IlistContainer Fsyms(all_function_symbols());
  
  Ilist p;

  for (p = Fsyms.get_head(); p; p = p->next) {
    Symbol n = lookup_by_id(p->i);
    if (n->skolem) {
      n->skolem = false;
      n->type = Symbol_Type::UNSPECIFIED_SYMBOL;
    }
  }
  Fsyms.zap_ilist();
}

void SymbolContainer::set_skolem_symbols(Ilist symnums){
  Ilist p;
  for (p = symnums; p; p = p->next) {
    Symbol sym = lookup_by_id(p->i);
    if (sym == NULL) fatal::fatal_error("set_skolem_symbols, symbol not found");
    sym->skolem = true;
  }
}

void SymbolContainer::set_lex_val(int symnum, int lex_val) {
  Symbol p = lookup_by_id(symnum);
  if (p == NULL)  fatal::fatal_error("set_lex_val, invalid symnum");
   p->lex_val = lex_val;

  /* printf("set_lex_val %s/%d, %d\n", p->name, p->arity, lex_val); */
}

int SymbolContainer::sn_to_lex_val(int sn){
  Symbol p = lookup_by_id(sn);
  return (p == NULL ? INT_MIN : p->lex_val);
}


OrderType SymbolContainer::sym_precedence(int symnum_1, int symnum_2)
{
  int p1, p2;
  if (symnum_1 == symnum_2) return OrderType::SAME_AS;
  else {
        p1 = sn_to_lex_val(symnum_1);
        p2 = sn_to_lex_val(symnum_2);
        if (p1 == INT_MAX || p2 == INT_MAX) return OrderType::NOT_COMPARABLE;
        else if (p1 > p2)      return OrderType::GREATER_THAN;
        else if (p1 < p2)      return OrderType::LESS_THAN;
        else  return OrderType::SAME_AS;
  }
}


Ilist SymbolContainer::syms_with_lex_val(void){
  IlistContainer g;
  int i;
  for (i = 0; i < SYM_TAB_SIZE; i++) {
    Plist p;
    for (p = LADR_GLOBAL_SYMBOL.By_id[i]; p; p = p->next) {
      Symbol s =(Symbol) p->v;
      if (s->lex_val != INT_MAX) g.ilist_append(s->symnum);
    }
  }
  return g.get_head();
}

bool SymbolContainer::exists_preliminary_precedence(Symbol_Type type){
  if (type == Symbol_Type::FUNCTION_SYMBOL) return LADR_GLOBAL_SYMBOL.Preliminary_prec_func.get_head() != NULL;
  else if (type == Symbol_Type::PREDICATE_SYMBOL) return LADR_GLOBAL_SYMBOL.Preliminary_prec_pred.get_head() != NULL;
  else return false;
}


OrderType SymbolContainer::preliminary_lex_compare(Symbol a, Symbol b) {
  int ai = -1;
  int bi = -1;
//  IlistContainer PPF, PPP;
//  PPF.set_head(Preliminary_prec_func);
//  PPP.set_head(Preliminary_prec_pred);

  if (a->type != b->type)  return OrderType::NOT_COMPARABLE;
  else if (a->type == Symbol_Type::UNSPECIFIED_SYMBOL) return OrderType::NOT_COMPARABLE;
  else if (a->type == Symbol_Type::FUNCTION_SYMBOL) {
    ai = LADR_GLOBAL_SYMBOL.Preliminary_prec_func.position_in_ilist(a->symnum);
    bi = LADR_GLOBAL_SYMBOL.Preliminary_prec_func.position_in_ilist(b->symnum);
  }
  else if (a->type == Symbol_Type::PREDICATE_SYMBOL) {
    ai = LADR_GLOBAL_SYMBOL.Preliminary_prec_pred.position_in_ilist(a->symnum);
    bi = LADR_GLOBAL_SYMBOL.Preliminary_prec_pred.position_in_ilist(b->symnum);
  }
  /* printf("%s=%d, %s=%d\n", a, ai, b, bi); */
  if (ai == -1)  ai = INT_MIN;
  if (bi == -1)  bi = INT_MIN;

  if (ai < bi)    return OrderType::LESS_THAN;
  else if (ai > bi)   return OrderType::GREATER_THAN;
  else if (ai == INT_MIN)  return OrderType::NOT_COMPARABLE;  /*neither in preliminary_precedence */
  else  return OrderType::SAME_AS;
}

OrderType SymbolContainer::lex_compare_base(Symbol s1, Symbol s2) {
  SymbolContainer S;
    
  if (s1 == s2)           return OrderType::SAME_AS;
  else if (s1 == NULL)    return OrderType::LESS_THAN;
  else if (s2 == NULL)    return OrderType::GREATER_THAN;

  /* FUNCTION < RELATION < others (don't know if there can be others) */

  else if (s1->type == Symbol_Type::FUNCTION_SYMBOL && s2->type != Symbol_Type::FUNCTION_SYMBOL)    return OrderType::LESS_THAN;
  else if (s1->type != Symbol_Type::FUNCTION_SYMBOL && s2->type == Symbol_Type::FUNCTION_SYMBOL)    return OrderType::GREATER_THAN;

  else if (s1->type == Symbol_Type::PREDICATE_SYMBOL && s2->type != Symbol_Type::PREDICATE_SYMBOL)    return OrderType::LESS_THAN;
  else if (s1->type != Symbol_Type::PREDICATE_SYMBOL && s2->type == Symbol_Type::PREDICATE_SYMBOL)    return OrderType::GREATER_THAN;

  /* Now they have the same type (FUNCTION, RELATION, other). */
  /* Check for preliminary order (lex command). */

  else if (preliminary_lex_compare(s1, s2) == OrderType::LESS_THAN)       return OrderType::LESS_THAN;
  else if (preliminary_lex_compare(s1, s2) == OrderType::GREATER_THAN)    return OrderType::GREATER_THAN;

  /* = < other relations */

  else if (s1->type == Symbol_Type::PREDICATE_SYMBOL && S.is_eq_symbol(s2->symnum))    return OrderType::GREATER_THAN;
  else if (s1->type == Symbol_Type::PREDICATE_SYMBOL && S.is_eq_symbol(s1->symnum))    return OrderType::LESS_THAN;

  /* if arities same:
     (1) Skolems > non-Skolems
     (2) if both Skolems, use sumnum
     (3) more-occurrences < fewer-occurrences
     (4) Use UNIX's strcomp, which is lexical ascii ordering.
  */

  else if (s1->arity == s2->arity) {

    if (s1->skolem || s2->skolem) {
      if (!s2->skolem)	return OrderType::GREATER_THAN;
      else if (!s1->skolem)	return OrderType::LESS_THAN;
      else if (s1->symnum > s2->symnum)	return OrderType::GREATER_THAN;
      else if (s1->symnum < s2->symnum)	return OrderType::LESS_THAN;
      else	return OrderType::SAME_AS;
    }
    
    else if (s1->occurrences < s2->occurrences)      return OrderType::GREATER_THAN;
    else if (s1->occurrences > s2->occurrences)      return OrderType::LESS_THAN;
    else {
      
      int i = strcmp((*s1->name).c_str(), (*s2->name).c_str() );
      if (i < 0)	return OrderType::LESS_THAN;
      else if (i > 0)	return OrderType::GREATER_THAN;
      else	return OrderType::SAME_AS;
    }
  }

  /* the type is the same, but arities are different */

  else
    return OrderType::NOT_COMPARABLE;  /* code for "not yet decided" */
}  /* lex_compare_base */

OrderType SymbolContainer::lex_compare_arity_0123(Symbol s1, Symbol s2) {
  OrderType base = lex_compare_base(s1, s2);

  if (base != OrderType::NOT_COMPARABLE) return base;
  else
    /* symbols same type, but with different arities */
    return s1->arity < s2->arity ? OrderType::LESS_THAN : OrderType::GREATER_THAN;
} 


OrderType SymbolContainer::lex_compare_arity_0213(Symbol s1, Symbol s2) {
  OrderType base = lex_compare_base(s1, s2);

  if (base != OrderType::NOT_COMPARABLE)
    return base;
  else {
    /* Symbols same type, but with different arities.
       Relations: order by arity.
       Functions: constants < arity-2 < arity-1 < arity-3 < arity-4 ... .
    */

    if (s1->type == Symbol_Type::PREDICATE_SYMBOL) return s1->arity < s2->arity ? OrderType::LESS_THAN : OrderType::GREATER_THAN;
    else if (s1->arity == 1)   return s2->arity >= 3 ? OrderType::LESS_THAN : OrderType::GREATER_THAN;
    else if (s2->arity == 1)   return s1->arity < 3 ? OrderType::LESS_THAN : OrderType::GREATER_THAN;
    else   return s1->arity < s2->arity ? OrderType::LESS_THAN : OrderType::GREATER_THAN;
  }
} 


Ilist SymbolContainer::skolem_insert(Ilist prec, int i){
  IlistContainer Prec;
  Prec.set_head(prec);
  if (prec == NULL)  return Prec.ilist_append(i);
  else  if (sn_to_arity(prec->i) > sn_to_arity(i))  return Prec.ilist_prepend(i); 
  else {
        prec->next = skolem_insert(prec->next, i);
        return prec;
  }
} 

void SymbolContainer::add_skolems_to_preliminary_precedence(void){
    
    if (LADR_GLOBAL_SYMBOL.Preliminary_prec_func.get_head()!=NULL) {
        Ilist skolems = skolem_symbols();
        Ilist p;
    /* printf("Before adding skolems: "); p_ilist(Preliminary_precedence); */
    
        for (p = skolems; p; p = p->next) {
         if(!LADR_GLOBAL_SYMBOL.Preliminary_prec_func.ilist_member(p->i))
             LADR_GLOBAL_SYMBOL.Preliminary_prec_func.set_head(skolem_insert(LADR_GLOBAL_SYMBOL.Preliminary_prec_func.get_head(), p->i) );
        }
    /* printf("After adding skolems: "); p_ilist(Preliminary_precedence); */
    IlistContainer Skolems(skolems);
    Skolems.zap_ilist();
  }
} 




void SymbolContainer::lex_order(Ilist fsyms, Ilist rsyms,I2list fsyms_multiset, I2list rsyms_multiset,  OrderType (*comp_proc) (Symbol, Symbol))
{
  IlistContainer Fsyms;
  IlistContainer Rsyms;
  I2listContainer Fsysms_multiset;
  I2listContainer Rsysms_multiset;
  
  
  Fsyms.set_head(fsyms);
  Rsyms.set_head(rsyms);
  Fsysms_multiset.set_head(fsyms_multiset);
  Rsysms_multiset.set_head(rsyms_multiset);

  
  int n = Fsyms.ilist_count() + Rsyms.ilist_count();
  
  
  Symbol *a = (Symbol *) malloc(n * sizeof(void *));
  Ilist p;
  int i = 0;
  for (p = Fsyms.get_head(); p; p = p->next, i++) {
    a[i] = lookup_by_id(p->i);
    a[i]->occurrences = Fsysms_multiset.multiset_occurrences(p->i);
  }
  for (p = rsyms; p; p = p->next, i++) {
    a[i] = lookup_by_id(p->i);
    a[i]->occurrences = Rsysms_multiset.multiset_occurrences(p->i);
  }
  add_skolems_to_preliminary_precedence();
  myOrder::merge_sort((void **) a, n, (OrderType (*)(void*, void*)) comp_proc);
  
  for (i = 0; i < n; i++)
    a[i]->lex_val = i;
  free(a);
}

Ilist SymbolContainer::current_rsym_precedence(void){
  Ilist syms = all_relation_symbols();
  syms = remove_syms_without_lex_val(syms);
  syms = sort_by_lex_val(syms);
  return syms;
}



Ilist SymbolContainer::not_in_preliminary_precedence(Ilist syms, Symbol_Type type){
  Ilist missing = NULL;
  Ilist p;
 
  IlistContainer M;

  for (p = syms; p; p = p->next) { //corre a lista syms
    if (type == Symbol_Type::FUNCTION_SYMBOL && LADR_GLOBAL_SYMBOL.Preliminary_prec_func.position_in_ilist(p->i) == -1) { //se não achou em PPF
        M.set_head(missing);
        missing=M.ilist_append(p->i);
    }
    else if (type == Symbol_Type::PREDICATE_SYMBOL &&   LADR_GLOBAL_SYMBOL.Preliminary_prec_pred.position_in_ilist(p->i) == -1) {
        M.set_head(missing);
        missing = M.ilist_append(p->i);
    }
  }
  return missing;
}

void SymbolContainer::print_fsym_precedence(ostream &o) {
  Ilist fsyms = current_fsym_precedence();
  Ilist p;
  cout<<"Function symbol precedence: function_order([";
  for (p = fsyms; p; p = p->next)
    o<< " "<<sn_to_str(p->i)<< (p->next? "," : "");
  cout<<" ])."<<endl;
  IlistContainer Fsyms(fsyms);
  Fsyms.zap_ilist();
}



void SymbolContainer::print_rsym_precedence(ostream &o) {
  Ilist rsyms = current_rsym_precedence();
  Ilist p;

  cout <<"Predicate symbol-precedence:  predicate_order([";
  for (p = rsyms; p; p = p->next)
    cout<<" "<<sn_to_str(p->i)<<(p->next ? "," : "");
  cout<<" ])."<<endl;
  IlistContainer Rsyms(rsyms);
  Rsyms.zap_ilist();
}


int SymbolContainer::min_lex_val(void) {
  Ilist a = syms_with_lex_val();
  Ilist p;
  int min = INT_MAX;
  for (p = a; p; p = p->next) {
    int x = sn_to_lex_val(p->i);
    min = IMIN(min, x);
  }
  IlistContainer A(a);
  A.zap_ilist();
  return min;
}


int SymbolContainer::max_lex_val(void) {
  Ilist a = syms_with_lex_val();
  Ilist p;
  int max = INT_MIN;
  for (p = a; p; p = p->next) {
    int x = sn_to_lex_val(p->i);
    max = IMAX(max, x);
  }
  IlistContainer A(a);
  A.zap_ilist();
  return max;
}


bool SymbolContainer::has_greatest_precedence(int symnum){
  return sn_to_lex_val(symnum) == max_lex_val();
}


void SymbolContainer::lex_insert_after_initial_constants(Ilist syms) {

  IlistContainer Syms;
  Syms.set_head(syms);
  if (syms) {
    Ilist all = current_fsym_precedence();
    Ilist a, s;
    int val = 1;

    for (a = all; a && sn_to_arity(a->i) == 0; a = a->next) {
      if (!Syms.ilist_member(a->i)) set_lex_val(a->i, val++);
    }

    syms = sort_by_lex_val(syms);  /* so that relative order is unchanged */

    for (s = syms; s; s = s->next)
      set_lex_val(s->i, val++);

    for (; a; a = a->next) {
      if (Syms.ilist_member(a->i)) set_lex_val(a->i, val++);
    }
  }
}


int SymbolContainer::fresh_symbol(const string &prefix, int arity) {
  string name;
  int i = 0;

  if (prefix.length() > MAX_NAME) {
    fatal::fatal_error("fresh_symbol, prefix is too big.");
  }

  do {
        name=prefix+to_string(i);
        i++;
  } while (str_exists(name));

  return str_to_sn(name, arity);
} 


int SymbolContainer::gen_new_symbol(const string &prefix, int arity, Ilist syms)
{
  
  string name;
  int symnum;
  int i = 0;
  IlistContainer Syms;
  Syms.set_head(syms);  
  
  
  if (prefix.length() > MAX_NAME)
    fatal::fatal_error("gen_new_symbol, prefix is too big.");
  name=prefix+to_string(i);
  symnum = str_to_sn(name, arity);

  while (Syms.ilist_member(symnum)) {
    i++;
    name=prefix+to_string(i);
    symnum = str_to_sn(name, arity);
  }

  return symnum;
} 


void SymbolContainer::mark_for_new_symbols(void)
{
  LADR_GLOBAL_SYMBOL.Mark_for_new_symbols = LADR_GLOBAL_SYMBOL.Symbol_count + 1;  /* next symnum */
} 


I2list SymbolContainer::new_symbols_since_mark(void) {
  I2list p = NULL;
  I2listContainer P;
  P.set_head(p);
  int sn;
  for (sn = LADR_GLOBAL_SYMBOL.Mark_for_new_symbols; sn <= LADR_GLOBAL_SYMBOL.Symbol_count; sn++) {
    if (function_or_relation_symbol(sn)) {
      P.i2list_append(sn, sn_to_arity(sn));
    }
  }
  return P.get_head();
}



void SymbolContainer::add_new_symbols(I2list syms) {
  I2list p;
  
  for (p = syms; p; p = p->next) {
    int sn;
    int symnum = p->i;
    int arity = p->j;
    if (symnum != LADR_GLOBAL_SYMBOL.Symbol_count+1) fatal::fatal_error("add_new_symbols, bad symnum");
    sn = fresh_symbol("child_symbol_", arity);
    if (sn != symnum) fatal::fatal_error("add_new_symbols, symnums do not match");
  }
} 


void SymbolContainer::new_constant_properties(int sn) {
  Symbol s = lookup_by_id(sn);

  if (s == NULL || s->arity != 0) fatal::fatal_error("new_constant_properties, bad symbol number");
    s->type = Symbol_Type::FUNCTION_SYMBOL;
    s->kb_weight = 1;
  
  {
    IlistContainer Syms;
    Syms.ilist_append(sn);
    lex_insert_after_initial_constants(Syms.get_head());
    Syms.zap_ilist();
  }
} 

Ilist SymbolContainer::collect_multiples(Ilist syms) {
  /* Example: Given  (f/0, f/1, g/0, h/2), 
              return (f/0, f/1).
  */
  Ilist p1;
  Ilist p2;
  IlistContainer bad_syms;
 
  for (p1 = syms; p1; p1 = p1->next) {
    string s1 = sn_to_str(p1->i);
    for (p2 = p1->next; p2; p2 = p2->next) {
      string s2 = sn_to_str(p2->i);
      if (myString::str_ident(s1, s2)) {
        if (!bad_syms.ilist_member(p1->i)) bad_syms.ilist_prepend(p1->i);
        if (!bad_syms.ilist_member(p2->i))  bad_syms.ilist_prepend(p2->i);
      }
    }
  }
  return bad_syms.get_head();
}


Ilist SymbolContainer::arity_check(Ilist fsyms, Ilist rsyms) {
 IlistContainer AUXF, AUXR;   
 
 AUXF.set_head(fsyms);
 Ilist copy_fsyms=AUXF.copy_ilist();
 
 AUXR.set_head(rsyms);
 Ilist copy_rsyms=AUXR.copy_ilist();
 
 AUXF.set_head(copy_fsyms);
 AUXR.set_head(copy_rsyms);
 
 
 Ilist syms= AUXF.ilist_cat(AUXR); //concatena copy_fsyms com copy_rsyms
 
 Ilist bad_syms = collect_multiples(syms); //criou-se uma nova lista 
 
 AUXF.zap_ilist();  //limpar a cópia total
 
 return bad_syms;
} 


int SymbolContainer::symbol_with_string(Ilist syms, const string &str) {
  if (syms == NULL)   return -1;
  else if (myString::str_ident(str, sn_to_str(syms->i)))   return syms->i;
  else   return symbol_with_string(syms->next, str);
} 


void SymbolContainer::process_skolem_list(Plist skolem_strings, Ilist fsyms){
  IlistContainer Skolems;
  Plist p;
  string s;
  
  for (p = skolem_strings; p; p = p->next) {
    s=*((string *) p->v);
    int sn = symbol_with_string(fsyms, s);
    if (sn == -1)
      cerr<<"WARNING, declared Skolem symbol not found in formulas:"<< (p->v)<<endl;
    else
      Skolems.ilist_append(sn);
  }
  set_skolem_symbols(Skolems.get_head());
  Skolems.zap_ilist();
}


void SymbolContainer::process_lex_list(Plist lex_strings, Ilist syms,Symbol_Type type) {
  IlistContainer lexs;
  Plist p;
  PlistContainer not_in_formulas;
  string s;
  
  for (p = lex_strings; p; p = p->next) {
    s=*((string *) p->v); 
    int sn = symbol_with_string(syms,s);
    if (sn == -1) not_in_formulas.plist_append(p->v);
    else lexs.ilist_append(sn);
  }

 if (not_in_formulas.get_head()) {
    string s = (type == Symbol_Type::FUNCTION_SYMBOL ? "function" :"predicate");
    cerr<<"WARNING, "<<s<<" symbols in "<< s << "_order (lex) command not found in forumlas:"<<endl;
    cout<<"WARNING, "<<s<<" symbols in "<< s << "_order (lex) command not found in forumlas:"<<endl;
    
    for (p = not_in_formulas.get_head(); p; p = p->next) {
        
        cerr<<*(string *) p->v << (p->next ? ", " : ".\n");
        cout<<*(string *) p->v << (p->next ? ", " : ".\n");
    }
    cerr<<endl;
    cout<<endl;
  }

  if (type == Symbol_Type::FUNCTION_SYMBOL) LADR_GLOBAL_SYMBOL.Preliminary_prec_func.set_head(lexs.get_head());
  else   LADR_GLOBAL_SYMBOL.Preliminary_prec_pred.set_head(lexs.get_head());
  not_in_formulas.zap_plist();
}



Ilist SymbolContainer::symnums_of_arity(Ilist p, int i) {
  if (p == NULL)   return NULL;
  else if (sn_to_arity(p->i) != i) {
    Ilist r = p->next;
    p->next=NULL;
    IlistContainer AUX;
    AUX.free_ilist(p);
    return symnums_of_arity(r, i);
  }
  else {
    p->next = symnums_of_arity(p->next, i);
    return p;
  }
}
