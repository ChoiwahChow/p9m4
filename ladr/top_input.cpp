#include "ladrvglobais.h"
#include "clauseid.h"
#include "top_input.h"
#include "paramod.h"
#include "parautil.h"
#include "maximal.h"
#include "parse.h"
#include "listterm.h"
#include "std_options.h"
#include "ioutil.h"
#include "fatal.h"
#include "mystring.h"
#include <iomanip>
#include <fstream>



TopInput::TopInput() {
        Input_lists = NULL;
        Lex_function_list = NULL;
        Lex_predicate_list = NULL;
        Skolem_list = NULL;  /* temp store of user-declared skolems */
        Program_name="";      
        /*
        ** Memory:: management
        */
        Readlist_gets=0;
        Readlist_frees=0;
}

//destruir as strings que existem em Lex_function_list
TopInput::~TopInput() {
    
}




Readlist TopInput::get_readlist(void) {
  Readlist p = (Readlist) Memory::memCNew(sizeof(struct readlist));
  Readlist_gets++;
  return(p);
} 



void TopInput::free_readlist(Readlist p) {
  Memory::memFree((void *)p, sizeof(struct readlist));
  Readlist_frees++;
}  /* free_readlist */


void TopInput::Free_Mem() {
    destroy_Input_lists();
}



void TopInput::destroy_Input_lists(void) {
    FormulaContainer F;
    TopformContainer TF;
    TermContainer T;
    PlistContainer P;
    JustContainer J;
    Readlist r,aux;
    Plist p;
    Just j;
    
    r=Input_lists;

    while (r!=NULL) {
        if(r->type==(int) Top_input_type::FORMULAS) {
         //because assumptions Readlist Plist of formulas is the same Plist of formulas of the Readlist sos
         //we cannot free twice this Plist of formulas, so we free only sos Readlist Plist of formulas
         //and not the assumptions Readlist Plist os formulas
         //Free the contens of the respective Plist
         //If they are formulas
        
        
        if ((*r->name !="assumptions") && (*r->name!="goals") ) //dont forget, assumptions,sos and goals are all in the same list
       
          for (p=*r->p; p; p=p->next) {            //run respective Plist  and free his components that are Formulas....sos and assumptions are the same in accept list, goals 
                                                    //are denied and added to assumptions...so we only need to free assumptions and the others, never goals neither sos
                TF.zap_topform((Topform) p->v);  //if the top forms are in the topform_id_tab they has been zapped, otherwise they are zapped here
          }        
        
        }
        else
            for (p=*r->p; p; p=p->next)  //run respective Plist  and free his components that are Terms
                T.zap_term(Term(p->v));

        //Free the Plist it self
        //the same reason as above
            
        if ( (*r->name !="assumptions") && (*r->name!="goals") ) {
            P.set_head(*r->p); //Free the respectives Plists whitch are then same as the pi-> lists. So pi->lists are released
            P.zap_plist();
        }
        
        if (r->name) delete r->name; //delete the Input_list name
        aux=r->next;
        free_readlist(r); //Free the readlist it self
        r=aux;
    }
    
    
    
}


void TopInput::fprint_top_input_mem(ostream &fp, bool heading) {
 int n;
  if (heading)
    fp<<"  type (bytes each)        gets      frees     in use      bytes"<<endl;
  n = sizeof(struct readlist);
  fp<<"readlist  ("<<setw(4)<<n<<")      ";
  fp<<setw(11)<<Readlist_gets;
  fp<<setw(11)<<Readlist_frees;
  fp<<setw(11)<<Readlist_gets-Readlist_frees;
  fp<<setw(9)<<( (Readlist_gets-Readlist_frees) *n ) /1024;
  fp<<endl;        
}


void TopInput::p_top_input_mem() {
 fprint_top_input_mem(cout, true);
}


void TopInput::fatal_input_error(ostream &fout, string msg, Term t) {
  ParseContainer P;
  fatal::bell(cerr);
  if (t) {
    fout<<endl<<"%%%%ERROR:"<<endl<<endl<<msg;
	fout<<"%%%%START ERROR%%%%"<<endl;
    P.fwrite_term_nl(fout, t);
    fout<<"%%%%END ERROR%%%%"<<endl;
    P.fwrite_term_nl(cerr, t);
  }
  else
    fout<<"%%%%ERROR:"<<msg<<"."<<endl;
    fatal::fatal_error(msg);
  
  
} 

void TopInput::set_program_name(string name) {
  Program_name = name;
}


bool TopInput::condition_is_true(Term t) {
    TermContainer T;
    return T.is_term(t, Program_name, 0);
} 


void TopInput::init_standard_ladr() {
  SymbolContainer S;
  ParseContainer P;
  Parautil Pu;
    
  myClock::init_wallclock();
  Pu.init_paramod();
  Basic::init_basic_paramod();
  Maximal::init_maximal();

  S.declare_base_symbols(); 
  P.declare_standard_parse_types(); //até aqui inclusivé, ok

  P.translate_neg_equalities(true);
  LADR_GLOBAL_STD_OPTIONS.init_standard_options();
  
  
  
}


void TopInput::process_op2(ostream & fout, Term t, int prec, Term type_term, Term symb_term) {
  TermContainer T;
  SymbolContainer S;
  ParseContainer P;
  if (ARITY(symb_term) != 0) {
    fatal_input_error(fout, "Bad symbol in op command (quotes needed?)", t);
  }
  else {
    ParseType pt = ParseType::NOTHING_SPECIAL;
    if (T.is_constant(type_term, "infix"))
      pt = ParseType::INFIX;
    else if (T.is_constant(type_term, "infix_left"))
      pt = ParseType::INFIX_LEFT;
    else if (T.is_constant(type_term, "infix_right"))
      pt = ParseType::INFIX_RIGHT;
    else if (T.is_constant(type_term, "prefix"))
      pt = ParseType::PREFIX;
    else if (T.is_constant(type_term, "prefix_paren"))
      pt = ParseType::PREFIX_PAREN;
    else if (T.is_constant(type_term, "postfix"))
      pt = ParseType::POSTFIX;
    else if (T.is_constant(type_term, "postfix_paren"))
      pt = ParseType::POSTFIX_PAREN;
    else if (T.is_constant(type_term, "ordinary"))
      pt = ParseType::NOTHING_SPECIAL;
    else
      fatal_input_error(fout, "Bad parse-type in op command", t);
    P.declare_parse_type(S.sn_to_str(SYMNUM(symb_term)), prec, pt);
  }
} 


