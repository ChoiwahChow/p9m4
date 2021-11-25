#include "random.h"
#include "fatal.h"
#include <cstdlib> //because rand


int Random::initialized=0;
int Random::symnum[13];
			


Term Random::random_term(int v, int a0, int a1, int a2, int a3, int max_depth) {
  int n, arity, j;
  TermContainer T;
  string s, symbol;

  if (max_depth == 0)
    n = rand() % (v+a0);
  else
    n = rand() % (v+a0+a1+a2+a3);

  if (n < v) {
    arity = -1;  /* variable */
    j = n;
  }
  else if (n < v+a0) {
    arity = 0;
    j = n - v;
  }
  else if (n < v+a0+a1) {
    arity = 1;
    j = n - (v+a0);
  }
  else if (n < v+a0+a1+a2) {
    arity = 2;
    j = n - (v+a0+a1);
  }
  else {
    arity = 3;
    j = n - (v+a0+a1+a2);
  }

  if (arity == -1) {
    return T.get_variable_term(j);
  }
  else {
    int i;
    Term t;

    switch (arity) {
		case 0: s = "a"; break;
		case 1: s = "g"; break;
		case 2: s = "f"; break;
		case 3: s = "h"; break;
		default: s = "?"; break;
    }
    symbol=s+to_string(j);
	t = T.get_rigid_term(symbol, arity);
    for (i = 0; i < arity; i++)
      ARG(t,i) = random_term(v, a0, a1, a2, a3, max_depth-1);
    return t;
  }
}


Term Random::random_nonvariable_term(int v, int a0, int a1, int a2, int a3, int max_depth) {
  TermContainer T;
  Term t = random_term(v, a0, a1, a2, a3, max_depth);
  /* Let's hope this terminates! */
  while (VARIABLE(t)) {
    T.zap_term(t);
    t = random_term(v, a0, a1, a2, a3, max_depth);
  }
  return t;
}  /* random_nonvariable_term */

Term Random::random_complex_term(int v, int a0, int a1, int a2, int a3, int max_depth) {
  TermContainer T;	
  Term t = random_term(v, a0, a1, a2, a3, max_depth);
  /* Let's hope this terminates! */
  while (VARIABLE(t)|| CONSTANT(t)) {
    T.zap_term(t);
    t = random_term(v, a0, a1, a2, a3, max_depth);
  }
  return t;
}

Ilist Random::random_path(int length_max, int value_max) {
  IlistContainer I;
  Ilist first, current, novo;
  int length = (rand() % length_max) + 1;  /* 1 .. length_max */
  int i, value;

  first = current = NULL;
  for (i = 0; i < length; i++) {
    value = (rand() % value_max) + 1;
    novo = I.get_ilist();
    novo->i = value;
    novo->next = NULL;
    if (first == NULL)
      first = novo;
    else
      current->next = novo;
    current = novo;
  }
  return first;
}  


void Random::random_permutation(int *a, int size) {
  int n, i, x;

  for (i = 0; i < size; i++)
    a[i] = -1;

  n = 0;
  while (n < size) {
    x = rand() % size;
    if (a[x] == -1)
      a[x] = n++;
    else {
      i = x+1;
      while (i < size && a[i] != -1)
	i++;
      if (i < size)
	a[i] = n++;
      else {
			i = x-1;
			while (i >= 0 && a[i] != -1) i--;
			if (i < 0) {
				fatal::fatal_error("random_permutation.");
			}
			a[i] = n++;
		}
    }
  }
}


Topform Random::random_clause(int v, int a0, int a1, int a2, int a3,int max_depth, int max_lits) {
  Term t;
  Topform c;
  TopformContainer TF;
 
  int i, sign;
  int n = (rand() % max_lits) + 1;  /* [1 .. max_lits] */
  
  c = TF.get_topform();
  for (i = 0; i < n; i++) {
    sign = rand() % 2;
    t = random_complex_term(v, a0, a1, a2, a3, max_depth);
    c->literals = LADRV_GLOBAIS_INST.Lit.append_literal(c->literals, LADRV_GLOBAIS_INST.Lit.new_literal(sign, t));
  }
  return c;
} 


Term Random::random_op_term(int depth) {
  SymbolContainer S;
  TermContainer T;
  ListtermContainer LT;
  if (!initialized) {
    symnum[0] = S.str_to_sn("a", 0);
    symnum[1] = S.str_to_sn("->", 2);
    symnum[2] = S.str_to_sn("|", 2);
    symnum[3] = S.str_to_sn("&", 2);
    symnum[4] = S.str_to_sn("~", 1);
    symnum[5] = S.str_to_sn("=", 2);
    symnum[6] = S.str_to_sn("+", 2);
    symnum[7] = S.str_to_sn("*", 2);
    symnum[8] = S.str_to_sn("\'", 1);
    symnum[9] = S.str_to_sn("$cons", 2);
    symnum[10] =S.str_to_sn("$nil", 0);
    symnum[11] =S.str_to_sn("$quantified", 1);
    symnum[12] =S.str_to_sn("f", 2);
    initialized = 1;
  }

  if (depth == 0)
    return T.get_rigid_term("b", 0);
  else {
    Term t;
    int arity, i, sn;
    
    sn = symnum[rand() % 13];
    arity = S.sn_to_arity(sn);

    if (S.is_symbol(sn, "$quantified", 1)) {
      Term q = T.get_rigid_term("$quantified", 1);

      Term a = T.get_rigid_term("all", 0);
      Term x = T.get_rigid_term("x", 0);
      Term r = random_op_term(depth-1);

      Term t = LT.get_nil_term();

      t = LT.listterm_cons(r, t);
      t = LT.listterm_cons(x, t);
      t = LT.listterm_cons(a, t);
      
      ARG(q,0) = t;
      return q;
    }
    else {

      t = T.get_rigid_term(S.sn_to_str(sn), arity);
      for (i = 0; i < arity; i++)
		ARG(t,i) = random_op_term(depth-1);
      return t;
    }
  }
} 
