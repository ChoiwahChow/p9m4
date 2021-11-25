#ifndef TP_DI_TREE_H
#define TP_DI_TREE_H

#include "features.h"
#include "topform.h"

#define BUMP_SUB_CALLS {LADR_GLOBAL_DI_TREE.Sub_calls++; if (LADR_GLOBAL_DI_TREE.Sub_calls == 0) LADR_GLOBAL_DI_TREE.Sub_calls_overflows++;}


typedef struct di_tree * Di_tree;

struct di_tree {       /* node in an integer vector discrimination tree */
  int label;           /* label of node */
  Di_tree   next;      /* sibling */
  union {
    Di_tree kids;      /* for internal nodes */
    Plist data;        /* for leaves */
  } u;
};


class GlobalDiTree {
	
						private:
									int Nonunit_fsub_tests;
									int Nonunit_bsub_tests;
									unsigned Sub_calls;
									unsigned Sub_calls_overflows;
									unsigned Di_tree_gets, Di_tree_frees;
                                    
                        public:            
                                    GlobalDiTree();    
                                    ~GlobalDiTree();
	
						friend class Di_treeContainer;
                        friend class LadrVGlobais;
};

class Di_treeContainer {

							private: Di_tree root;
										
									 bool subsume_di_literals(Literals, Context, Literals, Trail *);
									 bool subsumes_di(Literals, Literals, Context);
									 Topform di_tree_forward(Ilist, Di_tree, Literals , Context);
									 void di_tree_back(Ilist, Di_tree, Literals, Context, Plist *);
							
							public:
									Di_treeContainer();
									~Di_treeContainer();
									
									int nonunit_fsub_tests(void);
									int nonunit_bsub_tests(void);
									Di_tree get_di_tree(void);
									void free_di_tree(Di_tree);
									void fprint_di_tree_mem(ostream &, bool);
									void p_di_tree_mem(void);
									Di_tree init_di_tree(void);
									void di_tree_insert(Ilist, Di_tree, void *);
									bool di_tree_delete(Ilist, Di_tree, void *);
									void zap_di_tree(Di_tree, int);
									void p_di_tree(Ilist, Di_tree, int);
									Topform forward_feature_subsume(Topform, Di_tree);
									Plist back_feature_subsume(Topform, Di_tree);
									unsigned mega_sub_calls(void);
};



#endif