void TopInput::process_op(Term t, bool echo, ostream &fout) {
  TermContainer T;
  ParseContainer P;
  ListtermContainer LT;
  Term prec_term, type_term, symb_term;
  int prec;
  bool ok;

  if (ARITY(t) == 3) {
    prec_term = ARG(t,0);
    type_term = ARG(t,1);
    symb_term = ARG(t,2);
    ok = T.term_to_int(prec_term, &prec);
  }
  else {
    type_term = ARG(t,0);
    symb_term = ARG(t,1);
    if (!T.is_constant(type_term, "ordinary"))
      fatal_input_error(fout,"If no precedence, type must be \"ordinary\"",t);

    ok = true;
    prec = MIN_PRECEDENCE;  /* checked, but not used */
  }
  
  if (echo)
    P.fwrite_term_nl(fout, t);

  if (!ok || prec < MIN_PRECEDENCE || prec > MAX_PRECEDENCE) {
    fatal::bell(cerr);
    fatal_input_error(fout, "Precedence in op command is out of range", t);
  }
  else if (LT.proper_listterm(symb_term)) {
    while (LT.cons_term(symb_term)) {
      process_op2(fout, t, prec, type_term, ARG(symb_term, 0));
      symb_term = ARG(symb_term, 1);
    }
  }
  else
    process_op2(fout, t, prec, type_term, symb_term);
} 


void TopInput::process_redeclare(Term t, bool echo, ostream &fout) {
  ParseContainer P;
  SymbolContainer S;
  if (ARITY(t) != 2) {
    fatal_input_error(fout, "The redeclare command takes 2 arguments (symbol, operation)", t);
    }
  else {
    Term operation = ARG(t, 0);
    Term symbol  = ARG(t, 1);
    if (ARITY(symbol) != 0 || ARITY(operation) != 0) {
      fatal_input_error(fout, "The redeclare command takes 2 arguments (symbol, operation)", t);
    }
    else {
      bool ok;
      if (echo)
		P.fwrite_term_nl(fout, t);
		ok = P.redeclare_symbol_and_copy_parsetype(S.sn_to_str(SYMNUM(operation)), S.sn_to_str(SYMNUM(symbol)), echo, fout);
        if (!ok) {
		  fatal_input_error(fout, "The new symbol for the redeclared operation is already in use", t);
      }
    }
  }
} 



void TopInput::execute_unknown_action(ostream &fout, int unknown_action, Term t, string message) {
  ParseContainer P;
  if (unknown_action == (int) Unknown_type::KILL_UNKNOWN) {
    fatal_input_error(fout, message, t);
  }
  else if (unknown_action == (int) Unknown_type::WARN_UNKNOWN) {
    fatal::bell(cerr);
    fout<< "WARNING, " <<message<<": ";
    P.fwrite_term_nl(fout,   t);
    cerr<<"WARNING, " <<message<<": ";
	P.fwrite_term_nl(cerr, t);
  }
  else if (unknown_action == (int )Unknown_type::NOTE_UNKNOWN) {
    fout<<"NOTE, "<<message<<": ";
	P.fwrite_term_nl(fout,   t);
  }
} 


void TopInput::flag_handler(ostream &fout, Term t, bool echo, int unknown_action) {
  TermContainer T;
  ParseContainer P;
  SymbolContainer S;
  int flag = LADR_GLOBAL_OPTIONS.str_to_flag_id(S.sn_to_str(SYMNUM(ARG(t,0))));
  
  if (flag == -1)   execute_unknown_action(fout, unknown_action, t, "Flag not recognized");
  else {
    LADR_GLOBAL_OPTIONS.update_flag(fout, flag, T.is_term(t, "set", 1), echo);
    
    if (T.is_term(ARG(t,0), "arithmetic", 0)) {
      if (T.is_term(t,"set",1)) {
		cout<<endl<<"    %% Declaring Mace4 arithmetic parse types."<<endl;
		P.declare_parse_type("+",   490, ParseType::INFIX_RIGHT);
		P.declare_parse_type("*",   470, ParseType::INFIX_RIGHT);
		P.declare_parse_type("/",   460, ParseType::INFIX);
		P.declare_parse_type("mod", 460, ParseType::INFIX);
      }
    }
  }
}

void TopInput::parm_handler(ostream &fout, Term t, bool echo, int unknown_action) {
  SymbolContainer S;
  TermContainer T;
  string name   = S.sn_to_str(SYMNUM(ARG(t,0)));
  Term tval  = ARG(t,1);
  int id = LADR_GLOBAL_OPTIONS.str_to_parm_id(name);
  if (id != -1) {
    int val;
    if (T.term_to_int(tval, &val))
      LADR_GLOBAL_OPTIONS.assign_parm(id, val, echo);
    else {
      execute_unknown_action(fout, unknown_action, t, "Parm needs integer value");      
    }
  }
  else {
    id = LADR_GLOBAL_OPTIONS.str_to_floatparm_id(name);
    if (id != -1) {
      double val;
      if (T.term_to_number(tval, &val)) LADR_GLOBAL_OPTIONS.assign_floatparm(id, val, echo);
      else	execute_unknown_action(fout, unknown_action, t,"Floatparm needs integer or double value");
    }
    else {
      id = LADR_GLOBAL_OPTIONS.str_to_stringparm_id(name);
      if (id != -1) {
			if (CONSTANT(tval)) {
				string s = S.sn_to_str(SYMNUM(tval));
				LADR_GLOBAL_OPTIONS.assign_stringparm(id, s, echo);
			}
			else execute_unknown_action(fout, unknown_action, t,"Stringparm needs constant value");
      }
      else execute_unknown_action(fout, unknown_action, t, "Parameter not recognized");
    }
  }
}



