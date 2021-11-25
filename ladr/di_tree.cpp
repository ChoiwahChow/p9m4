#include "di_tree.h"
#include "memory.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>



GlobalDiTree::GlobalDiTree() {
    Sub_calls = 0;
    Sub_calls_overflows = 0;
    Di_tree_gets=0;
    Di_tree_frees=0;
}

GlobalDiTree::~GlobalDiTree() {

    
}


Di_treeContainer::Di_treeContainer() {
	root=NULL;
}

Di_treeContainer::~Di_treeContainer() {
	root=NULL;
}


int Di_treeContainer::nonunit_fsub_tests(void) {
  return LADR_GLOBAL_DI_TREE.Nonunit_fsub_tests;
}


int Di_treeContainer::nonunit_bsub_tests(void) {
  return LADR_GLOBAL_DI_TREE.Nonunit_bsub_tests;
}


Di_tree Di_treeContainer::get_di_tree(void) {
  Di_tree p = (Di_tree) Memory::memCNew(sizeof(struct di_tree));
  LADR_GLOBAL_DI_TREE.Di_tree_gets++;
  return(p);
}


/* PUBLIC */
void Di_treeContainer::free_di_tree(Di_tree p)
{
  Memory::memFree((void *)p, sizeof(struct di_tree));
  LADR_GLOBAL_DI_TREE.Di_tree_frees++;
} 

/* PUBLIC */
void Di_treeContainer::fprint_di_tree_mem(ostream &o, bool heading) {
  int n;
  if (heading)
   o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  
  n= sizeof(struct di_tree);
  
  o<<"di_tree     ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_DI_TREE.Di_tree_gets;
  o<<setw(11)<<LADR_GLOBAL_DI_TREE.Di_tree_frees;
  o<<setw(11)<<LADR_GLOBAL_DI_TREE.Di_tree_gets - LADR_GLOBAL_DI_TREE.Di_tree_frees;
  o<<setw(9)<< ( (LADR_GLOBAL_DI_TREE.Di_tree_gets - LADR_GLOBAL_DI_TREE.Di_tree_frees)*n) /1024<<"K"<<endl;
  
} 

void Di_treeContainer::p_di_tree_mem(void) {
  fprint_di_tree_mem(cout, true);
} 

Di_tree Di_treeContainer::init_di_tree(void)
{
  root=get_di_tree();
  return root;
}


void Di_treeContainer::di_tree_insert(Ilist vec, Di_tree node, void *datum) {
  if (vec == NULL) {
    PlistContainer P;
	Plist p = P.get_plist(); //colocar o Di_tree como friend do PlistContainer
    p->v = datum;
    p->next = node->u.data;
    node->u.data = p;
  }
  else {
    Di_tree prev = NULL;
    Di_tree curr = node->u.kids;
    /* kids are in increasing order */
    while (curr && vec->i > curr->label) {
      prev = curr;
      curr = curr->next;
    }
    if (curr == NULL || vec->i != curr->label) {
      Di_tree novo = get_di_tree();
      novo->label = vec->i;
      novo->next = curr;
      if (prev)
	prev->next = novo;
      else
	node->u.kids = novo;
      curr = novo;
    }
    di_tree_insert(vec->next, curr, datum);
  }
}


bool Di_treeContainer::di_tree_delete(Ilist vec, Di_tree node, void *datum) {
  if (vec == NULL) {
	PlistContainer P;
	P.set_head(node->u.data);
    node->u.data = P.plist_remove(datum);
    return node->u.data != NULL;  /* tells parent whether to keep node */
  }
  else {
    bool keep;
    Di_tree prev = NULL;
    Di_tree curr = node->u.kids;
    /* kids are in increasing order */
    while (curr && vec->i > curr->label) {
      prev = curr;
      curr = curr->next;
    }
    if (curr == NULL || vec->i != curr->label)
      fatal::fatal_error("di_tree_delete, node not found");
    keep = di_tree_delete(vec->next, curr, datum);
    if (keep)
      return true;
    else {
      if (prev)
	prev->next = curr->next;
      else
	node->u.kids = curr->next;
      free_di_tree(curr);
      return node->u.kids != NULL;
    }
  }
}  /* di_tree_delete */

