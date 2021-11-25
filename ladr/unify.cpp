#include "unify.h"
#include "fatal.h"
#include "memory.h"
#include "ladrvglobais.h"
#include <iomanip>





GlobalUnify::GlobalUnify() {
    Context_gets=0;
    Context_frees=0;
    Trail_gets=0;
    Trail_frees=0;
    AnyVarsInited=false;
    
    for (int i=0; i<MAX_MULTIPLIERS; i++)
        Multipliers[i]=false;
    
}


GlobalUnify::~GlobalUnify() {

}



/*************
 *
 *   next_available_multiplier()
 *
 *************/
int UnifyContainer::next_available_multiplier(void) const{
  int i;
  for (i = 0; i < MAX_MULTIPLIERS; i++)
    if (!LADR_GLOBAL_UNIFY.Multipliers[i]) {
        LADR_GLOBAL_UNIFY.Multipliers[i] = true;
        return i;
    }
    fatal::fatal_error("next_available_multiplier, none available(infinite loop?).");
  return -1;  /* to quiet compiler */
}


void UnifyContainer::free_context(Context p) const {
  if (LADR_GLOBAL_UNIFY.Multipliers[p->multiplier] == false)
    fatal::fatal_error("free_context, bad multiplier");
  LADR_GLOBAL_UNIFY.Multipliers[p->multiplier] = false;
  Memory::memFree(p, sizeof(struct context));
  LADR_GLOBAL_UNIFY.Context_frees++;
  p=NULL;
}  /* free_context */


Trail UnifyContainer::get_trail(void) const{
  Trail p = (Trail) Memory::memNew(sizeof (struct trail));  /* uninitialized */
  LADR_GLOBAL_UNIFY.Trail_gets++;
  return(p);
}  /* get_trail */


void UnifyContainer::free_trail(Trail p) const{
  Memory::memFree(p, sizeof(struct trail));
  LADR_GLOBAL_UNIFY.Trail_frees++;
  p=NULL;
}  /* free_trail */


/*************
 *
 *   Context get_context()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Context UnifyContainer::get_context(void) const{
  Context p = (Context) Memory::memCNew(sizeof (struct context) );
  p->multiplier = next_available_multiplier();
  LADR_GLOBAL_UNIFY.Context_gets++;
  return(p);
}  /* get_context */




/* PUBLIC */
void UnifyContainer::fprint_unify_mem(ostream &o, const bool heading) const{
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct context);

  o<<"context     ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_UNIFY.Context_gets<<setw(11)<<LADR_GLOBAL_UNIFY.Context_frees<<setw(11)<<LADR_GLOBAL_UNIFY.Context_gets-LADR_GLOBAL_UNIFY.Context_frees;
  o<<setw(9)<<((LADR_GLOBAL_UNIFY.Context_gets -LADR_GLOBAL_UNIFY.Context_frees) * n) / 1024<<"K"<<endl;

  n = sizeof(struct trail);
  o<<"trail       ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_UNIFY.Trail_gets<<setw(11)<<LADR_GLOBAL_UNIFY.Trail_frees<<setw(11)<<LADR_GLOBAL_UNIFY.Trail_gets-LADR_GLOBAL_UNIFY.Trail_frees;
  o<<setw(9)<< ((LADR_GLOBAL_UNIFY.Trail_gets - LADR_GLOBAL_UNIFY.Trail_frees) *n) / 1024<<"K"<<endl;

}  /* fprint_unify_mem */

/*************
 *
 *   p_unify_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) Memory:: usage statistics for data types
associated with the unify package.
*/

/* PUBLIC */
void UnifyContainer::p_unify_mem() const {
  fprint_unify_mem(cout, 1);
}  /* p_unify_mem */