void TopInput::process_symbol_list(Term t, string command, Plist terms) {
  
  PlistContainer STRS;
  PlistContainer LPL;
  PlistContainer LFL;
  PlistContainer SL;
  SymbolContainer S;
  Plist p;
  
  LPL.set_head(Lex_predicate_list);
  LFL.set_head(Lex_function_list);
  SL.set_head(Skolem_list);
  for (p = terms ; p; p = p->next) {
    Term c = (Term) p->v;
    if (!CONSTANT(c))
      fatal_input_error(cout, "Symbols in this command must not have arguments (arity is deduced from the clauses)", t);
      string *s= new string (S.sn_to_str(SYMNUM(c)));
      STRS.plist_append(s); //para já fica assim, recebe o endereço da string
  }
  if (myString::str_ident(command, "lex") || myString::str_ident(command, "function_order")) Lex_function_list = LFL.plist_cat(STRS);
  else if (myString::str_ident(command, "predicate_order"))   Lex_predicate_list = LPL.plist_cat(STRS);
  else if (myString::str_ident(command, "skolem"))   Skolem_list = SL.plist_cat(STRS);
  else fatal_input_error(cout, "Unknown command", t);
} 


Readlist TopInput::readlist_member(Readlist p, string name, int type) {
  if (p == NULL)     return NULL;
  else if (p->type == type && myString::str_ident(*(p->name), name))    return p;
  else   return readlist_member(p->next, name, type);
}

Readlist TopInput::readlist_member_wild(Readlist p, int type) {
  if (p == NULL)  return NULL;
  else if (p->type == type && myString::str_ident(*(p->name),myString::null_string() )  )   return p;
  else  return readlist_member_wild(p->next, type);
}

void TopInput::accept_list(string name, int type, bool aux, Plist *l) {
  Readlist p = readlist_member(Input_lists, name, type);
  if (p)
    fatal::fatal_error("accept_list, duplicate name/type");
  else {
    p = get_readlist();
    p->name = new string(name);
    p->type = type;
    p->auxiliary = aux;
    *l = NULL;
    p->p = l;
    p->next = Input_lists;
    Input_lists = p;
  }
} 

void TopInput::input_symbols(int type, bool aux, Ilist *fsyms, Ilist *rsyms) {
  SymbolContainer S;
  FormulaContainer F;
  I2list fsyms_multiset = NULL;
  I2list rsyms_multiset = NULL;
  
  IlistContainer FSYMS;
  IlistContainer RSYMS;
  I2listContainer AUX;
  
  Readlist p;
  for (p = Input_lists; p; p = p->next) {
    if (p->type == type && p->auxiliary == aux) {
      F.gather_symbols_in_formulas(*(p->p), &rsyms_multiset, &fsyms_multiset);
    }
  }
  
  *rsyms = RSYMS.multiset_to_set(rsyms_multiset);
  *fsyms = FSYMS.multiset_to_set(fsyms_multiset);
  
  *fsyms = S.remove_variable_symbols(*fsyms);
  AUX.set_head(fsyms_multiset);
  AUX.zap_i2list();
  AUX.set_head(rsyms_multiset);
  AUX.zap_i2list();

  if (false) {
    Ilist p;
    cout<<"RSYMS: ";
    for (p = *rsyms; p; p = p->next)    cout<<" "<<S.sn_to_str(p->i)<<endl;
    cout<<"FSYMS: ";
	for (p = *fsyms; p; p = p->next)
	  cout<<" "<<S.sn_to_str(p->i);
    cout<<endl;
  }
}



