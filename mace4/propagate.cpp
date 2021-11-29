

#include "../ladr/fatal.h"
#include "../ladr/interp.h"
#include "../ladr/ladrvglobais.h"

#include "propagate.h"

propagate::propagate(const Symbol_data s, int d, const Term* dn, const Cell c, const Symbol_data* Sn, struct mace_stats* m,
                     MstateContainer* mc, EstackContainer* ec, Ground* g, const Mace4GlobalValues* gv, const Mace4VGlobais* mg) :
    Symbols(s), Domain_size(d), Domain(dn), Cells(c), Sn_to_mace_sn(Sn), npi(s, d, Sn), Mstats(m), MScon(mc), EScon(ec), Grounder(g),
    Mace4_gv(gv), Mace4vglobais(mg), Ground_clauses(nullptr)
{
  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->negprop))
    npi.init_negprop_index();
}

propagate::~propagate()
{
  for (Plist g = Ground_clauses; g != nullptr; g = g->next)
    Grounder->zap_mclause(static_cast<Mclause>(g->v));

  PlistContainer p_con;
  p_con.set_head(Ground_clauses);
  p_con.zap_plist();
}

bool
propagate::check_that_ground_clauses_are_true(void)
{
  return Grounder->check_that_ground_clauses_are_true(Ground_clauses);
}

bool
propagate::check_with_arithmetic(void)
{
  return Mace4vglobais->Arith->check_with_arithmetic(Ground_clauses);
}

int
propagate::nterm_id(Term t)
{
  int id = Sn_to_mace_sn[SYMNUM(t)]->base;
  int mult = 1;
  for (int i = ARITY(t)-1; i >= 0; i--) {
    if (VARIABLE(ARG(t,i)))
      id += VARNUM(ARG(t,i)) * mult;
    mult *= Domain_size;
  }
  return id;
}

void
propagate::negprop_elim(int id, Term beta, Mstate state)
{
  const Cell c   = Cells + id;
  int arity      = c->symbol->arity;
  int sym        = c->symbol->mace_sn;
  Term alpha     = c->eterm;

  for (int i = 0; i < arity; i++) {
    Term results = npi.negprop_find_near(1, sym, VARNUM(beta), alpha, i);
    while (results) {
      /* We don't know the orientation. */
      Term found_alpha = (VARIABLE(ARG(results,0)) ? ARG(results,1) : ARG(results,0));

      if (VARIABLE(found_alpha))
        Mstats->neg_elim_agone++;
      else {
        Term e = ARG(found_alpha, i);
        int subterm_id;
        if (!Grounder->eterm(e, &subterm_id))
          Mstats->neg_elim_egone++;
        else {
          Mstats->neg_elim_attempts++;
          new_elimination(subterm_id, ARG(alpha,i), state);
          if (!state->ok)
            return;  /* contradiction */
        }
      }
      results = static_cast<Term>(results->u.vp);
    }
  }
}