/* DOCUMENTATION
This routine tries to unify two terms in their respective
contexts.  Trail * trp is the address of a Trail.
If successful, the trail is extended (at its front) with
substitutions that were made, and trp is updated to point to
the new beginning of the trail.  If unify fails, the Contexts and
the Trail * are not changed.
<P>
You must make sure, before calling unify(), that no variable v in
t1 or t2 has VARNUM(v) >= MAXVARS.  This is usually accomplished by
calling a routine that renames variables.
<P>
Here is an example how to use unify(),
apply(), and undo_subst().  Assume we have terms t1 and t2.
(Terms t1 and t2 may share variables, but we "separate" the
variables by using different contexts.  That is, variable v1 in
context c1 is different from variable v1 in context c2.)
<PRE>
    {
        Context c1 = get_context();
        Context c2 = get_context();
        Trail tr = NULL;
        if (unify(t1, c1, t2, c2, &tr)) {
            Term t3 = apply(t1, c1);
            Term t4 = apply(t2, c2);
            if (term_ident(t3, t4))
                printf("everything is OK\n");
            else
                printf("something is broken\n");
            undo_subst(tr);
            zap_term(t3);
            zap_term(t4);
        }
        else
            printf("unify fails\n");
        free_context(c1);
        free_context(c2);
    }
</PRE>
*/

/* PUBLIC */
bool UnifyContainer::unify(Term t1, Context c1, Term t2, Context c2,
Trail *trp) {
  Trail tpos, tp, t3;
  int vn1, vn2;
  DEREFERENCE(t1, c1)  /* dereference macro */
  DEREFERENCE(t2, c2)  /* dereference macro */
  /* Now, neither t1 nor t2 is a bound variable. */
  if (VARIABLE(t1)) {
        vn1 = VARNUM(t1);
        if (VARIABLE(t2)) {
            /* both t1 and t2 are variables */
            if (vn1 == VARNUM(t2) && c1 == c2) return true;  /* identical */
            else {
                    BIND_TR(vn1, c1, t2, c2, trp)
                    return true;
            }
        }
        else {
                /* t1 variable, t2 not variable */
                if (occur_check(vn1, c1, t2, c2)) {
                    BIND_TR(vn1, c1, t2, c2, trp) return true;
                }
        else return false;  /* failed occur_check */
        }
  }
  else if (VARIABLE(t2)) {
    /* t2 variable, t1 not variable */
    vn2 = VARNUM(t2);
    if (occur_check(vn2, c2, t1, c1)) {
      BIND_TR(vn2, c2, t1, c1, trp)
      return true;
    }
    else
      return false;  /* failed occur_check */
  }
  else if (SYMNUM(t1) != SYMNUM(t2)) return false;  /* fail because of symbol clash */
  else if (ARITY(t1) == 0)  return true;
  else {  /* both complex with same symbol */
    int i, arity;
    tpos = *trp;  /* save trail position in case of failure */
    i = 0; arity = ARITY(t1);
    while (i < arity && unify(ARG(t1,i), c1, ARG(t2,i), c2, trp)) i++;
    if (i == arity)    return true;
    else {  /* restore trail and fail */
        tp = *trp;
        while (tp != tpos) {
            tp->context->terms[tp->varnum] = NULL;
            tp->context->contexts[tp->varnum] = NULL;
            t3 = tp;
            tp = tp->next;
            free_trail(t3);
        }
        *trp = tpos;
       return false;
    }
  }
}  /* unify */



/* DOCUMENTATION
This function checks if a variable with index vn (in Context vc)
occurs in Term t (in Context c), including the top case, where t
is the variable in question.
*/

/* PUBLIC */
bool UnifyContainer::occur_check(int vn, Context vc, Term t, Context c) {
  if (!c) return true;
  else if (VARIABLE(t)) {  /* variable */
    int tvn;
    tvn = VARNUM(t);
    if (tvn == vn && c == vc) return false;  /* fail occur_check here */
    else if (c->terms[tvn] == NULL)   return true;  /* uninstantiated variable */
    else   return occur_check(vn, vc, c->terms[tvn], c->contexts[tvn]);
  }
  else {  /* constant or complex */
    int i;
    for (i = 0; i < ARITY(t); i++)
      if (!occur_check(vn, vc, ARG(t,i), c)) return false;
      return true;
  }
}  /* occur_check */