void TopInput::symbol_check_and_declare(void) {
  Ilist fsyms_theory, fsyms_aux, fsyms_all, fsyms_aux_only;
  Ilist rsyms_theory, rsyms_aux, rsyms_all;
  Ilist bad;
  IlistContainer IFT,IFA, IRT, IRA, IFALL, IRALL,IAUX;
  PlistContainer PAUX;
  SymbolContainer S;
  

  input_symbols((int) Top_input_type::FORMULAS,  false,  &fsyms_theory, &rsyms_theory);
  input_symbols((int) Top_input_type::FORMULAS,  true,   &fsyms_aux,    &rsyms_aux);
  
  IFT.set_head(fsyms_theory);  
  IFA.set_head(fsyms_aux);
  IFALL.ilist_union(IFT, IFA);
  fsyms_all = IFALL.get_head();
  
  IRT.set_head(rsyms_theory);
  IRA.set_head(rsyms_aux);
  IRALL.ilist_union(IRT, IRA);
  rsyms_all = IRALL.get_head();

  fsyms_aux_only = IFA.ilist_subtract(IFT);
  S.declare_aux_symbols(fsyms_aux_only);

  /* Check for variables in rsyms. */

  bad = S.variable_symbols(rsyms_all);
  if (bad) {
    Ilist g;
    StrbufContainer SB;
    SB.new_string_buf("The following symbols cannot be used as atomic formulas, because they are variables: ");
    
    for (g = bad; g; g = g->next) {
      SB.sb_append(S.sn_to_str(g->i));
      if (g->next)	SB.sb_append(", ");
    }
    fatal_input_error(cout, SB.sb_to_malloc_string(), NULL);
  }

  /* Check if any symbol is used as both a relation and function symbol. */

  IAUX.ilist_intersect(IFALL, IRALL);
  bad=IAUX.get_head();
  if (bad) {
    Ilist g;
    
    StrbufContainer SB;
    SB.new_string_buf("The following symbols/arities are used as both relation and function symbols: ");
    for (g = bad; g; g = g->next) {
      SB.sb_append(S.sn_to_str(g->i));
      SB.sb_append_char('/');
      SB.sb_append_int(S.sn_to_arity(g->i));
      if (g->next)	SB.sb_append(", ");
    }
    fatal_input_error(cout, SB.sb_to_malloc_string(), NULL);
  }

  /* Check if any symbol is used with multiple arities. */

  bad = S.arity_check(fsyms_all, rsyms_all);
  if (bad) {
    Ilist g;
    
    StrbufContainer SB;
    SB.new_string_buf("The following symbols are used with multiple arities: ");
    for (g = bad; g; g = g->next) {
      SB.sb_append(S.sn_to_str(g->i));
      SB.sb_append_char('/');
      SB.sb_append_int(S.sn_to_arity(g->i));
      if (g->next)	SB.sb_append(", ");
    }
    fatal_input_error(cout, SB.sb_to_malloc_string(), NULL);
  }

  /* Tell the symbol package the functions and relations.
     (needed for ordering the symbols) */

  S.declare_functions_and_relations(fsyms_all, rsyms_all);

  S.process_skolem_list(Skolem_list, fsyms_theory);
  
  PAUX.set_head(Skolem_list);
  PAUX.zap_plist();
  Skolem_list = NULL;
  
  S.process_lex_list(Lex_function_list, fsyms_all, Symbol_Type::FUNCTION_SYMBOL);
  S.process_lex_list(Lex_predicate_list, rsyms_all, Symbol_Type::PREDICATE_SYMBOL);


  for (Plist aux=Lex_function_list;aux;aux=aux->next)
      if(aux->v) delete (string *) aux->v;
  PAUX.set_head(Lex_function_list);
  PAUX.zap_plist();
  
  for (Plist aux=Lex_predicate_list;aux;aux=aux->next)
      if(aux->v) delete (string *) aux->v;
  
  PAUX.set_head(Lex_predicate_list);
  PAUX.zap_plist();
  Lex_function_list = NULL;
  Lex_predicate_list = NULL;

  /*
  p_syms();
  printf("fsyms_theory: "); p_ilist(fsyms_theory);
  printf("fsyms_aux: "); p_ilist(fsyms_aux);
  printf("fsyms_all: "); p_ilist(fsyms_all);
  printf("fsyms_aux_only: "); p_ilist(fsyms_aux_only);
  printf("rsyms_theory: "); p_ilist(rsyms_theory);
  printf("rsyms_aux: "); p_ilist(rsyms_aux);
  printf("rsyms_all: "); p_ilist(rsyms_all);
  */

  IFT.zap_ilist();
  IFA.zap_ilist();
  IFALL.zap_ilist();
  
  IAUX.set_head(fsyms_aux_only);
  IAUX.zap_ilist();
  
  

  IRT.zap_ilist();
  IRA.zap_ilist();
  IRALL.zap_ilist();

} 


