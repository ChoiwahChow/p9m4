#include "ladrvglobais.h"
#include "fpa.h"
#include "memory.h"
#include "symbols.h"
#include "fpalist.h"
#include "fatal.h"
#include <iomanip>
#include <iostream>




GlobalFpaIndex::GlobalFpaIndex() {
    Fpa_id_count=0;
    Next_calls=0;
    Next_calls_overflows=0;

    Fpa_trie_gets=0;
    Fpa_trie_frees=0;

    Fpa_state_gets=0;

    Fpa_index_gets=0;
    Fpa_index_frees=0;
    
}


GlobalFpaIndex::~GlobalFpaIndex() {
}


FpaIndexContainer::FpaIndexContainer() {
    head=NULL;
}

FpaIndexContainer::~FpaIndexContainer() {
    head=NULL;
}



Fpa_trie FpaIndexContainer::get_fpa_trie(void) {
  Fpa_trie p = (Fpa_trie) Memory::memCNew(sizeof(struct fpa_trie));
  p->label = -1;
  LADR_GLOBAL_FPA_INDEX.Fpa_trie_gets++;
  return(p);
}


void FpaIndexContainer::free_fpa_trie(Fpa_trie p) {
  Memory::memFree((void *)p, sizeof(fpa_trie));
  LADR_GLOBAL_FPA_INDEX.Fpa_trie_frees++;
}


Fpa_state FpaIndexContainer::get_fpa_state(void) {
  Fpa_state p = (Fpa_state) Memory::memCNew(sizeof(fpa_state));
  LADR_GLOBAL_FPA_INDEX.Fpa_state_gets++;
  return(p);
}


void FpaIndexContainer::free_fpa_state(Fpa_state p) {
  Memory::memFree((void *)p, sizeof(fpa_state));
  LADR_GLOBAL_FPA_INDEX.Fpa_state_frees++;
}


Fpa_index FpaIndexContainer::get_fpa_index(void) {
  Fpa_index p = (Fpa_index) Memory::memCNew(sizeof(fpa_index));
  p->depth = -1;
  LADR_GLOBAL_FPA_INDEX.Fpa_index_gets++;
  return(p);
}


void FpaIndexContainer::free_fpa_index(Fpa_index p)
{
  Memory::memFree((void *) p, sizeof(fpa_index));
  LADR_GLOBAL_FPA_INDEX.Fpa_index_frees++;
}


void FpaIndexContainer::flag_fpa_leaves(ostream &fp, Fpa_trie p, int depth) {
  FpalistContainer FPA;
  Fpa_trie q;
  if (p->terms)
    FPA.flag_fpa_leaf_clauses(p->terms->chunks);
  for (q = p->kids; q != NULL; q = q->next)
    flag_fpa_leaves(fp, q, depth+1);
} 

void FpaIndexContainer::flag_fpa_clauses(ostream &fp, Fpa_index idx) {
  
  flag_fpa_leaves(fp, idx->root, 0);
}

void FpaIndexContainer::fprint_fpa_mem(ostream &o, bool heading) {
  int n;
  if (heading)
        o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct fpa_trie);
  o<<"fpa_trie    ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_trie_gets;
  o<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_trie_frees;
  o<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_trie_gets-LADR_GLOBAL_FPA_INDEX.Fpa_trie_frees;
  o<<setw(9)<<((LADR_GLOBAL_FPA_INDEX.Fpa_trie_gets-LADR_GLOBAL_FPA_INDEX.Fpa_trie_frees) *n) /1024<<"K"<<endl;

  n = sizeof(struct fpa_state);
  o<<"fpa_state   ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_state_gets;
  o<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_state_frees;
  o<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_state_gets-LADR_GLOBAL_FPA_INDEX.Fpa_state_frees;
  o<<setw(9)<<((LADR_GLOBAL_FPA_INDEX.Fpa_state_gets-LADR_GLOBAL_FPA_INDEX.Fpa_state_frees)*n ) /1024<<"K"<<endl;

  n = sizeof(struct fpa_index);

  o<<"fpa_index   ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_index_gets;
  o<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_index_frees;
  o<<setw(11)<<LADR_GLOBAL_FPA_INDEX.Fpa_index_gets-LADR_GLOBAL_FPA_INDEX.Fpa_index_frees;
  o<<setw(9)<<((LADR_GLOBAL_FPA_INDEX.Fpa_index_gets-LADR_GLOBAL_FPA_INDEX.Fpa_index_frees)*n ) /1024<<"K"<<endl;
}


