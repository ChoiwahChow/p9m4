#include "fastparse.h"
#include "fatal.h"

#include <iostream>

int Fastparse::Arity[256];      /* table of arities for each symbol */
int Fastparse::Symnum[256];     /* table of arities for each symbol */
int Fastparse::Pos;            /* index for parsing */


void Fastparse::fast_set_symbol(char c, int arity) {
  SymbolContainer S;
  if (c >= 'r' && c <= 'z') {
    fatal::fatal_error("fast_set_symbol, r--z are variables and cannot be declared");
  }
  else {
    char str[2];
    str[0] = c;
    str[1] = '\0';
    Arity[(int) c] = arity;
    Symnum[(int) c] = S.str_to_sn(str, arity);
  }
} 


void Fastparse::fast_set_defaults(void) {
  fast_set_symbol('=', 2);
  fast_set_symbol('m', 2);
  fast_set_symbol('j', 2);
  fast_set_symbol('f', 2);
  fast_set_symbol('d', 2);
  fast_set_symbol('*', 2);
  fast_set_symbol('+', 2);
  fast_set_symbol('/', 2);
  fast_set_symbol('c', 1);
  fast_set_symbol('g', 1);
  fast_set_symbol('i', 1);
  fast_set_symbol('-', 1);
  fast_set_symbol('~', 1);
  fast_set_symbol('\'', 1);
}

Term Fastparse::fast_parse(string s) {
  char c = s.at(Pos++);
  Term t;
  TermContainer T;
  SymbolContainer S;

  if (c >= 'r' && c <= 'z') {
    switch (c) {
    case 'z': t = T.get_variable_term(0); break;
    case 'y': t = T.get_variable_term(1); break;
    case 'x': t = T.get_variable_term(2); break;
    case 'w': t = T.get_variable_term(3); break;
    case 'v': t = T.get_variable_term(4); break;
    case 'u': t = T.get_variable_term(5); break;
    case 't': t = T.get_variable_term(6); break;
    case 's': t = T.get_variable_term(7); break;
    case 'r': t = T.get_variable_term(8); break;
    default:  t = NULL;
    }
    return t;
  }
  else {
    int i;
    if (Symnum[(int) c] == 0) {
      /* Undeclared symbol; make it a constant. */
      char str[2];
      str[0] = c; str[1] = '\0';
      Symnum[(int) c] = S.str_to_sn(str, 0);
    }
    t = T.get_rigid_term_dangerously(Symnum[(int) c], Arity[(int) c]);
    for (i = 0; i < Arity[(int) c]; i++) {
      ARG(t,i) = fast_parse(s);
    }
    return t;
  }
} 


Term Fastparse::fast_read_term(istream &fin, ostream &fout) {
  string line;
  
  Term t;
  fin >> line;
  
  while (line.at(0) = '%')  { /* send comment lines to stdout */
    cout<<line<<endl;
	fin >>line;
  }
  if (line.length()==0) return NULL;
  else {
    Pos = 0;
    t = fast_parse(line);
    if (line.at(Pos) != '.') {
	  cerr<<line;
	  cout<<line;
      fatal::fatal_error("fast_read_term, term ends before period.");
    }
    return t;
  }
}


void Fastparse::fast_fwrite_term(ostream &fp, Term t) {
  char c;
  SymbolContainer S;
  if (VARIABLE(t)) {
    switch (VARNUM(t)) {
    case 0: c = 'z'; break;
    case 1: c = 'y'; break;
    case 2: c = 'x'; break;
    case 3: c = 'w'; break;
    case 4: c = 'v'; break;
    case 5: c = 'u'; break;
    case 6: c = 't'; break;
    case 7: c = 's'; break;
    case 8: c = 'r'; break;
    default: c = '?'; break;
    }
    fp<<c;
	
  }
  else {
    int i;
    fp<<S.sn_to_str(SYMNUM(t));
	for (i = 0; i < ARITY(t); i++)
      fast_fwrite_term(fp, ARG(t,i));
  }
} 

void Fastparse::fast_fwrite_term_nl(ostream &fp, Term t) {
  fast_fwrite_term(fp, t);
  fp<<"."<<endl;
}


Topform Fastparse::fast_read_clause(istream &fin, ostream &fout) {
  /* This is different from read_clause() in the following way.

     Read_clause() first calls read_term(), which does NOT set
     variables.  Variables are set after term_to_clause().

     Fast_read_clause() first calls fast_read_term(), which DOES set
     variables, so a call to clause_set_variables() is not needed.
  */
  Term t;
  TopformContainer TF;
  TermContainer T;

  t = fast_read_term(fin, fout);
  if (t == NULL)
    return NULL;
  else {
    Topform c = TF.term_to_clause(t);
    T.zap_term(t);
    TF.upward_clause_links(c);
    return c;
  }
}


void Fastparse::fast_fwrite_clause(ostream &fp, Topform c)
{
  TopformContainer TF;
  TermContainer T;
  Term t = TF.topform_to_term(c);

  if (t == NULL)
    fatal::fatal_error("fwrite_clause, clause_to_term returns NULL.");
  fast_fwrite_term_nl(fp, t);
  fp<<endl;
  T.zap_term(t);
}
