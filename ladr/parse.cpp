
#include "parse.h"
#include "ladrvglobais.h"
#include "fatal.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "mystring.h"




GlobalParse::GlobalParse() {
    Cons_char = ':';
    Quote_char = '\"';
    Quantifier_precedence = 0;  /* see
    declare_quantifier_precedence */

    Parenthesize=false;
    Check_for_illegal_symbols = true;
    Simple_parse = false;
    Translate_neg_equalities=false;
    Token_gets=0;
    Token_frees=0;
    Pterm_gets=0;
    Pterm_frees=0;
  
    
}

GlobalParse::~GlobalParse() {
    
    
 
    
}


void GlobalParse::Free_Mem(void) {

    Plist p;
    char *s;
        
    for (p=Multiple_char_special_syms.get_head();p; p=p->next) 
     if(p->v) {
          s=(char *)p->v;
          free(s);
     }
    Multiple_char_special_syms.zap_plist();
    
}


ParseContainer::ParseContainer() {

    
}


ParseContainer::~ParseContainer() {

}

Token ParseContainer::get_token(void){
  Token p = (Token) Memory::memCNew(sizeof(struct token));
  LADR_GLOBAL_PARSE.Token_gets++;
  p->next=NULL;
  return(p);
}  /* get_token */


void ParseContainer::free_token(Token p) {
  Memory::memFree(p, sizeof(struct token));
  LADR_GLOBAL_PARSE.Token_frees++;
}  /* free_token */


Pterm ParseContainer::get_pterm(void) {
  Pterm p =(Pterm) Memory::memCNew(sizeof(struct pterm));
  LADR_GLOBAL_PARSE.Pterm_gets++;
  return(p);
}  /* get_pterm */



void ParseContainer::free_pterm(Pterm p) {
  Memory::memFree(p, sizeof(struct pterm));
  LADR_GLOBAL_PARSE.Pterm_frees++;
}  /* free_pterm */


/* DOCUMENTATION
This routine prints (to FILE *fp) Memory:: usage statistics for data types
associated with the parse package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void ParseContainer::fprint_parse_mem(ostream &o, const bool heading) const{
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
    n = sizeof(struct token);
  o<<"token       ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_PARSE.Token_gets<<setw(11)<<LADR_GLOBAL_PARSE.Token_frees<<setw(11)<<LADR_GLOBAL_PARSE.Token_gets-LADR_GLOBAL_PARSE.Token_frees<<setw(9)<< ((LADR_GLOBAL_PARSE.Token_gets- LADR_GLOBAL_PARSE.Token_frees) * n) / 1024<<"K"<<endl;

  n = sizeof(struct pterm);
  o<<"pterm       ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_PARSE.Pterm_gets<<setw(11)<<LADR_GLOBAL_PARSE.Pterm_frees<<setw(11)<<LADR_GLOBAL_PARSE.Pterm_gets-LADR_GLOBAL_PARSE.Pterm_frees<<setw(9)<<((LADR_GLOBAL_PARSE.Pterm_gets- LADR_GLOBAL_PARSE.Pterm_frees) * n) / 1024<<"K"<<endl;

}  /* fprint_parse_mem */


/* DOCUMENTATION
This routine prints (to stdout) Memory:: usage statistics for data types
associated with the parse package.
*/

/* PUBLIC */
void ParseContainer::p_parse_mem(void)const{
  fprint_parse_mem(cout, true);
}  /* p_parse_mem */





/* DOCUMENTATION
This routine sets or clears the flag which tells the parser
to automatically translate alpha!=beta to ~(alpha=beta).
This happens in read_term(), which is called by
read_clause(), read_formula(), and read_term_list().
*/

/* PUBLIC */
void ParseContainer::translate_neg_equalities(bool flag) {
  LADR_GLOBAL_PARSE.Translate_neg_equalities = flag;
}  /* translate_neg_equalities */



Term ParseContainer::translate_neg_eq(Term t){
  if (t != NULL && COMPLEX(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++)  ARG(t,i) = translate_neg_eq(ARG(t,i));
    SymbolContainer S;
    if (S.is_symbol(SYMNUM(t), S.neq_sym(), 2)) {
            TermContainer T;
            Term eq_term =  T.get_rigid_term(S.eq_sym(), 2);
            Term not_term = T.get_rigid_term(S.not_sym(), 1);
            ARG(eq_term,0) = ARG(t, 0);
            ARG(eq_term,1) = ARG(t, 1);
            ARG(not_term,0) = eq_term;
            T.free_term(t);
            t = not_term;
    }
  }
  return t;
}  /* translate_neg_eq */




Term ParseContainer::untranslate_neg_eq(Term t) {
  if (t != NULL && COMPLEX(t)) {
        int i;
        SymbolContainer S;
        TermContainer T;
        for (i = 0; i < ARITY(t); i++) ARG(t,i) = untranslate_neg_eq(ARG(t,i));
        if (S.is_symbol(SYMNUM(t), S.not_sym(), 1) && S.is_symbol(SYMNUM(ARG(t,0)), S.eq_sym(), 2)) {
        Term neq_term = T.get_rigid_term(S.neq_sym(), 2);
        ARG(neq_term,0) = ARG(ARG(t,0), 0);
        ARG(neq_term,1) = ARG(ARG(t,0), 1);
        T.free_term(ARG(t,0));
        T.free_term(t);
        t = neq_term;
    }
  }
  return t;
}  /* untranslate_neg_eq */

void ParseContainer::free_pterm_list(Pterm p) {
  Pterm p1;
  TermContainer T;  
  while (p != NULL) {
        if (p->t != NULL) T.zap_term(p->t);
        p1 = p;
        p = p->next;
        free_pterm(p1);
  }
}  /* free_pterm_list */


void ParseContainer::free_token_list(Token p) {
  Token p1;
  StrbufContainer SB;
  while (p != NULL) {
        p1 = p;
        p = p->next;
        SB.set_string_buf(p1->sb);
        if (!SB.null()) 
            SB.zap_string_buf();
        free_token(p1);
  }
}  /* free_token_list */

bool ParseContainer::end_char(char c) {
    return (c == '.');
}  /* end_char */



bool ParseContainer::comment_char(char c)
{
    return (c == COMMENT_CHAR);
}  /* comment_char */

bool ParseContainer::quote_char(char c){
    return (c == LADR_GLOBAL_PARSE.Quote_char);
}  /* quote_char */



bool ParseContainer::punctuation_char(char c) {
    return (c == ',' || c == LADR_GLOBAL_PARSE.Cons_char || c == '(' || c == ')' ||  c == '[' || c == ']' || c == '{' || c == '}' || c == '.');
}  /* punctuation_char */