void FpaIndexContainer::p_fpa_mem(void){
fprint_fpa_mem(cout, true);
}


void FpaIndexContainer::fprint_path(ostream &o, Ilist p) {
  int i;
  SymbolContainer S;
  o<<"(";
  for (i = 0; p != NULL; p = p->next, i++) {
    if (i%2 == 0) {
      if (p->i == 0)o<<"*";
      else S.fprint_sym(o, p->i);
    }
    else  o<<p->i;
    if (p->next != NULL)  o<<" ";
  }
  o<<")";
}

void FpaIndexContainer::p_path(Ilist p) {
  fprint_path(cout, p);
  cout<<endl;
}


void FpaIndexContainer::fprint_fpa_trie(ostream &o, Fpa_trie p, int depth) {
  int i;
  Fpa_trie q;
  SymbolContainer S;
  for (i = 0; i < depth; i++)  o<<" - ";
  if (depth == 0) o<<"root";
  else if (depth % 2 == 1) {
    if (p->label == 0) o<<"*";
    else
      S.fprint_sym(o, p->label);
  }
  else
    o<<setw(2)<<p->label;

  if (p->terms) {
      FpalistContainer FP;  
      FP.p_fpa_list(p->terms->chunks);
  }

#ifdef FPA_DEBUG
  if (p->path != NULL)
    fprint_path(o, p->path);
#endif
  o<<endl;
  for (q = p->kids; q != NULL; q = q->next)
    fprint_fpa_trie(o, q, depth+1);
}



void FpaIndexContainer::fprint_fpa_index(ostream &o, Fpa_index idx) {
  o<<"FPA/Path index, depth is"<<idx->depth<<endl;
  fprint_fpa_trie(o, idx->root, 0);
}

void FpaIndexContainer::p_fpa_index(Fpa_index idx){
  fprint_fpa_index(cout, idx);
}


Fpa_trie FpaIndexContainer::fpa_trie_member_insert(Fpa_trie node, Ilist path){
  if (path == NULL)  return node;
  else {
    /* Find child node that matches first member of path;
     * if it doesn't exist, create it. Children are in increasing order.
     */
    int val = path->i;
    Fpa_trie curr = node->kids;
    Fpa_trie prev = NULL;
    while (curr != NULL && curr->label < val) {
      prev = curr;
      curr = curr->next;
    }
    if (curr != NULL && curr->label == val)
      return fpa_trie_member_insert(curr, path->next);
    else {
      /* Get a new node and insert it before curr (which may be NULL). */
      Fpa_trie novo = get_fpa_trie();
      novo->parent = node;
      novo->label = val;
      novo->next = curr;
      if (prev == NULL)  node->kids = novo;
      else prev->next = novo;
      return fpa_trie_member_insert(novo, path->next);
    }
  }
}


Fpa_trie FpaIndexContainer::fpa_trie_member(Fpa_trie node, Ilist path) {
  if (path == NULL)   return node;
  else {
    /* Find child node that matches first member of path;
     * Children are in increasing order.
     */
    int val = path->i;
    Fpa_trie curr = node->kids;
    while (curr != NULL && curr->label < val)
      curr = curr->next;
    if (curr != NULL && curr->label == val)
      return fpa_trie_member(curr, path->next);
    else  return NULL;
  }
}


void FpaIndexContainer::fpa_trie_possible_delete(Fpa_trie node){
   FpalistContainer FP; 
   if (node->parent && node->terms && FP.fpalist_empty(node->terms) && node->kids == NULL) {
    if (node->parent->kids == node)  node->parent->kids = node->next;
    else {
      Fpa_trie p = node->parent->kids;
      while (p->next != node) p = p->next;
      p->next = node->next;
    }
    fpa_trie_possible_delete(node->parent);
    free_fpa_trie(node);
  }
}


void FpaIndexContainer::path_insert(Term t, Ilist path, Fpa_trie index){
 FpalistContainer FP; 
 Fpa_trie node = fpa_trie_member_insert(index, path);

#ifdef FPA_DEBUG
  if (node->path == NULL)  node->path = copy_ilist(path);
#endif

  if (node->terms == NULL)  node->terms = FP.get_fpa_list();
  FP.fpalist_insert(node->terms, t);
}


