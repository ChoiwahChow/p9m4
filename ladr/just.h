#ifndef TP_JUST_H
#define TP_JUST_H


#include "parse.h"
#include "topform.h"



enum class Just_type {
  INPUT_JUST,         /* Primary                              */
  GOAL_JUST,          /* Primary                              */
  DENY_JUST,          /* Primary    int       (ID)            */
  CLAUSIFY_JUST,      /* Primary    int       (ID)            */
  COPY_JUST,          /* Primary    int       (ID)            */
  BACK_DEMOD_JUST,    /* Primary    int       (ID)            */
  BACK_UNIT_DEL_JUST, /* Primary    int       (ID)            */
  NEW_SYMBOL_JUST,    /* Primary    int       (ID)            */
  EXPAND_DEF_JUST,    /* Primary    Ilist     (ID,def-ID)     */
  BINARY_RES_JUST,    /* Primary    Ilist                     */
  HYPER_RES_JUST,     /* Primary    Ilist                     */
  UR_RES_JUST,        /* Primary    Ilist                     */
  FACTOR_JUST,        /* Primary    Ilist     (ID,lit1,lit2)  */
  XXRES_JUST,         /* Primary    Ilist     (ID,lit)        */
  PARA_JUST,          /* Primary    Parajust                  */
  PARA_FX_JUST,       /* Primary    Parajust                  */
  PARA_IX_JUST,       /* Primary    Parajust                  */
  PARA_FX_IX_JUST,    /* Primary    Parajust                  */
  INSTANCE_JUST,      /* Primary    Instancejust              */
  PROPOSITIONAL_JUST, /* Primary    int       (ID)            */

  DEMOD_JUST,         /* Secondary  I3list                    */
  UNIT_DEL_JUST,      /* Secondary  Ilist     (lit,ID)        */
  FLIP_JUST,          /* Secondary  int       (lit)           */
  XX_JUST,            /* Secondary  int       (lit)           */
  MERGE_JUST,         /* Secondary  int       (lit)           */
  EVAL_JUST,          /* Secondary  int       (count)         */

  IVY_JUST,           /* Primary    Ivyjust                   */

  UNKNOWN_JUST        /* probably an error                    */
};





struct parajust {
  int from_id;
  int into_id;
  Ilist from_pos;
  Ilist into_pos;
};

typedef struct parajust * Parajust;


struct instancejust {
  int parent_id;
  Plist pairs;
};

typedef struct instancejust * Instancejust;



struct ivyjust {
  Just_type type;  /* input,resolve,paramod,instantiate,flip,propositional */
  int parent1;
  int parent2;
  Ilist pos1;
  Ilist pos2;
  Plist pairs;  /* for instantiate */
};


typedef struct ivyjust * Ivyjust;


struct just {
  Just_type type;
  just *next;
  union {
    int id;
    Ilist lst;
    Parajust para;
    I3list demod;
    Instancejust instance;
    Ivyjust ivy;
  } u;
};

typedef struct just * Just;

class GlobalJust {
					private:
								unsigned Just_gets, Just_frees;
								unsigned Parajust_gets, Parajust_frees;
								unsigned Instancejust_gets, Instancejust_frees;
								unsigned Ivyjust_gets, Ivyjust_frees;
                                
                    public:            
                                GlobalJust();
                                ~GlobalJust();
								
					friend class JustContainer;
                    friend class LadrVGlobais;

};



class JustContainer {

					private:
							void free_just(Just);
							void free_parajust(Parajust);
							void free_instancejust(Instancejust);
							Ivyjust get_ivyjust(void);
							void free_ivyjust(Ivyjust);
                            int jstring_to_jtype(string);
                            char itoc(int);
                            int ctoi(char);
                            void sb_write_res_just(String_buf, Just, I3list);
                            void sb_write_position(String_buf, Ilist);
                            void zap_parajust(Parajust);
                            void zap_instancejust(Instancejust);
                            void zap_ivyjust(Ivyjust);
                            Plist get_clanc(int, Plist);
                            int map_id(I2list, int);
                            int lit_string_to_int(string);
                            Ilist args_to_ilist(Term);
                            void sb_tagged_write_res_just(String_buf, Just, I3list);
				
					public:
						JustContainer();
						~JustContainer();
						Just get_just(void);
						Parajust get_parajust(void);
						Instancejust get_instancejust(void);
						void fprint_just_mem(ostream &, bool heading);
						void p_just_mem();
						Just ivy_just(Just_type,int, Ilist,int, Ilist,Plist);
						Just input_just(void);
						Just goal_just(void);
						Just deny_just(Topform);
						Just clausify_just(Topform);
						Just expand_def_just(Topform, Topform);
						Just copy_just(Topform);
						Just propositional_just(Topform);
						Just new_symbol_just(Topform);
						Just back_demod_just(Topform);
						Just back_unit_deletion_just(Topform);
						Just binary_res_just(Topform, int, Topform, int);
						Just binary_res_just_by_id(int, int, int, int);
						Just factor_just(Topform, int, int);
						Just xxres_just(Topform, int);
						Just resolve_just(Ilist, Just_type);
						Just demod_just(I3list);
						Just para_just(Just_type,Topform, Ilist,Topform, Ilist);
						Just instance_just(Topform, Plist);
						Just para_just_rev_copy(Just_type,Topform,Ilist,Topform,Ilist);
						Just unit_del_just(Topform, int);
						Just flip_just(int);
						Just xx_just(int);
						Just merge_just(int);
						Just eval_just(int);
						Just append_just(Just, Just);
						Just copy_justification(Just);
						string jstring(Just);
						int jmap1(I3list, int);
						string jmap2(I3list, int, string a);
						void sb_append_id(String_buf, int, I3list);
						void sb_write_ids(String_buf, Ilist, I3list);
						void sb_write_just(String_buf, Just, I3list);
						void sb_xml_write_just(String_buf, Just, I3list);
						void p_just(Just);
						void zap_just(Just);
						Ilist get_parents(Just, bool);
						Topform first_negative_parent(Topform);
						Plist get_clause_ancestors(Topform);
						int proof_length(Plist);
						void map_just(Just, I2list);
						int just_count(Just);
						void mark_parents_as_used(Topform);
						int clause_level(Topform);
						Just term_to_just(Term);
						bool primary_just_type(Topform, Just_type);
						bool has_input_just(Topform);
						bool has_copy_just(Topform);
						bool has_copy_flip_just(Topform);
						void sb_tagged_write_just(String_buf, Just, I3list);
                        I2list cl_level(Topform, I2list);
								

                      
};


#endif