bool UnifyContainer::match(Term t1, Context c1, Term t2, Trail *trp) {
  bool ret;
  Ilist anytrp = NULL;

  //printf("MATCH t1: "); p_term(apply(t1,c1)); 
  //printf("      t2: "); p_term(t2); 

  if ((MATCH_HINTS_ANYCONST == true) && (LADR_GLOBAL_TERM.AnyConstsEnabled == true)) {
    int anyctx[MAX_ANYCONSTS];
    int i;
    for (i=0; i<MAX_ANYCONSTS; i++) {
      anyctx[i] = -1;
    }
    ret = match_anyctx(t1, c1, t2, trp, anyctx, &anytrp);
    IlistContainer I(anytrp);
    I.zap_ilist();
  }
  else {
    ret = match_anyctx(t1, c1, t2, trp, NULL, &anytrp);
    /* zap_ilist(anytrp) not needed */
  }

  //printf("  result: %s\n", (ret==TRUE)?"YES":"NO");
  return ret;
}  /* match */


/*************
 *
 *    int match(t1, c1, t2, trail_address) -- one-way unification.
 *
 *        Match returns 1 if t2 is an instance of {t1 in context c1}.
 *    This is not a very general version, but it is useful for
 *    demodulation and subsumption.  It assumes that the variables
 *    of t1 and t2 are separate, that none of the variables in t2
 *    have been instantiated, and that none of those t2's variables
 *    will be instantiatied.  Hence, there is no context for t2,
 *    no need to dereference more than one level, and no need for
 *    an occur_check.
 *
 *        The use of the trail is the same as in `unify'.
 *
 *************/

/* DOCUMENTATION
This routine checks if Term t2 (without a Context) is an
instance of Term t1 (in Context c1).
If successful, Context c1 and Trail * trp are updated.
The calling sequence and the use of Contexts and Trails is similar
to those for unify().
*/

/* PUBLIC */

bool UnifyContainer::match2(Term t1, Context c1, Term t2, Trail *trp){
  int vn;
  if (VARIABLE(t1)) {
    vn = VARNUM(t1);
    if (c1->terms[vn] == NULL) {
      BIND_TR(vn, c1, t2, NULL, trp)  return true;
    }
    else { TermContainer T;  return T.term_ident(c1->terms[vn], t2);}
  }
  else if (VARIABLE(t2)) return false;
  else {  /* neither term is a variable */
    if (SYMNUM(t1) != SYMNUM(t2))
      return false;  /* fail because of symbol clash */
    else {
      Trail tpos, tp, t3;
      int i, arity;

      tpos = *trp;  /* save trail position in case of failure */
      i = 0; arity = ARITY(t1);
      while (i < arity && match(ARG(t1,i), c1, ARG(t2,i), trp)) i++;
      if (i == arity) return true;
      else {  /* restore from trail and fail */
            tp = *trp;
            while (tp != tpos) {
                tp->context->terms[tp->varnum] = NULL;
                t3 = tp;
                tp = tp->next;
                free_trail(t3);
            }
            *trp = tpos;
            return false;
        }
    }
  }
}  /* match */


/*************
 *
 *    Term apply(term, context) -- Apply a substitution to a term.
 *
 *    Apply always succeeds and returns a pointer to the
 *    instantiated term.
 *
 *************/

/* DOCUMENTATION
This routine applies the substitution in Context c to Term t.
See the explanation of unify() for an example of the use of apply().
*/

/* PUBLIC */
Term UnifyContainer::apply(Term t, Context c) {
  DEREFERENCE(t, c)

  /* A NULL context is ok.  It happens when c is built by match. */
  /* If the context is NULL, then apply just copies the term.    */
  TermContainer T;  
  if (VARIABLE(t)) {
    if (!c)
      return T.get_variable_term(VARNUM(t));
    else
      return T.get_variable_term(c->multiplier * MAX_VARS + VARNUM(t));
  }
  else {  /* constant or complex term */
    Term t2 = T.get_rigid_term_like(t);
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = apply(ARG(t,i), c);
    return t2;
  }
}  /* apply */

/* PUBLIC */
bool UnifyContainer::variant(Term t1, Context c1, Term t2, Trail *trp)
{
  /* If this gets used a lot, it should be recoded so that it won't
   * traverse the terms twice.
   */
  bool ok;
  Trail tr = NULL;
  Context c2 = get_context();

  if (match(t2, c2, t1, &tr)) {
    undo_subst(tr);
    ok = match(t1, c1, t2, trp);
  }
  else
    ok = 0;

  free_context(c2);
  return ok;
}

/*************
 *
 *    apply_substitute()
 *
 *************/

