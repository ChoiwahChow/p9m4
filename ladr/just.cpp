#include "just.h"
#include "memory.h"
#include "mystring.h"
#include "strbuf.h"
#include "clauseid.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>



GlobalJust::GlobalJust() {
    Just_gets=0;
    Just_frees=0;
    Parajust_gets=0;
    Parajust_frees=0;
    Instancejust_gets=0;
    Instancejust_frees=0;
    Ivyjust_gets=0;
    Ivyjust_frees=0;
}

GlobalJust::~GlobalJust()
{
}





JustContainer::JustContainer(){

}


JustContainer::~JustContainer() {
}


Just JustContainer::get_just(void){
  Just p = (Just) Memory::memCNew(sizeof(struct just));
  LADR_GLOBAL_JUST.Just_gets++;
  return(p);
} 

void JustContainer::free_just(Just p) {
  Memory::memFree((void*) p, sizeof(struct just));
  LADR_GLOBAL_JUST.Just_frees++;
} 

Parajust JustContainer::get_parajust(void) {
  Parajust p= (Parajust) Memory::memCNew(sizeof(struct parajust));
  LADR_GLOBAL_JUST.Parajust_gets++;
  return(p);
}


void JustContainer::free_parajust(Parajust p) {
  Memory::memFree((void *) p, sizeof(struct parajust));
  LADR_GLOBAL_JUST.Parajust_frees++;
} 


Instancejust JustContainer::get_instancejust(void) {
  Instancejust p = (Instancejust) Memory::memCNew(sizeof(instancejust));
  LADR_GLOBAL_JUST.Instancejust_gets++;
  return(p);
}


void JustContainer::free_instancejust(Instancejust p){
  Memory::memFree((void *) p, sizeof(struct instancejust));
  LADR_GLOBAL_JUST.Instancejust_frees++;
} 

Ivyjust JustContainer::get_ivyjust(void) {
  Ivyjust p = (Ivyjust) Memory::memCNew(sizeof(struct ivyjust));
  LADR_GLOBAL_JUST.Ivyjust_gets++;
  return(p);
} 

void JustContainer::free_ivyjust(Ivyjust p) {
  Memory::memFree((void *) p, sizeof(struct ivyjust));
  LADR_GLOBAL_JUST.Ivyjust_frees++;
} 

void JustContainer::fprint_just_mem(ostream &o, bool heading)
{
  int n;
  if (heading)
	o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;

  n = sizeof(struct just);
  o<<"Just          ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_JUST.Just_gets;
  o<<setw(11)<<LADR_GLOBAL_JUST.Just_frees;
  o<<setw(11)<<LADR_GLOBAL_JUST.Just_gets-LADR_GLOBAL_JUST.Just_frees;
  o<<setw(9)<<( (LADR_GLOBAL_JUST.Just_gets-LADR_GLOBAL_JUST.Just_frees)*n)/1024<<"K"<<endl;
  
  n = sizeof(struct parajust);
  o<<"Parajust      ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_JUST.Parajust_gets;
  o<<setw(11)<<LADR_GLOBAL_JUST.Parajust_frees;
  o<<setw(11)<<LADR_GLOBAL_JUST.Parajust_gets-LADR_GLOBAL_JUST.Parajust_frees;
  o<<setw(9)<<( (LADR_GLOBAL_JUST.Parajust_gets-LADR_GLOBAL_JUST.Parajust_frees)*n)/1024<<"K"<<endl;

  n = sizeof(struct instancejust);
  o<<"Instancejust  ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_JUST.Instancejust_gets;
  o<<setw(11)<<LADR_GLOBAL_JUST.Instancejust_frees;
  o<<setw(11)<<LADR_GLOBAL_JUST.Instancejust_gets-LADR_GLOBAL_JUST.Instancejust_frees;
  o<<setw(9)<<( (LADR_GLOBAL_JUST.Instancejust_gets-LADR_GLOBAL_JUST.Instancejust_frees)*n)/1024<<"K"<<endl;
		  

  n = sizeof(struct ivyjust);
  o<<"Ivyjust       ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_JUST.Ivyjust_gets;
  o<<setw(11)<<LADR_GLOBAL_JUST.Ivyjust_frees;
  o<<setw(11)<<LADR_GLOBAL_JUST.Ivyjust_gets-LADR_GLOBAL_JUST.Ivyjust_frees;
  o<<setw(9)<<( (LADR_GLOBAL_JUST.Ivyjust_gets-LADR_GLOBAL_JUST.Ivyjust_frees)*n)/1024<<"K"<<endl;
}  /* fprint_just_mem */


/* PUBLIC */
void JustContainer::p_just_mem() {
  fprint_just_mem(cout, true);
} 


Just JustContainer::ivy_just(Just_type type,    int parent1, Ilist pos1,   int parent2, Ilist pos2, Plist pairs) {
  Just j = get_just();
  j->type = Just_type::IVY_JUST;
  j->u.ivy = get_ivyjust();
  j->u.ivy->type = type;
  j->u.ivy->parent1 = parent1;
  j->u.ivy->parent2 = parent2;
  j->u.ivy->pos1 = pos1;
  j->u.ivy->pos2 = pos2;
  j->u.ivy->pairs = pairs;
  return j;
} 


Just JustContainer::input_just(void) {
  /* (INPUT_JUST) */
  Just j = get_just();
  j->type = Just_type::INPUT_JUST;
  return j;
} 

Just JustContainer::goal_just(void) {
  /* (GOAL_JUST) */
  Just j = get_just();
  j->type = Just_type::GOAL_JUST;
  return j;
} 


Just JustContainer::deny_just(Topform tf){
  /* (DENY_JUST) */
  Just j = get_just();
  j->type = Just_type::DENY_JUST;
  j->u.id = tf->id;
  return j;
}

Just JustContainer::clausify_just(Topform tf){
  /* (CLAUSIFY_JUST) */
  Just j = get_just();
  j->type = Just_type::CLAUSIFY_JUST;
  j->u.id = tf->id;
  return j;
}

Just JustContainer::expand_def_just(Topform tf, Topform def) {
  /* (expand_def_JUST) */
  IlistContainer I;
  Just j = get_just();
  j->type = Just_type::EXPAND_DEF_JUST;
  I.ilist_append(tf->id);
  j->u.lst = I.ilist_append(def->id);
  return j;
} 

Just JustContainer::copy_just(Topform c) {
  /* (COPY_JUST parent_id) */
  Just j = get_just();
  j->type = Just_type::COPY_JUST;
  j->u.id = c->id;
  return j;
} 

Just JustContainer::propositional_just(Topform c) {
  /* (PROPOSITIONAL_JUST parent_id) */
  Just j = get_just();
  j->type = Just_type::PROPOSITIONAL_JUST;
  j->u.id = c->id;
  return j;
} 


Just JustContainer::new_symbol_just(Topform c) {
  /* (NEW_SYMBOL_JUST parent_id) */
  Just j = get_just();
  j->type = Just_type::NEW_SYMBOL_JUST;
  j->u.id = c->id;
  return j;
} 


