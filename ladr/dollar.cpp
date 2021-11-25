#include "dollar.h"
#include "symbols.h"
#include "fatal.h"
#include "parautil.h"


int Dollar::Symbols_size;
int * Dollar::Op_codes;
struct rule ** Dollar::Rules;
int Dollar::Local_evals;


void Dollar::init_dollar_eval(Clist rules) {
  SymbolContainer S;
  
  Parautil Pu;
  TermContainer T;
  Clist_pos p;

  Symbols_size = S.greatest_symnum() + 1;
  Op_codes =(int *) calloc(Symbols_size, sizeof(int));
  
  Op_codes[S.str_to_sn("+", 2)]        =(int) Op_Type::SUM_OP;
  Op_codes[S.str_to_sn("*", 2)]        =(int) Op_Type::PROD_OP;
  Op_codes[S.str_to_sn("/", 2)]        =(int) Op_Type::DIV_OP;
  Op_codes[S.str_to_sn("mod", 2)]      =(int) Op_Type::MOD_OP;
  Op_codes[S.str_to_sn("min", 2)]      =(int) Op_Type::MIN_OP;
  Op_codes[S.str_to_sn("max", 2)]      =(int) Op_Type::MAX_OP;
  Op_codes[S.str_to_sn("abs", 2)]      =(int) Op_Type::ABS_OP;
  Op_codes[S.str_to_sn("-", 1)]        =(int) Op_Type::NEG_OP;

  Op_codes[S.str_to_sn("<", 2)]        =(int) Op_Type::LT_OP;
  Op_codes[S.str_to_sn("<=", 2)]       =(int) Op_Type::LE_OP;
  Op_codes[S.str_to_sn(">", 2)]        =(int) Op_Type::GT_OP;
  Op_codes[S.str_to_sn(">=", 2)]       =(int) Op_Type::GE_OP;

  Op_codes[S.str_to_sn("@<", 2)]       =(int) Op_Type::LLT_OP;
  Op_codes[S.str_to_sn("@<=", 2)]      =(int) Op_Type::LLE_OP;
  Op_codes[S.str_to_sn("@>", 2)]       =(int) Op_Type::LGT_OP;
  Op_codes[S.str_to_sn("@>=", 2)]      =(int) Op_Type::LGE_OP;

  Op_codes[S.str_to_sn("==", 2)]       =(int) Op_Type::ID_OP;
  Op_codes[S.str_to_sn("!==", 2)]      =(int) Op_Type::NID_OP;

  Op_codes[S.str_to_sn("variable", 1)] =(int) Op_Type::VAR_OP;
  Op_codes[S.str_to_sn("constant", 1)] =(int) Op_Type::CONST_OP;
  Op_codes[S.str_to_sn("ground", 1)]   =(int) Op_Type::GROUND_OP;

  Op_codes[S.str_to_sn("&&", 2)]       =(int) Op_Type::AND2_OP;
  Op_codes[S.str_to_sn("||", 2)]       =(int) Op_Type::OR2_OP;
  Op_codes[S.str_to_sn("&", 2)]        =(int) Op_Type::AND_OP;
  Op_codes[S.str_to_sn("|", 2)]        =(int) Op_Type::OR_OP;
  Op_codes[S.str_to_sn("if", 3)]       =(int) Op_Type::IF_OP;

  Rules = (rule **) calloc(Symbols_size, sizeof(void *));

  for (p = rules->last; p; p = p->prev) {  /* backward */
    Topform c = (Topform)p->c;
    Term rule = c->literals->atom;
    Term alpha = NULL, beta, condition;
    if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) != 1)
      fatal::fatal_error("demodulator has too many literals");

    Pu.mark_oriented_eq(c->literals->atom);  /* ok if not eq */

    if (!c->literals->sign) {
      condition = NULL;
      alpha = rule;
      beta  = T.get_rigid_term(S.false_sym(), 0);
    }
    else if (T.is_term(rule, "->", 2) &&
	     (T.eq_term(ARG(rule,1)) || T.is_term(ARG(rule,1), "<->", 2))) {
      condition = ARG(rule,0);
      alpha = ARG(ARG(rule,1),0);
      beta  = ARG(ARG(rule,1),1);
    }
    else if (T.is_term(rule, "<-", 2) &&
	     (T.eq_term(ARG(rule,0)) || T.is_term(ARG(rule,0), "<->", 2))) {
      condition = ARG(rule,1);
      alpha = ARG(ARG(rule,0),0);
      beta  = ARG(ARG(rule,0),1);
    }
    else if (T.is_term(rule, "<->", 2) || T.eq_term(rule)) {
      condition = NULL;
      alpha = ARG(rule,0);
      beta  = ARG(rule,1);
    }
    else {
      /* Assume it's an atomic formula to be rewritten to $T. */
      condition = NULL;
      alpha = rule;
      beta  = T.get_rigid_term(S.true_sym(), 0);
    }
    {
      int symnum = SYMNUM(alpha);
      if (symnum >= Symbols_size)
			fatal::fatal_error("init_dollar_eval, symnum too big");
      
    
      struct rule *r = (struct rule *) malloc(sizeof(struct rule));
      r->c = c;
      r->alpha = alpha;
      r->beta = beta;
      r->condition = condition;
      r->next = Rules[symnum];  /* insert at beginning */
      Rules[symnum] = r;
    }
  }
} 