void
propagate::negprop_assign(int id, Mstate state)
{
  const Cell c   = Cells + id;
  int arity      = c->symbol->arity;
  int sym        = c->symbol->mace_sn;
  Term alpha     = c->eterm;
  Term beta      = c->value;

  if (c->symbol->type == type_FUNCTION) {
    for (int i = 0; i < arity; i++) {
      Term results = npi.negprop_find_near(0, sym, VARNUM(beta), alpha, i);
      while (results) {
        /* We don't know the orientation. */
        Term found_alpha = (VARIABLE(ARG(results,0)) ? ARG(results,1) : ARG(results,0));
        if (VARIABLE(found_alpha))
          Mstats->neg_assign_agone++;
        else {
          Term e = ARG(found_alpha, i);
          int subterm_id;
          if (!Grounder->eterm(e, &subterm_id))
            Mstats->neg_assign_egone++;
          else {
            Mstats->neg_assign_attempts++;
            new_elimination(subterm_id, ARG(alpha,i), state);
            if (!state->ok)
              return;  /* contradiction */
          }
        }
        results = static_cast<Term>(results->u.vp);
      }
    }
  }

  /* Now make inferences like  f(3,4)=5,  f(3,g(2))=6  ->  g(2)!=4.
     This applies to non-equations as well as equations, because
     P(0) is handled as P(0)=1, and ~P(0) is handled as P(0)=0.
  */

  for (int i = 0; i < arity; i++) {
    int n = (c->symbol->type == type_FUNCTION ? Domain_size : 2);
    for (int j = 0; j < n; j++) {
      if (j != VARNUM(beta)) {
        Term results = npi.negprop_find_near(1, sym, j, alpha, i);
        /* results can look like:  f(1,2)=3,  3=f(1,2),  P(2),  ~P(3)  */
        while (results) {
          Term found_alpha;
          if (c->symbol->type == type_RELATION)
            found_alpha = results;
          else
            found_alpha = (VARIABLE(ARG(results,0)) ? ARG(results,1) : ARG(results,0));
          if (VARIABLE(found_alpha))
            Mstats->neg_assign_agone++;
          else {
            Term e = ARG(found_alpha, i);
            int subterm_id;
            if (!Grounder->eterm(e, &subterm_id))
              Mstats->neg_assign_egone++;
            else {
              Mstats->neg_assign_attempts++;
              new_elimination(subterm_id, ARG(alpha,i), state);
              if (!state->ok)
                return;  /* contradiction */
            }
          }
          results = static_cast<Term>(results->u.vp);
        }
      }
    }
  }
}

void
propagate::negprop_near_elim(int subterm_id, Term alpha, Term beta, int pos, Mstate state)
{
  if (VARIABLE(alpha))
    Mstats->neg_near_elim_agone++;
  else if (VARIABLE(ARG(alpha,pos)))
    Mstats->neg_near_elim_egone++;
  else {
    int id = nterm_id(alpha);
    InterpContainer interp_con;
    int increment = interp_con.int_power(Domain_size, (ARITY(alpha) - 1) - pos);
    for (int i = 0; i < Domain_size; i++) {
      if (Cells[id].value == beta) {
        Mstats->neg_near_elim_attempts++;
        new_elimination(subterm_id, Domain[i], state);
        if (!state->ok)
          return;
      }
      id += increment;
    }
  }
}

void
propagate::negprop_near_assign(int subterm_id, Term alpha, Term beta, int pos, Mstate state)
{
  if (VARIABLE(alpha))
    Mstats->neg_near_assign_agone++;
  else if (VARIABLE(ARG(alpha,pos)))
    Mstats->neg_near_assign_egone++;
  else {
    int base_id = nterm_id(alpha);
    InterpContainer interp_con;
    int increment = interp_con.int_power(Domain_size, (ARITY(alpha) - 1) - pos);
    int id = base_id;

    if (!Mace4_gv->LITERAL(alpha)) {
      for (int i = 0; i < Domain_size; i++) {
        if (Cells[id].possible[VARNUM(beta)] == nullptr) {
          Mstats->neg_near_assign_attempts++;
          new_elimination(subterm_id, Domain[i], state);
          if (!state->ok)
            return;
        }
        id += increment;
      }
    }

    /* Now make inferences like  f(3,g(2))=5,  f(3,4)=6  ->  g(2)!=4.
       This applies to nonequations as well as equations, because
       P(0) is handled as P(0)=1, and ~P(0) is handled as P(0)=0.
    */

    {
      int n = (Mace4_gv->LITERAL(alpha) ? 2 : Domain_size);
      for (int j = 0; j < n; j++) {
        if (j != VARNUM(beta)) {
          int id = base_id;
          for (int i = 0; i < Domain_size; i++) {
            if (Cells[id].value == Domain[j]) {
              Mstats->neg_near_assign_attempts++;
              new_elimination(subterm_id, Domain[i], state);
              if (!state->ok)
                return;
            }
            id += increment;
          }
        }
      }
    }
  }
}