Just JustContainer::back_demod_just(Topform c) {
  /* (BACK_DEMOD_JUST parent_id) */
  Just j = get_just();
  j->type = Just_type::BACK_DEMOD_JUST;
  j->u.id = c->id;
  return j;
}



Just JustContainer::back_unit_deletion_just(Topform c) {
  /* (BACK_UNIT_DEL_JUST parent_id) */
  Just j = get_just();
  j->type = Just_type::BACK_UNIT_DEL_JUST;
  j->u.id = c->id;
  return j;
}  


Just JustContainer::binary_res_just(Topform c1, int n1, Topform c2, int n2) {
  /* (BINARY_RES_JUST (id1 lit1 id2 lit2) */
  IlistContainer I;
  Just j = get_just();
  j->type = Just_type::BINARY_RES_JUST;
  I.ilist_append(c1->id);
  I.ilist_append(n1);
  I.ilist_append(c2->id);
  I.ilist_append(n2);
  j->u.lst=I.get_head();  
  return j;
}  /* binary_res_just */



Just JustContainer::binary_res_just_by_id(int c1, int n1, int c2, int n2) {
  /* (BINARY_RES_JUST (id1 lit1 id2 lit2) */
  IlistContainer I;
  Just j = get_just();
  j->type = Just_type::BINARY_RES_JUST;
  I.ilist_append(c1);
  I.ilist_append(n2);
  I.ilist_append(c2);
  I.ilist_append(n2);
  j->u.lst = I.get_head();
  return j;
} 


Just JustContainer::factor_just(Topform c, int lit1, int lit2) {
  /* (FACTOR_JUST (clause_id lit1 lit2)) */
  IlistContainer I;
  Just j = get_just();
  j->type = Just_type::FACTOR_JUST;
  I.ilist_append(c->id);
  I.ilist_append(lit1);
  I.ilist_append(lit2);
  j->u.lst = I.get_head();
  return j;
}  

Just JustContainer::xxres_just(Topform c, int lit) {
  /* (XXRES_JUST (clause_id lit)) */
  IlistContainer I;
  Just j = get_just();
  j->type = Just_type::XXRES_JUST;
  I.ilist_append(c->id);
  I.ilist_append(lit);
  j->u.lst = I.get_head();
  return j;
}


Just JustContainer::resolve_just(Ilist g, Just_type type) {
  Just j = get_just();
  j->type = type;
  j->u.lst = g;
  return j;
}

Just JustContainer::demod_just(I3list steps) {
  Just j = get_just();
  j->type = Just_type::DEMOD_JUST;
  j->u.demod = steps;
  return j;
}  /* demod_just */


Just JustContainer::para_just(Just_type rule, Topform from, Ilist from_vec, Topform into, Ilist into_vec) {
  Just j = get_just();
  j->type = rule;
  j->u.para = get_parajust();
  j->u.para->from_id = from->id;
  j->u.para->into_id = into->id;
  j->u.para->from_pos = from_vec;
  j->u.para->into_pos = into_vec;
  return j;
}

Just JustContainer::instance_just(Topform parent, Plist pairs) {
  Just j = get_just();
  j->type = Just_type::INSTANCE_JUST;
  j->u.instance = get_instancejust();
  j->u.instance->parent_id = parent->id;
  j->u.instance->pairs = pairs;
  return j;
} 


Just JustContainer::para_just_rev_copy(Just_type rule, Topform from, Ilist from_vec, Topform into, Ilist into_vec) {
    IlistContainer I1,I2;
    Ilist i1, i2;
    I1.set_head(from_vec);
    I2.set_head(into_vec);
    
    i1=I1.copy_ilist();//fazer uma cópia
    i2=I2.copy_ilist();
    I1.set_head(i1);
    I2.set_head(i2);
    I1.reverse_ilist(); //reverter a lista
    I2.reverse_ilist();
    return para_just(rule, from, I1.get_head(), into, I2.get_head());
}


Just JustContainer::unit_del_just(Topform deleter, int literal_num) {
  /* UNIT_DEL (literal-num clause-id) */
  IlistContainer I;
  Just j = get_just();
  j->type = Just_type::UNIT_DEL_JUST;
  I.ilist_append(literal_num);
  I.ilist_append(deleter->id);
  j->u.lst = I.get_head();
  return j;
}

Just JustContainer::flip_just(int n) {
  Just j = get_just();
  j->type = Just_type::FLIP_JUST;
  j->u.id = n;
  return j;
}

Just JustContainer::xx_just(int n) {
  Just j = get_just();
  j->type = Just_type::XX_JUST;
  j->u.id = n;
  return j;
}

Just JustContainer::merge_just(int n) {
  Just j = get_just();
  j->type = Just_type::MERGE_JUST;
  j->u.id = n;
  return j;
}  


Just JustContainer::eval_just(int n) {
  Just j = get_just();
  j->type = Just_type::EVAL_JUST;
  j->u.id = n;
  return j;
}  


Just JustContainer::append_just(Just j1, Just j2) {
  if (j1 == NULL)   return j2;
  else {
        j1->next = append_just(j1->next, j2);
        return j1;
  }
} 

Just JustContainer::copy_justification(Just j) {
  IlistContainer I;
  I3listContainer I3;
  TermContainer T;
  if (j == NULL)   return NULL;
  else {
        Just j2 = get_just();
        j2->type = j->type;
        j2->next = copy_justification(j->next);
        switch (j->type) {
            case Just_type::INPUT_JUST:
            case Just_type::GOAL_JUST:
                                        break;
            case Just_type::DENY_JUST:
            case Just_type::CLAUSIFY_JUST:
            case Just_type::COPY_JUST:
            case Just_type::PROPOSITIONAL_JUST:
            case Just_type::NEW_SYMBOL_JUST:
            case Just_type::BACK_DEMOD_JUST:
            case Just_type::BACK_UNIT_DEL_JUST:
            case Just_type::FLIP_JUST:
            case Just_type::XX_JUST:
            case Just_type::MERGE_JUST:
            case Just_type::EVAL_JUST:
                                        j2->u.id = j->u.id;
                                        break;
            case Just_type::EXPAND_DEF_JUST:
            case Just_type::BINARY_RES_JUST:
            case Just_type::HYPER_RES_JUST:
            case Just_type::UR_RES_JUST:
            case Just_type::UNIT_DEL_JUST:
            case Just_type::FACTOR_JUST:
            case Just_type::XXRES_JUST:
                                        I.set_head(j->u.lst);
                                        j2->u.lst = I.copy_ilist();
                                        break;
            case Just_type::DEMOD_JUST:
                                        I3.set_head(j->u.demod);
                                        j2->u.demod = I3.copy_i3list();
                                        break;
            case Just_type::PARA_JUST:
            case Just_type::PARA_FX_JUST:
            case Just_type::PARA_IX_JUST:
            case Just_type::PARA_FX_IX_JUST:
                                        j2->u.para = get_parajust();
                                        j2->u.para->from_id = j->u.para->from_id;
                                        j2->u.para->into_id = j->u.para->into_id;
                                        I.set_head(j->u.para->from_pos);
                                        j2->u.para->from_pos = I.copy_ilist();
                                        I.set_head(j->u.para->into_pos);
                                        j2->u.para->into_pos = I.copy_ilist();
                                        break;
            case Just_type::INSTANCE_JUST:
                                        j2->u.instance = get_instancejust();
                                        j2->u.instance->parent_id = j->u.instance->parent_id;
                                        j2->u.instance->pairs = T.copy_plist_of_terms(j->u.instance->pairs);
                                        break;
            case Just_type::IVY_JUST:
                                        j2->u.ivy = get_ivyjust();
                                        j2->u.ivy->type = j->u.ivy->type;
                                        j2->u.ivy->parent1 = j->u.ivy->parent1;
                                        j2->u.ivy->parent2 = j->u.ivy->parent2;
                                        I.set_head(j->u.ivy->pos1);
                                        j2->u.ivy->pos1 = I.copy_ilist();
                                        I.set_head(j->u.ivy->pos2);
                                        j2->u.ivy->pos2 = I.copy_ilist();
                                        j2->u.ivy->pairs = T.copy_plist_of_terms(j->u.ivy->pairs);
                                        break;
            default: fatal::fatal_error("copy_justification: unknown type");
    }
    return j2;
  }
}