bool Dollar::evaluable_predicate(int symnum) {
  if (symnum >= Symbols_size)
    return false;
  else if (Rules[symnum])
    return true;
  else if (Op_codes[symnum])
    return true;
  else
    return true;
}

Term Dollar::dollar_eval(Term t)
{
  TermContainer T;
  TermOrder TO;
  if (SYMNUM(t) < 0 || SYMNUM(t) >= Symbols_size)
    return NULL;
  else {
    int op_code = Op_codes[SYMNUM(t)];
    if (op_code == 0)
      return NULL;
    else {
      int i0, i1;
      bool b0, b1;
      Term result = NULL;
      switch (op_code) {

	/* INT x INT -> INT */

      case (int) Op_Type::SUM_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.int_to_term(i0 + i1);
	break;
      case (int) Op_Type::PROD_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.int_to_term(i0 * i1);
	break;
      case (int) Op_Type::DIV_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.int_to_term(i0 / i1);
	break;
      case (int) Op_Type::MOD_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.int_to_term(i0 % i1);
	break;
      case (int) Op_Type::MIN_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.int_to_term(i0 < i1 ? i0 : i1);
	break;
      case (int) Op_Type::MAX_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.int_to_term(i0 > i1 ? i1 : i1);
	break;

	/* INT -> INT */

      case (int) Op_Type::ABS_OP:
	if (T.term_to_int(ARG(t,0), &i0))
	  result = T.int_to_term(i0 >= 0 ? i0 : -i0);
	break;

	/* INT -> INT, BOOL->BOOL */

      case (int) Op_Type::NEG_OP:
	if (T.term_to_int(ARG(t,0), &i0))
	  result = T.int_to_term(-i0);
	else if (T.term_to_bool(ARG(t,0), &b0))
	  result = T.bool_to_term(!b0);
	break;

	/* INT x INT -> BOOL */

      case (int) Op_Type::LT_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.bool_to_term(i0 < i1);
	break;
      case (int) Op_Type:: LE_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.bool_to_term(i0 <= i1);
	break;
      case (int) Op_Type::GT_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.bool_to_term(i0 > i1);
	break;
      case (int) Op_Type::GE_OP:
	if (T.term_to_int(ARG(t,0), &i0) && T.term_to_int(ARG(t,1), &i1))
	  result = T.bool_to_term(i0 >= i1);
	break;

	/* BOOL x BOOL -> BOOL */

	/* Ok if one of the args to be non-Bool, e.g., ($T & junk) = junk */

      case (int) Op_Type::AND_OP:
      case (int) Op_Type::AND2_OP:
	if (T.term_to_bool(ARG(t,0), &b0)) {
	  if (b0)
	    result = T.copy_term(ARG(t,1));
	  else
	    result =T.bool_to_term(false);
	}
	else if (T.term_to_bool(ARG(t,1), &b1)) {
	  if (b1)
	    result = T.copy_term(ARG(t,0));
	  else
	    result = T.bool_to_term(false);
	}
	break;

      case (int) Op_Type::OR_OP:
      case (int) Op_Type::OR2_OP:
	if (T.term_to_bool(ARG(t,0), &b0)) {
	  if (b0)
	    result = T.bool_to_term(true);
	  else
	    result = T.copy_term(ARG(t,1));
	}
	else if (T.term_to_bool(ARG(t,1), &b1)) {
	  if (b1)
	    result = T.bool_to_term(true);
	  else
	    result = T.copy_term(ARG(t,0));
	}
	break;

	/* Term x Term -> BOOL */

      case (int) Op_Type::ID_OP:
	result = T.bool_to_term(T.term_ident(ARG(t,0), ARG(t,1)));
	break;

      case (int) Op_Type::NID_OP:
	result = T.bool_to_term(!T.term_ident(ARG(t,0), ARG(t,1)));
	break;

	/* INT x INT -> BOOL */

      case (int) Op_Type::LLT_OP:
	result = T.bool_to_term(TO.term_compare_basic(ARG(t,0),ARG(t,1)) == OrderType::LESS_THAN);
	break;
      case (int) Op_Type::LLE_OP: {
        OrderType r = TO.term_compare_basic(ARG(t,0),ARG(t,1));
        result = T.bool_to_term(r == OrderType::LESS_THAN || r == OrderType::SAME_AS);
	break;
      }
      case (int) Op_Type::LGT_OP:
	result = T.bool_to_term(TO.term_compare_basic(ARG(t,0),ARG(t,1)) == OrderType::GREATER_THAN);
	break;
      case (int) Op_Type::LGE_OP: {
            OrderType r = TO.term_compare_basic(ARG(t,0),ARG(t,1));
            result = T.bool_to_term(r == OrderType::GREATER_THAN || r == OrderType::SAME_AS);
	break;
      }

	/* Term -> BOOL */

      case (int) Op_Type::VAR_OP:
	result = T.bool_to_term(VARIABLE(ARG(t,0)));
	break;

      case (int) Op_Type::CONST_OP:
	result = T.bool_to_term(CONSTANT(ARG(t,0)));
	break;

      case (int) Op_Type::GROUND_OP:
	result = T.bool_to_term(T.ground_term(ARG(t,0)));
	break;

	/* else error */

      default:
		cout<<"bad code is "<<op_code<<endl;
		fatal::fatal_error("dollar_eval: bad opcode");
      }
      return result;
    }
  }
}  