/*************
 *
 *   propagate_negative()
 *
 * There are 4 inference rules to derive new eliminations:
 *
 *   NEG_ELIM   - new clause is an elimination:
 *   NEG_ASSIGN - new clause is a assignment:
 *   NEG_ELIM_NEAR   - new clause is an near elimination:
 *   NEG_ASSIGN_NEAR - new clause is a near assignment:
 *
 *     type             id    alpha       beta     pos      comment
 *
 *   (for the first 2, id gives the alpha, e.g., 32 is g(0))
 *
 *   ASSIGNMENT         32    NULL        NULL      -1   g(0)={in table}
 *   ELIMINATION        32    NULL           2      -1   g(0)!=2
 *
 *   (for the next 4, id gives the eterm subterm, e.g., 32 is g(0))
 *
 *   NEAR_ASSIGNMENT    32    g(g(0))        2       0   g(g(0))=2
 *   NEAR_ASSIGNMENT    32    P(g(0))        1       0   P(g(0))
 *   NEAR_ASSIGNMENT    32    P(g(0))        0       0  ~P(g(0))
 *   NEAR_ELIMINATION   32    g(g(0))        2       0   g(g(0))!=2
 *
 *   Notes:
 *
 *   Eterms and alphas may have been simplified by the time they arrive here.
 *
 *   For ASSIGNMENT, the assignment has already been made
 *   For ELIMINATION, the cross-off has already been done.
 *
 *   If a contradiction is found, set state->ok to false.
 *
 *************/

void
propagate::propagate_negative(int type, int id, Term alpha, Term beta, int pos, Mstate state)
{
  switch (type) {
  case ELIMINATION:
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->neg_elim))
      negprop_elim(id, beta, state);
    break;
  case ASSIGNMENT:
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->neg_assign))
      negprop_assign(id, state);
    break;
  case NEAR_ELIMINATION:
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->neg_elim_near))
      negprop_near_elim(id, alpha, beta, pos, state);
    break;
  case NEAR_ASSIGNMENT:
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->neg_assign_near))
      negprop_near_assign(id, alpha, beta, pos, state);
    break;
  }
}

/*
 *   Build and return the e-term corresponding to the given ID.
 *   This is the inverse of eterm_id.  If it is temporary, make sure
 *   to call zap_mterm(t) when finished with it.
 */
Term
propagate::decode_eterm_id(int id)
{
  /* Assume the id is in range. */
  Symbol_data s = Cells[id].symbol;
  TermContainer  term_con;
  Term t = term_con.get_rigid_term_dangerously(s->sn, s->arity);
  int x = id - s->base;
  InterpContainer interp_con;
  for (int i = s->arity - 1; i >= 0; i--) {
    int p = interp_con.int_power(Domain_size, i);
    int e = x / p;
    ARG(t, (s->arity-1) - i) = Domain[e];
    x = x % p;
  }
  return t;
}

Term
propagate::pvalues_check(Term* a, int n)
{
  //If there is exactly 1 possible value, return it; else return nullptr;

  Term b = nullptr;
  for (int i = 0; i < n; i++) {
    if (a[i] != nullptr) {
      if (b != nullptr)
        return nullptr;
      else
        b = a[i];
    }
  }
  if (b == nullptr)
    fatal::fatal_error("pvalues_check: no possible values\n");
  return b;
}

/*
 *   Given a new unit (which is not an ASSIGNMENT or ELIMINATION),
 *   check to see if it is a NEAR_ASSIGNMENT or NEAR_ELIMINATION.
 *   If so, insert it into the index (which updates the stack),
 *   and put it into the job list.
 *
 *   Note that these operations occur even if the some of the
 *   individual operations are disabled.  However, this routine
 *   should not be called if all negative propagation is disabled.
 */

