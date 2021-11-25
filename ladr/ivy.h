#ifndef TP_IVY_H
#define TP_IVY_H

#include "xproofs.h"

#define DICT_SIZE 6



class Ivy {
				private:
						static string Dict[DICT_SIZE][2];
						
						static string dict_lookup(string key);
						static void ivy_term_trans(Term);
						static void ivy_clause_trans(Topform);
						static void sb_ivy_write_term(String_buf, Term);
						static void sb_ivy_write_pair(String_buf, Term);
						static void sb_ivy_write_pairs(String_buf, Plist);
						static void sb_ivy_write_position(String_buf, Ilist);
						static void sb_ivy_write_lit(String_buf, Literals);
						static void sb_ivy_write_literals(String_buf, Literals);
						static Topform instantiate_inference(Topform, Context);
						static Topform renumber_inference(Topform);
						static Ilist ivy_lit_position(int, int);
						static Ilist ivy_para_position(Ilist, bool, int);
						static Plist paramod2_instances(Topform, Ilist,Topform, Ilist, int *);
						static Topform flip_inference(Topform, int);
						static Plist resolve2_instances(Topform, int, Topform, int, int *);
						static Plist factor2_instances(Topform, int, int, int *);
						static Plist copy_proof_and_rename_symbols_for_ivy(Plist);
				
	
				public:
						static void sb_ivy_write_just(String_buf, Ivyjust, I3list);
						static void sb_ivy_write_clause_jmap(String_buf, Topform, I3list);
						static Plist expand_proof_ivy(Plist);
				
};

#endif