string JustContainer::jstring(Just j) {
  switch (j->type) {

    /* primary justifications */

  case Just_type::INPUT_JUST:         return "assumption";
  case Just_type::GOAL_JUST:          return "goal";
  case Just_type::DENY_JUST:          return "deny";
  case Just_type::CLAUSIFY_JUST:      return "clausify";
  case Just_type::COPY_JUST:          return "copy";
  case Just_type::PROPOSITIONAL_JUST: return "propositional";
  case Just_type::NEW_SYMBOL_JUST:    return "new_symbol";
  case Just_type::BACK_DEMOD_JUST:    return "back_rewrite";
  case Just_type::BACK_UNIT_DEL_JUST: return "back_unit_del";
  case Just_type::EXPAND_DEF_JUST:    return "expand_def";
  case Just_type::BINARY_RES_JUST:    return "resolve";
  case Just_type::HYPER_RES_JUST:     return "hyper";
  case Just_type::UR_RES_JUST:        return "ur";
  case Just_type::FACTOR_JUST:        return "factor";
  case Just_type::XXRES_JUST:         return "xx_res";
  case Just_type::PARA_JUST:          return "para";
  case Just_type::PARA_FX_JUST:       return "para_fx";
  case Just_type::PARA_IX_JUST:       return "para_ix";
  case Just_type::PARA_FX_IX_JUST:    return "para_fx_ix";
  case Just_type::INSTANCE_JUST:      return "instantiate";
  case Just_type::IVY_JUST:           return "ivy";

    /* secondary justifications */

  case Just_type::FLIP_JUST:          return "flip";
  case Just_type::XX_JUST:            return "xx";
  case Just_type::MERGE_JUST:         return "merge";
  case Just_type::EVAL_JUST:          return "eval";
  case Just_type::DEMOD_JUST:         return "rewrite";
  case Just_type::UNIT_DEL_JUST:      return "unit_del";
  case Just_type::UNKNOWN_JUST:       return "unknown";
  }
  return "unknown";
}  /* jstring */

int JustContainer::jstring_to_jtype(string s) {
  
  if (myString::str_ident(s, "assumption")) return (int) Just_type::INPUT_JUST;
  
  else if (myString::str_ident(s, "goal"))  return (int) Just_type::GOAL_JUST;
  else if (myString::str_ident(s, "deny"))  return (int) Just_type::DENY_JUST;
  else if (myString::str_ident(s, "clausify"))  return (int) Just_type::CLAUSIFY_JUST;
  else if (myString::str_ident(s, "copy"))   return (int) Just_type::COPY_JUST;
  else if (myString::str_ident(s, "propositional"))    return (int) Just_type::PROPOSITIONAL_JUST;
  else if (myString::str_ident(s, "new_symbol"))    return (int) Just_type::NEW_SYMBOL_JUST;
  else if (myString::str_ident(s, "back_rewrite"))    return (int) Just_type::BACK_DEMOD_JUST;
  else if (myString::str_ident(s, "back_unit_del"))    return (int) Just_type::BACK_UNIT_DEL_JUST;
  else if (myString::str_ident(s, "expand_def"))    return (int) Just_type::EXPAND_DEF_JUST;
  else if (myString::str_ident(s, "resolve"))    return (int) Just_type::BINARY_RES_JUST;
  else if (myString::str_ident(s, "hyper"))    return (int) Just_type::HYPER_RES_JUST;
  else if (myString::str_ident(s, "ur"))    return (int) Just_type::UR_RES_JUST;
  else if (myString::str_ident(s, "factor"))    return (int) Just_type::FACTOR_JUST;
  else if (myString::str_ident(s, "xx_res"))    return (int) Just_type::XXRES_JUST;
  else if (myString::str_ident(s, "para"))    return (int) Just_type::PARA_JUST;
  else if (myString::str_ident(s, "para_fx"))    return (int) Just_type::PARA_FX_JUST;
  else if (myString::str_ident(s, "para_ix"))    return (int) Just_type::PARA_IX_JUST;
  else if (myString::str_ident(s, "instantiate"))    return (int) Just_type::INSTANCE_JUST;
  else if (myString::str_ident(s, "para_fx_ix"))    return (int) Just_type::PARA_FX_IX_JUST;
  else if (myString::str_ident(s, "flip"))    return (int) Just_type::FLIP_JUST;
  else if (myString::str_ident(s, "xx"))    return (int) Just_type::XX_JUST;
  else if (myString::str_ident(s, "merge"))    return (int) Just_type::MERGE_JUST;
  else if (myString::str_ident(s, "eval"))    return (int) Just_type::EVAL_JUST;
  else if (myString::str_ident(s, "rewrite"))    return (int) Just_type::DEMOD_JUST;
  else if (myString::str_ident(s, "unit_del"))    return (int) Just_type::UNIT_DEL_JUST;
  else if (myString::str_ident(s, "ivy"))    return (int) Just_type::IVY_JUST;
  else   return (int) Just_type::UNKNOWN_JUST;
}  /* jstring_to_jtype */



char JustContainer::itoc(int i) {
  if (i <= 0)
    return '?';
  else if (i <= 26)
    return 'a' + i - 1;
  else if (i <= 52)
    return 'A' + i - 27;
  else
    return '?';
}  /* itoc */

/*************
 *
 *   ctoi()
 *
 *************/


int JustContainer::ctoi(char c) {
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 1;
  else if (c >= 'A' && c <= 'Z')
    return c - 'A' + 27;
  else
    return INT_MIN;
} 


