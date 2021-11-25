#ifndef _VGLOBAIS_H
#define _VGLOBAIS_H



#include "symbols.h"
#include "term.h"
#include "listterm.h"
#include "termflag.h"
#include "unify.h"
#include "parse.h"
#include "attrib.h"
#include "avltree.h"
#include "basic.h"
#include "btm.h"
#include "btu.h"
#include "clash.h"
#include "clause_eval.h"
#include "clauseid.h"
#include "clist.h"
#include "clock.h"
#include "cnf.h"
#include "di_tree.h"
#include "discrim.h"
#include "discrimb.h"
#include "discrimw.h"
#include "fpa.h"
#include "fpalist.h"
#include "glist.h"
#include "hash.h"
#include "interp.h"
#include "just.h"
#include "lindex.h"
#include "literals.h"
#include "memory.h"
#include "mindex.h"
#include "options.h"
#include "parautil.h"
#include "pindex.h"
#include "std_options.h"
#include "strbuf.h"
#include "top_input.h"
#include "topform.h"
#include "order.h"
#include "accanon.h"



#define LADRV_GLOBAIS_INST              LADR_V_GLOBAIS::ladr_v_globais         


#define LADR_GLOBAL_TERM                LADRV_GLOBAIS_INST.global_term
#define LADR_GLOBAL_PARSE               LADRV_GLOBAIS_INST.global_parse
#define LADR_GLOBAL_TERM_FLAG           LADRV_GLOBAIS_INST.global_term_flag
#define LADR_GLOBAL_UNIFY               LADRV_GLOBAIS_INST.global_unify
#define LADR_GLOBAL_SYMBOL              LADRV_GLOBAIS_INST.global_symbol
#define LADR_GLOBAL_ATTRIBUTE           LADRV_GLOBAIS_INST.global_attribute
#define LADR_GLOBAL_AVL_TREE            LADRV_GLOBAIS_INST.global_avl_tree
#define LADR_GLOBAL_BASIC               LADRV_GLOBAIS_INST.global_basic
#define LADR_GLOBAL_BTM                 LADRV_GLOBAIS_INST.global_btm
#define LADR_GLOBAL_BTU                 LADRV_GLOBAIS_INST.global_btu
#define LADR_GLOBAL_CLASH               LADRV_GLOBAIS_INST.global_clash
#define LADR_GLOBAL_CLAUSE_EVAL         LADRV_GLOBAIS_INST.global_clause_eval
#define LADR_GLOBAL_CLAUSE_ID           LADRV_GLOBAIS_INST.global_clause_id
#define LADR_GLOBAL_CLIST               LADRV_GLOBAIS_INST.global_clist
#define LADR_GLOBAL_CLOCK               LADRV_GLOBAIS_INST.global_clock
#define LADR_GLOBAL_CNF                 LADRV_GLOBAIS_INST.global_cnf
#define LADR_GLOBAL_DI_TREE             LADRV_GLOBAIS_INST.global_di_tree
#define LADR_GLOBAL_DISCRIM             LADRV_GLOBAIS_INST.global_discrim
#define LADR_GLOBAL_DISCRIM_B           LADRV_GLOBAIS_INST.global_discrim_b
#define LADR_GLOBAL_DISCRIM_W           LADRV_GLOBAIS_INST.global_discrim_w
#define LADR_GLOBAL_FEATURES            LADRV_GLOBAIS_INST.global_features
#define LADR_GLOBAL_FLATTERM            LADRV_GLOBAIS_INST.global_flatterm
#define LADR_GLOBAL_FORMULA             LADRV_GLOBAIS_INST.global_formula
#define LADR_GLOBAL_FPA_INDEX           LADRV_GLOBAIS_INST.global_fpa_index
#define LADR_GLOBAL_FPA_LIST            LADRV_GLOBAIS_INST.global_fpa_list
#define LADR_GLOBAL_G_LIST              LADRV_GLOBAIS_INST.global_g_list
#define LADR_GLOBAL_HASH                LADRV_GLOBAIS_INST.global_hash
#define LADR_GLOBAL_INTERP              LADRV_GLOBAIS_INST.global_interp
#define LADR_GLOBAL_JUST                LADRV_GLOBAIS_INST.global_just
#define LADR_GLOBAL_LINDEX              LADRV_GLOBAIS_INST.global_lindex
#define LADR_GLOBAL_LITERALS            LADRV_GLOBAIS_INST.global_literals
#define LADR_GLOBAL_MEMORY              LADRV_GLOBAIS_INST.global_memory
#define LADR_GLOBAL_MINDEX              LADRV_GLOBAIS_INST.global_mindex
#define LADR_GLOBAL_OPTIONS             LADRV_GLOBAIS_INST.global_options
#define LADR_GLOBAL_PARAUTIL            LADRV_GLOBAIS_INST.global_parautil
#define LADR_GLOBAL_PINDEX              LADRV_GLOBAIS_INST.global_pindex
#define LADR_GLOBAL_STD_OPTIONS         LADRV_GLOBAIS_INST.global_std_options
#define LADR_GLOBAL_STR_BUF             LADRV_GLOBAIS_INST.global_str_buf
#define LADR_GLOBAL_TOP_INPUT           LADRV_GLOBAIS_INST.global_top_input
#define LADR_GLOBAL_TOPFORM             LADRV_GLOBAIS_INST.global_topform