void TopInput::read_from_string_buf(String_buf sb, ostream &fout, bool echo, int unknown_action) {
  int if_depth = 0;  /* for conditional inclusion */
  ParseContainer P;
  TermContainer T;
  SymbolContainer S;
  ListtermContainer L;
  PlistContainer PAUX;
  FormulaContainer F;
  int pos=0;
  
   //forrar a bomba
  Term t = P.read_term_from_string_buf(sb, fout,pos); //tem de se avançar no pos
  
    while (t!=NULL) {
  
           
         
            if (T.is_term(t, "set", 1) || T.is_term(t, "clear", 1)) {
            /********************************************************** set, clear */
            if (echo)	P.fwrite_term_nl(fout, t);
                flag_handler(fout, t, echo, unknown_action);
            }
            else if (T.is_term(t, "assign", 2)) {
            /************************************************************** assign */
            if (echo)	P.fwrite_term_nl(fout, t);
                parm_handler(fout, t, echo, unknown_action);
            }
            else if (T.is_term(t, "assoc_comm", 1) ||T.is_term(t, "commutative", 1)) {
            /************************************************************ AC, etc. */
            Term f = ARG(t,0);
            if (!CONSTANT(f)) {
                fatal_input_error(fout, "Argument must be symbol only", t);
            }
            else {
                    if (echo)	  P.fwrite_term_nl(fout, t);
                    if (T.is_term(t, "assoc_comm",1))	  S.set_assoc_comm(S.sn_to_str(SYMNUM(f)), true);
                    else  S.set_commutative(S.sn_to_str(SYMNUM(f)),true);
            }
            }
            else if (T.is_term(t, "op", 3) || T.is_term(t, "op", 2)) {
            /****************************************************************** op */
            /* e.g., op(300, infix, +).  op(ordinary, *) */
            process_op(t, echo, fout);
            }
            else if (T.is_term(t, "redeclare", 2)) {
            /*********************************************************** redeclare */
            /* e.g., redeclare(~, negation). */
            process_redeclare(t, echo, fout);
            }
            else if (T.is_term(t, "lex", 1) || T.is_term(t, "predicate_order", 1) ||  T.is_term(t, "function_order", 1) || T.is_term(t, "skolem", 1)) {
            /********************************************************* lex, skolem */
            string command = S.sn_to_str(SYMNUM(t));
            Plist p = L.listterm_to_tlist(ARG(t,0));
           
            if (p == NULL) {
                        fatal_input_error(fout, "Function_order/predicate_order/skolem command must contain a list, e.g., [a,b,c]", t);
            }
            else {
                    if (echo) P.fwrite_term_nl(fout, t);
                    process_symbol_list(t, command, p);
                    PAUX.set_head(p);
                    PAUX.zap_plist();
                }
            }
            else 
            
            if (T.is_term(t, "formulas", 1) || T.is_term(t, "clauses", 1) ||  T.is_term(t, "terms", 1) ||  T.is_term(t, "list", 1)) { //list of formulas or terms
            /***************************************************** list of objects */
                int type = (T.is_term(t, "formulas", 1) || T.is_term(t, "clauses", 1)	  ?  (int)Top_input_type::FORMULAS : (int)Top_input_type::TERMS);
                string name = T.term_symbol(ARG(t,0));
                Plist objects = NULL;
                int echo_id = LADR_GLOBAL_OPTIONS.str_to_flag_id("echo_input");
                bool echo_objects = (echo_id == -1 ? true : LADR_GLOBAL_OPTIONS.flag(echo_id));

                if (T.is_term(t, "clauses", 1)) {
                    fatal::bell(cerr);
                    cerr<<"\nWARNING: \"clauses(...)\" should be replaced with \"formulas(...)\".\n Please update your input files.  Future versions will not accept \"clauses(...)\".\n\n";
                }
                else if (T.is_term(t, "terms", 1)) {
                    fatal::bell(cerr);
                    cerr<<"\nWARNING: \"terms(...)\" should be replaced with \"list(...)\".\n Please update your input files.  Future versions will not accept \"terms(...)\".\n\n";
                }

            
                objects = Ioutil::read_term_list(sb, fout,pos);    //isto devolve uma p_list com os elementos que são termos
                                                                //até aqui tudo bem   
                                                                //so we have a list of terms, the term symbols are created
                Term t1;
                Plist p;                                               //i think there are no variables until now  
                FormulaContainer F;
                if (type == (int) Top_input_type::FORMULAS) {
                   
                    //fazer a limpeza dos termos da plist
                    for (p=objects; p; p=p->next) {
                        t1=(Term)  p->v;
                        p->v= (Formula) F.term_to_formula(t1);      //term_to_formula creates the formula terms
                        // F.fprint_formula(cout,Formula(p->v));     //only for debug
                        T.zap_term(t1);                             //so we can zap this term
                    }
                }
                
               if (echo) {
                    if(echo_objects) {
                        
                        if(type==(int) Top_input_type::FORMULAS)
                           Ioutil::fwrite_formula_list(fout,objects,name);
                        else
                          Ioutil::fwrite_term_list(fout,objects,name); 
                    }
                    else {
                          PAUX.set_head(objects);
                          fout<<endl<<"%% ";
                          P.fwrite_term(fout, t1);
                          fout<<".  %% not echoed ("<<PAUX.plist_count()<<" "<<(type ==(int) Top_input_type::FORMULAS ? "formulas" : "terms")<<")"<<endl;
                    }
               } //do echo
               
                
               {
                   //procura a input_list e adiciona as fórmulas
                   PlistContainer PR;
                   Readlist r = readlist_member(Input_lists, name, type);
                   
                   if (r == NULL)
                        r = readlist_member_wild(Input_lists, type);
                    if (r == NULL) {
                         fatal_input_error(fout, "List name/type not recognized", t);
                    }
                    else {
                          PR.set_head(*(r->p));  
                          *(r->p)=PR.plist_cat(objects);
                    }
                    
               } //no final disto tenho de libertar tudo das Input_lists

          
           
               
        }  //list of terms and formulas
        
        else if (T.is_term(t, "if", 1)) {
             Term condition = ARG(t,0);
             if (echo)
               P.fwrite_term_nl(fout, t);
             if (condition_is_true(condition)) {
                if (echo)
                    fout<<"%% Conditional input included."<<endl;
                if_depth++;
             }
             else {
                    int depth = 1;  /* if-depth */
                    Term t2;
                    do {
                        t2 = P.read_term_from_string_buf(sb, fout,pos);
                        if (!t2)
                            fatal_input_error(fout, "Missing end_if (condition is false)", t);
                        else if (T.is_term(t2, "if", 1))	    depth++;
                        else if (T.is_term(t2, "end_if", 0))  depth--;
                        T.zap_term(t2);
                    }	while (depth > 0);
                    if (echo)
                        fout<<"%% Conditional input ommited."<<endl<<"end_if."<<endl;
            }
        } else if (T.is_term(t, "end_if", 0)) {
               /***************************************************** end_if (true case) */
              if_depth--;
              if (echo)
              P.fwrite_term_nl(fout, t);
              if (if_depth < 0) fatal_input_error(fout, "Extra end_if", t);
        } else fatal_input_error(fout, "Unrecognized command or list", t);
        
       T.zap_term(t);
       t = P.read_term_from_string_buf(sb, fout,pos);
       
      
  } //while
  
 
  
  if (if_depth != 0) {
       fatal_input_error(fout, "Missing end_if (condition is true)", t);
  }
 
  
    
    
}



