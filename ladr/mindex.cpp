#include "mindex.h"
#include "memory.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>




GlobalMindex::GlobalMindex() {
    Mindex_gets=0;
    Mindex_frees=0;

    Mindex_pos_gets=0;
    Mindex_pos_frees=0;
}


GlobalMindex::~GlobalMindex() {
    
}

MindexContainer::MindexContainer()  {head=NULL;}
MindexContainer::~MindexContainer() {head=NULL;}


Mindex MindexContainer::get_mindex(void) {
  Mindex p = (Mindex) Memory::memCNew(sizeof(struct mindex));
  p->index_type =(Mindextype) -1;
  p->unif_type = (Uniftype) -1;
  LADR_GLOBAL_MINDEX.Mindex_gets++;
  return(p);
} 


void MindexContainer::free_mindex(Mindex p){
  Memory::memFree((void *)p, sizeof(struct mindex));
  LADR_GLOBAL_MINDEX.Mindex_frees++;
} 

Mindex_pos MindexContainer::get_mindex_pos(void) {
  Mindex_pos p= (Mindex_pos) Memory::memCNew(sizeof(struct mindex_pos));
  p->query_type = (Querytype)-1;
  LADR_GLOBAL_MINDEX.Mindex_pos_gets++;
  return(p);
} 

void MindexContainer::free_mindex_pos(Mindex_pos p) {
  Memory::memFree((void *)p, sizeof(struct mindex_pos));
  LADR_GLOBAL_MINDEX.Mindex_pos_frees++;
} 

void MindexContainer::fprint_mindex_mem(ostream &o, bool heading) {
  int n;
  if (heading)
	o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct mindex);
  o<<"mindex      ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_MINDEX.Mindex_gets;
  o<<setw(11)<<LADR_GLOBAL_MINDEX.Mindex_frees;
  o<<setw(11)<<LADR_GLOBAL_MINDEX.Mindex_gets-LADR_GLOBAL_MINDEX.Mindex_frees;
  o<<setw(9)<<((LADR_GLOBAL_MINDEX.Mindex_gets-LADR_GLOBAL_MINDEX.Mindex_frees)*n)/1024<<"K"<<endl;
  
  n = sizeof(struct mindex_pos);
  o<<"mindex_pos  ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_MINDEX.Mindex_pos_gets;
  o<<setw(11)<<LADR_GLOBAL_MINDEX.Mindex_pos_frees;
  o<<setw(11)<<LADR_GLOBAL_MINDEX.Mindex_pos_gets-LADR_GLOBAL_MINDEX.Mindex_pos_frees;
  o<<setw(9)<<((LADR_GLOBAL_MINDEX.Mindex_pos_gets-LADR_GLOBAL_MINDEX.Mindex_pos_frees)*n)/1024<<"K"<<endl;
  
} 

void MindexContainer::p_mindex_mem() {
  fprint_mindex_mem(cout, true);
}



Mindex MindexContainer::mindex_init(Mindextype mtype, Uniftype utype, int fpa_depth)
{
  FpaIndexContainer FPA;
  DiscrimContainer D;
  Mindex mdx = get_mindex();

  mdx->index_type = mtype;
  mdx->unif_type = utype;

  switch(mtype) {
  case Mindextype::LINEAR:       mdx->linear_first = mdx->linear_last = NULL; break;
  case Mindextype::FPA:          mdx->fpa = FPA.fpa_init_index(fpa_depth); break;
  case Mindextype::DISCRIM_WILD: mdx->discrim_tree = D.discrim_init(); break;
  case Mindextype::DISCRIM_BIND: mdx->discrim_tree = D.discrim_init(); break;
  default:  free_mindex(mdx); mdx = NULL;
  }
  return mdx;
}



bool MindexContainer::mindex_empty(Mindex mdx) {
  FpaIndexContainer F;
  DiscrimContainer D;
  switch (mdx->index_type) {
  case Mindextype::FPA:
		return F.fpa_empty(mdx->fpa);
    break;
  case Mindextype::LINEAR:
		return mdx->linear_first == NULL;
    break;
  case Mindextype::DISCRIM_WILD:
  case Mindextype::DISCRIM_BIND:
		return D.discrim_empty(mdx->discrim_tree);
    break;
  }
  return false;
}  