/* DOCUMENTATION
This routine is like apply(), but when it reaches a particular subterm
(into_term) of the source term (t), it continues with another source
term (beta).
This routine is intended to be used for paramodulation, to avoid
unnecessary work.  For example, when paramodulating alpha=beta into
p[into_term], where alpha unifies with into_term, we construct
the appropriate instance of p[beta] in one step by using this routine.
*/

/* PUBLIC */
Term UnifyContainer::apply_substitute(Term t, Term beta, Context c_from,Term into_term, Context c_into) {
  if (t == into_term)  return apply(beta, c_from);
  else if (VARIABLE(t)) return apply(t, c_into);
  else {
        TermContainer T;
        Term t2 = T.get_rigid_term_like(t);
        int i;
        for (i = 0; i < ARITY(t); i++) ARG(t2,i) = apply_substitute(ARG(t,i), beta, c_from, into_term, c_into);
        return t2;
  }
}  /* apply_substitute */



/*************
 *
 *    apply_substitute2()
 *
 *************/

/* DOCUMENTATION
Similar to apply_substitute, but the into_term is specified with
a position vector instead of the term itself.  This is so that
the into term can be a variable.  (Recall that variables are
probably shared, and we have to specify an *occurrence*.)
*/

/* PUBLIC */
Term UnifyContainer::apply_substitute2(Term t, Term beta, Context c_from, Ilist into_pos, Context c_into) {
  if (into_pos == NULL)  return apply(beta, c_from);
  else if (VARIABLE(t))  return apply(t, c_into);
  else {
        TermContainer T;
        Term t2 = T.get_rigid_term_like(t);
        int arg_pos = into_pos->i - 1;  /* Position vectors count from 1. */
        int i;
        for (i = 0; i < ARITY(t); i++) {
            if (i == arg_pos) ARG(t2,i) = apply_substitute2(ARG(t,i), beta, c_from, into_pos->next, c_into);
            else ARG(t2,i) = apply(ARG(t,i), c_into);
        }
        return t2;
  }
}  /* apply_substitute2 */


/*************
 *
 *    apply_demod()
 *
 *    Special-purpose apply for ordinary demodulation.
 *    Assume every variable in t is instantated by the
 *    substitution.  Terms that come from instantiating
 *    variables get the flag set, indicating that the
 *    term is fully demodulated (assuming inside-out
 *    demodulation).
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term UnifyContainer::apply_demod(Term t, Context c, int flag){
  Term b;
  TermContainer T;
  TermflagContainer TF;
  if (VARIABLE(t)) {
    b = T.copy_term(c->terms[VARNUM(t)]);
    TF.term_flag_set(b, flag);
  }
  else {
    int i;
    b = T.get_rigid_term_like(t);
    for (i = 0; i < ARITY(t); i++) ARG(b,i) = apply_demod(ARG(t,i), c, flag);
  }
  return b;
}  /* apply_demod */



/*************
 *
 *    undo_subst(tr) -- Clear a substitution.
 *
 *************/

/* DOCUMENTATION
This routine clears substitution entries recoded in Trail tr,
and frees the corresponding Trail nodes.
*/

/* PUBLIC */
void UnifyContainer::undo_subst(Trail tr){
  Trail t3;
  while (tr != NULL) {
    tr->context->terms[tr->varnum] = NULL;
    tr->context->contexts[tr->varnum] = NULL;
    t3 = tr;
    tr = tr->next;
    free_trail(t3);
  }
}  /* undo_subst */


/*************
 *
 *    undo_subst_2(trail_1, trail_2) -- Clear part of a substitution.
 *
 *    It is assumed that trail_2 (possibly NULL) is a subtrail
 *    of trail_1. This routine clears entries starting at trail_1,
 *    up to (but not including) trail_2.
 *
 *************/

/* DOCUMENTATION
It is assumed that Trail sub_tr is a subtrail of Trail tr.
This routine clears part (maybe all) of a substitution, by
clearing the entries from tr up to, but not including sub_tr.
The corresponding Trail nodes are deallocated, so the
caller should no longer refer to tr.  (This is useful for
inference rules like hyperresolution, which backtrack,
undoing parts of substitutions.)
*/