int JustContainer::jmap1(I3list map, int i){
  I3listContainer I3;
  I3.set_head(map);
  int id = I3.assoc2a(i);
  return (id == INT_MIN ? i : id);
}

string JustContainer::jmap2(I3list map, int i, string a){
  I3listContainer I3;
  I3.set_head(map);
  int n = I3.assoc2b(i);
  if (n == INT_MIN) a=myString::null_string();
  else if (n >= 0 && n <= 25) {   /* "A" -- "Z" */
    a.at(0) = 'A' + n;
    
  }
  else {               /* "A26", ... */
    a.at(0) = 'A';
    a=a+to_string(n);
  }
  return a;
} 



void JustContainer::sb_append_id(String_buf sb, int id, I3list map) {
  string s;
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  SBUF.sb_append_int(jmap1(map, id));
  SBUF.sb_append(jmap2(map, id, s));
} 


void JustContainer::sb_write_res_just(String_buf sb, Just g, I3list map) {
  Ilist q;
  Ilist p = g->u.lst;
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  
  SBUF.sb_append(jstring(g));
  SBUF.sb_append(string("("));
  sb_append_id(sb, p->i, map);

  for (q = p->next; q != NULL; q = q->next->next->next) {
    int nuc_lit = q->i;
    int sat_id  = q->next->i;
    int sat_lit = q->next->next->i;
    SBUF.sb_append(",");
    SBUF.sb_append_char(itoc(nuc_lit));
    if (sat_id == 0)
      SBUF.sb_append(",xx");
    else {
      SBUF.sb_append(",");
      sb_append_id(sb, sat_id, map);
      SBUF.sb_append(",");
      if (sat_lit > 0)
        SBUF.sb_append_char(itoc(sat_lit));
      else {
            SBUF.sb_append_char(itoc(-sat_lit));
            SBUF.sb_append("(flip)");
      }
    }
  }
  SBUF.sb_append(")");
}  



void JustContainer::sb_write_position(String_buf sb, Ilist p) {
  Ilist q;
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  SBUF.sb_append("(");
  SBUF.sb_append_char(itoc(p->i));
  for (q = p->next; q != NULL; q = q->next) {
    SBUF.sb_append(",");
    SBUF.sb_append_int(q->i);
  }
  SBUF.sb_append(")");
} 


void JustContainer::sb_write_ids(String_buf sb, Ilist p, I3list map) {
  Ilist q;
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  for (q = p; q; q = q->next) {
    sb_append_id(sb, q->i, map);
    if (q->next)
      SBUF.sb_append(" ");
  }
} 


void JustContainer::sb_write_just(String_buf sb, Just just, I3list map) {
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  Just g = just;
  SBUF.sb_append("[");
  while (g != NULL) {
    Just_type rule = g->type;
    if (rule == Just_type::INPUT_JUST || rule == Just_type::GOAL_JUST)
      SBUF.sb_append(jstring(g));
    else if (rule==Just_type::BINARY_RES_JUST || rule==Just_type::HYPER_RES_JUST || rule==Just_type::UR_RES_JUST) {
      sb_write_res_just(sb, g, map);
    }
    else if (rule == Just_type::DEMOD_JUST) {
      I3list p;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("([");
      for (p = g->u.demod; p; p = p->next) {
        SBUF.sb_append_int(p->i);
        if (p->j > 0) {
            SBUF.sb_append("(");
            SBUF.sb_append_int(p->j);
            if (p->k == 2)
                SBUF.sb_append(",R");
            SBUF.sb_append(")");
        }

        SBUF.sb_append(p->next ? "," : "");
      }
      SBUF.sb_append("])");
    }
    else if (rule == Just_type::UNIT_DEL_JUST) {
      Ilist p = g->u.lst;
      int n = p->i;
      int id = p->next->i;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      if (n < 0) {
        SBUF.sb_append_char(itoc(-n));
        SBUF.sb_append("(flip),");
      }
      else {
        SBUF.sb_append_char(itoc(n));
        SBUF.sb_append(",");
      }
      sb_append_id(sb, id, map);
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::FACTOR_JUST) {
      Ilist p = g->u.lst;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      sb_append_id(sb, p->i, map);
      SBUF.sb_append(",");
      SBUF.sb_append_char(itoc(p->next->i));
      SBUF.sb_append(",");
      SBUF.sb_append_char(itoc(p->next->next->i));
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::XXRES_JUST) {
      Ilist p = g->u.lst;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      sb_append_id(sb, p->i, map);
      SBUF.sb_append(",");
      SBUF.sb_append_char(itoc(p->next->i));
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::EXPAND_DEF_JUST) {
      Ilist p = g->u.lst;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      sb_append_id(sb,p->i, map);
      SBUF.sb_append(",");
      sb_append_id(sb, p->next->i, map);
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::BACK_DEMOD_JUST ||
	     rule == Just_type::BACK_UNIT_DEL_JUST ||
	     rule == Just_type::NEW_SYMBOL_JUST ||
	     rule == Just_type::COPY_JUST ||
	     rule == Just_type::DENY_JUST ||
	     rule == Just_type::CLAUSIFY_JUST ||
	     rule == Just_type::PROPOSITIONAL_JUST) {
            int id = g->u.id;
            SBUF.sb_append(jstring(g));
            SBUF.sb_append("(");
            sb_append_id(sb, id, map);
            SBUF.sb_append(")");
    }
    else if (rule == Just_type::FLIP_JUST ||
	     rule == Just_type::XX_JUST ||
	     rule == Just_type::MERGE_JUST) {
            int id = g->u.id;
            SBUF.sb_append(jstring(g));
            SBUF.sb_append("(");
            SBUF.sb_append_char(itoc(id));
            SBUF.sb_append(")");
    }
    else if (rule == Just_type::EVAL_JUST) {
            int id = g->u.id;
            SBUF.sb_append(jstring(g));
            SBUF.sb_append("(");
            SBUF.sb_append_int(id);
            SBUF.sb_append(")");
    }
    else if (rule == Just_type::PARA_JUST || rule == Just_type::PARA_FX_JUST ||
                rule == Just_type::PARA_IX_JUST || rule == Just_type::PARA_FX_IX_JUST) {
            Parajust p = g->u.para;
            SBUF.sb_append(jstring(g));
            SBUF.sb_append("(");
            sb_append_id(sb, p->from_id, map);
            sb_write_position(sb, p->from_pos);
            SBUF.sb_append(",");
            sb_append_id(sb, p->into_id, map);
            sb_write_position(sb, p->into_pos);
            SBUF.sb_append(")");
    }
    else if (rule == Just_type::INSTANCE_JUST) {
      Plist p;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      SBUF.sb_append_int(g->u.instance->parent_id);
      SBUF.sb_append(",[");
      ParseContainer P;  
      for (p = g->u.instance->pairs; p; p = p->next) {
        P.sb_write_term(sb, Term(p->v));
        if (p->next) 	  SBUF.sb_append(",");
      }
      SBUF.sb_append("])");
    }
    else if (rule == Just_type::IVY_JUST) {
      SBUF.sb_append(jstring(g)
    );
    }
    else {
      printf("\nunknown rule: %d\n",(int) rule);
      fatal::fatal_error("sb_write_just: unknown rule");
    }
    g = g->next;
    if (g)
      SBUF.sb_append(",");
  }
  SBUF.sb_append("].");
}  