bool ParseContainer::ordinary_char(char c) {
    return ( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_' || c == '$') );
}  /* ordinary_char */


bool ParseContainer::special_char(char c) {
  /* This allows us to have special chars in the list below. */
  if (quote_char(c) || end_char(c) || comment_char(c) || punctuation_char(c)) return 0;
  else
    return (c == '+'    ||
            c == '-'    ||
            c == '*'    ||
            c == '/'    ||
            c == '\\'   ||
            c == '^'    ||
            c == '<'    ||
            c == '>'    ||
            c == '='    ||
            c == '`'    ||
            c == '~'    ||
            c == '?'    ||
            c == '@'    ||
            c == '&'    ||
            c == '|'    ||
            c == ':'    ||
            c == '!'    ||
            c == '#'    ||
            c == '%'    ||
            c == '\''   ||
            c == '\"'   ||
            c == '.'    ||
            c == ';'    );
}  /* special_char */

bool ParseContainer::white_char(char c) {
  return (  c == ' '  ||
            c == '\t' ||  /* tab */
            c == '\n' ||  /* newline */
            c == '\v' ||  /* vertical tab */
            c == '\r' ||  /* carriage return */
            c == '\f');   /* form feed */
}  /* white_char */


bool ParseContainer::all_whitespace(String_buf sb) {
  StrbufContainer SB;
  int i = 0;
  bool ok = true;
  SB.set_string_buf(sb);
  int n = SB.sb_size();

  while (ok && i < n) {
    char c = SB.sb_char(i);
    if (white_char(c)) i++;
    else ok = false;
  }
  return ok;
}  /* all_whitespace */



int ParseContainer::getc(istream &in) {
    int c;
    c=in.get();
    return c;
}


int ParseContainer::finish_comment(String_buf sb , int &pos) {

  StrbufContainer SBIN;
  SBIN.set_string_buf(sb);  
  int c;
    if (((c = SBIN.sb_char(pos++)) == 'B') && 
        ((c = SBIN.sb_char(pos++)) == 'E') && 
        ((c = SBIN.sb_char(pos++)) == 'G') && 
        ((c = SBIN.sb_char(pos++)) == 'I') && 
        ((c = SBIN.sb_char(pos++)) == 'N')) {

        /* We have a block comment.  Read up to 'END%' */

        while (true) {
        while (c != 'E' && c != EOF) c = SBIN.sb_char(pos++);
        if (c == '\0') 	return EOF;
        else if (((c = SBIN.sb_char(pos++)) == 'N') && 
            ((c = SBIN.sb_char(pos++)) == 'D') && 
            ((c = SBIN.sb_char(pos++)) == COMMENT_CHAR)) return COMMENT_CHAR;
        else if (c == '\0') return EOF;
        }
    }
    else {
        /* We have a line comment. */
        while (c != '\n' && c != '\0')
        c = SBIN.sb_char(pos++);
    }
    return c;
}


int ParseContainer::finish_comment(istream &in) {

  int c;

  if (((c = getc(in)) == 'B') && 
      ((c = getc(in)) == 'E') && 
      ((c = getc(in)) == 'G') && 
      ((c = getc(in)) == 'I') && 
      ((c = getc(in)) == 'N')) {

    /* We have a block comment.  Read up to 'END%' */

    while (true) {
      while (c != 'E' && c != EOF) c = getc(in);
      if (c == EOF) 	return EOF;
      else if (((c = getc(in)) == 'N') && 
	       ((c = getc(in)) == 'D') && 
	       ((c = getc(in)) == COMMENT_CHAR)) return COMMENT_CHAR;
      else if (c == EOF) return EOF;
    }
  }
  else {
    /* We have a line comment. */
    while (c != '\n' && c != EOF)
      c = getc(in);
  }
  return c;
}  /* finish_comment */



int ParseContainer::read_buf(String_buf sbin, String_buf sbout, int &pos) {
  StrbufContainer SBOUT;
  StrbufContainer SBIN;
  int c;                 /* character read */
  bool end, eof, eof_q;  /* stop conditions */
  
  SBOUT.set_string_buf(sbout);
  SBIN.set_string_buf(sbin);
  end = eof = eof_q = false;
  
  
  
  while (!end && !eof && !eof_q) {
            c = SBIN.sb_char(pos++);
            if (c=='\0')  eof = true; //when the string buf is empty then we return '\0'
            else if (comment_char(c)) {
                    c = finish_comment(SBIN.get_string_buf(), pos);
                    if (c == EOF) eof = true;
                 }
                else {
                        SBOUT.sb_append_char(c);
                        if (end_char(c)) end = true;
                        else  if (quote_char(c)) {  /* quoted string */
                                    int qc = c;
                                    c = SBIN.sb_char(pos++);
                                    while (c != qc && c != EOF) {
                                        SBOUT.sb_append_char(c);
                                        c=SBIN.sb_char(pos++);
                                    }
                        if (c == EOF) eof_q = true;
                        else SBOUT.sb_append_char(c);
                    }
            }
  }
  sbout=SBOUT.get_string_buf();
  
  if (end) return  (int) Read_rc::READ_BUF_OK;
  else 
      if (eof_q) return (int) Read_rc::READ_BUF_QUOTE_ERROR;
        else {
                /* eof -- make sure that the only things in the buffer are whitespace */
                if (all_whitespace(sbout))  return (int) Read_rc::READ_BUF_EOF;
                else  return (int) Read_rc::READ_BUF_ERROR;
        }
}

int ParseContainer::read_buf(istream &in, String_buf sb) {
  StrbufContainer SB;
  int c;                 /* character read */
  bool end, eof, eof_q;  /* stop conditions */
  
  
  SB.set_string_buf(sb);
  end = eof = eof_q = false;
  while (!end && !eof && !eof_q) {
            c = getc(in);
            if (in.eof())  eof = true;
            else if (comment_char(c)) {
                    c = finish_comment(in);
                    if (c == EOF) eof = true;
                 }
                else {
                        SB.sb_append_char(c);
                        if (end_char(c)) end = true;
                        else  if (quote_char(c)) {  /* quoted string */
                                    int qc = c;
                                    c = getc(in);
                                    while (c != qc && c != EOF) {
                                        SB.sb_append_char(c);
                                        c=getc(in);
                                    }
                        if (c == EOF) eof_q = true;
                        else SB.sb_append_char(c);
                    }
            }
  }
  sb=SB.get_string_buf();
  
  if (end) return  (int) Read_rc::READ_BUF_OK;
  else 
      if (eof_q) return (int) Read_rc::READ_BUF_QUOTE_ERROR;
        else {
                /* eof -- make sure that the only things in the buffer are whitespace */
                if (all_whitespace(sb))  return (int) Read_rc::READ_BUF_EOF;
                else  return (int) Read_rc::READ_BUF_ERROR;
        }
  
}  /* read_buf */



String_buf ParseContainer::max_special_symbol(String_buf sb, int *ip) {
  StrbufContainer SB,TOK_SB;
  
  int m = LADR_GLOBAL_PARSE.Multiple_char_special_syms.longest_string_in_plist();

  char *work = (char *) malloc(m+1);
  int n = 0;
  int i = *ip;
  bool ok = false;

  /* Get the longest possible token. */
  SB.set_string_buf(sb);
  char c = SB.sb_char(i++);
  while (special_char(c) && n < m) {
    work[n++] = c;
    c = SB.sb_char(i++);
  }
  i--;
  work[n] = '\0';

  /* Keep chopping one from the end until a known special symbol is found.
     If none is known, the token is a single character. */

  while (!ok && n > 1) {
    /* printf("checking symbol :%s:\n", work); fflush(stdout); */
    ok = (LADR_GLOBAL_PARSE.Multiple_char_special_syms.position_of_string_in_plist(work) != -1);
    if (!ok) {
      n--;
      work[n] = '\0';
      i--;
    }
  }
  TOK_SB.new_string_buf(work);
  free(work);
  *ip = i;
  return TOK_SB.get_string_buf();
}  /* max_special_symbol */



Token ParseContainer::tokenize(String_buf sb) {
  StrbufContainer SB;
  StrbufContainer TOK;
  
    
  SB.set_string_buf(sb);
  int i = 0;
  char c = SB.sb_char(i);
  Token first, last, tok;
  first = last = NULL;
  
  
  while (!end_char(c) && c != '\0') {
    tok = get_token();  /* delete if not needed, i.e., white space */
    tok->buf_pos = i;
    tok->sb=NULL;
    /* Make sure that each case, when finished, sets c to the next char. */

    if (white_char(c)) {
      do {
           c = SB.sb_char(++i);
      } while (white_char(c));
      free_token(tok);
      tok = NULL;
    }
    else 
     if (punctuation_char(c)) {
      tok->type = TokType::TOK_PUNC;
      tok->c = c;
      c = SB.sb_char(++i);
    }
    else if (ordinary_char(c)) {
      tok->type = TokType::TOK_ORDINARY;
      TOK.new_string_buf();
      while (ordinary_char(c)) {
        TOK.sb_append_char(c);
        c = SB.sb_char(++i);
      }
      tok->sb=TOK.get_string_buf();
    }
    else if (special_char(c)) {
    
      tok->type = TokType::TOK_SPECIAL;
      if (LADR_GLOBAL_PARSE.Simple_parse) {
        /* token is maximal sequence of special chars */
        TOK.new_string_buf();
        while (special_char(c)) {
            TOK.sb_append_char(c);
            c = SB.sb_char(++i);
        }
        tok->sb=TOK.get_string_buf();
      }
      else {
	/* token is maximal sequence that is a known special symbol */
            tok->sb = max_special_symbol(SB.get_string_buf(),&i);
            c = SB.sb_char(i);
      }
    }
    
    else if (quote_char(c)) {
      char qc = c;
      tok->type = TokType::TOK_STRING;
      TOK.new_string_buf();
      TOK.sb_append_char(c);
      do {
            c = SB.sb_char(++i);
            TOK.sb_append_char(c);
      } while (c != qc && c != '\0');
      tok->sb=TOK.get_string_buf();
      if (c == qc) c = SB.sb_char(++i);
    }
    else {
      tok->type = TokType::TOK_UNKNOWN;
      tok->c = c;
      c = SB.sb_char(++i);
    }

    
  
    
    
    if (tok != NULL) {
      if (first == NULL) first = tok;
      else last->next = tok;
      last = tok;
    }
  }  /* while */
  
  
  
  
  return first;
}  /* tokenize */



int ParseContainer::comma_terms(Term t) {
  SymbolContainer S;
  if (ARITY(t) == 0 || !S.is_symbol(SYMNUM(t), ",", 2)) return 1;
  else
    return comma_terms(ARG(t,0)) + comma_terms(ARG(t,1));
}  




void ParseContainer::transfer_comma_term(Term t, Term dest, int *p) {
  SymbolContainer S;
  if (ARITY(t) == 0 || !S.is_symbol(SYMNUM(t), ",", 2)) {
    TermContainer T;
    ARG(dest,*p) = T.copy_term(t);
    (*p)++;
  }
  else {
    transfer_comma_term(ARG(t,0), dest, p);
    transfer_comma_term(ARG(t,1), dest, p);
  }
} 

bool ParseContainer::quantifier(Term t) {
  SymbolContainer S;
  return (S.is_symbol(SYMNUM(t), S.all_sym(),0) || S.is_symbol(SYMNUM(t), S.exists_sym(),0));
}  /* quantifier */


bool ParseContainer::ordinary_constant_string(char *s) {
    while (ordinary_char(*s)) s++;
    return *s == '\0';
}  /* ordinary_constant_string */




bool ParseContainer::ordinary_constant_string(string s) {
  int i=0;  
  int j=s.length();
  while ( i<j && ordinary_char(s.at(i))   ) i++;
  return i==j;
}  /* ordinary_constant_string */


bool ParseContainer::ordinary_constant(Term t) {
  if (ARITY(t) != 0) return false;
  else {
        SymbolContainer S;
        return ordinary_constant_string(S.sn_to_str(SYMNUM(t)));
  }
}  /* ordinary_constant */

bool ParseContainer::quant_prefix(Pterm start, Pterm end) {
  return   quantifier(start->t) && start->next != end && ordinary_constant(start->next->t);
}  /* quant_prefix */

/*************
 *
 *   terms_to_term()
 *
 *   This routine takes a sequence of terms/symbols, and attempts
 *   to construct a term with precedence <= m.
 *   
 *   On success, the resulting term is an entirely new copy.
 *
 *************/


Term ParseContainer::terms_to_term(Pterm start, Pterm end, int m) {
 TermContainer T;
 SymbolContainer S;
   
 
   if (start == end) {
    
    if (T.is_term(start->t, ",", 0)) return NULL;  /* don't allow commas as constants */
    else 
         
            if (ARITY(start->t) == 0 && S.special_parse_type(S.sn_to_str(SYMNUM(start->t))) == 1 &&  m < 998)
            /* Don't allow prefix or postfix as constants.  This prevents expressions
            that would otherwise be ambiguous, like -v(a) and -v&(a->b),
            where - is prefix and v is infix.
            An exception: allow them as members of comma-list, e.g., f(-), [-],
            f(a,_), [a,-].  This allows lex terms, e.g., lex([-,a,b,f,*]).
           */
           return NULL;
        
    else return 
            T.copy_term(start->t);
  }
  else {
    int rc, prec;
    ParseType type;
    string str;
    /* Try for quantified formula; return if successful. */
    if (LADR_GLOBAL_PARSE.Quantifier_precedence <= m && quant_prefix(start, end)) {
      Term t1 = terms_to_term(start->next->next, end, LADR_GLOBAL_PARSE.Quantifier_precedence);
      if (t1 != NULL) {
            Term t = T.get_rigid_term(S.quant_sym(), 3);
            ARG(t,0) = T.copy_term(start->t);
            ARG(t,1) = T.copy_term(start->next->t);
            ARG(t,2) = t1;
            return t;
      }
    }

    /* Try for prefix op; return if successful. */
    if (ARITY(start->t) == 0) {
      str = S.sn_to_str(SYMNUM(start->t));
     
      rc = S.unary_parse_type(str, &prec, &type);
      if (rc && prec <= m && (type == ParseType::PREFIX_PAREN || type == ParseType::PREFIX)) {
        int p = (type == ParseType::PREFIX_PAREN ? prec-1 : prec);
        Term t1 = terms_to_term(start->next, end, p);
        if (t1 != NULL) {
            TermContainer T;
            Term t = T.get_rigid_term(str, 1);
            ARG(t,0) = t1;
            return t;
        }
      }
    }

    /* Try for postfix op; return if successful. */
    if (ARITY(end->t) == 0) {
      str = S.sn_to_str(SYMNUM(end->t));
      rc = S.unary_parse_type(str, &prec, &type);
      if (rc && prec <= m && (type == ParseType::POSTFIX_PAREN || type == ParseType::POSTFIX)) {
            int p = (type == ParseType::POSTFIX_PAREN ? prec-1 : prec);
            Term t1 = terms_to_term(start, end->prev, p);
            if (t1 != NULL) {
                TermContainer T;
                Term t =T.get_rigid_term(str, 1);
                ARG(t,0) = t1;
                return t;
            }
      }
    }

    /* Try for an application, e.g., f(a,b); return if successful. */
   
    if (start->next == end && ARITY(start->t) == 0 && !T.is_term(start->t, ",", 0) && end->parenthesized) {
	
      int num_args = comma_terms(end->t);  /* number of args for application */
	       
      int argnum = 0;
      Term t = T.get_rigid_term(S.sn_to_str(SYMNUM(start->t)), num_args);
      transfer_comma_term(end->t, t, &argnum);
      return t;
    }

    /* Try for infix op; return if successful. */
    if (start->next != end) {
      /* Try each possible infix op, until success or exhausted. */
      Pterm op;
      int backward = 0;

      /* If we parse a long left-associated expression left-to-right,
       * it ends up trying all the different associations before finding
       * the correct one.  Therefore, as a heuristic, if the second
       * symbol is INFIX_LEFT, then we try to parse backward.  This
       * doesn't always work efficiently, for example, with & right
       * and + left, a&...&a -> a+...+a (symmetric shape) is slow both
       * forward and backward.  To speed things up, the user can include
       * parentheses, i.e., (a&...&a) -> (a+...+a).
       */

      if (ARITY(start->next->t) == 0) {
        str = S.sn_to_str(SYMNUM(start->next->t));
        rc = S.binary_parse_type(str, &prec, &type);
        backward = (rc && type == ParseType::INFIX_LEFT);
      }

      op = (backward ? end->prev : start->next);
      while (backward ? op != start : op != end) {
        if (ARITY(op->t) == 0) {
            str = S.sn_to_str(SYMNUM(op->t));
            rc = S.binary_parse_type(str, &prec, &type);
            /* If "v" is infix, prevent "(v)" from being infix. */
            if (rc && prec <= m && !op->parenthesized) {
                Term t1, t2;
                int p1 = (type == ParseType::INFIX || type == ParseType::INFIX_RIGHT ? prec-1 : prec);
                int p2 = (type == ParseType::INFIX || type == ParseType::INFIX_LEFT  ? prec-1 : prec);
	    
                t1 = terms_to_term(start, op->prev, p1);
                if (t1 != NULL) {
                    t2 = terms_to_term(op->next, end, p2);
                    if (t2 == NULL)  T.zap_term(t1);
                    else {
                        Term t = T.get_rigid_term(str, 2);
                        ARG(t,0) = t1;
                        ARG(t,1) = t2;
                        return t;
	      }
	    }
	  }
	 }  /* arity 0 */
        op = (backward ? op->prev : op->next);
     }  /* while (binary attempts) */
    }

    /* nothing works */
    return NULL;
  }  /* start != end */
}  /* terms_to_term */




/*************
 *
 *   next_token()
 *
 *   This routine is called when it's time to move to the next token.
 *   The current token (including any sb) is deleted.  Don't call this
 *   routine if an error is found; instead, set the error message and
 *   return NULL (from whereever you are).
 *
 *************/


void ParseContainer::next_token(Tok_pos p) {
  StrbufContainer TOK;
  Token tok = p->tok;
  p->tok = p->tok->next;
  TOK.set_string_buf(tok->sb);
  if (!TOK.null())  
      TOK.zap_string_buf();
  free_token(tok);
}  /* next_token */


Term toks_to_set(Tok_pos p) {
  p->error_message =  "Set parsing is not available (see end of marked string)";
  p->start_error = 0;
  p->end_error = p->tok->buf_pos;
  return NULL;
}  /* toks_to_set */


/*************
 *
 *   make_a_list()
 *
 *   Prepend, to tail, copies of comma-elements in t.
 *
 *************/


Term ParseContainer::make_a_list(Term t, Term tail) {
  ListtermContainer L;
  TermContainer T; 
  SymbolContainer S;
  if (ARITY(t) == 0 || ! S.is_symbol(SYMNUM(t), ",", 2)) {
    return L.listterm_cons(T.copy_term(t), tail);
  }
  else {
    Term l = make_a_list(ARG(t,1), tail);
    return make_a_list(ARG(t,0), l);
  }
}  /* make_a_list */



void ParseContainer::p_token(Token p) { //only for debugging
  StrbufContainer SB;  
    switch (p->type)  {
        case TokType::TOK_ORDINARY:
        case TokType::TOK_SPECIAL:
        case TokType::TOK_STRING: 
        case TokType::TOK_COMMENT:    
                                    
                                    SB.set_string_buf(p->sb);
                                    SB.p_sb();
                                    break;
        case TokType::TOK_PUNC: 
        case TokType::TOK_UNKNOWN:  cout << p->c<<endl;    
                                    break;
      
                                    
    }
    
}

void ParseContainer::p_pterm(Pterm pt) {
    TermContainer T;
    cout<<"Debug: Pterm list of terms------------------"<<endl;
    for(Pterm aux=pt; aux; aux=aux->next) {
        T.p_term(aux->t);
        cout<<"Parenthesized:"<<aux->parenthesized<<endl;
    }
    cout<<"------------------------------------------"<<endl;
    
}



/*************
 *
 *   toks_to_terms()
 *
 *************/

Pterm ParseContainer::toks_to_terms(Tok_pos p) { //Take a list of tokens and return a double list of pterms.....each element of this list has a term and info about parenthesized
  Term t;
  bool done = false;
  bool error = false;
  bool parenthesized;
  Pterm first = NULL;
  Pterm last = NULL;
  Pterm novo;
  int start_pos;
  TermContainer T;  

  
  while (!done && !error) {
        
        parenthesized = false;//the term are not parenthesized
        t = NULL;
        start_pos = p->tok->buf_pos; //the star position of this term is the str buffer position of the list of tokens
        
        //to ordinary, special, and string tokens
        if (p->tok->type == TokType::TOK_ORDINARY ||p->tok->type == TokType::TOK_SPECIAL ||	p->tok->type == TokType::TOK_STRING ) { 
            StrbufContainer TOK;        //if we have a ordinary, special or string token then we get a term with that
            TOK.set_string_buf(p->tok->sb);
            char *str =TOK.sb_to_malloc_string();
            t = T.get_rigid_term(str, 0);
            free(str);
        }
        else if (p->tok->type == TokType::TOK_PUNC && p->tok->c == ',') {
                /* Special case: comma is both punctuation and operator. */
            t = T.get_rigid_term(",", 0);
        }
        else if (p->tok->type == TokType::TOK_PUNC) {
        if (p->tok->c == '(') {
            next_token(p);
            if (p->tok == NULL) {
            p->error_message = "Extra open parenthesis";
            p->start_error = start_pos;
            p->end_error = -1;
            error = true;
            }
            else {
                t = toks_to_term(p);
                if (t == NULL)  error = true;
                else if (p->tok == NULL || p->tok->c != ')') {
                    p->error_message = "Closing parenthesis expected";
                    p->start_error = start_pos;
                    p->end_error = (p->tok ? p->tok->buf_pos : -1);
                   
                    T.zap_term(t);
                    t = NULL;
                    error = true;
                }
                else  parenthesized = true;
         }
      }
       else if (p->tok->c == '[') {
            t = toks_to_list(p);
            error = (t == NULL);
       }
       else if (p->tok->c == '{') {
        t = toks_to_set(p);
        error = (t == NULL);
      }
      else {
            /* bad punctuation */
            p->error_message = "Unexpected character (see end of marked string)";
            p->start_error = 0;  /* mark whole string */
            p->end_error = p->tok->buf_pos;
            error = true;
      }
    }
    else if (p->tok->type == TokType::TOK_COMMENT) {
      ;  /* do nothing */
    }
    else if (p->tok->type == TokType::TOK_UNKNOWN) {
      p->error_message= "Unexpected character (see end of marked string)";
      p->start_error = 0;  /* mark whole string */
      p->end_error = p->tok->buf_pos;
      error = true;
    }

    if (t != NULL) {
      /* Add a node to the terms list. */
      novo = get_pterm(); //é aqui qu isto de fode tudo
      novo->prev = last;
      if (first == NULL) first = novo;
      else	last->next = novo;
      novo->t = t;
      novo->parenthesized = parenthesized;
      last = novo;
    }

    if (!error) {
      /*  */
      next_token(p);
      if (p->tok == NULL) done = true;
      else if(p->tok->type == TokType::TOK_PUNC && (p->tok->c == ')' || p->tok->c == ']' || p->tok->c == '}' || p->tok->c == LADR_GLOBAL_PARSE.Cons_char)) done = true;
    }

  
      
  }/* while */

  if (error) {
    free_pterm_list(first);
    return NULL;
  }
  else 
        return first;
   
}  /* toks_to_terms */




/*************
 *
 *   toks_to_term()
 *
 *************/

Term ParseContainer::toks_to_term(Tok_pos p) {
  Term t;
  int start_pos = p->tok->buf_pos;
  Pterm terms = toks_to_terms(p); //terms is a list of pterms
 
  if (terms == NULL)    t = NULL;
  else {
    Pterm _end;
    for (_end = terms; _end->next != NULL; _end = _end->next);
    t = terms_to_term(terms, _end, 1000);
    free_pterm_list(terms);
    if (t == NULL) {
      p->error_message = "A term cannot be constructed from the marked string";
      p->start_error = start_pos;
      p->end_error = (p->tok ? p->tok->buf_pos-1 : -1);
    }
  }
  return t;
}  /* toks_to_term */



/*************
 *
 *   toks_to_list()
 *
 *   On entry, current token is [.
 *   On successful exit, current token should be ].
 *
 *************/
Term ParseContainer::toks_to_list(Tok_pos p) {
  /* Assume current token is "[". */
  int start_pos = p->tok->buf_pos;
   TermContainer T;
  next_token(p);
  if (p->tok == NULL) {
    p->error_message= "Extra open bracket \'[\'";
    p->start_error = start_pos;
    p->end_error = -1;
    return NULL;
  }
  else if (p->tok->c == ']') {
      ListtermContainer L;
      return L.get_nil_term();
  }
  else {
    Term cterm = toks_to_term(p);
    if (cterm == NULL)
      return NULL;
    else if (p->tok == NULL ||
	     p->tok->type != TokType::TOK_PUNC ||
	     (p->tok->c != ']' && p->tok->c != LADR_GLOBAL_PARSE.Cons_char)) {
     
      p->error_message = "Character \']\' or \':\' expected in list";
      p->start_error = start_pos;
      p->end_error = p->tok->buf_pos;
      T.zap_term(cterm);
      return NULL;
    }
    else if (p->tok->type == TokType::TOK_PUNC && p->tok->c == LADR_GLOBAL_PARSE.Cons_char) {
      Term tail;
      next_token(p);
      tail = toks_to_term(p);
      if (tail == NULL)
	return NULL;
      else if (p->tok == NULL ||
	       p->tok->type != TokType::TOK_PUNC || p->tok->c != ']') {
	p->error_message = "Character \']\' expected in list";
	p->start_error = start_pos;
	p->end_error = p->tok->buf_pos;
	T.zap_term(cterm);
	T.zap_term(tail);
	return NULL;
      }
      else {
            Term list = make_a_list(cterm, tail);
            T.zap_term(cterm);
            return list;
      }
    }
    else {
      /* current token is ']' */
      ListtermContainer L;
      Term list = make_a_list(cterm, L.get_nil_term());
      TermContainer T;
      T.zap_term(cterm);
      return list;
    }
  }
}  /* toks_to_list */




/*************
 *
 *   fprint_parse_error()
 *
 *   This routine prints an error message, pointing to a
 *   substring of a String_buf.
 *
 *   (There is a similar routine that points to a whole term.)
 *
 *************/
void ParseContainer::fprint_parse_error(ostream &o, string msg, String_buf sb, int start_pos, int end_pos) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  int n = SB.sb_size();
  if (end_pos == -1)   end_pos = n-1;
  o<<"%%%ERROR: "<<msg<<endl<<endl;
  for (int i = 0; i < start_pos; i++)  o<<SB.sb_char(i);
  o<<"%%%%START ERROR%%%%";
  for (int i = start_pos; i <= end_pos; i++) o<<SB.sb_char(i);
  o<<"%%%%END ERROR%%%%";
  for (int i = end_pos+1; i < n; i++) o<<SB.sb_char(i);
  o<<endl;
}  /* fprint_parse_error */


