#include "int_code.h"
#include "ibuffer.h"
#include "fatal.h"
#include <iostream>



void IntCode::put_ilist_to_ibuf(Ibuffer Ibuf, Ilist p) {
  /* size of Ilist, then members */
  IlistContainer I;
  I.set_head(p);
  Ibuf.ibuf_write(I.ilist_count());
  for (; p; p = p->next)
    Ibuf.ibuf_write(p->i);
}



Ilist IntCode::get_ilist_from_ibuf(Ibuffer Ibuf) {
  /* size of Ilist, then members */
  int i;
  IlistContainer P;
  int n = Ibuf.ibuf_xread();
  for (i = 0; i < n; i++)
    P.ilist_append(Ibuf.ibuf_xread());
  return P.get_head();
}




void IntCode::put_i3list_to_ibuf(Ibuffer Ibuf, I3list p) {
  /* size of I3list, then members */
  I3listContainer I3;
  I3.set_head(p);
  Ibuf.ibuf_write(I3.i3list_count());
  for (; p; p = p->next) {
    Ibuf.ibuf_write(p->i);
    Ibuf.ibuf_write(p->j);
    Ibuf.ibuf_write(p->k);
  }
}



I3list IntCode::get_i3list_from_ibuf(Ibuffer Ibuf) {
  /* size of I3list, then members */
  int i;
  I3listContainer P;
  int n = Ibuf.ibuf_xread();
  for (i = 0; i < n; i++) {
    int i1 = Ibuf.ibuf_xread();
    int i2 = Ibuf.ibuf_xread();
    int i3 = Ibuf.ibuf_xread();
    P.i3list_append(i1, i2, i3);
  }
  return P.get_head();
} 


void IntCode::put_term_to_ibuf(Ibuffer Ibuf, Term t) {
  if (VARIABLE(t))
    Ibuf.ibuf_write(-VARNUM(t));
  else {
    int i;
    Ibuf.ibuf_write(SYMNUM(t));
    for (i = 0; i < ARITY(t); i++) {
      put_term_to_ibuf(Ibuf, ARG(t,i));
    }
  }
}



Term IntCode::get_term_from_ibuf(Ibuffer Ibuf) {
  TermContainer T;
  SymbolContainer S;
  int a = Ibuf.ibuf_xread();
  if (a <= 0)
    return T.get_variable_term(-a);
  else {
    Term t;
    int i;
    int arity = S.sn_to_arity(a);
    if (arity == -1) {
	  cout<<"bad symnum: "<<a<<endl;
      fatal::fatal_error("get_term_from_ibuf, symbol not in symbol table");
    }
    t = T.get_rigid_term_dangerously(a, arity);
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = get_term_from_ibuf(Ibuf);
    return t;
  }
} 

//Just_type 



void IntCode::put_just_to_ibuf(Ibuffer Ibuf, Just j) {
  /* size of Just, then members */
  JustContainer J;
  Ibuf.ibuf_write(J.just_count(j));

  for (; j; j = j->next) {
    Ibuf.ibuf_write((int)j->type);
    switch (j->type) {
    case Just_type::INPUT_JUST:
    case Just_type::GOAL_JUST:
      /* nothing */
      break;
    case Just_type::CLAUSIFY_JUST:
    case Just_type::DENY_JUST:
    case Just_type::COPY_JUST:
    case Just_type::BACK_DEMOD_JUST:
    case Just_type::BACK_UNIT_DEL_JUST:
    case Just_type::FLIP_JUST:
    case Just_type::XX_JUST:
    case Just_type::MERGE_JUST:
    case Just_type::NEW_SYMBOL_JUST:
      /* integer */
      Ibuf.ibuf_write(j->u.id);
      break;
    case Just_type::BINARY_RES_JUST:
    case Just_type::HYPER_RES_JUST:
    case Just_type::UR_RES_JUST:
    case Just_type::UNIT_DEL_JUST:
    case Just_type::FACTOR_JUST:
    case Just_type::XXRES_JUST:
      /* list of integers */
      put_ilist_to_ibuf(Ibuf, j->u.lst);
      break;
    case Just_type::DEMOD_JUST:
      /* list of integer triples */
      put_i3list_to_ibuf(Ibuf, j->u.demod);
      break;
    case Just_type::PARA_JUST:
    case Just_type::PARA_FX_JUST:
    case Just_type::PARA_IX_JUST:
    case Just_type::PARA_FX_IX_JUST:
            Ibuf.ibuf_write(j->u.para->from_id);
            put_ilist_to_ibuf(Ibuf,j->u.para->from_pos);
            Ibuf.ibuf_write(j->u.para->into_id);
            put_ilist_to_ibuf(Ibuf, j->u.para->into_pos);
      break;
    default: fatal::fatal_error("put_just_to_ibuf, unknown type");
    }
  }
}