void
propagate::nterm_check_and_process(Term lit, Mstate state)
{
  int pos;
  int id;
  bool neg = Mace4_gv->NEGATED(lit);
  bool eq = Mace4_gv->EQ_TERM(lit);
  int type = (neg && eq ? NEAR_ELIMINATION : NEAR_ASSIGNMENT);
  if (eq) {
    Term a0 = ARG(lit,0);
    Term a1 = ARG(lit,1);

    if (VARIABLE(a1) && nterm(a0, &pos, &id)) {
      npi.insert_negprop_eq(lit, a0, VARNUM(a1), state, *EScon, *Mace4_gv);
      MScon->job_prepend(state, type, id, a0, a1, pos);
    }
    else if (VARIABLE(a0) && nterm(a1, &pos, &id)) {
      npi.insert_negprop_eq(lit, a1, VARNUM(a0), state, *EScon, *Mace4_gv);
      MScon->job_prepend(state, type, id, a1, a0, pos);
    }
  }
  else if (nterm(lit, &pos, &id)) {
    npi.insert_negprop_noneq(lit, state, *EScon, *Mace4_gv);
    MScon->job_prepend(state, NEAR_ASSIGNMENT, id, lit,
                      (neg ? Domain[0] : Domain[1]), pos);
  }
}

void
propagate::new_assignment(int id, Term value, Mstate state)
{
  ParseContainer pc;
  if (Cells[id].value == nullptr) {
    /* Note that alpha of the new rule is indexed, so the
       rule will rewrite itself.  That IS what we want. */
    state->stack = EScon->update_and_push((void **) &(Cells[id].value), value, state->stack);
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace)) {
      std::cout << "\t\t\t\t\t";
      pc.fwrite_term(std::cout, Cells[id].eterm);
      std::cout << " = " << VARNUM(value) << "\n";
    }
    MScon->job_prepend(state, ASSIGNMENT, id, nullptr, nullptr, -1);
    Mstats->propagations++;
    return;
  }
  else if (Cells[id].value == value)
    return;   /* ok: we already have the rule */
  else {
    /* contradiction: we have an incompatible rule */
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace)) {
      std::cout << "\t\t\t\t\t";
      pc.fwrite_term(std::cout, Cells[id].eterm);
      std::cout << " = " << VARNUM(value) << " BACKUP!\n";
    }
    state->ok = false;
    return;
  }
}

void
propagate::new_elimination(int id, Term beta, Mstate state)
{
  ParseContainer pc;
  if (Cells[id].value == beta) {
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace)) {
      std::cout << "\t\t\t\t\t";
      pc.fwrite_term(std::cout, Cells[id].eterm);
      std::cout << " != " << VARNUM(beta) << " BACKUP!\n";
    }
    state->ok = false;   /* contradiction: cell already has that value! */
    return;
  }
  else if (Cells[id].value != nullptr)
    return;   /* ok: cell already has a (different) value */
  else if (Cells[id].possible[VARNUM(beta)] == nullptr)
    return;   /* ok: already crossed off */
  else {
    /* New unit f(1,2) != 3.  Cross it off and push for negprop. */
    Mstats->cross_offs++;
    state->stack = EScon->update_and_push((void **) &(Cells[id].possible[VARNUM(beta)]),
                                          nullptr, state->stack);
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace)) {
      std::cout << "\t\t\t\t\t";
      pc.fwrite_term(std::cout, Cells[id].eterm);
      std::cout << " != " << VARNUM(beta) << "\n";
    }
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->negprop))
      MScon->job_prepend(state, ELIMINATION, id, nullptr, beta, -1);

    Term value = pvalues_check(Cells[id].possible, Domain_size);
    if (value == nullptr)
      return;  /* ok: nothing more to do */
    else {
      Mstats->rules_from_neg++;
      new_assignment(id, value, state);
    }
  }
}