void TopInput::read_from_file(istream &fin, ostream &fout, bool echo, int unknown_action) {
  int if_depth = 0;  /* for conditional inclusion */
  ParseContainer P;
  TermContainer T;
  SymbolContainer S;
  ListtermContainer L;
  PlistContainer PAUX;
  FormulaContainer F;
  
  
   //forrar a bomba
  Term t = P.read_term(fin, fout);
  
  
   
  while (t!=NULL) {
  
           
         
            if (T.is_term(t, "set", 1) || T.is_term(t, "clear", 1)) {
            /********************************************************** set, clear */
            if (echo)	P.fwrite_term_nl(fout, t);
                flag_handler(fout, t, echo, unknown_action);
            }
            else if (T.is_term(t, "assign", 2)) {
            /************************************************************** assign */
            if (echo)	P.fwrite_term_nl(fout, t);
                parm_handler(fout, t, echo, unknown_action);
            }
            else if (T.is_term(t, "assoc_comm", 1) ||T.is_term(t, "commutative", 1)) {
            /************************************************************ AC, etc. */
            Term f = ARG(t,0);
            if (!CONSTANT(f)) {
                fatal_input_error(fout, "Argument must be symbol only", t);
            }
            else {
                    if (echo)	  P.fwrite_term_nl(fout, t);
                    if (T.is_term(t, "assoc_comm",1))	  S.set_assoc_comm(S.sn_to_str(SYMNUM(f)), true);
                    else  S.set_commutative(S.sn_to_str(SYMNUM(f)),true);
            }
            }
            else if (T.is_term(t, "op", 3) || T.is_term(t, "op", 2)) {
            /****************************************************************** op */
            /* e.g., op(300, infix, +).  op(ordinary, *) */
            process_op(t, echo, fout);
            }
            else if (T.is_term(t, "redeclare", 2)) {
            /*********************************************************** redeclare */
            /* e.g., redeclare(~, negation). */
            process_redeclare(t, echo, fout);
            }
            else if (T.is_term(t, "lex", 1) || T.is_term(t, "predicate_order", 1) ||  T.is_term(t, "function_order", 1) || T.is_term(t, "skolem", 1)) {
            /********************************************************* lex, skolem */
            string command = S.sn_to_str(SYMNUM(t));
            Plist p = L.listterm_to_tlist(ARG(t,0));
           
            if (p == NULL) {
                        fatal_input_error(fout, "Function_order/predicate_order/skolem command must contain a list, e.g., [a,b,c]", t);
            }
            else {
                    if (echo) P.fwrite_term_nl(fout, t);
                    process_symbol_list(t, command, p);
                    PAUX.set_head(p);
                    PAUX.zap_plist();
                }
            }
            else 
            
            if (T.is_term(t, "formulas", 1) || T.is_term(t, "clauses", 1) ||  T.is_term(t, "terms", 1) ||  T.is_term(t, "list", 1)) { //list of formulas or terms
            /***************************************************** list of objects */
                int type = (T.is_term(t, "formulas", 1) || T.is_term(t, "clauses", 1)	  ?  (int)Top_input_type::FORMULAS : (int)Top_input_type::TERMS);
                string name = T.term_symbol(ARG(t,0));
                Plist objects = NULL;
                int echo_id = LADR_GLOBAL_OPTIONS.str_to_flag_id("echo_input");
                bool echo_objects = (echo_id == -1 ? true : LADR_GLOBAL_OPTIONS.flag(echo_id));

                if (T.is_term(t, "clauses", 1)) {
                    fatal::bell(cerr);
                    cerr<<"\nWARNING: \"clauses(...)\" should be replaced with \"formulas(...)\".\n Please update your input files.  Future versions will not accept \"clauses(...)\".\n\n";
                }
                else if (T.is_term(t, "terms", 1)) {
                    fatal::bell(cerr);
                    cerr<<"\nWARNING: \"terms(...)\" should be replaced with \"list(...)\".\n Please update your input files.  Future versions will not accept \"terms(...)\".\n\n";
                }

            
                objects = Ioutil::read_term_list(fin, fout);    //isto devolve uma p_list com os elementos que são termos
                                                                //até aqui tudo bem   
                                                                //so we have a list of terms, the term symbols are created
                Term t1;
                Plist p;                                               //i think there are no variables until now  
                FormulaContainer F;
                if (type == (int) Top_input_type::FORMULAS) {
                   
                    //fazer a limpeza dos termos da plist
                    for (p=objects; p; p=p->next) {
                        t1=(Term)  p->v;
                        p->v= (Formula) F.term_to_formula(t1);      //term_to_formula creates the formula terms
                        // F.fprint_formula(cout,Formula(p->v));     //only for debug
                        T.zap_term(t1);                             //so we can zap this term
                    }
                }
                
               if (echo) {
                    if(echo_objects) {
                        
                        if(type==(int) Top_input_type::FORMULAS)
                           Ioutil::fwrite_formula_list(fout,objects,name);
                        else
                          Ioutil::fwrite_term_list(fout,objects,name); 
                    }
                    else {
                          PAUX.set_head(objects);
                          fout<<endl<<"%% ";
                          P.fwrite_term(fout, t1);
                          fout<<".  %% not echoed ("<<PAUX.plist_count()<<" "<<(type ==(int) Top_input_type::FORMULAS ? "formulas" : "terms")<<")"<<endl;
                    }
               } //do echo
               
                
               {
                   //procura a input_list e adiciona as fórmulas
                   PlistContainer PR;
                   Readlist r = readlist_member(Input_lists, name, type);
                   
                   if (r == NULL)
                        r = readlist_member_wild(Input_lists, type);
                    if (r == NULL) {
                         fatal_input_error(fout, "List name/type not recognized", t);
                    }
                    else {
                          PR.set_head(*(r->p));  
                          *(r->p)=PR.plist_cat(objects);
                    }
                    
               } //no final disto tenho de libertar tudo das Input_lists

          
           
               
        }  //list of terms and formulas
        
        else if (T.is_term(t, "if", 1)) {
             Term condition = ARG(t,0);
             if (echo)
               P.fwrite_term_nl(fout, t);
             if (condition_is_true(condition)) {
                if (echo)
                    fout<<"%% Conditional input included."<<endl;
                if_depth++;
             }
             else {
                    int depth = 1;  /* if-depth */
                    Term t2;
                    do {
                        t2 = P.read_term(fin, fout);
                        if (!t2)
                            fatal_input_error(fout, "Missing end_if (condition is false)", t);
                        else if (T.is_term(t2, "if", 1))	    depth++;
                        else if (T.is_term(t2, "end_if", 0))  depth--;
                        T.zap_term(t2);
                    }	while (depth > 0);
                    if (echo)
                        fout<<"%% Conditional input ommited."<<endl<<"end_if."<<endl;
            }
        } else if (T.is_term(t, "end_if", 0)) {
               /***************************************************** end_if (true case) */
              if_depth--;
              if (echo)
              P.fwrite_term_nl(fout, t);
              if (if_depth < 0) fatal_input_error(fout, "Extra end_if", t);
        } else fatal_input_error(fout, "Unrecognized command or list", t);
        
       T.zap_term(t);
       t = P.read_term(fin, fout);
       
      
  } //while
  
 
  
  if (if_depth != 0) {
       fatal_input_error(fout, "Missing end_if (condition is true)", t);
  }
 
}