Term Dollar::rewrite_top(Term t, int flag, I3list *steps) {
  TermContainer T;
  UnifyContainer U;
  I3listContainer I3;
  Term t1 = dollar_eval(t);
  if (t1 != NULL) {
    T.zap_term(t);
    Local_evals++;
    return t1;
  }
  else if (SYMNUM(t) >= Symbols_size)
    return t;
  else if (Rules[SYMNUM(t)] == NULL)  /* we know it's not a variable */
    return t;
  else {
    struct rule *r;
    Context c = U.get_context();
    Trail tr;
    for (r = Rules[SYMNUM(t)]; r; r = r->next) {
      Term alpha = r->alpha;
      Term beta  = r->beta;
      Term condition = r->condition;
      tr = NULL;
      if (U.match(alpha, c, t, &tr)) {
	bool ok;
	if (condition == NULL)
	  ok = true;
	else {
	  Term condition_rewritten = rewrite(U.apply(condition, c), flag, steps);
	  ok = T.true_term(condition_rewritten);
	  T.zap_term(condition_rewritten);
	  }
	if (ok) {
	  Term contractum = U.apply_demod(beta, c, flag);
	  U.undo_subst(tr);
	  U.free_context(c);
	  T.zap_term(t);
	  I3.set_head(*steps);
	  if (!I3.i3list_member(r->c->id, 0, 1))
	    *steps = I3.i3list_prepend(r->c->id, 0, 1);  /* for just. */
	  return(rewrite(contractum, flag, steps));
	}
	else
	  U.undo_subst(tr);
      }
    }
    U.free_context(c);
    return t;  /* not rewritten */
  }
}