void
propagate::process_clause(Mclause c, Mstate state)
{
  if (c->subsumed)
    return;
  else if (c->u.active == 0) {
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace))
      std::cout << "\t\t\t\t\t** BACKUP **\n";
    state->ok = false;
    return;
  }
  else if (c->u.active != 1)
    return;   /* nonunit, so do nothing */
  else {
    /* OK, we have a nonsubsumed unit. */
    int i = 0;
    while (Ground::FALSE_TERM(Ground::LIT(c,i), Domain))
      i++;

    Term lit = Ground::LIT(c,i);
    bool negated = Mace4_gv->NEGATED(lit);
    bool eq = Mace4_gv->EQ_TERM(lit);
    Term beta;
    int id;

    if (!eq && Grounder->eterm(lit, &id))
      beta = Domain[negated ? 0 : 1]; /* P(1,2,3) or ~P(1,2,3) */
    else if (eq && Grounder->eterm(ARG(lit,0), &id) && VARIABLE(ARG(lit,1)))
      beta = ARG(lit,1);  /* f(1,2)=3 or f(1,2)!=3 */
    else if (eq && Grounder->eterm(ARG(lit,1), &id) && VARIABLE(ARG(lit,0)))
      beta = ARG(lit,0);  /* 3=f(1,2) or 3!=f(1,2) */
    else {
      if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->negprop))
        /* If it is an nterm, index and insert into job list. */
        nterm_check_and_process(lit, state);
      return;  /* We cannot do anything else with the unit. */
    }

    if (eq && negated)
      new_elimination(id, beta, state);  /* f(1,2) != 3 */
    else
      new_assignment(id, beta, state);   /* f(1,2) = 3, P(0), ~P(0) */
  }
}

Mclause
propagate::handle_literal(Term lit, Term result, Mstate state)
{
  Mclause clause_to_process = nullptr;
  Mclause parent_clause = static_cast<Mclause>(lit->container);
  /* evaluable eterm literal -- this will rewrite to true or false */
  int pos = Ground::lit_position(parent_clause, lit);
  bool negated = Mace4_gv->NEGATED(lit);
  Mstats->rewrite_bools++;
  /* Result should be either 0 or 1, because lit is a literal.
     if the literal is negated, negate the result. */
  if (negated)
    result = (result == Domain[0] ? Domain[1] : Domain[0]);
  state->stack = EScon->update_and_push((void **) (Ground::LIT_l(parent_clause, pos)), result, state->stack);

  /* Now we have to update fields in the clause. */
  if (Ground::FALSE_TERM(result, Domain)) {
    /* decrement the count of active literals */
    state->stack = EScon->update_and_push((void **) &(parent_clause->u.active),
                                          (void *) (parent_clause->u.active-1), state->stack);
    clause_to_process = parent_clause;
  }
  else
    /* mark clause as subsumed */
    state->stack = EScon->update_and_push((void **) &(parent_clause->subsumed),
                                          (void *) true, state->stack);
  return clause_to_process;
}

bool
propagate::nterm(Term t, int *ppos, int *pid)
{
  if (t == nullptr || VARIABLE(t) || Mace4vglobais->Arith->arith_rel_term(t) || Mace4vglobais->Arith->arith_op_term(t))
    return false;
  else {
    int pos = -1;
    for (int i = 0; i < ARITY(t); i++) {
      if (!VARIABLE(ARG(t,i))) {
        if (pos != -1)
          return false;
        else if (Grounder->eterm(ARG(t,i), pid))
          pos = i;
        else
          return false;
      }
    }
    if (pos == -1)
      return false;
    else {
      *ppos = pos;
      return true;
    }
  }
}