Just IntCode::get_just_from_ibuf(Ibuffer Ibuf) {
  /* size of Just, then members */
  JustContainer J;
  Just j_collect = NULL;
  int i;
  int n = Ibuf.ibuf_xread();
  for (i = 0; i < n; i++) {
    int type = Ibuf.ibuf_xread();
    Just j = J.get_just();
    j->type = (Just_type) type;
    switch ((Just_type) type) {
    case Just_type::INPUT_JUST:
    case Just_type::GOAL_JUST:
      /* nothing */
      break;
    case Just_type::CLAUSIFY_JUST:
    case Just_type::DENY_JUST:
    case Just_type::COPY_JUST:
    case Just_type::BACK_DEMOD_JUST:
    case Just_type::BACK_UNIT_DEL_JUST:
    case Just_type::FLIP_JUST:
    case Just_type::XX_JUST:
    case Just_type::MERGE_JUST:
    case Just_type::NEW_SYMBOL_JUST:
      /* integer */
      j->u.id = Ibuf.ibuf_xread();
      break;
    case Just_type::BINARY_RES_JUST:
    case Just_type::HYPER_RES_JUST:
    case Just_type::UR_RES_JUST:
    case Just_type::UNIT_DEL_JUST:
    case Just_type::FACTOR_JUST:
    case Just_type::XXRES_JUST:
      /* list of integers */
      j->u.lst = get_ilist_from_ibuf(Ibuf);
      break;
    case Just_type::DEMOD_JUST:
      /* list of integer triples */
      j->u.demod = get_i3list_from_ibuf(Ibuf);
      break;
    case Just_type::PARA_JUST:
    case Just_type::PARA_FX_JUST:
    case Just_type::PARA_IX_JUST:
    case Just_type::PARA_FX_IX_JUST:
      j->u.para = J.get_parajust();
      j->u.para->from_id = Ibuf.ibuf_xread();
      j->u.para->from_pos = get_ilist_from_ibuf(Ibuf);
      j->u.para->into_id =  Ibuf.ibuf_xread();
      j->u.para->into_pos =  get_ilist_from_ibuf(Ibuf);
      break;
    default:
	  cout <<"unknown just type:  "<<type<<endl;
      fatal::fatal_error("get_just_from_ibuf, unknown just");
    }
    j_collect = J.append_just(j_collect, j);
  }
  return j_collect;
} 


void IntCode::put_clause_to_ibuf(Ibuffer Ibuf, Topform c) {
  /* id is_formula weight number-of-justs justs lits/atts [atomflags] */
  TopformContainer TF; 	
  TermContainer T;
  Ibuf.ibuf_write(c->id);
  Ibuf.ibuf_write(c->is_formula);
  Ibuf.ibuf_write(c->weight);
  put_just_to_ibuf(Ibuf, c->justification);

  /* literals and attributes */
  {
    Term t = TF.topform_to_term(c);
    put_term_to_ibuf(Ibuf, t);
    T.zap_term(t);
  }

  if (!c->is_formula)
    {
      /* flags on atoms (maximal, oriented, etc.) */
      Literals l;
      for (l = c->literals; l; l = l->next)
	    Ibuf.ibuf_write(l->atom->private_flags);
    }
} 



Topform IntCode::get_clause_from_ibuf(Ibuffer Ibuf) {
  /* id is_formula weight number-of-justs justs lits/atts [atomflags] */
  TopformContainer TF;
  TermContainer T;
  int id = Ibuf.ibuf_xread();
  int is_formula = Ibuf.ibuf_xread();
  int weight = Ibuf.ibuf_xread();
  Just j = get_just_from_ibuf(Ibuf);

  Term t = get_term_from_ibuf(Ibuf);
  Topform c = TF.term_to_topform(t, is_formula);
  T.zap_term(t);

  c->id = id;
  c->is_formula = is_formula;
  c->weight = weight;
  c->justification = j;

  if (!is_formula) {
    Literals l;
    for (l = c->literals; l; l = l->next)
      l->atom->private_flags = Ibuf.ibuf_xread();
  }
  return c;
} 

void IntCode::check_ibuf_clause(Topform c) {
  Ibuffer Ibuf; 
  Ibuf.ibuf_init();
  put_clause_to_ibuf(Ibuf, c);
  Ibuf.p_ibuf();
  Ibuf.ibuf_rewind();
  
  {
    TopformContainer TF;
    
	Topform d = get_clause_from_ibuf(Ibuf);
    TF.fprint_clause(cout, c);
    TF.fprint_clause(cout, d);
    if (!LADRV_GLOBAIS_INST.Lit.clause_ident(c->literals, d->literals))
      fatal::fatal_error("check_ibuf_clause, clasues not ident");
    TF.zap_topform(d);
  }
  Ibuf.ibuf_free();
}