/*************
 *
 *   sread_term()
 *
 *************/

/* DOCUMENTATION
This routine reads a term (from String_buf *sb).  The term may be
in readable form, that is with infix operations and without
extra parentheses.
<P>
If there is no term to be read, NULL is returned.  If an error
occurs, a message is sent to FILE *fout, and fatal error occurs.
<P>
See the documentation on mixfix terms and the routine set_parse_type().
*/

/* PUBLIC */
Term ParseContainer::sread_term(String_buf sb, ostream &o) {
  StrbufContainer SB;
  Token tokens;
  Term t;
  struct tok_pos tp;
  SB.set_string_buf(sb);


  tokens = tokenize(sb); //tokens, is a list with all the tokens in this string buf
  
 
  
  if (tokens == NULL) {
    fprint_parse_error(o, "Empty term (too many periods?)",SB.get_string_buf(), 0,SB.sb_size()-1);
    free_token_list(tokens);
    fatal::fatal_error("sread_term, empty term (too many periods?)");
  }
  tp.tok = tokens; //tp is a strucutre with "tok" with the token list and other info 
  
  tp.error_message = "";
  tp.start_error = 0;
  tp.end_error   = 0;  /* index of last char (not like python) */

  t = toks_to_term(&tp); //t gets the terms found in tp.ok
  
  if (t == NULL) {
    fprint_parse_error(o, tp.error_message, sb, tp.start_error,tp.end_error);
    free_token_list(tp.tok);
    fatal::fatal_error("sread_term error");
  }
  else if (tp.tok != NULL) {
    fprint_parse_error(o,"Unexpected character (extra closing parenthesis?)",sb, 0, tp.tok->buf_pos);
    free_token_list(tp.tok);
    fatal::fatal_error("sread_term error");
  }
  return t;
}  /* sread_term */