void
propagate::propagate_positive(int id, Mstate state)
{
  /*
   *   Propagate a positive assignment.  This includes negated non-equality
   *   atoms.  For example, ~P(1,2,3) is thought of as p(1,2,3) = false.
   *
   *   If a contradiction is found, set state->ok to false.
   */
  for (Term t = Cells[id].occurrences; t != nullptr; t = static_cast<Term>(t->u.vp)) {
    // foreach term the rule applies to
    Term curr = t;

    /* The following loop iterates up toward the root of the clause,
       rewriting terms.  We stop when we get to a literal, an eterm that
       cannot be rewritten (we then index the eterm in this case), or when
       we get to a non-eterm. */
    TermContainer term_con;
    while (!Mace4_gv->LITERAL(curr) &&                        /* stop if literal */
           !Mace4vglobais->Arith->arith_op_term(curr) &&      /* stop if arithmetic term */
           Grounder->eterm(curr, &id) &&                      /* stop if not eterm */
           Cells[id].value != nullptr) {                      /* stop if eterm not evaluable*/
      Term result = Cells[id].value;
      Term parent = static_cast<Term>(curr->container);
      int pos = term_con.arg_position(parent, curr);
      state->stack = EScon->update_and_push((void **) &(ARG(parent,pos)),
                                            result, state->stack);
      Mstats->rewrite_terms++;
      curr = parent;
    }

    Mclause clause_to_process = nullptr;  /* set to possible new rule */
    bool    index_it = false;             /* should curr be indexed? */

    if (Mace4vglobais->Arith->arith_rel_term(curr) || Mace4vglobais->Arith->arith_op_term(curr)) {
      Term    parent_lit = Grounder->containing_mliteral(curr);
      Mclause parent_clause = static_cast<Mclause>(parent_lit->container);
      if (!parent_clause->subsumed) {
        bool evaluated;
        int b = Mace4vglobais->Arith->arith_eval(parent_lit, &evaluated);
        if (evaluated) {
          Term result;
          if (b != 0 && b != 1)
            fatal::fatal_error("propagate_positive, arith_eval should be Boolean");
          result = (b ? Domain[1] : Domain[0]);
          clause_to_process = handle_literal(parent_lit, result, state);
        }
        else if (Mace4_gv->EQ_TERM(curr))
          clause_to_process = parent_clause;
      }
    }
    else if (!Mace4_gv->LITERAL(curr)) {
      /* curr is a term */
      Term parent = static_cast<Term>(curr->container);
      if (id != -1)
        index_it = true;  /* curr is a non-evaluable eterm */
      /* If curr is 1 or 2 steps away from a literal, process it. */
      if (Mace4_gv->LITERAL(parent))
        clause_to_process = static_cast<Mclause>(parent->container);
      else {
        parent = static_cast<Term>(parent->container);
        if (Mace4_gv->LITERAL(parent))
          clause_to_process = static_cast<Mclause>(parent->container);
      }
    }
    else {
      /* curr is a literal (equality or nonquality, positive or negative) */
      Mclause parent_clause = static_cast<Mclause>(curr->container);
      if (!Grounder->eterm(curr, &id))
        /* non-eterm literal */
        clause_to_process = parent_clause;
      else if (Cells[id].value ==   nullptr) {
        /* non-evaluable eterm literal */
        index_it = true;
        clause_to_process = parent_clause;
      }
      else if (!parent_clause->subsumed) {
        clause_to_process = handle_literal(curr, Cells[id].value, state);
      }
    }

    if (index_it) {
      /* curr is an evaluable term or literal, e.g., f(1,2), but there is
       * no rule for it. Therefore, we index it so that it can be
       * found in case a rule appears later. */
      Mstats->indexes++;
      state->stack = EScon->update_and_push((void **) &(curr->u.vp),
                                            Cells[id].occurrences, state->stack);
      state->stack = EScon->update_and_push((void **) &(Cells[id].occurrences),
                                            curr, state->stack);
    }

    if (clause_to_process !=   nullptr) {
      process_clause(clause_to_process, state);
      if (!state->ok)
        return;
    }
  }
}

void
propagate::propagate_all(Mstate state)
{
  /*
   *   Do all of the jobs in the Mstate.  If a contradiction is found,
   *   clean up the Mstate by flushing any undone jobs and restoring
   *   from the stack.
   */
  while (state->ok && state->first_job != nullptr) {
    /* Negative propagation is applied to all types.
       Positive propagation is applied to ASSIGNMENT only.
    */
    //std::cout << "debug propagate_all initial job " << state->first_job << std::endl;
    int type = state->first_job->type;
    int id = state->first_job->id;
    Term alpha = state->first_job->alpha;
    Term beta = state->first_job->beta;
    int pos = state->first_job->pos;
    MScon->job_pop(state);

    if (type == ASSIGNMENT)
      propagate_positive(id, state);

    if (state->ok && LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->negprop))
      propagate_negative(type, id, alpha, beta, pos, state);
  }

  if (!state->ok) {
    MScon->zap_jobs(state);
    EScon->restore_from_stack(state->stack);
    state->stack = nullptr;
  }
}