//Global Variables - This is a class with all the ladr global variables
class LadrVGlobais {
    
    //public properties
    public:
                     
                        GlobalTermflag          global_term_flag;
                        GlobalParse             global_parse;
                        GlobalTerm              global_term;
                        GlobalUnify             global_unify;
                        GlobalSymbol            global_symbol;
                        GlobalAttribute         global_attribute;
                        GlobalAvltree           global_avl_tree;
                        GlobalBasic             global_basic;
                        GlobalBtm               global_btm;
                        GlobalBtu               global_btu;
                        GlobalClash             global_clash;
                        GlobalClauseEval        global_clause_eval;
                        GlobalClauseId          global_clause_id;
                        GlobalClist             global_clist;
                        GlobalClocks            global_clock;
                        GlobalCnf               global_cnf;
                        GlobalDiTree            global_di_tree;
                        GlobalDiscrim           global_discrim;
                        GlobalDiscrimB          global_discrim_b;
                        GlobalDiscrimW          global_discrim_w;
                        GlobalFeatures          global_features;
                        GlobalFlatterm          global_flatterm;
                        GlobalFormula           global_formula;
                        GlobalFpaIndex          global_fpa_index;
                        GlobalFpaList           global_fpa_list; 
                        GlobalGlist             global_g_list;       
                        GlobalHash              global_hash;
                        GlobalInterp            global_interp;
                        GlobalJust              global_just;    
                        GlobalLindex            global_lindex;
                        GlobalLiterals          global_literals;  
                        GlobalMemory            global_memory;   
                        GlobalMindex            global_mindex;
                        GlobalOptions           global_options;
                        GlobalParautil          global_parautil;
                        GlobalPindex            global_pindex;
                        GlobalStdOptions        global_std_options;
                        GlobalStrbuf            global_str_buf;   
                        TopInput                global_top_input;
                        GlobalTopform           global_topform;
                        
                        LiteralsContainer       Lit;
                        
                        
                        
    
    //public nethods             
    public:
                        
                        LadrVGlobais();     //no nedd for nothing
                        ~LadrVGlobais();    //no need for nothing
                        void FREE_MEM(void);
        
                     
};



//This is the global instance of Ladr global variables
//this instance is static, so, the compiler creates a global variable with this instance
//It will be used by all the LADR code
class LADR_V_GLOBAIS {
public:
        static LadrVGlobais ladr_v_globais; //this static instance makes all of its objects constructors be called on startup
                                            //and its destructors on exit
                                            
                 
    
};







#endif