//prints a list of tokens

void ParseContainer::p_tokens(Token t) {
  StrbufContainer TOK;
  Token p;
  cout<<"Tokens"<<endl<<endl;
  for (p = t; p; p = p->next) {
    TOK.set_string_buf(p->sb);
    
    if (TOK.null())   printf("%c", p->c);
    else TOK.p_sb();
  }
  cout<<endl<<endl;
}


void ParseContainer::look_for_illegal_symbols(string str) {
  if (str.length() > 1 && myString::string_of_repeated('-', str)) {
    cout<<"bad string: "<<str<<endl;
    fatal::fatal_error("operations that are strings of repeated \"-\" are not allowed");
  }
  else if (str.length() > 1 && myString::string_of_repeated('\'', str))
    fatal::fatal_error("operations that are strings of repeated \"\'\" are not allowed");
}  /* look_for_illegal_symbols */




void ParseContainer::declare_parse_type(string str, int precedence, ParseType type) {
  string str2;
  str2 = process_quoted_symbol(str);  
  if (LADR_GLOBAL_PARSE.Check_for_illegal_symbols) {  
    look_for_illegal_symbols(str2);
    SymbolContainer S;  
    S.set_parse_type(str2, precedence, type);
     
  }
  if (type == ParseType::NOTHING_SPECIAL)   forget_multiple_char_special_syms(str2);
  else   remember_multiple_char_special_syms(str2);
  
} 