void TopInput::read_all_input(int argc, char *argv[], ostream &fout, bool echo, int unknown_action) {
  int ns=myString::witch_string_member("-s", argv, argc);
  pid_t mypid=getpid(); 
  
  
  
  
  if(ns!=-1) { 
            
            StrbufContainer SB;
            SB.new_string_buf();
            for (int i = ns+1; i < argc; i++) {
                for(int j=0; j<strlen(argv[i]); j++)
                    SB.sb_append_char(argv[i][j]);
            }
            
            read_from_string_buf(SB.get_string_buf(), fout, echo, unknown_action);
            SB.zap_string_buf();
            return;
}

 
    
int n = myString::witch_string_member("-f", argv, argc);
 
  if (n == -1) {
    read_from_file(cin, fout, echo, unknown_action);
  }
  else {
    int i;
    for (i = n+1; i < argc; i++) {
      ifstream _fin(argv[i]);
      if (!_fin) {
        char s[100];
        sprintf(s, "read_all_input, file %s not found", argv[i]);
        fatal::fatal_error(s);
      }
      if (echo)
        cout<<endl<<"% Reading from file "<<argv[i]<<endl<<endl;
      read_from_file(_fin, fout, echo, unknown_action);
      _fin.close();
    }
  }
 
  
  LADR_GLOBAL_STD_OPTIONS.process_standard_options();
  symbol_check_and_declare();
  
} 


void TopInput::check_formula_attributes(Formula f) {
  FormulaContainer F;
  AttributeContainer A;
  if (F.subformula_contains_attributes(f)) {
    Term t = F.formula_to_term(f);
    fatal_input_error(cout, "Subformulas cannot contain attributes", t);
  }
  else if (!F.clausal_formula(f) &&   A.attributes_contain_variables(f->attributes)) {
    Term t = F.formula_to_term(f);
    fatal_input_error(cout, "Answer attributes on non-clausal formulas cannot contain variables", t);
  }
}



Plist TopInput::process_input_formulas(Plist formulas, bool echo) {
  FormulaContainer F;
  TopformContainer TF;
  ClauseidContainer CI;
  AttributeContainer A;
  Clausify C;
  Plist novo = NULL;  /* collect Topforms (clauses) to be returned */
  Plist p;
  PlistContainer P;
  JustContainer J;
  ParseContainer PC;
  PlistContainer AUX;
  

  for (p = formulas; p; p = p->next) {
    Topform tf =(Topform) p->v;
    
    if (F.clausal_formula(tf->formula)) {  //this build a topformula with the original topformula literals
      /* just make it into a clause data structure and use the same Topform */
      //tf->formula has the clausal formula, so lets transform them in to literals

      tf->literals = C.formula_to_literals(tf->formula);  //now tf->literals has all the literals that existed on tf->formula
      TF.upward_clause_links(tf); //the literals gets the parents address with is tf
      F.zap_formula(tf->formula); //the formula part of tf is not needed any more
      tf->formula = NULL;         //
      tf->is_formula = false;     //we have no mora a formula, but a list os literals   
      TF.clause_set_variables(tf, MAX_VARS);  //changes the constants that should be variables
      novo = P.plist_prepend(tf);  //append this topform of literals to the returned Plist
    }
    else {
      /* Clausify, collecting new Topforms to be returned. */
      Formula f2;
      Plist clauses, p;
      CI.assign_clause_id(tf); //the topform_id_tab gets this topform
      
      f2 = F.universal_closure(F.formula_copy(tf->formula)); //f2 gets a new copy of the topformula formula and with the universal_closure added
      clauses = C.clausify_formula(f2); //clauses gets a new list of clauses with are topformulas with the new clauses
      //f2 is not needed any more
      F.zap_formula(f2); //carlos
      
      for (p = clauses; p; p = p->next) {
        Topform c = (Topform) p->v;
        c->attributes = A.copy_attributes(tf->attributes); //copy the attributes of the non clausal formula to the respective clauses
        c->justification = J.clausify_just(tf);
        novo = P.plist_prepend(c);
      }
      TF.append_label_attribute(tf, "non_clause"); //tf gets a new attribute
      
      if (echo) Ioutil::fwrite_clause(cout, tf,(int) Clause_print_format::CL_FORM_STD);
      /* After this point, tf will be accessible only from the ID table. */
    
      AUX.set_head(clauses); //--carlos
      AUX.zap_plist();       //--carlos 
        
    }
  }
  
 
  AUX.set_head(formulas);
  AUX.zap_plist();  /* shallow */
  novo = P.reverse_plist();
  return novo;
}  /* process_input_formulas */


Plist TopInput::process_demod_formulas(Plist formulas, bool echo) {
  Plist novo = NULL;  /* collect Topforms (clauses) to be returned */
  Plist p;
  PlistContainer P;
  FormulaContainer F;
  Clausify C;
  TopformContainer TF;  
  for (p = formulas; p; p = p->next) {
    Topform tf =(Topform) p->v;
    if (F.clausal_formula(tf->formula)) {
      /* just make it into a clause data structure and use the same Topform */
      tf->literals = C.formula_to_literals(tf->formula);
      TF.upward_clause_links(tf);
      F.zap_formula(tf->formula);
      tf->formula = NULL;
      tf->is_formula = false;
      TF.clause_set_variables(tf, MAX_VARS);
      novo = P.plist_prepend(tf);
    }
    else if (tf->formula->type == Ftype::IMP_FORM ||tf->formula->type == Ftype::IMPBY_FORM ||tf->formula->type == Ftype::IFF_FORM) {
        novo= P.plist_prepend(tf);
    }
    else fatal::fatal_error("process_demod_formulas: bad demodulator");
  }
  PlistContainer AUX;
  AUX.set_head(formulas);
  AUX.zap_plist();  /* shallow */
  novo = P.reverse_plist();
  return novo;
}  /* process_demod_formulas */