void JustContainer::sb_xml_write_just(String_buf sb, Just just, I3list map)
{
  Just g;
  
  /* Put the standard form into an attribute. */
  StrbufContainer SBUF;
  StrbufContainer SBUF1;
  String_buf sb_standard = SBUF1.get_string_buf();
  SBUF.set_string_buf(sb);
  
  sb_write_just(sb_standard, just, map);
  SBUF.sb_append("    <justification jstring=\""); 
  SBUF.sb_cat(sb_standard);  /* this frees sb_standard */
  SBUF.sb_append("\">\n");

  /* Put an abbreviated form (rule, parents) into an XML elements. */

  for (g = just; g; g = g->next) {

    Ilist parents = get_parents(g, false);  /* for this node only */

    if (g == just)  SBUF.sb_append("      <j1 rule=\"");
    else   SBUF.sb_append("      <j2 rule=\"");
    SBUF.sb_append(jstring(g));
    SBUF.sb_append("\"");

    if (parents) {
      SBUF.sb_append(" parents=\"");
      sb_write_ids(sb, parents, map);
      IlistContainer I(parents);
      I.zap_ilist();
      SBUF.sb_append("\"");
    }

    SBUF.sb_append("/>\n");  /* close the <j1 or <j2 */
  }
  SBUF.sb_append("    </justification>\n");
}  /* sb_xml_write_just */


void JustContainer::p_just(Just j) {
  StrbufContainer SBUF;
  String_buf sb = SBUF.get_string_buf();
  sb_write_just(sb, j, NULL);
  SBUF.sb_append("\n");
  SBUF.p_sb();
  SBUF.zap_string_buf();
}  /* p_just */

/*************
 *
 *   zap_parajust()
 *
 *************/


void JustContainer::zap_parajust(Parajust p) {
  IlistContainer I;
  I.set_head(p->from_pos);
  I.zap_ilist();
  I.set_head(p->into_pos);
  I.zap_ilist();
  free_parajust(p);
} 




void JustContainer::zap_instancejust(Instancejust p) {
  TermContainer T;
  T.zap_plist_of_terms(p->pairs);
  free_instancejust(p);
}  /* zap_instancejust */

/*************
 *
 *   zap_ivyjust()
 *
 *************/


void JustContainer::zap_ivyjust(Ivyjust p)
{
  IlistContainer I;
  TermContainer T;
  I.set_head(p->pos1);
  I.zap_ilist();
  I.set_head(p->pos2);
  I.zap_ilist();
  T.zap_plist_of_terms(p->pairs);
  free_ivyjust(p);
}  /* zap_ivyjust */

/*************
 *
 *   zap_just()
 *
 *************/

/* DOCUMENTATION
This routine frees a justification list, including any sublists.
*/

/* PUBLIC */
void JustContainer::zap_just(Just just) {
  IlistContainer I;
  I3listContainer I3;
  if (just != NULL) {
    zap_just(just->next);
    
    switch (just->type) {
        case Just_type::INPUT_JUST:
        case Just_type::GOAL_JUST:
        case Just_type::DENY_JUST:
        case Just_type::CLAUSIFY_JUST:
        case Just_type::COPY_JUST:
        case Just_type::PROPOSITIONAL_JUST:
        case Just_type::NEW_SYMBOL_JUST:
        case Just_type::BACK_DEMOD_JUST:
        case Just_type::BACK_UNIT_DEL_JUST:
        case Just_type::FLIP_JUST:
        case Just_type::XX_JUST:
        case Just_type::MERGE_JUST:
        case Just_type::EVAL_JUST:
        break;  /* nothing to do */
        case Just_type::EXPAND_DEF_JUST:
        case Just_type::BINARY_RES_JUST:
        case Just_type::HYPER_RES_JUST:
        case Just_type::UR_RES_JUST:
        case Just_type::UNIT_DEL_JUST:
        case Just_type::FACTOR_JUST:
        case Just_type::XXRES_JUST:
                I.set_head(just->u.lst);
                I.zap_ilist(); 
                break;
        case Just_type::DEMOD_JUST:
                I3.set_head(just->u.demod);
                I3.zap_i3list();
                break;
        case Just_type::PARA_JUST:
        case Just_type::PARA_FX_JUST:
        case Just_type::PARA_IX_JUST:
        case Just_type::PARA_FX_IX_JUST:
            zap_parajust(just->u.para); break;
        case Just_type::INSTANCE_JUST:
            zap_instancejust(just->u.instance); break;
        case Just_type::IVY_JUST:
            zap_ivyjust(just->u.ivy); break;
        default: 
            fatal::fatal_error("zap_just: unknown type");
    }
    free_just(just);
  }
}  /* zap_just */


Ilist JustContainer::get_parents(Just just, bool all) {
 
  Just g = just;
  IlistContainer I;

  while (g) {
    Just_type rule = g->type;
    if (rule==Just_type::BINARY_RES_JUST || rule==Just_type::HYPER_RES_JUST || rule==Just_type::UR_RES_JUST) {
      /* [rule (nucid nuclit sat1id sat1lit nuclit2 sat2id sat2lit ...)] */
      Ilist p = g->u.lst;
      int nuc_id = p->i;
      I.ilist_prepend(nuc_id);
      p = p->next;
      while (p != NULL) {
	int sat_id = p->next->i;
	if (sat_id == 0)
	  ; /* resolution with x=x */
	else
	  I.ilist_prepend(sat_id);
	p = p->next->next->next;
      }
    }
    else if (rule == Just_type::PARA_JUST || rule == Just_type::PARA_FX_JUST ||
	     rule == Just_type::PARA_IX_JUST || rule == Just_type::PARA_FX_IX_JUST) {
      Parajust p   = g->u.para;
      I.ilist_prepend(p->from_id);
      I.ilist_prepend(p->into_id);
    }
    else if (rule == Just_type::INSTANCE_JUST) {
      I.ilist_prepend(g->u.instance->parent_id);
    }
    else if (rule == Just_type::EXPAND_DEF_JUST) {
      I.ilist_prepend(g->u.lst->i);
      I.ilist_prepend(g->u.lst->next->i);
    }
    else if (rule == Just_type::FACTOR_JUST || rule == Just_type::XXRES_JUST) {
      int parent_id = g->u.lst->i;
      I.ilist_prepend(parent_id);
    }
    else if (rule == Just_type::UNIT_DEL_JUST) {
      int parent_id = g->u.lst->next->i;
      I.ilist_prepend(parent_id);
    }
    else if (rule == Just_type::BACK_DEMOD_JUST ||
	     rule == Just_type::COPY_JUST ||
	     rule == Just_type::DENY_JUST ||
	     rule == Just_type::CLAUSIFY_JUST ||
	     rule == Just_type::PROPOSITIONAL_JUST ||
	     rule == Just_type::NEW_SYMBOL_JUST ||
	     rule == Just_type::BACK_UNIT_DEL_JUST) {
      int parent_id = g->u.id;
      I.ilist_prepend(parent_id);
    }
    else if (rule == Just_type::DEMOD_JUST) {
      I3list p;
      /* list of triples: (i=ID, j=position, k=direction) */
      for (p = g->u.demod; p; p = p->next)
        I.ilist_prepend(p->i);
    }
    else if (rule == Just_type::IVY_JUST) {
      if (g->u.ivy->type == Just_type::FLIP_JUST ||
	  g->u.ivy->type == Just_type::BINARY_RES_JUST ||
	  g->u.ivy->type == Just_type::PARA_JUST ||
	  g->u.ivy->type == Just_type::INSTANCE_JUST ||
	  g->u.ivy->type == Just_type::PROPOSITIONAL_JUST)   I.ilist_prepend(g->u.ivy->parent1);
      if (g->u.ivy->type == Just_type::BINARY_RES_JUST ||
	  g->u.ivy->type == Just_type::PARA_JUST) I.ilist_prepend(g->u.ivy->parent2);
    }
    else if (rule == Just_type::FLIP_JUST ||
	     rule == Just_type::XX_JUST ||
	     rule == Just_type::MERGE_JUST ||
	     rule == Just_type::EVAL_JUST ||
	     rule == Just_type::GOAL_JUST ||
	     rule == Just_type::INPUT_JUST)
      ;  /* do nothing */
    else
      fatal::fatal_error("get_parents, unknown rule.");

    g = (all ? g->next : NULL);
  }
  I.reverse_ilist();
  return I.get_head();
}  /* get_parents */