/* PUBLIC */
void UnifyContainer::undo_subst_2(Trail tr, Trail sub_tr){
  Trail t3;
  while (tr != sub_tr) {
    tr->context->terms[tr->varnum] = NULL;
    tr->context->contexts[tr->varnum] = NULL;
    t3 = tr;
    tr = tr->next;
    free_trail(t3);
  }
}  /* undo_subst_2 */

/* DOCUMENTATION
This routine prints (to FILE *fp) a Context.
*/

/* PUBLIC */
void UnifyContainer::fprint_context(ostream &o, const Context c) const{
  int i;
  TermContainer T;
  if (c == NULL) o<<"Substitution NULL"<<endl;
  else {
    o<<"Substitution, multiplier"<<c->multiplier<<endl;
    for (i=0; i< MAX_VARS; i++) {
        if (c->terms[i] != NULL) {
        Term t = T.get_variable_term(i);
      
        T.fprint_term(o, t);
        T.free_term(t);
        o<<"["<<  c  <<   "] -> ";
        T.fprint_term(o, c->terms[i]);
        if (c->contexts[i] == NULL) o<<" (NULL context)"<<endl;
        else o<< " ["<< c->contexts[i]<<":"<<c->contexts[i]->multiplier<<"]"<<endl;
      }
    }
#if 0
    if (c->partial_term) {
      printf("partial_term: ");
      print_term(fp, c->partial_term);
      printf("\n");
    }
#endif
  }
}  /* fprint_context */

/* DOCUMENTATION
This routine prints (to stdout) a Context.
*/

/* PUBLIC */
void UnifyContainer::p_context(const Context c) const{
  fprint_context(cout, c);
}  /* p_context */


/*************
 *
 *    fprint_trail(file_ptr, context)
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a Trail.  The whole list is printed.
*/

/* PUBLIC */
void UnifyContainer::fprint_trail(ostream &o, const Trail t)const {
  Trail t2;
  o<<"Trail:";
  t2 = t;
  while (t2 != NULL) {
    o<<"<<"<<t2->varnum<<","<<t2->context<<">";
    t2 = t2->next;
  }
  o<<endl;
}  /* fprint_trail */

/*************
 *
 *    p_trail(context)
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) a Trail.  The whole list is printed.
*/

/* PUBLIC */
void UnifyContainer::p_trail(const Trail t)const {
  fprint_trail(cout, t);
}  /* p_trail */



int UnifyContainer::any_var(int sn) {
  SymbolContainer S;
  int i;
  
  /* Initialize AnyVars if necessary */
  if (LADR_GLOBAL_UNIFY.AnyVarsInited == false) {
    LADR_GLOBAL_UNIFY.AnyVars[0] = S.str_to_sn("_", 0);
    char str[16]; /* this should be enough for any reasonable MAX_ANYVARS */
    for (i=1; i<MAX_ANYVARS; i++) {
      snprintf(str, 16, "_%d", i);
      LADR_GLOBAL_UNIFY.AnyVars[i] = S.str_to_sn(str, 0);
    } 
    LADR_GLOBAL_UNIFY.AnyVarsInited = true;
  }

  /* Lookup sn in AnyVars */
  for (i=0; i<MAX_ANYVARS; i++) {
    if (LADR_GLOBAL_UNIFY.AnyVars[i] == sn) {
      return i;
    }
  }

  return -1;
}


bool UnifyContainer::match_anyvar(int anyvar, Term t2, int *anyvar_ctx) {
  if (!VARIABLE(t2)) {
    return false; /* AnyVars can match only variables */
  }

  if (anyvar == 0) {
    return true; /* AnyVars[0] ("_") matches any variable */
  }

  if (anyvar_ctx[anyvar] == -1) {
    /* AnyVars[anyvar] is not bound yet. */
    int i;
    /* First check whether some other AnyVar is bound to t2. */
    for (i=1; i<MAX_ANYVARS; i++) {
      if (anyvar_ctx[i] == VARNUM(t2)) {
        return false;
      }
    }
    /* No AnyVar is bound to t2, so bind anyvar to t2 in the context */
    anyvar_ctx[anyvar] = VARNUM(t2);
    return true;
  }
  else {
    /* AnyVars[anyvar] was already bound earlier, so check the binding */
    return (anyvar_ctx[anyvar] == VARNUM(t2)) ? true : false;
  }
}


/*************
 *
 *   match_weight()
 *
 *************/