void FpaIndexContainer::path_delete(Term t, Ilist path, Fpa_trie index){
  FpalistContainer FP;  
  Fpa_trie node = fpa_trie_member(index, path);
  if (node == NULL) {
    fatal::fatal_error("path_delete, trie node not found.");
  }
  FP.fpalist_delete(node->terms, t);

#ifdef FPA_DEBUG
  if (FP.fpalist_empty(node->terms)) {
        Ilist I;
        I.set_head(node->path);
        I.zap_ilist();
        node->path = NULL;
  }
#endif
  fpa_trie_possible_delete(node);
}



Ilist FpaIndexContainer::path_push(struct path *p, int i) {
  IlistContainer I;
  Ilist save = p->last;
  I.ilist_append(i);
  if (p->last == NULL)  p->first = I.get_head();
  else  p->last->next = I.get_head();
  p->last = I.get_head();
  return save;
}



void FpaIndexContainer::path_restore(struct path *p, Ilist save){
  IlistContainer I;
  I.free_ilist(p->last);
  
  p->last = save;
  if (save != NULL)  save->next = NULL;
  else  p->first = NULL;
}



void FpaIndexContainer::fpa_paths(Term root, Term t, struct path *p,int bound, Indexop op, Fpa_trie index) {
  SymbolContainer S;
  Ilist save1;

  if (VARIABLE(t))
    save1 = path_push(p, 0);
  else
    save1 = path_push(p, SYMNUM(t));
  if (COMPLEX(t) && bound > 0 && !S.is_assoc_comm(SYMNUM(t))) {
    int i;
    Ilist save2 = path_push(p, 0);
    for (i = 0; i < ARITY(t); i++) {
      p->last->i = i+1;  /* Count arguments from 1. */
      fpa_paths(root, ARG(t,i), p, bound-1, op, index);
    }
    path_restore(p, save2);
  }
  else {
    /* printf("    ");  p_path(p->first); */

    if (op == Indexop::INSERT)
      path_insert(root, p->first, index);
    else
      path_delete(root, p->first, index);
  }
  path_restore(p, save1);
}  /* fpa_paths */


Fpa_index FpaIndexContainer::fpa_init_index(int depth) {
  Fpa_index f = get_fpa_index();
  f->depth = depth;
  f->root = get_fpa_trie();
  return f;
}


void FpaIndexContainer::fpa_update(Term t, Fpa_index idx, Indexop op){
 struct path p;
 if (FPA_ID(t) == 0) {
    if (op == Indexop::INSERT) FPA_ID(t) = ++LADR_GLOBAL_FPA_INDEX.Fpa_id_count;
    else
      fatal::fatal_error("fpa_update: FPA_ID=0.");
  }
  p.first = p.last = NULL;
  fpa_paths(t, t, &p, idx->depth, op, idx->root);
}


#ifdef FPA_DEBUG

Fpa_state FpaIndexContainer::query_leaf_full(Ilist path, Fpa_trie index) {
  Fpa_trie n = fpa_trie_member(index, path);
  Fpa_state q = get_fpa_state();
  q->type = (int) Fpa_state_type::LEAF;
  q->terms = (n == NULL ? NULL : n->terms);
  q->path = I.copy_ilist(path);
  return q;
}  /* query_leaf_full */
#endif


Fpa_state FpaIndexContainer::query_leaf(Ilist path, Fpa_trie index){
  Fpa_trie n;
  /* return query_leaf_full(path, index); */
  n = fpa_trie_member(index, path);
  if (n == NULL)  return NULL;
  else {
        FpalistContainer FP;
        Fpa_state q = get_fpa_state();
        q->type = Fpa_state_type::LEAF;
        q->fpos = FP.first_fpos(n->terms);
        #ifdef FPA_DEBUG
            q->path = copy_ilist(path);
        #endif
        return q;
  }
}



Fpa_state FpaIndexContainer::query_intersect(Fpa_state q1, Fpa_state q2){
  /* Assume neither is NULL. */
  Fpa_state q = get_fpa_state();
  q->type = Fpa_state_type::INTERSECT;
  q->left = q1;
  q->right = q2;
  return q;
}