Term Dollar::rewrite(Term t, int flag, I3list *steps) {
  TermflagContainer TF;
  TermContainer T;
  if (TF.term_flag(t, flag) || VARIABLE(t))
    return t;
  else {
    int op_code = (SYMNUM(t) < Symbols_size ? Op_codes[SYMNUM(t)] : -1);
    int i;

    switch (op_code) {

    /* There are a few cases where we don't evaluate all args first. */

    /* a & b, a && b */

    case (int) Op_Type::AND_OP:
    case (int) Op_Type::AND2_OP:
      ARG(t,0) = rewrite(ARG(t,0), flag, steps);
      if (T.true_term(ARG(t,0))) {
		Term tmp = ARG(t,1);
		T.zap_term(ARG(t,0));
		T.free_term(t);
		return rewrite(tmp, flag, steps);
      }
      else if (T.false_term(ARG(t,0))) {
		T.zap_term(t);
		return T.bool_to_term(false);
      }
      break;

    /* a | b, a || b */

    case (int) Op_Type::OR_OP:
    case (int) Op_Type::OR2_OP:
      ARG(t,0) = rewrite(ARG(t,0), flag, steps);
      if (T.false_term(ARG(t,0))) {
		Term tmp = ARG(t,1);
		T.zap_term(ARG(t,0));
		T.free_term(t);
		return rewrite(tmp, flag, steps);
      }
      else if (T.true_term(ARG(t,0))) {
		T.zap_term(t);
		return T.bool_to_term(true);
      }
      break;
      
    /* if(cond, then_part, else_part) */

    case (int) Op_Type::IF_OP:
      ARG(t,0) = rewrite(ARG(t,0), flag, steps);
      if (T.true_term(ARG(t,0))) {
		Term tmp = ARG(t,1);
		T.zap_term(ARG(t,0));
		T.zap_term(ARG(t,2));
		T.free_term(t);
		return rewrite(tmp, flag, steps);
      }
      else if (T.false_term(ARG(t,0))) {
		Term tmp = ARG(t,2);
		T.zap_term(ARG(t,0));
		T.zap_term(ARG(t,1));
		T.free_term(t);
		return rewrite(tmp, flag, steps);
      }
      break;
    }

    /* rewrite subterms */

    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = rewrite(ARG(t,i), flag, steps);

    /* rewrite top */

    t = rewrite_top(t, flag, steps);

    TF.term_flag_set(t, flag);  /* Mark as fully demodulated. */
    return t;
  }
} 

void Dollar::rewrite_with_eval(Topform c) {
  TermflagContainer TF;
  TermContainer T;
  JustContainer J;
  int flag = TF.claim_term_flag();
  Literals lit;
  I3list steps = NULL;
  I3listContainer I3;
  Local_evals = 0;

  for (lit = c->literals; lit; lit = lit->next) {
    lit->atom = rewrite(lit->atom, flag, &steps);
    TF.term_flag_clear_recursively(lit->atom, flag);
    if (evaluable_predicate(SYMNUM(lit->atom))) {
      fprintf(stdout, "Fails to evaluate: "); T.p_term(lit->atom);
	  cout<<"Fails to evaluate: ";
	  T.p_term(lit->atom);
      fatal::fatal_error("rewrite_with_eval: evaluable_formula fails to evaluate");
    }
  }

  if (steps != NULL) {
    I3.set_head(steps);
	steps = I3.reverse_i3list();
	c->justification = J.append_just(c->justification, J.demod_just(steps));
  }

  if (Local_evals > 0)
    c->justification = J.append_just(c->justification, J.eval_just(Local_evals));

  TF.release_term_flag(flag);
}