string ParseContainer::process_quoted_symbol(string str) {
   if (quote_char(str.at(0))) {
    int sn;
    string str2;
    SymbolContainer S;
    str2=str.substr(1,str.length()-2);
    /* str2 is nwo quote-free. */
    sn = S.str_to_sn(str2, 0);  /* Inserts a copy into the symbol table. */
    /* Free what was returned from new_str_copy above. */
    /* Get the quote-free string out of the symbol table. */
    str2 = S.sn_to_str(sn);
    return str2;
  }
  else return str;
}  /* process_quoted_symbol */




void ParseContainer::forget_multiple_char_special_syms(string str) {
  /* If string is multiple-char-special, add it to a global list
     (which is used for parsing).
  */
  char *s=myString::string_to_new_char_pointer(str);
  
  if (special_char(str.at(0)) &&   str.length() > 1 &&  LADR_GLOBAL_PARSE.Multiple_char_special_syms.string_member_plist(s)) {        
      LADR_GLOBAL_PARSE.Multiple_char_special_syms.plist_remove_string(s);
  }
  free(s); 
}  /* forget_multiple_char_special_syms */


void ParseContainer::remember_multiple_char_special_syms(string str) {
  /* If string is multiple-char-special, add it to a global list
     (which is used for parsing).
  */
  char *s=myString::string_to_new_char_pointer(str);  
  if (special_char(str.at(0)) &&  str.length() > 1 &&  ! LADR_GLOBAL_PARSE.Multiple_char_special_syms.string_member_plist(s) ) {
      LADR_GLOBAL_PARSE.Multiple_char_special_syms.plist_prepend(s);
  }else free(s);
  
}  /* remember_multiple_char_special_syms */