Fpa_state FpaIndexContainer::query_special(Fpa_trie n){
  /* There are 2 kinds of nodes: argument position (1,2,3,...) and
   * symbol (a,b,f,g,h); the two types alternate in a path.  The
   * given node n is a symbol node.  What we wish to do is construct
   * the union of all leaves, excluding those that have an argument
   * position greater than 1.  This should contain all terms that
   * have a path corresponding to node n.
   */

  if (n->kids == NULL) {
    FpalistContainer FP;
    Fpa_state q = get_fpa_state();
    q->type = Fpa_state_type::LEAF;
    q->fpos = FP.first_fpos(n->terms);
#ifdef FPA_DEBUG
    q->path = copy_ilist(n->path);
#endif
    return q;
  }
  else {
    Fpa_state q1 = NULL;
    Fpa_trie pos_child;
    for (pos_child=n->kids; pos_child!=NULL; pos_child=pos_child->next) {
      if (pos_child->label == 1) {
            Fpa_trie sym_child;
            for (sym_child=pos_child->kids;sym_child!=NULL; sym_child=sym_child->next) {
                Fpa_state q2 = query_special(sym_child);
                q1 = query_union(q1, q2);
            }
      }
    }
    return q1;
  }
}

void FpaIndexContainer::zap_fpa_state(Fpa_state q){
  if (q != NULL) {
    zap_fpa_state(q->left);
    zap_fpa_state(q->right);
#ifdef FPA_DEBUG
    zap_ilist(q->path);
#endif
    free_fpa_state(q);
  }
}


Fpa_state FpaIndexContainer::union_commuted(Fpa_state q, Term t,Context c, Querytype type, struct path *p, int bound, Fpa_trie index) {
  Fpa_state q1;
  int empty, i;
#if 0
  printf("enter union_commuted with\n");
  p_fpa_state(q);
#endif
  q1 = NULL;
  empty = 0;

  for (i = 0; i < 2 && !empty; i++) {
    p->last->i = (i == 0 ? 2 : 1);
    /* Skip this arg if VARIABLE && (UNIFY || INSTANCE). */
    if (!VARIABLE(ARG(t,i)) || type==Querytype::GENERALIZATION||type==Querytype::VARIANT || type==Querytype::IDENTICAL) {
      Fpa_state q2 = build_query(ARG(t,i), c, type, p, bound-1, index);
      if (q2 == NULL) {
                        empty = 1;
                        zap_fpa_state(q1);
                        q1 = NULL;
      }
      else if (q1 == NULL) q1 = q2;
      else q1 = query_intersect(q1, q2);
    }
  }
  if (q1 != NULL)    q1 = query_union(q, q1);
  else    q1 = q;
#if 0
  printf("exit union_commuted with\n");
  p_fpa_state(q1);
#endif
  return(q1);
}



bool FpaIndexContainer::var_in_context(Term t, Context c){
  DEREFERENCE(t, c);
  return VARIABLE(t);
}

bool FpaIndexContainer::all_args_vars_in_context(Term t, Context c) {
  /* Assume t is not a variable. */
  int i = 0;
  bool ok = true;
  while (i < ARITY(t) && ok) {
    ok = var_in_context(ARG(t,i), c);
    i++;
  }
  return ok;
}

Fpa_state FpaIndexContainer::build_query(Term t, Context c, Querytype type,    struct path *p, int bound, Fpa_trie index) {
  TermContainer T;
  if (VARIABLE(t)) {
    int i = VARNUM(t);
    if (c != NULL && c->terms[i] != NULL)
      return build_query(c->terms[i], c->contexts[i], type, p, bound, index);
    else if (type == Querytype::UNIFY || type == Querytype::INSTANCE) {
      fatal::fatal_error("build_query, variable.");
      return NULL;  /* to quiet compiler */
    }
    else {
      Ilist save = path_push(p, 0);
      Fpa_state q = query_leaf(p->first, index);
      path_restore(p, save);
      return q;
    }
  }
  else {  /* non-variable */
    Fpa_state q1 = NULL;
    TermContainer T;
    Ilist save1 = path_push(p, SYMNUM(t));
    SymbolContainer S;
    if (CONSTANT(t) || bound <= 0 || S.is_assoc_comm(SYMNUM(t))) {
      q1 = query_leaf(p->first, index);
      if ((MATCH_HINTS_ANYCONST == true) && (LADR_GLOBAL_TERM.AnyConstsEnabled == true)) {
         int i;
         for (i=0; i<MAX_ANYCONSTS; i++) {
            p->last->i = T.any_const_sn(i);
            Fpa_state qAny = query_leaf(p->first, index);
            if (qAny) {
               q1 = (q1 == NULL) ? qAny : query_union(q1, qAny);
            }
         }
      }
      
    }
    else if ((type == Querytype::INSTANCE || type == Querytype::UNIFY) &&
	     all_args_vars_in_context(t, c)) {
      Fpa_trie n = fpa_trie_member(index, p->first);
      q1 = (n == NULL ? NULL : query_special(n));
    }
    else {
      Ilist save2 = path_push(p, 0);
      int empty = 0;
      int i;
      for (i = 0; i < ARITY(t) && !empty; i++) {
	p->last->i = i+1;
	/* Skip this arg if VARIABLE && (UNIFY || INSTANCE). */
	if (!var_in_context(ARG(t,i),c) || type==Querytype::GENERALIZATION || type==Querytype::VARIANT || type==Querytype::IDENTICAL) {
	  Fpa_state q2 = build_query(ARG(t,i), c, type, p, bound-1, index);
					      
	  if (q2 == NULL) {
	    empty = 1;
	    zap_fpa_state(q1);
	    q1 = NULL;
	  }
	  else if (q1 == NULL)
	    q1 = q2;
	  else
	    q1 = query_intersect(q1, q2);
	}
      }
  
      SymbolContainer S;
      if (S.is_commutative(SYMNUM(t)) && !T.term_ident(ARG(t,0), ARG(t,1)))
            q1 = union_commuted(q1, t, c, type, p, bound, index);
            path_restore(p, save2);
    }
    if (type == Querytype::UNIFY || type == Querytype::GENERALIZATION) {
      Fpa_state q2;
      p->last->i = 0;
      q2 = query_leaf(p->first, index);
      q1 = query_union(q1, q2);
    }
    path_restore(p, save1);
    return q1;
  }
} 