Topform JustContainer::first_negative_parent(Topform c)
{
  Ilist parents = get_parents(c->justification, true);
  Ilist p;
  
  TopformContainer T;
  ClauseidContainer CI;
  Topform first_neg = NULL;
  for (p = parents; p && first_neg == NULL; p = p->next) {
    Topform c = CI.find_clause_by_id(p->i);
    if (T.negative_clause_possibly_compressed(c))
      first_neg = c;
  }
  IlistContainer I(p);
  I.zap_ilist();
  return first_neg;
}

Plist JustContainer::get_clanc(int id, Plist anc) {
  ClauseidContainer C;
  Topform c = C.find_clause_by_id(id);

  if (c == NULL) {
    printf("get_clanc, clause with id=%d not found.\n", id);
    fatal::fatal_error("get_clanc, clause not found.");
  }
  PlistContainer P;
  P.set_head(anc);
  if (!P.plist_member(c)) {
    Ilist parents, p;
    anc = C.insert_clause_into_plist(anc, c, true);
    parents = get_parents(c->justification, true);

    for (p = parents; p; p = p->next) {
      anc = get_clanc(p->i, anc);
    }
    IlistContainer I(parents);
    I.zap_ilist();
  }
  return anc;
} 

Plist JustContainer::get_clause_ancestors(Topform c) {
  Plist ancestors = get_clanc(c->id, NULL);
  return ancestors;
} 

int JustContainer::proof_length(Plist proof) {
    PlistContainer P;
    P.set_head(proof);
    return P.plist_count();
}

int JustContainer::map_id(I2list a, int arg) {
  I2listContainer I2;  
  I2.set_head(a);
  int val = I2.assoc(arg);
  return val == INT_MIN ? arg : val;
} 


void JustContainer::map_just(Just just, I2list map) {
  Just j;

  for (j = just; j; j = j->next) {
    Just_type rule = j->type;
    if (rule==Just_type::BINARY_RES_JUST || rule==Just_type::HYPER_RES_JUST || rule==Just_type::UR_RES_JUST) {
      /* [rule (nucid n sat1id n n sat2id n ...)] */
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);  /* nucleus */
      p = p->next;
      while (p != NULL) {
        int sat_id = p->next->i;
        if (sat_id == 0);  /* resolution with x=x */
        else p->next->i = map_id(map, p->next->i);  /* satellite */
        p = p->next->next->next;
      }
    }
    else if (rule == Just_type::PARA_JUST || rule == Just_type::PARA_FX_JUST ||
	     rule == Just_type::PARA_IX_JUST || rule == Just_type::PARA_FX_IX_JUST) {
      Parajust p   = j->u.para;
      p->from_id = map_id(map, p->from_id);
      p->into_id = map_id(map, p->into_id);
    }
    else if (rule == Just_type::INSTANCE_JUST) {
      Instancejust p   = j->u.instance;
      p->parent_id = map_id(map, p->parent_id);
    }
    else if (rule == Just_type::EXPAND_DEF_JUST) {
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);
      p->next->i = map_id(map, p->next->i);
    }
    else if (rule == Just_type::FACTOR_JUST || rule == Just_type::XXRES_JUST) {
      Ilist p = j->u.lst;
      p->i = map_id(map, p->i);
    }
    else if (rule == Just_type::UNIT_DEL_JUST) {
      Ilist p = j->u.lst;
      p->next->i = map_id(map, p->next->i);
    }
    else if (rule == Just_type::BACK_DEMOD_JUST ||
	     rule == Just_type::COPY_JUST ||
	     rule == Just_type::DENY_JUST ||
	     rule == Just_type::CLAUSIFY_JUST ||
	     rule == Just_type::PROPOSITIONAL_JUST ||
	     rule == Just_type::NEW_SYMBOL_JUST ||
	     rule == Just_type::BACK_UNIT_DEL_JUST) {
            j->u.id = map_id(map, j->u.id);
    }
    else if (rule == Just_type::DEMOD_JUST) {
      I3list p;
      /* list of triples: <ID, position, direction> */
      for (p = j->u.demod; p; p = p->next)
        p->i = map_id(map, p->i);
    }
    else if (rule == Just_type::IVY_JUST) {
      if (j->u.ivy->type == Just_type::FLIP_JUST ||
	  j->u.ivy->type == Just_type::BINARY_RES_JUST ||
	  j->u.ivy->type == Just_type::PARA_JUST ||
	  j->u.ivy->type == Just_type::INSTANCE_JUST ||
	  j->u.ivy->type == Just_type::PROPOSITIONAL_JUST)	j->u.ivy->parent1 = map_id(map, j->u.ivy->parent1);
      
      if (j->u.ivy->type == Just_type::BINARY_RES_JUST ||
	  j->u.ivy->type == Just_type::PARA_JUST)	j->u.ivy->parent2 = map_id(map, j->u.ivy->parent2);
    }
    else if (rule == Just_type::FLIP_JUST ||
	     rule == Just_type::XX_JUST ||
	     rule == Just_type::MERGE_JUST ||
	     rule == Just_type::EVAL_JUST ||
	     rule == Just_type::GOAL_JUST ||
	     rule == Just_type::INPUT_JUST)  ;  /* do nothing */
    else
      fatal::fatal_error("get_clanc, unknown rule.");
  }
}  /* map_just */