void ParseContainer::set_cons_char(char c) {
  LADR_GLOBAL_PARSE.Cons_char = c;
}  /* set_cons_char */


void ParseContainer::declare_quantifier_precedence(int prec) {
  LADR_GLOBAL_PARSE.Quantifier_precedence = prec;
}  /* declare_quantifier_precedence */

void ParseContainer::declare_standard_parse_types(void) {
    SymbolContainer S;
   
    declare_parse_type(",",                         999, ParseType::INFIX_RIGHT);  
    declare_parse_type(S.attrib_sym(),              810, ParseType::INFIX_RIGHT);
    declare_parse_type(S.iff_sym(),                 800, ParseType::INFIX);
    declare_parse_type(S.imp_sym(),                 800, ParseType::INFIX);
    declare_parse_type(S.impby_sym(),               800, ParseType::INFIX);
    declare_parse_type(S.or_sym(),                  790, ParseType::INFIX_RIGHT);
    declare_parse_type("||",                        790, ParseType::INFIX_RIGHT);  
    declare_parse_type(S.and_sym(),                 780, ParseType::INFIX_RIGHT);
    declare_parse_type("&&",                        780, ParseType::INFIX_RIGHT);  
    declare_quantifier_precedence(750);  
    
    declare_parse_type(S.eq_sym(),                  700, ParseType::INFIX);
    declare_parse_type(S.neq_sym(),                 700, ParseType::INFIX);
    declare_parse_type("==",                        700, ParseType::INFIX);
    declare_parse_type("!==",                       700, ParseType::INFIX);
    
    declare_parse_type("<",                         700, ParseType::INFIX);
    declare_parse_type("<=",                        700, ParseType::INFIX);
    
    declare_parse_type(">",                         700, ParseType::INFIX);
    declare_parse_type(">=",                        700, ParseType::INFIX);
    declare_parse_type("@<",                        700, ParseType::INFIX);
    declare_parse_type("@<=",                       700, ParseType::INFIX);
    declare_parse_type("@>",                        700, ParseType::INFIX);
    declare_parse_type("@>=",                       700, ParseType::INFIX);
    declare_parse_type("+",                         500, ParseType::INFIX);
    declare_parse_type("*",                         500, ParseType::INFIX);
    declare_parse_type("@",                         500, ParseType::INFIX);
    declare_parse_type("/",                         500, ParseType::INFIX);
    declare_parse_type("\\",                        500, ParseType::INFIX);
    declare_parse_type("^",                         500, ParseType::INFIX);
    declare_parse_type("v",                         500, ParseType::INFIX);
    declare_parse_type(S.not_sym(),                 350, ParseType::PREFIX);
    declare_parse_type("\'",                        300, ParseType::POSTFIX);
    set_cons_char(':');  
}  