Estack
propagate::assign_and_propagate(int id, Term value)
{
  /*
   *   Make an assignment, and propagate its effects.
   *   Return the stack of events that occur.  If the propagation
   *   gives a contradiction, return nullptr.
   */
  Estack tmp_stack;
  Mstate state = MScon->get_mstate();

  if (Cells[id].value == value)
    fatal::fatal_error("assign_and_propagate: repeated assignment");
  if (Cells[id].value != nullptr)
    fatal::fatal_error("assign_and_propagate: contradictory assignment");

  /* First make the assignment and initialize the job list. */

  state->stack = EScon->update_and_push((void **) &(Cells[id].value), value, nullptr);
  MScon->job_prepend(state, ASSIGNMENT, id, nullptr, nullptr, -1);

  /* Process the job list (which can grow during propagation). */

  propagate_all(state);

  /* Return the stack (which is nullptr iff we have a contradiction). */

  tmp_stack = state->stack;
  MScon->free_mstate(state);
  return tmp_stack;
}

void
propagate::process_initial_clause(Mclause c, Mstate state)
{
  /*
   *   This routine processes the initial ground clauses.  This includes
   *   checking for the empty clause, checking for unit conflicts, and
   *   unit propagation.  If a contradiction is found, state->ok is set
   *   to false.
   */
  process_clause(c, state);  /* handles empty clause and nonsubsumed units */

  if (state->ok && state->first_job != nullptr)
    propagate_all(state);  /* Process_clause pushed a job, so we propagate it. */
}

void
propagate::instances_recurse(Topform c, int *vals, int *domains, int nextvar, int nvars, Mstate state)
{
  if (nextvar == nvars) {
    TopformContainer tf_cont;
    Term t = tf_cont.topform_to_term_without_attributes(c);
    ParseContainer pc;
    Grounder->subst_domain_elements_term(t, vals);
    t = Grounder->simp_tv(t);
    Mstats->ground_clauses_seen++;
    if (Ground::FALSE_TERM(t, Domain)) {
      fprintf(stdout, "\nNOTE: unsatisfiability detected on input.\n");
      fprintf(stderr, "\nNOTE: unsatisfiability (FALSE_TERM) detected on input.\n");
      state->ok = false;
      return;
    }
    else if (!Ground::TRUE_TERM(t, Domain)) {
      Mclause m = Grounder->term_to_mclause(t);
      for (int i = 0; i < m->numlits; i++) {
        Grounder->eterm_index_term(Ground::LIT(m,i));
        Grounder->set_parent_pointers(Ground::LIT(m,i));
        Ground::LIT(m,i)->container = m;
      }
      process_initial_clause(m, state);
      if (!state->ok) {
        fprintf(stdout, "\nNOTE: unsatisfiability detected on input.\n");
        fprintf(stderr, "\nNOTE: unsatisfiability (TRUE_TERM) detected on input.\n");
        return;
      }
      PlistContainer  p0;
      p0.set_head(Ground_clauses);
      Ground_clauses = p0.plist_prepend(m);
      Mstats->ground_clauses_kept++;
    }
  }
  else if (domains[nextvar] == -1) {
    /* in case the current variable does not appear in the clause */
    instances_recurse(c, vals, domains, nextvar+1, nvars, state);
    if (!state->ok)
      return;
  }
  else {
    for (int i = 0; i < domains[nextvar]; i++) {
      vals[nextvar] = i;
      instances_recurse(c, vals, domains, nextvar+1, nvars, state);
      if (!state->ok)
        return;
    }
  }
}


void
propagate::generate_ground_clauses(Topform c, Mstate state)
{
  // SIDE EFFECT: global Stack is updated.
  int biggest_var, vals[MAX_MACE_VARS], domains[MAX_MACE_VARS];

  LiteralsContainer lit_con;
  biggest_var = lit_con.greatest_variable_in_clause(c->literals);

  for (int i = 0; i <= biggest_var; i++)
    domains[i] = Domain_size;
  instances_recurse(c, vals, domains, 0, biggest_var+1, state);
}
