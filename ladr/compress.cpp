#include "compress.h"
#include "fatal.h"
#include "clauses.h"

Term Compress::uncompress_term(string s, int *ip) {
  TermContainer T;
  SymbolContainer S;
  
  char c = s.at((*ip)++);
  if (c <= 0)
    return T.get_variable_term(-c);
  else {
    int arity = S.sn_to_arity(c);
    Term t = T.get_rigid_term_dangerously(c, arity);
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = uncompress_term(s, ip);
    return t;
  }
}

void Compress::compress_term_recurse(String_buf sb, Term t) {
  StrbufContainer SB;
  SB.set_string_buf(sb);
  if (VARIABLE(t))
    SB.sb_append_char(-VARNUM(t));
  else {
    int i;
    SB.sb_append_char(SYMNUM(t));
    for (i = 0; i < ARITY(t); i++) {
      compress_term_recurse(SB.get_string_buf(), ARG(t,i));
    }
  }
} 

char * Compress::compress_term(Term t) {
  StrbufContainer SB;
  SB.new_string_buf();
  compress_term_recurse(SB.get_string_buf(), t);
  {
	char *s;
    s = SB.sb_to_malloc_char_array();
    SB.zap_string_buf();
    return s;
  }
}

void Compress::compress_clause(Topform c) {
  SymbolContainer S;
  
  TermContainer T;
  if (c->compressed != NULL)
    fatal::fatal_error("compress_clause, clause already compressed");
  else if (-MAX_VARS < CHAR_MIN || S.greatest_symnum() > CHAR_MAX)
    return;  /* unable to compress, because symbols don't fit in char */
  else if (c->literals == NULL)
    return;
  else {
    Term t = LADRV_GLOBAIS_INST.Lit.lits_to_term(c->literals);
    /* printf("compressing clause %d\n", c->id); */
    c->compressed = compress_term(t);
    LADRV_GLOBAIS_INST.Lit.free_lits_to_term(t);
    c->neg_compressed = LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals);
    LADRV_GLOBAIS_INST.Lit.zap_literals(c->literals);
    c->literals = NULL;
  }
}

void Compress::uncompress_clause(Topform c) {
  
  TermContainer T;
  TopformContainer TF;
  if (c->compressed!=NULL) {
    int i = 0;
    Term t = uncompress_term(c->compressed, &i);
    c->literals = LADRV_GLOBAIS_INST.Lit.term_to_literals(t, NULL);
    TF.upward_clause_links(c);
    T.zap_term(t);
    /* printf("UNcompressed clause %d\n", c->id); */
    free(c->compressed);
    c->compressed = NULL;
    c->neg_compressed = false;
    if (!c->used) {
      printf("\n%% Uncompressing unused clause: ");
      cout<<endl<<"%% Uncompressing unused clause: ";
	  TF.fprint_clause(cout, c);
    }
  }
} 

void Compress::uncompress_clauses(Plist p) {
  Plist a;
  Parautil PU;
  for (a = p; a; a = a->next) {
    Topform c = (Topform) a->v;
    if (c->compressed) {
      uncompress_clause(c);
      PU.orient_equalities(c, false);  /* mark, but don't flip */
    }
  }
}  /* uncompress_clauses */