char ParseContainer::get_cons_char(void){
  return LADR_GLOBAL_PARSE.Cons_char;
}  /* set_cons_char */


/* PUBLIC */
void ParseContainer::set_quote_char(char c) {
  LADR_GLOBAL_PARSE.Quote_char = c;
}  /* set_quote_char */


char ParseContainer::get_quote_char(void) {
return LADR_GLOBAL_PARSE.Quote_char;
}

void ParseContainer::parenthesize(bool setting) {
  LADR_GLOBAL_PARSE.Parenthesize = setting;
}  /* parenthesize */


void ParseContainer::simple_parse(bool setting) {
  LADR_GLOBAL_PARSE.Simple_parse = setting;
}  /* simple_parse */



void ParseContainer::sb_remove_some_space(String_buf sb, int begin, int end) {
 int i;
 StrbufContainer sbuf;
 sbuf.set_string_buf(sb);
 bool in_quote = false;
  for (i = begin; i < end-1; i++) {
    char c = sbuf.sb_char(i);
    if (quote_char(c)) in_quote = !in_quote;
    else if (!in_quote && sbuf.sb_char(i) == ' ') {
      if (sbuf.sb_char(i-1) == '-') {
        if (!special_char(sbuf.sb_char(i-2)))  sbuf.sb_replace_char(i, '\0');
    }
    else if (sbuf.sb_char(i+1) == '\'') {
	if (!special_char(sbuf.sb_char(i+2)))
	    sbuf.sb_replace_char(i, '\0');
      }
    }
  }
    
}
        

void ParseContainer::sb_write_term(String_buf sb, Term t)
{
  TermContainer T;
  StrbufContainer sbuf;
  sbuf.set_string_buf(sb);
  int begin = sbuf.sb_size() + 1;
  int end;
  if (LADR_GLOBAL_PARSE.Translate_neg_equalities) {
    Term temp_term = untranslate_neg_eq(T.copy_term(t));
    arrange_term(sb, temp_term, 1000);
    T.zap_term(temp_term);
  }
  else
    arrange_term(sb, t, 1000);
  end = sbuf.sb_size()+1;
  sb_remove_some_space(sb, begin, end);
} 




void ParseContainer::arrange_term(String_buf sb, Term t, int par_prec) {
 StrbufContainer SBUF; 
 SymbolContainer S;
 TermContainer T;
 ListtermContainer LT;
 SBUF.set_string_buf(sb);
 if (t == NULL)
    SBUF.sb_append("arrange_term gets NULL term");
  else if (VARIABLE(t)) {
    string str;
    S.symbol_for_variable(str, VARNUM(t));
    SBUF.sb_append(str);
  } 

  else if (CONSTANT(t)) {
    if (LT.nil_term(t))
      SBUF.sb_append("[]");
    else
      SBUF.sb_append(S.sn_to_str(SYMNUM(t)));
  } 

  else if (T.is_term(t, S.quant_sym(), 3)) {
    Term q = t;
    SBUF.sb_append("(");
    do {
      arrange_term(sb, ARG(q,0), 1000); 
      SBUF.sb_append(" ");
      arrange_term(sb, ARG(q,1), 1000); 
      SBUF.sb_append(" ");
      q = ARG(q,2);
    } while (T.is_term(q, S.quant_sym(), 3));
    arrange_term(sb, q, LADR_GLOBAL_PARSE.Quantifier_precedence);
    SBUF.sb_append(")");
  } 

  else if (LT.cons_term(t)) {
    Term t1 = t;
    SBUF.sb_append("[");
    while (LT.cons_term(t1)) {
      arrange_term(sb, ARG(t1,0), 1000);
      t1 = ARG(t1,1);
      if (LT.cons_term(t1))	SBUF.sb_append(",");
    }
    if (!LT.nil_term(t1)) {
      SBUF.sb_append(":");
      arrange_term(sb, t1, 1000);
    }
    SBUF.sb_append("]");
  } 

  else {
    ParseType type;
    int op_prec;
    if (ARITY(t) == 2 && S.binary_parse_type(S.sn_to_str(SYMNUM(t)), &op_prec, &type)) {
      int p1 = (type == ParseType::INFIX || type == ParseType::INFIX_RIGHT ? op_prec-1 : op_prec);
      int p2 = (type == ParseType::INFIX || type == ParseType::INFIX_LEFT  ? op_prec-1 : op_prec);
		
      if (op_prec > par_prec || LADR_GLOBAL_PARSE.Parenthesize) SBUF.sb_append("(");
      arrange_term(sb, ARG(t,0), p1);
      SBUF.sb_append(" ");
      SBUF.sb_append(S.sn_to_str(SYMNUM(t)));
      SBUF.sb_append(" ");
      arrange_term(sb, ARG(t,1), p2);
      if (op_prec > par_prec || LADR_GLOBAL_PARSE.Parenthesize) SBUF.sb_append(")");
    }  
    else if (ARITY(t) == 1 &&  S.unary_parse_type(S.sn_to_str(SYMNUM(t)), &op_prec, &type)) {
      int p1 = ((type == ParseType::PREFIX_PAREN || type == ParseType::POSTFIX_PAREN) ?	op_prec-1 : op_prec);
      if (op_prec > par_prec || LADR_GLOBAL_PARSE.Parenthesize)	SBUF.sb_append("(");
      if (type == ParseType::PREFIX_PAREN || type == ParseType::PREFIX) {
        SBUF.sb_append(S.sn_to_str(SYMNUM(t)));
        SBUF.sb_append(" ");
        arrange_term(sb, ARG(t,0), p1);
      }  
      else {
        arrange_term(sb, ARG(t,0), p1);
        SBUF.sb_append(" ");
        SBUF.sb_append(S.sn_to_str(SYMNUM(t)));
      }

      if (op_prec > par_prec || LADR_GLOBAL_PARSE.Parenthesize) SBUF.sb_append(")");

    } 
    else {
      
      int i;
      SBUF.sb_append(S.sn_to_str(SYMNUM(t)));
      SBUF.sb_append("(");
      for (i = 0; i < ARITY(t); i++) {
        arrange_term(sb, ARG(t,i), 1000);
        if (i < ARITY(t)-1) SBUF.sb_append(",");
      }
      SBUF.sb_append(")");
    }  
  }
} 