/* DOCUMENTATION
Special-purpose match for weighting.
*/

/* PUBLIC */
bool UnifyContainer::match_weight(Term t1, Context c1, Term t2, Trail *trp, int *anyvar_ctx) {
  SymbolContainer S;
  TermContainer T;
  static int Aconst_sn;
  Aconst_sn=S.str_to_sn("@",0);  
  int anyvar=any_var(SYMNUM(t1));
    
  
//  if (SYMNUM(t1) == var_sn) {
//    return VARIABLE(t2);
//  }
  
  if (anyvar!=-1) {
    return match_anyvar(anyvar, t2, anyvar_ctx);
  }
  else if (SYMNUM(t1) == Aconst_sn) {
    return CONSTANT(t2);
  }
  else if (VARIABLE(t1)) {
    int vn = VARNUM(t1);
    if (c1->terms[vn] == NULL) {
      BIND_TR(vn, c1, t2, NULL, trp)
      return true;
    }
    else return T.term_ident(c1->terms[vn], t2);
  }
  else if (VARIABLE(t2))   return false;
  else {  /* neither term is a variable */
        if (SYMNUM(t1) != SYMNUM(t2)) return false;  /* fail because of symbol clash */
        else {
                Trail tpos, tp, t3;
                int i, arity;
                tpos = *trp;  /* save trail position in case of failure */
                i = 0; arity = ARITY(t1);
                while (i < arity && match_weight(ARG(t1,i), c1, ARG(t2,i), trp, anyvar_ctx)) i++;
                if (i == arity)  return true;
                else {  /* restore from trail and fail */
                        tp = *trp;
                        while (tp != tpos) {
                            tp->context->terms[tp->varnum] = NULL;
                            t3 = tp;
                            tp = tp->next;
                            free_trail(t3);
                        }
                        *trp = tpos;
                        return false;
      }
    }
  }
  

}  /* match_weight */



/* DOCUMENTATION
Return the list of variables (as integers) in a trail.  Note that this
ignores the contexts associated with the varibles.
*/

/* PUBLIC */
Ilist UnifyContainer::vars_in_trail(Trail tr){
  if (tr == NULL) return NULL;
  else  {
            IlistContainer IL;
            IL.set_head(vars_in_trail(tr->next));
            return IL.ilist_append(tr->varnum);
  }

}  /* vars_in_trail */


/*************
 *
 *   context_to_pairs()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist UnifyContainer::context_to_pairs(Ilist varnums, Context c){
  PlistContainer P;
  IlistContainer I;
  TermContainer T;
  ListtermContainer LT;
  I.set_head(varnums);
  
  int i;
  for (i = 0; i < MAX_VARS; i++) {
    if (I.ilist_member(i)) {
        Term var = T.get_variable_term(i);
        Term t = apply(var, c);
        if (!T.term_ident(var, t)) {
        Term pair = LT.listterm_cons(var, t);
        P.plist_append(pair);
      }
      else {
        T.zap_term(var);
        T.zap_term(t);
      }
    }
  }
  return P.get_head();
}  /* context_to_pairs */

/* DOCUMENTATION
*/

/* PUBLIC */
bool UnifyContainer::empty_substitution(Context s){
    for (int i = 0; i < MAX_VARS; i++) {
    if (s->terms[i] != NULL)
      return false;
  }
  return true;
}  /* empty_substitution */


/* DOCUMENTATION
*/

/* PUBLIC */
bool UnifyContainer::variable_substitution(Context s)
{

  for (int i = 0; i < MAX_VARS; i++) {
    if (s->terms[i]) {
      Term t = s->terms[i];
      Context c = s->contexts[i];
      DEREFERENCE(t,c);
      if (!VARIABLE(t)) return false;
    }
  }
  return true;
}  /* variable_substitution */


/* DOCUMENTATION
This routine checks if a subsitution would change a term, if applied.
*/

/* PUBLIC */
bool UnifyContainer::subst_changes_term(Term t, Context c) {
  if (VARIABLE(t)) {
    return c->terms[VARNUM(t)] != NULL;
  }
  else {
        for (int i = 0; i < ARITY(t); i++)
        if (subst_changes_term(ARG(t,i), c)) return true;
        return false;
  }
}  /* subst_changes_term */