Fpa_state FpaIndexContainer::query_union(Fpa_state q1, Fpa_state q2) {
  if (q1 == NULL)
    return q2;
  else if (q2 == NULL)
    return q1;
  else {
    Fpa_state q = get_fpa_state();
    q->type = Fpa_state_type::UNION;
    q->left = q1;
    q->right = q2;
    return q;
  }
}

void FpaIndexContainer::fprint_fpa_state(ostream &o, Fpa_state q, int depth) {
  int i;
  FpalistContainer FP;
  for (i = 0; i < depth; i++)o<<"- - ";
  switch (q->type) {
  case Fpa_state_type::UNION: o<<"OR"<<endl; break;
  case Fpa_state_type::INTERSECT: o<<"AND"<<endl; break;
  case Fpa_state_type::LEAF:
#ifdef FPA_DEBUG
    fprint_path(o, q->path);
    o<<" ";
#endif
    FP.p_fpa_list(q->fpos.f);
    {
#if 0
      Plist p;
      o<<"[";
  for (p = q->terms; p != NULL; p = p->next)
o<<FPA_ID(p->v)<<(p->next == NULL ? "" : ",");
  o<<"]"<<endl;
#endif
    }
    break;
  }
  o<<endl;
  if (q->type == Fpa_state_type::UNION || q->type == Fpa_state_type::INTERSECT) {
    fprint_fpa_state(o, q->right, depth+1);
    fprint_fpa_state(o, q->left, depth+1);
  }
}


void FpaIndexContainer::p_fpa_state(Fpa_state q) {
  fprint_fpa_state(cout, q, 0);
}



void FpaIndexContainer::p_fpa_query(Term t, Querytype query_type,Fpa_index idx) {
  Fpa_state q;
  string s;
  struct path p;
  p.first = p.last = NULL;

  switch (query_type) {
  case Querytype::UNIFY:          s = "UNIFY         "; break;
  case Querytype::INSTANCE:       s = "INSTANCE      "; break;
  case Querytype::GENERALIZATION: s = "GENERALIZATION"; break;
  case Querytype::VARIANT:        s = "VARIANT       "; break;
  case Querytype::IDENTICAL:      s = "IDENTICAL     "; break;
  default:                 s = "FPA_??            "; break;
  }
  cout<<endl<<s<<" with term "<<(unsigned) FPA_ID(t)<<":";
  TermContainer T;
  T.p_term(t);
  cout<<endl;
  q = build_query(t, NULL, query_type, &p, idx->depth, idx->root);
  p_fpa_state(q);
  zap_fpa_state(q);
}