bool ParseContainer::redeclare_symbol_and_copy_parsetype(string operation, string str,bool echo, ostream &fout) {
  SymbolContainer S;
  string new_str = process_quoted_symbol(str);  /* if quoted, remove quotes */
  
  if (S.symbol_in_use(new_str))
    return false;
  else {
    ParseType type;
    int prec;
    string old_str = S.get_operation_symbol(operation);
    S.set_operation_symbol(operation, new_str);
    remember_multiple_char_special_syms(new_str);  /* for subsequent parsing */
    if (S.unary_parse_type(old_str, &prec, &type) ||
        S.binary_parse_type(old_str, &prec, &type)) {
        S.set_parse_type(new_str, prec, type);
        if (echo) {
            fout<<"  %% op("<<prec<<", "<<S.parse_type_to_str(type)<<", "<<new_str<<").   %copying parse/print properties from "<<old_str<<" to "<<new_str<<endl;
      }
    }
    return true;  /* success */
  }
} 

void ParseContainer::fwrite_term(ostream &o, Term t) {
  StrbufContainer SB;
  TermContainer T;
  SB.new_string_buf();
  
  
  if (LADR_GLOBAL_PARSE.Translate_neg_equalities) {
    Term temp_term = untranslate_neg_eq(T.copy_term(t));
    arrange_term(SB.get_string_buf(), temp_term, 1000);
    T.zap_term(temp_term);
  }
  else
    arrange_term(SB.get_string_buf(), t, 1000);
  SB.fprint_sb(o);
  SB.zap_string_buf();
}  

void ParseContainer::fwrite_term_nl(ostream &o, Term t) {
  fwrite_term(o, t);
  o<<"."<<endl;
}






Term ParseContainer::read_term_from_string_buf(String_buf passed_sb, ostream& fout, int &pos) {
 
 int rc;
  StrbufContainer SB;
  SB.new_string_buf();
  
  String_buf sb=SB.get_string_buf();

  rc=(int) read_buf(passed_sb, sb,pos); //faz a limpeza do buffer original e devolve em sb
 
  
  if (rc ==(int) Read_rc::READ_BUF_EOF) {
      SB.zap_string_buf(); 
      return NULL;
  }
  
  else if (rc !=(int) Read_rc::READ_BUF_OK) {
    string msg;
    switch (rc) {
    case (int)Read_rc::READ_BUF_ERROR:
      msg = "EOF found while reading term (missing period?)";
      break;
    case (int) Read_rc::READ_BUF_QUOTE_ERROR:
      msg = "EOF found while reading quoted string";
      break;
    default:
      msg = "error reading characters from file";
      break;
    }
  
    fprint_parse_error(fout, msg, SB.get_string_buf(), 0,SB.sb_size());
    SB.zap_string_buf(); 
    fatal::fatal_error("read_term error");
    return NULL; 
  }
  else {
    Term t = sread_term(sb, fout); //lets get a term in witch all the kids are the terms in the formula
    SB.zap_string_buf(); 
    if (LADR_GLOBAL_PARSE.Translate_neg_equalities)    t = translate_neg_eq(t);
    return t;
  }
  
    
}



Term ParseContainer::read_term(istream &fin, ostream &fout) {
  int rc;
  StrbufContainer SB;
  SB.new_string_buf();
  
  String_buf sb=SB.get_string_buf();

  rc =(int) read_buf(fin, sb); //sb has all the line until "."

 
  
  if (rc ==(int) Read_rc::READ_BUF_EOF) {
      SB.zap_string_buf(); 
      return NULL;
  }
  
  else if (rc !=(int) Read_rc::READ_BUF_OK) {
    string msg;
    switch (rc) {
    case (int)Read_rc::READ_BUF_ERROR:
      msg = "EOF found while reading term (missing period?)";
      break;
    case (int) Read_rc::READ_BUF_QUOTE_ERROR:
      msg = "EOF found while reading quoted string";
      break;
    default:
      msg = "error reading characters from file";
      break;
    }
  
    fprint_parse_error(fout, msg, SB.get_string_buf(), 0,SB.sb_size());
    SB.zap_string_buf(); 
    fatal::fatal_error("read_term error");
    return NULL; 
  }
  else {
    Term t = sread_term(sb, fout); //lets get a term in witch all the kids are the terms in the formula
    SB.zap_string_buf(); 
    if (LADR_GLOBAL_PARSE.Translate_neg_equalities)    t = translate_neg_eq(t);
    return t;
  }
  

}



Term ParseContainer::parse_term_from_string(const string &s) {
  StrbufContainer SB;
  SB.new_string_buf();
  Term t = sread_term(SB.get_string_buf(), cout);
  SB.zap_string_buf();
  return t;
}

//TODO: [choiwah] see if it can be done with tokenize
Plist ParseContainer::split_string(const char *onlys)
{
  int len = strlen(onlys);
  char* work = new char[len+1];
  int i = 0;
  PlistContainer plc;
  plc.set_head(nullptr);
  while (i < len) {
    while (i < len && white_char(onlys[i])) i++;
    if (i < len) {
      int j = 0;
      while (i < len && !white_char(onlys[i]))
        work[j++] = onlys[i++];
      work[j] = '\0';
      char* p = new char[strlen(work)+1];  //TODO: [choiwah] change to use new_str_copy when and if it is available
      strcpy(p, work);
      plc.plist_append(static_cast<void*>(p));
    }
  }
  free(work);
  return plc.get_head();
} 