void MindexContainer::mindex_free(Mindex mdx) {
  FpaIndexContainer F;
  DiscrimContainer D;
  if (!mindex_empty(mdx))
    cerr<<"WARNING, mindex_free called with nonempty mindex"<<endl;
  else {
    switch (mdx->index_type) {
    case Mindextype::FPA:
						F.zap_fpa_index(mdx->fpa);
      break;
    case Mindextype::LINEAR:
      break;
    case Mindextype::DISCRIM_WILD:
    case Mindextype::DISCRIM_BIND:
					D.discrim_dealloc(mdx->discrim_tree);
      break;
    }
    free_mindex(mdx);
  }
}

void MindexContainer::mindex_destroy(Mindex mdx) {
  FpaIndexContainer F;
  PlistContainer P;
  DiscrimContainer D;
  if (!mindex_empty(mdx)) {
    cout<<endl<<"WARNING: destroying nonempty mindex."<<endl<<endl;
	cerr<<endl<<"WARNING: destroying nonempty mindex."<<endl<<endl;
  }

  switch (mdx->index_type) {
  case Mindextype::FPA:
				F.zap_fpa_index(mdx->fpa);
    break;
  case Mindextype::LINEAR:
				P.zap_plist(mdx->linear_first);
    break;
  case Mindextype::DISCRIM_WILD:
  case Mindextype::DISCRIM_BIND:
			D.destroy_discrim_tree(mdx->discrim_tree);
    break;
  }
  free_mindex(mdx);
} 

void MindexContainer::linear_insert(Mindex mdx, Term t) {
  PlistContainer P;
  Plist p = P.get_plist();
  p->v = t;
  p->next = NULL;
  if (mdx->linear_last != NULL)
    mdx->linear_last->next = p;
  else
    mdx->linear_first = p;
  mdx->linear_last = p;
} 

void MindexContainer::linear_delete(Mindex mdx, Term t) {
  TermContainer T;
  PlistContainer P;
  Plist curr, prev;
  prev = NULL;
  curr = mdx->linear_first;
  while (curr != NULL && curr->v != t) {
    prev = curr;
    curr = curr->next;
  }
  if (curr == NULL) {
    T.fprint_term(cerr, t);
    fatal::fatal_error("mindex_delete (linear), term not found.");
  }
  else {
    if (prev != NULL)
      prev->next = curr->next;
    else
      mdx->linear_first = curr->next;
    if (curr == mdx->linear_last)
      mdx->linear_last = prev;
    P.free_plist(curr);
  }
}


void MindexContainer::linear_update(Mindex mdx, Term t, Indexop op) {
  if (op == Indexop::INSERT)
    linear_insert(mdx, t);
  else
    linear_delete(mdx, t);
}



void MindexContainer::mindex_update(Mindex mdx, Term t, Indexop op) {
  DiscrimWContainer DW;
  DiscrimBContainer B;
  FpaIndexContainer F;
  if (mdx->index_type == Mindextype::FPA)
    F.fpa_update(t, mdx->fpa, op);
  else if (mdx->index_type == Mindextype::LINEAR)
    linear_update(mdx, t, op);
  else if (mdx->index_type == Mindextype::DISCRIM_WILD)
    DW.discrim_wild_update(t, mdx->discrim_tree, t, op);
  else if (mdx->index_type == Mindextype::DISCRIM_BIND)
    B.discrim_bind_update(t, mdx->discrim_tree, t, op);
  else {
		fatal::fatal_error("ERROR, mindex_update: bad mindex type.");
  }
}

Term MindexContainer::mindex_retrieve_first(Term t, Mindex mdx, Querytype qtype,
			   Context query_subst, Context found_subst,
			   bool partial_match,
			   Mindex_pos *ppos)
{
  Mindex_pos pos;

  if ((mdx->index_type == Mindextype::DISCRIM_WILD || mdx->index_type == Mindextype::DISCRIM_BIND) && qtype != Querytype::GENERALIZATION)
    return NULL;

  else {
    pos = get_mindex_pos();
    pos->index = mdx;
    pos->query_type = qtype;
    pos->query_term = t;
    pos->query_subst = query_subst;
    pos->found_term = NULL;
    pos->found_subst = found_subst;
    pos->tr = NULL;
    pos->btu_position = NULL;
    pos->btm_position = NULL;
    pos->partial_match = partial_match;

    if (mdx->index_type == Mindextype::FPA)
      pos->fpa_position = NULL;
    else if (mdx->index_type == Mindextype::DISCRIM_WILD)
      pos->discrim_position = NULL;
    else if (mdx->index_type == Mindextype::DISCRIM_BIND)
      pos->discrim_position = NULL;
    else if (mdx->index_type == Mindextype::LINEAR)
      pos->linear_position = mdx->linear_first;

    *ppos = pos;
    return mindex_retrieve_next(pos);
  }
}