Term FpaIndexContainer::next_term(Fpa_state q, FPA_ID_TYPE max) {
  BUMP_NEXT_CALLS;
  if (q == NULL)    return NULL;
  else if (q->type == Fpa_state_type::LEAF) {
            Term t = FTERM(q->fpos);
            FpalistContainer FP;
            while (t != NULL && FPA_ID(t) > max) {
                q->fpos = FP.next_fpos(q->fpos);
                t = FTERM(q->fpos);
            }
            if (t == NULL) {
                zap_fpa_state(q);
                return NULL;
            }
            else {
                q->fpos = FP.next_fpos(q->fpos);
                return t;
            }
  }

  else if (q->type == Fpa_state_type::INTERSECT) {
    Term t1, t2;
    t1 = next_term(q->left, max);
    if (t1 != NULL)   t2 = next_term(q->right, FPA_ID(t1));
    else   t2 = (Term) &t2;  /* anything but NULL */

    while (t1 != t2 && t1 != NULL && t2 != NULL) {
      if (FGT(t1,t2)) t1 = next_term(q->left, FPA_ID(t2));
      else t2 = next_term(q->right, FPA_ID(t1));
    }
    if (t1 == NULL || t2 == NULL) {
      if (t1 == NULL) q->left = NULL;
      if (t2 == NULL) q->right = NULL;
      zap_fpa_state(q);
      return NULL;
    }
    else
      return t1;
  }

  else {  /* UNION node */
    Term t1, t2;
    /* first get the left term */
    t1 = q->left_term;
    if (t1 == NULL) {
      /* it must be brought up */
      if (q->left) {
        t1 = next_term(q->left, max);
        if (t1 == NULL) q->left = NULL;
      }
    }
    else  /* it was saved from a previous call */
      q->left_term = NULL;

    /* now do the same for the right side */
    t2 = q->right_term;
    if (t2 == NULL) {
      if (q->right) {
        t2 = next_term(q->right, max);
        if (t2 == NULL)
        q->right = NULL;
      }
    }
    else
      q->right_term = NULL;

    /* At this point, both left_term and right_term are NULL.
     * Now decide which of t1 and t2 to return.  If both are
     * non-NULL (and different), save the smaller for the next
     * call, and return the larger.
     */
    if (t1 == NULL) {
      if (t2 == NULL) {
        zap_fpa_state(q);
        return NULL;
      }
      else return t2;
    }
    else if (t2 == NULL)      return t1;
    else if (t1 == t2)        return t1;
    else if (FGT(t1,t2)) {
      q->right_term = t2;  /* save t2 for next time */
      return t1;
    }
    else {
      q->left_term = t1;  /* save t1 for next time */
      return t2;
    }
  }
}


Term FpaIndexContainer::fpa_next_answer(Fpa_state q){
  return next_term(q, FPA_ID_MAX);
}


Term FpaIndexContainer::fpa_first_answer(Term t, Context c, Querytype
query_type, Fpa_index idx, Fpa_state *ppos) {
  struct path p;
  p.first = p.last = NULL;
  *ppos = build_query(t, c, query_type, &p, idx->depth, idx->root);
  return fpa_next_answer(*ppos);
}

void FpaIndexContainer::fpa_cancel(Fpa_state q) {
  zap_fpa_state(q);
}


void FpaIndexContainer::zap_fpa_trie(Fpa_trie n) {
  Fpa_trie k, prev;

  k = n->kids;
  while (k != NULL) {
    prev = k;
    k = k->next;
    zap_fpa_trie(prev);
  }
  FpalistContainer FP;
  FP.zap_fpalist(n->terms);

#ifdef FPA_DEBUG
  zap_ilist(n->path);
#endif

  free_fpa_trie(n);
}

void FpaIndexContainer::zap_fpa_index(Fpa_index idx){
  zap_fpa_trie(idx->root);
  free_fpa_index(idx);
}

bool FpaIndexContainer::fpa_empty(Fpa_index idx){
  return (idx == NULL ? true : idx->root->kids == NULL);
}

void FpaIndexContainer::fpa_density(Fpa_trie p) {
  Fpa_trie q;
  for (q = p->kids; q; q = q->next)
    fpa_density(q);
  if (p->terms != NULL) {
       cout<<"Fpa_list: chunks="<<p->terms->num_chunks;
   cout<<", size="<<p->terms->chunksize;
   cout<<", terms="<<p->terms->num_terms<<endl;
  }
}

void FpaIndexContainer::p_fpa_density(Fpa_index idx) {
  fpa_density(idx->root);
}

unsigned FpaIndexContainer::mega_next_calls(void){
  return
    (LADR_GLOBAL_FPA_INDEX.Next_calls / 1000000) +
    ((UINT_MAX / 1000000) * LADR_GLOBAL_FPA_INDEX.Next_calls_overflows);
}