int JustContainer::just_count(Just j) {
  if (j == 0)  return 0;
  else   return 1 + just_count(j->next);
} 

void JustContainer::mark_parents_as_used(Topform c) {
  ClauseidContainer C;
  Ilist parents = get_parents(c->justification, true);
  Ilist p;
  for (p = parents; p; p = p->next) {
    Topform parent = C.find_clause_by_id(p->i);
    parent->used = true;
  }
  IlistContainer I(parents);
  I.zap_ilist();
} 

I2list JustContainer::cl_level(Topform c, I2list others) {
  I2listContainer I2;
  ClauseidContainer C;
  I2.set_head(others);
  int level = I2.assoc(c->id);
  if (level != INT_MIN)
    return others;
  else {
    Ilist parents = get_parents(c->justification, true);
    Ilist p;
    int max = -1;
    for (p = parents; p; p = p->next) {
      Topform parent = C.find_clause_by_id(p->i);
      int parent_level;
      others = cl_level(parent, others);
      I2.set_head(others);
      parent_level = I2.assoc(parent->id);
      max = IMAX(max, parent_level);
    }
    I2.set_head(others);
    others = I2.alist_insert(c->id, max+1);
    return others;
  }
} 

int JustContainer::clause_level(Topform c) {
  I2listContainer I2;
  I2list ancestors = cl_level(c, NULL);
  I2.set_head(ancestors);
  int level = I2.assoc(c->id);
  I2.zap_i2list();
  return level;
}

int JustContainer::lit_string_to_int(string s) {
  int i;

  if (myString::str_to_int(s, &i))    return i;
  else if (s.length() > 1)    return INT_MIN;
  else  return int(s[0])-48;
} 

Ilist JustContainer::args_to_ilist(Term t) {
  IlistContainer P ;
  SymbolContainer S;
  TermContainer T;
  int i;
  for (i = 0; i < ARITY(t); i++) {
    Term a = ARG(t,i);
    string s = S.sn_to_str(SYMNUM(a));
    int x = lit_string_to_int(s);
    if (x > 0) {
      if (ARITY(a) == 1 && T.is_constant(ARG(a,0), "flip")) P.ilist_append(-x);
      else	P.ilist_append(x);
    }
    else if (myString::str_ident(s, "xx")) {
        P.ilist_append(0);
        P.ilist_append(0);
    }
    else
      fatal::fatal_error("args_to_ilist, bad arg");
  }
  return P.get_head();
}




Just JustContainer::term_to_just(Term lst) {
  ListtermContainer LT;
  TermContainer T;
  SymbolContainer S;
  if (LT.nil_term(lst))  return NULL;
  else {
    Term t = ARG(lst,0);
    Just j = get_just();
    j->next = term_to_just(ARG(lst,1));  /* do the tail */
    
    j->type = (Just_type) jstring_to_jtype(S.sn_to_str(SYMNUM(t)));
    switch (j->type) {

            /* primary and secondary are mixed */

        case Just_type::INPUT_JUST:
        case Just_type::GOAL_JUST:
                    return j;

            case Just_type::COPY_JUST:
            case Just_type::DENY_JUST:
            case Just_type::CLAUSIFY_JUST:
            case Just_type::PROPOSITIONAL_JUST:
            case Just_type::NEW_SYMBOL_JUST:
            case Just_type::BACK_DEMOD_JUST:
            case Just_type::BACK_UNIT_DEL_JUST:        {
                int id;
                if (T.term_to_int(ARG(t,0), &id)) j->u.id = id;
                else fatal::fatal_error("term_to_just, bad just id");
                return j;
            }

            case Just_type::FLIP_JUST:   /* secondary */
            case Just_type::XX_JUST:     /* secondary */
            case Just_type::EVAL_JUST:   /* secondary */
            case Just_type::MERGE_JUST: { /* secondary */
                        j->u.id = lit_string_to_int(S.sn_to_str(SYMNUM(ARG(t,0))));
                        return j;
            }

            case Just_type::DEMOD_JUST: {     /* secondary, rewrite([id(pos,side), ... ]) */
                    I3list p = NULL;
                    Term lst = ARG(t,0);
                    if (!LT.proper_listterm(lst)) fatal::fatal_error("term_to_just: rewrites must be proper list");
                    while(LT.cons_term(lst)) {
                        I3listContainer I3;
                        Term a = ARG(lst,0);
                        int i, j;
                        int k = 0;
                        I3.set_head(p);
                        string s = S.sn_to_str(SYMNUM(a));
                        if (ARITY(a) == 2 &&  myString::str_to_int(s,&i) &   T.term_to_int(ARG(a,0),&j)) {
                            if (T.is_constant(ARG(a,1), "L"))    k = 1;
                            else if (T.is_constant(ARG(a,1), "R"))   k = 2;
                            else   fatal::fatal_error("term_to_just: bad justification term (demod 1)");
                            
                            I3.i3list_append(i, j, k);
                        }
                        else if (ARITY(a) == 1 && myString::str_to_int(s,&i) &  T.term_to_int(ARG(a,0),&j)) {
                                I3.i3list_append(i, j, 1);
                            }
                        else if (ARITY(a) == 0 && myString::str_to_int(s,&i)) {
                            I3.i3list_append(i, 0, 1);
                          }
                        else  fatal::fatal_error("term_to_just: bad justification term (demod 2)");
                        lst = ARG(lst,1);
                    }
                    j->u.demod = p;
                    return j;
            }
            case Just_type::EXPAND_DEF_JUST:
            case Just_type::BINARY_RES_JUST:
            case Just_type::HYPER_RES_JUST:
            case Just_type::UR_RES_JUST:
            case Just_type::FACTOR_JUST:
            case Just_type::XXRES_JUST:
            case Just_type::UNIT_DEL_JUST:   /* secondary */
                        j->u.lst = args_to_ilist(t);
                        return j;

            case Just_type::PARA_JUST:
            case Just_type::PARA_FX_JUST:
            case Just_type::PARA_IX_JUST:
            case Just_type::PARA_FX_IX_JUST:        {
                int id;
                Term from = ARG(t,0);
                Term into = ARG(t,1);
                Parajust p = get_parajust();
                j->u.para = p;
                if (myString::str_to_int(S.sn_to_str(SYMNUM(from)), &id))  p->from_id = id;
                else  fatal::fatal_error("term_to_just, bad from_id");
                p->from_pos = args_to_ilist(from);
                if (myString::str_to_int(S.sn_to_str(SYMNUM(into)), &id)) p->into_id = id;
                else  fatal::fatal_error("term_to_just, bad into_id");
                p->into_pos = args_to_ilist(into);
                return j;
            }
            case Just_type::INSTANCE_JUST:  {
                int id;
                Term parent = ARG(t,0);
                Term pairs = ARG(t,1);
                Instancejust ij = get_instancejust();
                j->u.instance = ij;
                if (myString::str_to_int(S.sn_to_str(SYMNUM(parent)), &id)) ij->parent_id = id;
                else  fatal::fatal_error("term_to_just, bad parent_id");
                ij->pairs = NULL;
                PlistContainer P;
                P.set_head(ij->pairs);
                while (LT.cons_term(pairs)) {
                    P.plist_append( T.copy_term(ARG(pairs,0)));
                    pairs = ARG(pairs,1);
                }
                return j;
            }
            
            case Just_type::IVY_JUST:
                                fatal::fatal_error("term_to_just, IVY_JUST not handled");
                                return NULL;

            case Just_type::UNKNOWN_JUST:
            default:
                    printf("unknown: %d, %s\n",(int) j->type, jstring(j).c_str());
                    fatal::fatal_error("term_to_just, unknown justification");
                    return NULL;
    }
  }
} 