Term MindexContainer::next_candidate(Mindex_pos pos) {
 Term tf;
 FpaIndexContainer F;
 DiscrimWContainer DW;
 DiscrimBContainer B;
  if (pos->index->index_type == Mindextype::FPA) {
    if (pos->fpa_position == NULL) {
      tf = F.fpa_first_answer(pos->query_term,
				pos->query_subst,
			    pos->query_type,
			    pos->index->fpa,
			    &(pos->fpa_position));
    }
    else
      tf = F.fpa_next_answer(pos->fpa_position);
  }
  else if (pos->index->index_type == Mindextype::DISCRIM_WILD) {
    if (pos->discrim_position == NULL)
      tf = (Term) DW.discrim_wild_retrieve_first(pos->query_term,
				       pos->index->discrim_tree,
				       &(pos->discrim_position));
    else
      tf = (Term) DW.discrim_wild_retrieve_next(pos->discrim_position);
  }

  else if (pos->index->index_type == Mindextype::DISCRIM_BIND) {
    if (pos->discrim_position == NULL)
      tf = (Term) B.discrim_bind_retrieve_first(pos->query_term,
				       pos->index->discrim_tree,
				       pos->found_subst,
				       &(pos->discrim_position));
    else
      tf = (Term) B.discrim_bind_retrieve_next(pos->discrim_position);
  }

  else if (pos->index->index_type == Mindextype::LINEAR) {
    if (pos->linear_position == NULL)
      tf = NULL;
    else {
      tf = (Term) pos->linear_position->v;
      pos->linear_position = pos->linear_position->next;
    }
  }

  else
    tf = NULL;
  return tf;
}  


Term MindexContainer::retrieve_next_backtrack(Mindex_pos pos) {
  Term tq = pos->query_term;
  Term tf = pos->found_term;
  Context cq = pos->query_subst;
  Context cf = pos->found_subst;
  BtuContainer B;
  BtmContainer Bm;
  

  if (pos->query_type ==Querytype::UNIFY) {
    /* We already have a found_term from a previous call;
     * try for another unifier.
     */
    if (pos->btu_position != NULL) {
      pos->btu_position = B.unify_bt_next(pos->btu_position);
      if (pos->btu_position == NULL)
	tf = NULL;
    }
    if (pos->btu_position == NULL) {
      /* This is either the first call for the query, or there are
       * no more unifiers for the previous found_term.
       */
      tf = next_candidate(pos);
      while (tf != NULL && pos->btu_position == NULL) {
	pos->btu_position = B.unify_bt_first(tq, cq, tf, cf);
	if (pos->btu_position == NULL)
	  tf = next_candidate(pos);
      }
    }
  }  /* UNIFY */

  else if (pos->query_type == Querytype::INSTANCE || pos->query_type == Querytype::GENERALIZATION) {
    if (pos->btm_position != NULL) {
      pos->btm_position = Bm.match_bt_next(pos->btm_position);
      if (pos->btm_position == NULL)
	tf = NULL;
    }
    if (pos->btm_position == NULL) {
      tf = next_candidate(pos);
      while (tf != NULL && pos->btm_position == NULL) {
	if (pos->query_type ==Querytype::INSTANCE)
	  pos->btm_position = Bm.match_bt_first(tq, cq, tf, pos->partial_match);
	else
	  pos->btm_position = Bm.match_bt_first(tf, cf, tq, pos->partial_match);
	if (pos->btm_position == NULL)
	  tf = next_candidate(pos);
      }
    }
  }  /* INSTANCE || GENERALIZATION */

  else if (pos->query_type == Querytype::VARIANT) {
    fatal::fatal_error("retrieve_next_backtrack, VARIANT not supported.");
  }  /* VARIANT */

  else if (pos->query_type == Querytype::IDENTICAL) {
    fatal::fatal_error("retrieve_next_backtrack, IDENTICAL not supported.");
  }  /* IDENTICAL */

  if (tf == NULL) {
    free_mindex_pos(pos);
    return NULL;
  }
  else {
    pos->found_term = tf;
    return tf;
  }
} 