/*************
 *
 *   zap_di_tree()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void Di_treeContainer::zap_di_tree(Di_tree node, int depth) {
  
  if (depth == 0) {
	PlistContainer P;
	P.set_head(node->u.data);	
	P.zap_plist();
  }
  else {
    Di_tree kid = node->u.kids;
    while (kid) {
      Di_tree tmp = kid;
      kid = kid->next;
      zap_di_tree(tmp, depth-1);
    }
  }
  free_di_tree(node);
} 

void Di_treeContainer::p_di_tree(Ilist vec, Di_tree node, int depth) {
  int i;
  for (i = 0; i < depth; i++)
    cout<<" ";
  if (vec == NULL) {
    Plist p = node->u.data;
    cout<<"IDs:";
    while (p) {
      Topform c = (Topform) p->v;
      cout<<" "<<c->id;
	  p = p->next;
    }
    cout<<endl;
  }
  else {
    Di_tree kid;
    cout<<node->label<<endl;
    for (kid = node->u.kids; kid; kid = kid->next)
      p_di_tree(vec->next, kid, depth+1);
  }
}

bool Di_treeContainer::subsume_di_literals(Literals clit, Context subst, Literals d, Trail *trp) {
  bool subsumed = false;
  Literals dlit;
  BUMP_SUB_CALLS;
  if (clit == NULL)
    return true;
  else {
    for (dlit = d; !subsumed && dlit != NULL; dlit = dlit->next) {
      if (clit->sign == dlit->sign) {
	Trail mark = *trp;
	UnifyContainer U;
	if (U.match(clit->atom, subst, dlit->atom, trp)) {
	  if (subsume_di_literals(clit->next, subst, d, trp))
	    subsumed = true;
	  else {
	    U.undo_subst_2(*trp, mark);
	    *trp = mark;
	  }
	}
      }
    }
    return subsumed;
  }
}

bool Di_treeContainer::subsumes_di(Literals c, Literals d, Context subst) {
  Trail tr = NULL;
  UnifyContainer U;
  bool subsumed = subsume_di_literals(c, subst, d, &tr);
  if (subsumed)  U.undo_subst(tr);
  return subsumed;
}




Topform Di_treeContainer::di_tree_forward(Ilist vec, Di_tree node, Literals dlits, Context subst) {
  BUMP_SUB_CALLS;
  if (vec == NULL) {
    Plist p = node->u.data;
    while (p) {
      Topform c = (Topform) p->v;
      LADR_GLOBAL_DI_TREE.Nonunit_fsub_tests++;
      if (subsumes_di(c->literals, dlits, subst)) return (Topform) p->v;
      p = p->next;
    }
    return NULL;
  }
  else {
    void *datum = NULL;
    Di_tree kid = node->u.kids;
    /* kids are in increasing order; look at those <= vec->i */
    while (!datum && kid && kid->label <= vec->i) {
      datum = di_tree_forward(vec->next, kid, dlits, subst);
      kid = kid->next;
    }
    return (Topform) datum;
  }
}


Topform Di_treeContainer::forward_feature_subsume(Topform d, Di_tree root) {
  Ilist f = LADR_GLOBAL_FEATURES.features(d->literals);
  UnifyContainer U;
  
  Context subst = U.get_context();
  Topform c = di_tree_forward(f, root, d->literals, subst);
  U.free_context(subst);
  IlistContainer I(f);
  I.zap_ilist();
  return c;
}

void Di_treeContainer::di_tree_back(Ilist vec, Di_tree node, Literals clits, Context subst, Plist *subsumees) {
  BUMP_SUB_CALLS;
  PlistContainer P;
  if (vec == NULL) {
    Plist p = node->u.data;
    while (p) {
      Topform d = (Topform) p->v;
      LADR_GLOBAL_DI_TREE.Nonunit_bsub_tests++;
      if (clits != d->literals && subsumes_di(clits, d->literals, subst)) {
		  P.set_head(*subsumees);
          *subsumees = P.plist_prepend(d);
      }
      p = p->next;
    }
  }
  else {
    Di_tree kid = node->u.kids;
    /* kids are in increasing order; look at those >= vec->i */
    while (kid && kid->label < vec->i)
      kid = kid->next;
    while (kid) {
      di_tree_back(vec->next, kid, clits, subst, subsumees);
      kid = kid->next;
    }
  }
} 


Plist Di_treeContainer::back_feature_subsume(Topform c, Di_tree root) {
  Ilist f = LADR_GLOBAL_FEATURES.features(c->literals);
  
  UnifyContainer U;
  Context subst = U.get_context();
  Plist subsumees = NULL;
  di_tree_back(f, root, c->literals, subst, &subsumees);
  U.free_context(subst);
  IlistContainer I(f);
  I.zap_ilist();
  return subsumees;
} 

unsigned Di_treeContainer::mega_sub_calls(void) {
  return
    (LADR_GLOBAL_DI_TREE.Sub_calls / 1000000) +
    ((UINT_MAX / 1000000) * LADR_GLOBAL_DI_TREE.Sub_calls_overflows);
}