bool JustContainer:: primary_just_type(Topform c, Just_type t) {
  return c->justification && c->justification->type == t;
} 

bool JustContainer::has_input_just(Topform c) {
  return primary_just_type(c, Just_type::INPUT_JUST);
} 


bool JustContainer::has_copy_just(Topform c) {
  return primary_just_type(c, Just_type::COPY_JUST);
}


bool JustContainer::has_copy_flip_just(Topform c) {
  return (c->justification &&
	  c->justification->type == Just_type::COPY_JUST &&
	  c->justification->next &&
	  c->justification->next->type == Just_type::FLIP_JUST &&
	  c->justification->next->next == NULL);
} 

void JustContainer::sb_tagged_write_res_just(String_buf sb, Just g, I3list map) {
  Ilist q;
  Ilist p = g->u.lst;

#if 1
   /* BV(2007-jul-10) */
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  SBUF.sb_append(jstring(g));
  SBUF.sb_append("\np ");
  sb_append_id(sb, p->i, map);
  for (q = p->next; q != NULL; q = q->next->next->next) {
    int sat_id  = q->next->i;
    SBUF.sb_append( "\np ");
    if (sat_id == 0)
      SBUF.sb_append(",xx");
    else
      sb_append_id(sb, sat_id, map);
    }
  return;
#endif

} 


void JustContainer::sb_tagged_write_just(String_buf sb, Just just, I3list map) {
  Just g = just;
  /* sb_append(sb, "["); */
  StrbufContainer SBUF;
  SBUF.set_string_buf(sb);
  while (g != NULL) {
    Just_type rule = g->type;
    SBUF.sb_append("i ");
    if (rule == Just_type::INPUT_JUST || rule == Just_type::GOAL_JUST)   SBUF.sb_append(jstring(g));
    else if (rule==Just_type::BINARY_RES_JUST ||  rule==Just_type::HYPER_RES_JUST ||   rule==Just_type::UR_RES_JUST) {
      sb_tagged_write_res_just(sb, g, map);
    }
    else if (rule == Just_type::DEMOD_JUST) {
      I3list p;
      SBUF.sb_append(jstring(g));
      for (p = g->u.demod; p; p = p->next) {
        SBUF.sb_append("\np ");
        SBUF.sb_append_int(p->i);
      }
    }
    else if (rule == Just_type::UNIT_DEL_JUST) {
      Ilist p = g->u.lst;
      int n = p->i;
      int id = p->next->i;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      if (n < 0) {
        SBUF.sb_append_char(itoc(-n));
        SBUF.sb_append("(flip),");
      }
      else {
        SBUF.sb_append_char(itoc(n));
        SBUF.sb_append(",");
      }
      sb_append_id(sb, id, map);
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::FACTOR_JUST) {
      Ilist p = g->u.lst;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      sb_append_id(sb, p->i, map);
      SBUF.sb_append(",");
      SBUF.sb_append_char(itoc(p->next->i));
      SBUF.sb_append(",");
      SBUF.sb_append_char(itoc(p->next->next->i));
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::XXRES_JUST) {
      Ilist p = g->u.lst;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      sb_append_id(sb, p->i, map);
      SBUF.sb_append(",");
      SBUF.sb_append_char(itoc(p->next->i));
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::EXPAND_DEF_JUST) {
      Ilist p = g->u.lst;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      sb_append_id(sb,p->i, map);
      SBUF.sb_append(",");
      sb_append_id(sb,p->next->i, map);
      SBUF.sb_append(")");
    }
    else if (rule == Just_type::BACK_DEMOD_JUST ||
	     rule == Just_type::BACK_UNIT_DEL_JUST ||
	     rule == Just_type::NEW_SYMBOL_JUST ||
	     rule == Just_type::COPY_JUST ||
	     rule == Just_type::DENY_JUST ||
	     rule == Just_type::CLAUSIFY_JUST ||
	     rule == Just_type::PROPOSITIONAL_JUST) {
      int id = g->u.id;
      SBUF.sb_append(jstring(g));
      SBUF.sb_append("\np ");
      sb_append_id(sb, id, map);
    }
    else if (rule == Just_type::FLIP_JUST ||
	     rule == Just_type::XX_JUST ||
	     rule == Just_type::EVAL_JUST ||
	     rule ==Just_type::MERGE_JUST) {
      /* int id = g->u.id; */

#if 1
      /* BV(2007-jul-10) */
      SBUF.sb_append("(flip)");
      /* break; */
#endif

    }
    else if (rule == Just_type::PARA_JUST || rule == Just_type::PARA_FX_JUST ||
	     rule == Just_type::PARA_IX_JUST || rule == Just_type::PARA_FX_IX_JUST) {
      Parajust p = g->u.para;

#if 1
      /* BV(2007-jul-10) */
      SBUF.sb_append("para");
      SBUF.sb_append("\np ");
      sb_append_id(sb, p->from_id, map);
      SBUF.sb_append("\np ");
      sb_append_id(sb, p->into_id, map);
      /* break; */
#endif

    }
    else if (rule == Just_type::INSTANCE_JUST) {
      Plist p;

      SBUF.sb_append(jstring(g));
      SBUF.sb_append("(");
      SBUF.sb_append_int(g->u.instance->parent_id);
      SBUF.sb_append(",[");
      ParseContainer P;  
      for (p = g->u.instance->pairs; p; p = p->next) {
         P.sb_write_term(sb, (Term)p->v);
        if (p->next)
        SBUF.sb_append(",");
    }
      SBUF.sb_append("])");
    }
    else if (rule == Just_type::IVY_JUST) {
      SBUF.sb_append(jstring(g));
    }
    else {
      printf("\nunknown rule: %d\n", (int)rule);
      fatal::fatal_error("sb_write_just: unknown rule");
    }
    g = g->next;
    /* if (g) */
    /*   sb_append(sb, ","); */
    SBUF.sb_append("\n");
  }
  /* sb_append(sb, "]."); */
} 

//Carlos Sousa - 09-11-2018