Plist TopInput::process_goal_formulas(Plist formulas, bool echo) {
  
  PlistContainer NOVO;
  FormulaContainer F;
  JustContainer J;
  Clausify C;
  AttributeContainer A;
  ClauseidContainer CI;
  TopformContainer TF;
  PlistContainer AUX;
  
  
  AUX.set_head(formulas);
  bool must_be_positive = (AUX.plist_count() > 1);
  
  Plist novo = NULL;
  Plist p;

  
    
  for (p = formulas; p; p = p->next) {
    Topform tf = (Topform) p->v;  
    Plist clauses, q;
    Formula f2;
    f2 = F.universal_closure(F.formula_copy(tf->formula));
    if (must_be_positive && !F.positive_formula(f2)) {
      Term t = F.formula_to_term(tf->formula);
      fatal_input_error(cout, "If there are multiple goals, all must be positive", t);
    }
    
    f2 = F.negate(f2); //o negate devolve a mesma fórmula mas com um kid que é a fórmula antiga...até aqui ok, não posso fazer nada para libertar
    clauses = C.clausify_formula(f2);
    
    F.zap_formula(f2); //--carlos
    
    CI.assign_clause_id(tf);
    
  
    
    for (q = clauses; q; q = q->next) {
      Topform c =(Topform) q->v;
      c->attributes = A.copy_attributes(tf->attributes);
      c->justification = J.deny_just(tf);
      novo = NOVO.plist_prepend(c);
    }
    
    TF.append_label_attribute(tf, "non_clause");   
    TF.append_label_attribute(tf, "goal");
    
    AUX.set_head(clauses); //--carlos
    AUX.zap_plist();       //--carlos 
    
    if (echo)
        Ioutil::fwrite_clause(cout, tf, (int) Clause_print_format::CL_FORM_STD);
    

    
    /* After this point, tf will be accessible only from the ID table. */
  }
  
  
  
  AUX.set_head(formulas);
  AUX.zap_plist();
  
  novo = NOVO.reverse_plist(); //return new list with goals
  
  
  
         
  
  
  return novo;
} 



Term TopInput::read_commands(istream &fin, ostream &fout, bool echo, int unknown_action) {
  TermContainer T;
  SymbolContainer S;
  PlistContainer AUX;
  ParseContainer P;
  ListtermContainer LT;
  Term t = P.read_term(fin, fout);
  bool go = (t != NULL);

  while (go) {
    bool already_echoed = false;
    /************************************************************ set, clear */
    if (T.is_term(t, "set", 1) || T.is_term(t, "clear", 1)) {
      if (echo) {
        P.fwrite_term_nl(fout, t);
        already_echoed = true;
      }
      flag_handler(fout, t, echo, unknown_action);

      /* SPECIAL CASES: these need action right now! */
    if (T.is_term(ARG(t,0), "prolog_style_variables", 0)) {
        if (T.is_term(t,"set",1))  S.set_variable_style(Variable_Style::PROLOG_STYLE);
        else S.set_variable_style(Variable_Style::STANDARD_STYLE);
      }
    }
    else if (T.is_term(t, "assign", 2)) {
      /************************************************************** assign */
      if (echo) {
        P.fwrite_term_nl(fout, t);
        already_echoed = true;
      }
      parm_handler(fout, t, echo, unknown_action);
    }
    else if (T.is_term(t, "assoc_comm", 1) || T.is_term(t, "commutative", 1)) {
      /************************************************************ AC, etc. */
      Term f = ARG(t,0);
      if (!CONSTANT(f)) {
        fatal_input_error(fout, "Argument must be symbol only", t);
      }
      else {
        if (T.is_term(t, "assoc_comm", 1)) S.set_assoc_comm(S.sn_to_str(SYMNUM(f)), true);
        else S.set_commutative(S.sn_to_str(SYMNUM(f)), true);
      }
    }
    else if (T.is_term(t, "op", 3)) {
      /****************************************************************** op */
      /* e.g., op(300, infix, +); */
      process_op(t, echo, fout);
    }
    else if (T.is_term(t, "lex", 1)) {
      /***************************************************************** lex */
      Plist p = LT.listterm_to_tlist(ARG(t,0));
      if (p == NULL) {
            fatal_input_error(fout, "Lex command must contain a proper list, e.g., [a,b,c]", t);
      }
      else {
            process_symbol_list(t, "lex", p);
            AUX.set_head(p);
            AUX.zap_plist();
      }
    }
    else {
      /******************************************************** unrecognized */
      /* return this unknown term */
      go = false;
    }

    if (go) {
      if (echo && !already_echoed) P.fwrite_term_nl(fout, t);
      T.zap_term(t);
      t = P.read_term(fin, fout);
      go = (t != NULL);
    }
  }
  return t;
}


Plist TopInput::embed_formulas_in_topforms(Plist formulas, bool assumption) {
  Plist p;
  TopformContainer TF;  
  JustContainer J;
  for (p = formulas; p; p = p->next) {
    Formula f =(Formula) p->v;
    Topform tf = TF.get_topform();
    tf->is_formula = true;
    tf->formula = f;
    check_formula_attributes(f);
    tf->justification = (assumption ? J.input_just() : J.goal_just());
    tf->attributes = f->attributes;
    f->attributes = NULL;
    p->v = tf;
  }
  return formulas;
} 