Term MindexContainer::mindex_retrieve_next(Mindex_pos pos) {
  TermContainer T;
  UnifyContainer U;
  if (pos->index->unif_type == Uniftype::BACKTRACK_UNIF)
    return retrieve_next_backtrack(pos);
  else {
    Term tq, tf;
    Context cq, cf;
    Trail tr;
    bool ok;

    tq = pos->query_term;
    cq = pos->query_subst;
    cf = pos->found_subst;

    U.undo_subst(pos->tr);  /* ok if NULL or not used */
    pos->tr = NULL;

    tf = next_candidate(pos);
    if (tf != NULL && pos->index->index_type == Mindextype::DISCRIM_BIND) 
      ok = true;
    else
      ok = false;
    while (tf && !ok) {
#if 0
      printf("potential mate, %d: ", tf->INDEX_ID); p_term(tf);
#endif	
      tr = NULL;
      switch (pos->query_type) {
      case Querytype::UNIFY:
			ok = U.unify(tq, cq, tf, cf, &(pos->tr)); break;
      case Querytype::GENERALIZATION:
			ok = U.match(tf, cf, tq, &(pos->tr)); break;
	  case Querytype::INSTANCE:
			ok = U.match(tq, cq, tf, &(pos->tr)); break;
      case Querytype::VARIANT: ok = U.variant(tq, cq, tf, &(pos->tr)); break;
	  case Querytype::IDENTICAL:
			ok = T.term_ident(tq, tf); break;
      default:
			ok = false; break;
      }
      if (!ok)
	tf = next_candidate(pos);
    }

    if (ok) {
#if 0
      printf("          MATE, %d: ", tf->INDEX_ID); p_term(tf);
#endif	
      return tf;
    }
    else {
      free_mindex_pos(pos);
      return NULL;
    }
  }
} 


void MindexContainer::mindex_retrieve_cancel(Mindex_pos pos) {
  /* Clean up the unification states.  The Mindex_pos doesn't know 
   * what kind of unification applies, so try them all.
   */
   UnifyContainer U; 
   BtmContainer Bt;
   BtuContainer Bu;
   FpaIndexContainer F;
   DiscrimWContainer DW;
   DiscrimBContainer DB;
   
  if (pos->tr != NULL)
    U.undo_subst(pos->tr);
  else if (pos->btm_position != NULL)
    Bt.match_bt_cancel(pos->btm_position);
  else if (pos->btu_position != NULL)
    Bu.unify_bt_cancel(pos->btu_position);

  if (pos->index->index_type == Mindextype::FPA)
    F.fpa_cancel(pos->fpa_position);
  else if (pos->index->index_type == Mindextype::LINEAR)
    ;  /* do nothing */
  else if (pos->index->index_type == Mindextype::DISCRIM_WILD)
    DW.discrim_wild_cancel(pos->discrim_position);
  else if (pos->index->index_type == Mindextype::DISCRIM_BIND)
    DB.discrim_bind_cancel(pos->discrim_position);

  free_mindex_pos(pos);
}

void MindexContainer::fprint_linear_index(ostream &o, Plist first) {
  Plist p;
  TermContainer T;
  for (p = first; p != NULL; p = p->next) {
    Term t = (Term) p->v;
	o<<"FPA_ID="<<(unsigned )FPA_ID(t)<<": ";
    T.fprint_term(o, t);
    o<<endl;
  }
}

void MindexContainer::fprint_mindex(ostream &o, Mindex mdx) {
  FpaIndexContainer FP;
  DiscrimWContainer DW;
  DiscrimBContainer DB;
  
  switch (mdx->index_type) {
  case Mindextype::LINEAR:
    o<<endl<<"This is an Mindex of type LINEAR."<<endl;
    fprint_linear_index(o, mdx->linear_first);
    break;
  case Mindextype::FPA:
    o<<endl<<"This is an Mindex of type FPA."<<endl;
	FP.fprint_fpa_index(o, mdx->fpa);
    break;
  case Mindextype::DISCRIM_WILD:
    o<<endl<<"This is an Mindex of type DISCRIM_WILD."<<endl; 
    DW.fprint_discrim_wild_index(o, mdx->discrim_tree);
    break;
  case Mindextype::DISCRIM_BIND:
    o<<endl<<"This is an Mindex of type DISCRIM_BIND."<<endl;
	DB.fprint_discrim_bind_index(o, mdx->discrim_tree);
    break;
  }
} 