bool UnifyContainer::match_anyconst(Term t1, Term t2, int *anyctx, Ilist *anytrp)
{
  TermContainer T;
  int anyconst1 = T.any_const(SYMNUM(t1));
  int anyconst2 = T.any_const(SYMNUM(t2));
  
  if ((anyconst1 == -1) && (anyconst2 == -1)) {
    /* CASE 1: none of the two is anyconst => standard check */
    if (SYMNUM(t1) == SYMNUM(t2))
      return true; /* the constants are equal */
    else
      return false;
  }

  int anyconst;
  Term t;
  if ((anyconst1 != -1) && (anyconst2 != -1)) {
    /* CASE 2: both t1 and t2 are _AnyConst* */
    if ((anyconst1 == 0) || (anyconst2 == 0)) {
      /* both are general _AnyConst */
      return true;
    }
    /* at least one of them is specific _AnyConst_n */
    return (anyconst1 == anyconst2) ? true : false;
  }

  /* CASE 3: only t1 or t2 is _AnyConst* */
  if (anyconst1 == -1) {
    anyconst = anyconst2;
    t = t1;
  }
  else {
    anyconst = anyconst1;
    t = t2;
  }
  /* CASE 3 continues: Can 'anyconst' match 't'? */
  if (anyconst == 0) {
    return true; /* _AnyConst matches any constant */
  }

  if (anyctx[anyconst] == -1) {
    /* AnyConsts[anyconst] is not bound yet. */
    int i;
    /* First check whether some other AnyConst is bound to t. */
    for (i=1; i<MAX_ANYCONSTS; i++) {
      if (anyctx[i] == SYMNUM(t)) {
        return false;
      }
    }
    /* No AnyConst is bound to t, so bind anyconst to t in the context */
    IlistContainer I;
    I.set_head(*anytrp);
    anyctx[anyconst] = SYMNUM(t);
    *anytrp = I.ilist_prepend(anyconst); /* record binding */
    return true;
  }
  else {
    /* AnyConsts[anyconst] was already bound earlier, check the binding */
    return (anyctx[anyconst] == SYMNUM(t)) ? true : false;
  }
}


bool UnifyContainer::match_anyctx(Term t1, Context c1, Term t2, Trail *trp, int *anyctx, Ilist* anytrp) {
TermContainer T;
  int vn;

  if (VARIABLE(t1)) {
    vn = VARNUM(t1);
    if (c1->terms[vn] == NULL) {
      BIND_TR(vn, c1, t2, NULL, trp)
      return true;
    }
    else
      return T.term_ident(c1->terms[vn], t2);
  }
  else if (VARIABLE(t2))
    return false;

  /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
  else if (ARITY(t1) == 0 && ARITY(t2) == 0) { 
    /* both terms are constants */
    if (!anyctx) {
      /* no anyconst matching requested */
      if (SYMNUM(t1) == SYMNUM(t2)) {
        return true; /* the constants are equal */
      }
      return true; /* the constants differ */
    }
    else {
      /* do matching with anyconsts */
      return match_anyconst(t1, t2, anyctx, anytrp);
    }
  }

  else {  /* neither term is a variable */
    if (SYMNUM(t1) != SYMNUM(t2))
      return false;  /* fail because of symbol clash */
    else {
      Trail tpos, tp, t3;
      int i, arity;
      Ilist cpos;

      tpos = *trp;  /* save trail position in case of failure */

      /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
      cpos = *anytrp; /* save "trail" for anyconst bindings */

      i = 0; arity = ARITY(t1);
      while (i < arity && match_anyctx(ARG(t1,i), c1, ARG(t2,i), trp, anyctx, anytrp))
        i++;
      if (i == arity)
        return true;
      else {  /* restore from trail and fail */
        tp = *trp;
        while (tp != tpos) {
          tp->context->terms[tp->varnum] = NULL;
          t3 = tp;
          tp = tp->next;
          free_trail(t3);
        }
        *trp = tpos;

        /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
        IlistContainer I;
        while (*anytrp != cpos) { /* restore anyconst bindings */
          anyctx[(*anytrp)->i] = -1;
          I.set_head(*anytrp);
          *anytrp = I.ilist_pop();
        }

        return false;
      }
    }
  }
    
}
