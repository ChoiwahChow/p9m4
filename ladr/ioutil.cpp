

#include "clauseid.h"
#include "ioutil.h"
#include "mystring.h"
#include "fatal.h"
#include "mystring.h"



    

void Ioutil::print_flagged_hints(ostream &fp, Clist lst, int format) {
  Clist_pos p;
   
  if (lst->name == NULL || myString::str_ident(*lst->name, ""))
    fp<<endl<<"formulas(anonymous)."<<endl;
  else
    fp<<endl<<"formulas("<<*lst->name<<")."<<endl;
  for (p = lst->first; p != NULL; p = p->next)
    // if (!p->c->used)
    //    fwrite_clause(fp, p->c, format);
    if (p->c->used)
    {
       // p->c->id = -p->c->id;
       fwrite_clause(fp, p->c, format);
    }
  fp<<"end_of_list."<<endl;  
} 

void Ioutil::fwrite_formula(ostream &fp, Formula f){
  FormulaContainer F;
  ParseContainer P;
  TermContainer T;
  Term t = F.formula_to_term(f);
  if (t == NULL)
    fatal::fatal_error("fwrite_formula, formula_to_term returns NULL.");
  P.fwrite_term_nl(fp, t);
 
  T.zap_term(t);
} 

Topform Ioutil::read_clause(istream &fin, ostream &fout) {
  TermContainer T;	
  TopformContainer TF;
  ParseContainer P;
  Term t = P.read_term(fin, fout);
  if (t == NULL)   return NULL;
  else {
    Topform c = TF.term_to_clause(t);
    T.zap_term(t);
    TF.clause_set_variables(c, MAX_VARS);  /* fatal if too many vars */
    return c;
  }
}


Topform Ioutil::parse_clause_from_string(string s) {
  ParseContainer P;
  TopformContainer TF;
  TermContainer T;
  Term t = P.parse_term_from_string(s);
  Topform c = TF.term_to_clause(t);
  T.zap_term(t);
  TF.clause_set_variables(c, MAX_VARS);  /* fatal if too many vars */
  return c;
}


bool Ioutil:: end_of_list_clause(Topform c) {
  
  SymbolContainer S;
  if (c == NULL)   return false;
  else if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) != 1)   return false;
  else {
		Term a = c->literals->atom;
		if (!CONSTANT(a))   return false;
    else
      return S.is_symbol(SYMNUM(a), "end_of_list", 0);
  }	
}


Clist Ioutil::read_clause_clist(istream &fin, ostream &fout, string name, bool assign_id) {
  ClistContainer CL;
  ClauseidContainer CI;
  TopformContainer TF;
  JustContainer J;

  Clist lst = CL.clist_init(name);
  Topform c;

  c = read_clause(fin, fout);
  while (c != NULL && !end_of_list_clause(c)) {
    if (assign_id)  CI.assign_clause_id(c);
    c->justification = J.input_just();
    CL.clist_append(c, lst);
    c = read_clause(fin, fout);
  }
  if (c != NULL)
    TF.zap_topform(c);  /* end_of_list_clause */
  return lst;
}


Plist Ioutil::read_clause_list(istream &fin, ostream &fout, bool assign_id) {
  ClistContainer CL;
  Clist a = read_clause_clist(fin, fout, myString::null_string(), assign_id);
  return CL.move_clist_to_plist(a);
}

void Ioutil::sb_write_clause_jmap(String_buf sb, Topform c, int format, I3list map){
  TopformContainer TF;
  StrbufContainer SB;
  ParseContainer P;
  JustContainer J;
  TermContainer T;
  SB.set_string_buf(sb);
  Term t;
 
  
  
  if (c->compressed)  t = NULL;
  else   t = TF.topform_to_term(c);
  
  if (format == (int) Clause_print_format::CL_FORM_BARE) {
    if (t == NULL)    SB.sb_append("clause_is_compressed");
    else  P.sb_write_term(SB.get_string_buf(),t);
    SB.sb_append(".");
  }
  else {
    if (c->id != 0) {
      J.sb_append_id(SB.get_string_buf(),c->id, map);
      SB.sb_append(" ");
    }

    if (t == NULL)
      SB.sb_append("clause_is_compressed");
    else
      P.sb_write_term(SB.get_string_buf(), t);
    SB.sb_append(".  ");
    if (format == (int)Clause_print_format::CL_FORM_STD)
      J.sb_write_just(SB.get_string_buf(),c->justification, map);
    else {
      /* CL_FORM_PARENTS */
      Ilist parents = J.get_parents(c->justification, true);
      Ilist p;
      SB.sb_append("[");
      for (p = parents; p; p = p->next) {
		J.sb_append_id(SB.get_string_buf(), p->i, map);
		if (p->next) SB.sb_append(",");
      }
      SB.sb_append("].");
    }
  }
  SB.sb_append("\n");
  if (t)    T.zap_term(t);
}


void Ioutil::sb_write_clause(String_buf sb, Topform c, int format) {
  sb_write_clause_jmap(sb, c, format, NULL);
}



void sb_xml_write_clause_jmap(String_buf sb, Topform c, I3list map) {
  StrbufContainer SB;
  SymbolContainer S;

  ParseContainer P;
  TermContainer T;
  AttributeContainer A;
  JustContainer J;
  SB.set_string_buf(sb);
  
  SB.sb_append("\n  <clause id=\"");
  J.sb_append_id(SB.get_string_buf(), c->id, map);
  SB.sb_append("\"");
  if (c->justification && c->justification->type == Just_type::GOAL_JUST)
    SB.sb_append(" type=\"goal\"");
  else if (c->justification && c->justification->type == Just_type::DENY_JUST)
    SB.sb_append(" type=\"deny\"");
  else if (c->justification && c->justification->type == Just_type::INPUT_JUST)
    SB.sb_append(" type=\"assumption\"");
  else if (c->justification && c->justification->type == Just_type::CLAUSIFY_JUST)
    SB.sb_append(" type=\"clausify\"");
  else if (c->justification && c->justification->type == Just_type::EXPAND_DEF_JUST)
    SB.sb_append(" type=\"expand_def\"");
  SB.sb_append(">\n");
  if (c->compressed)
    SB.sb_append("    <literal>clause_is_compressed</literal>\n");
  else {
    Literals lit;
    Term atts;
    if (c->literals == NULL) {
      SB.sb_append("    <literal><![CDATA[\n      ");
      SB.sb_append(S.false_sym());
      SB.sb_append("\n    ]]></literal>\n");
    }
    else {
      for (lit = c->literals; lit; lit = lit->next) {
		Term t = LADRV_GLOBAIS_INST.Lit.literal_to_term(lit);
		SB.sb_append("    <literal><![CDATA[\n      ");
		P.sb_write_term(SB.get_string_buf(), t);
		SB.sb_append("\n    ]]></literal>\n");
		T.zap_term(t);
      }
    }
    atts = A.attributes_to_term(c->attributes, S.attrib_sym());
    if (atts) {
      Term t = atts;
      while (T.is_term(t, S.attrib_sym(), 2)) {
	SB.sb_append("    <attribute><![CDATA[\n      ");
	P.sb_write_term(SB.get_string_buf(), ARG(t,0));
	SB.sb_append("\n    ]]></attribute>\n");
	t = ARG(t,1);
      }
      SB.sb_append("    <attribute><![CDATA[\n      ");
      P.sb_write_term(sb, t);
      SB.sb_append("\n    ]]></attribute>\n");
      T.zap_term(atts);
    }
  }
  J.sb_xml_write_just(SB.get_string_buf(), c->justification, map);
  SB.sb_append("  </clause>\n");
}



void Ioutil::sb_xml_write_clause_jmap(String_buf sb, Topform c, I3list map) {
  StrbufContainer SB;
  ParseContainer P;
  SymbolContainer S;
  TermContainer T;

  AttributeContainer AT;
  JustContainer J;
  
  SB.set_string_buf(sb);
  SB.sb_append("\n  <clause id=\"");
  J.sb_append_id(SB.get_string_buf(), c->id, map);
  SB.sb_append("\"");
  if (c->justification && c->justification->type == Just_type::GOAL_JUST)               SB.sb_append(" type=\"goal\"");
  else if (c->justification && c->justification->type == Just_type::DENY_JUST)          SB.sb_append(" type=\"deny\"");
  else if (c->justification && c->justification->type == Just_type::INPUT_JUST)         SB.sb_append(" type=\"assumption\"");
  else if (c->justification && c->justification->type == Just_type::CLAUSIFY_JUST)      SB.sb_append(" type=\"clausify\"");
  else if (c->justification && c->justification->type == Just_type::EXPAND_DEF_JUST)    SB.sb_append(" type=\"expand_def\"");
  SB.sb_append(">\n");
  
  if (c->compressed)
    SB.sb_append("    <literal>clause_is_compressed</literal>\n");
  else {
    Literals lit;
    Term atts;
    if (c->literals == NULL) {
      SB.sb_append("    <literal><![CDATA[\n      ");
      SB.sb_append(S.false_sym());
      SB.sb_append("\n    ]]></literal>\n");
    }
    else {
      for (lit = c->literals; lit; lit = lit->next) {
            Term t = LADRV_GLOBAIS_INST.Lit.literal_to_term(lit);
            SB.sb_append("    <literal><![CDATA[\n      ");
            P.sb_write_term(SB.get_string_buf(), t);
            SB.sb_append("\n    ]]></literal>\n");
            T.zap_term(t);
      }
    }
    atts = AT.attributes_to_term(c->attributes, S.attrib_sym());
    if (atts) {
      Term t = atts;
      while (T.is_term(t, S.attrib_sym(), 2)) {
        SB.sb_append("    <attribute><![CDATA[\n      ");
        P.sb_write_term(SB.get_string_buf(), ARG(t,0));
        SB.sb_append("\n    ]]></attribute>\n");
        t = ARG(t,1);
      }
      SB.sb_append("    <attribute><![CDATA[\n      ");
      P.sb_write_term(SB.get_string_buf(), t);
      SB.sb_append("\n    ]]></attribute>\n");
      T.zap_term(atts);
    }
  }
  J.sb_xml_write_just(sb, c->justification, map);
  
  SB.sb_append("  </clause>\n");
} 


void Ioutil::sb_tagged_write_clause_jmap(String_buf sb, Topform c, I3list map) {
 TopformContainer TF;
 StrbufContainer SB;
 ParseContainer P;
 TermContainer T;
 JustContainer J;
 SB.set_string_buf(sb);
 Term t;
  if (c->compressed)
    t = NULL;
  else
    t = TF.topform_to_term(c);
  
  if (c->id != 0) {

    /* BV(2007-jul-13) */
    SB.sb_append("c ");
    
    J.sb_append_id(SB.get_string_buf(), c->id, map);
    SB.sb_append("  ");
  }

  if (t == NULL)
    SB.sb_append("clause_is_compressed");
  else
    P.sb_write_term(SB.get_string_buf(), t);

  SB.sb_append("\n");

  J.sb_tagged_write_just(SB.get_string_buf(), c->justification, map);
  /* sb_append(sb, " *** call to sb_tagged_write_just ***\n"); */
  SB.sb_append("e\n");

  if (t)
    T.zap_term(t);
}

void Ioutil::fwrite_clause_jmap(ostream &fp, Topform c, int format, I3list map) {
  if (c == NULL)
    fp<<"fwrite_clause_jmap: NULL clause"<<endl;
  else {
    StrbufContainer SB;
	SB.new_string_buf("");
	if (format ==(int) Clause_print_format::CL_FORM_XML)  sb_xml_write_clause_jmap(SB.get_string_buf(), c, map);
    else if (format == (int) Clause_print_format::CL_FORM_IVY)  Ivy::sb_ivy_write_clause_jmap(SB.get_string_buf(), c, map);
    else if (format == (int) Clause_print_format::CL_FORM_TAGGED) sb_tagged_write_clause_jmap(SB.get_string_buf(), c, map);
    else sb_write_clause_jmap(SB.get_string_buf(), c, format, map);
    SB.fprint_sb(fp);    
    SB.zap_string_buf();
  }
  fp<<flush;
} 


void Ioutil::fwrite_clause(ostream &fp, Topform c, int format) {
  fwrite_clause_jmap(fp, c, format, NULL);
}


void Ioutil::f_clause(Topform c) {
 fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_STD);
}

void Ioutil::fwrite_clause_clist(ostream &fp, Clist lst, int format) {
  Clist_pos p;
    
  if (lst->name == NULL || myString::str_ident(*lst->name, "")   )
    fp<<"\nformulas(anonymous)."<<endl;

  else
    fp<<endl<<"formulas("<<*lst->name<<")."<<endl;
	  
  for (p = lst->first; p != NULL; p = p->next) 
    fwrite_clause(fp, p->c, format);
  fp<<"end_of_list."<<endl;
  fp<<flush;
} 


void Ioutil::fwrite_demod_clist(ostream &fp, Clist lst, int format) {
  TopformContainer TF;
  
  Parautil Pu;
  Clist_pos p;
    
  if (lst->name == NULL || myString::str_ident(*lst->name, ""))
    fp<<endl<<"formulas(anonymous)."<<endl;
	
  else
    fp<<endl<<"formulas("<<*lst->name<<")."<<endl;
	
	  
  for (p = lst->first; p != NULL; p = p->next) {
    Topform c = p->c;
    fwrite_clause(fp, c, format);
    if (LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals)&& LADRV_GLOBAIS_INST.Lit.pos_eq(c->literals) &&	!Pu.oriented_eq(c->literals->atom)) {
      fp<<"        %% (lex-dep)"<<endl;
    }
  }
  fp<<"end_of_list."<<endl<<flush;
  
}




void Ioutil::fwrite_clause_list(ostream &fp, Plist lst, string name, int format) {
  Plist p;
  if (name == myString::null_string() || myString::str_ident(name, ""))
    fp<<endl<<"formulas(anonymous)."<<endl;
  else
	fp<<endl<<"formulas("<<name<<")"<<endl;
  for (p = lst; p != NULL; p = p->next)
    fwrite_clause(fp, (Topform) p->v, format);
  fp<<"end_of_list."<<endl<<flush;
} 

void Ioutil::f_clauses(Plist p) {
	 fwrite_clause_list(cout, p, "", (int) Clause_print_format::CL_FORM_STD);
}


Formula Ioutil::read_formula(istream &fin, ostream &fout) {
  ParseContainer P;
  FormulaContainer F;
  TermContainer T;
  Term t = P.read_term(fin, fout);
  if (t == NULL)   return NULL;
  else {
    Formula f = F.term_to_formula(t);
    T.zap_term(t);
    return f;
  }
}


bool Ioutil::end_of_list_formula(Formula f) {
  SymbolContainer S;
  if (f == NULL)    return false;
  else if (f->type != Ftype::ATOM_FORM)    return false;
  else {
    Term a = f->atom;
    if (!CONSTANT(a))
      return false;
    else
      return S.is_symbol(SYMNUM(a), "end_of_list", 0);
 }
}
  
Plist Ioutil::read_formula_list(istream &fin, ostream &fout) {
  Plist p = NULL;
  PlistContainer P;
  FormulaContainer F;
  Formula f;

  f = read_formula(fin, fout);
  while (f != NULL && !end_of_list_formula(f)) {
    p = P.plist_prepend(f);
    f = read_formula(fin, fout);
  }
  if (f != NULL)    F.zap_formula(f);
  p = P.reverse_plist();
  return p;
}  
  
  
void Ioutil::fwrite_formula_list(ostream &fp, Plist lst, string name)
{
  Plist p;

  if (name == myString::null_string() || myString::str_ident(name, "") )
    fp<<endl<<"formulas(anonymous)."<<endl;
	
  else
    fp<<endl<<"formulas("<<name<<")."<<endl;
  for (p = lst; p != NULL; p = p->next)
    fwrite_formula(fp, (Formula) p->v);
  fp<<"end_of_list."<<endl;
}   

void Ioutil::zap_formula_list(Plist lst){
  FormulaContainer F;
  PlistContainer P;
  Plist p = lst;
  while (p != NULL) {
    Plist p2 = p;
    p = p->next;
    F.zap_formula((Formula)p2->v);
	P.free_plist(p2);
  }
}


bool Ioutil::end_of_list_term(Term t) {
  SymbolContainer S;
  if (t == NULL)   return false;
  else if (!CONSTANT(t)) return false;
  else return S.is_symbol(SYMNUM(t), "end_of_list", 0);
}

bool Ioutil::end_of_commands_term(Term t) {
  SymbolContainer S;
  if (t == NULL)   return false;
  else if (!CONSTANT(t))    return false;
  else
    return S.is_symbol(SYMNUM(t), "end_of_commands", 0);
}  /* end_of_commands_term */



Plist Ioutil::read_term_list(String_buf sb, ostream &fout,int &pos) {
  ParseContainer P;
  TermContainer T;
  Plist p = NULL;
  PlistContainer PL;
  Term t;
  //depois de uma lista de fórmulas declarada toca a ler os termos até aparecer o end_of _lits
  t = P.read_term_from_string_buf(sb, fout,pos); //forra a bomba
  
  while ( (t != NULL) && (!end_of_list_term(t)) ) {
    p = PL.plist_prepend((void *) t );
    t = P.read_term_from_string_buf(sb, fout,pos);
  }

  if (t != NULL)  
    T.zap_term(t);  //limpa o último que pode ser lixo
      
  p = PL.reverse_plist();
  return p;
    
}


Plist Ioutil::read_term_list(istream &fin, ostream &fout) {
  ParseContainer P;
  TermContainer T;
  Plist p = NULL;
  PlistContainer PL;
  Term t;
  //depois de uma lista de fórmulas declarada toca a ler os termos até aparecer o end_of _lits
  t = P.read_term(fin, fout); //forra a bomba
  
  while ( (t != NULL) && (!end_of_list_term(t)) ) {
    p = PL.plist_prepend((void *) t );
    t = P.read_term(fin, fout);
  }

  if (t != NULL)  
    T.zap_term(t);  //limpa o último que pode ser lixo
      
  p = PL.reverse_plist();
  return p;
} 

void Ioutil::fwrite_term_list(ostream &fp, Plist lst, string name){
  ParseContainer P;
  Plist p;

  if (name == myString::null_string() || myString::str_ident(name, ""))
    fp<<endl<<"list(anonymous)."<<endl;
	
  else
    fp<<endl<<"list("<<name<<")"<<endl;
	

  for (p = lst; p != NULL; p = p->next) {
    P.fwrite_term(fp,(Term) p->v);
    fp<<"."<<endl;
	
  }
  fp<<"end_of_list."<<endl<<flush;
} 


Term Ioutil::term_reader(bool fast) {
  ParseContainer P;
  if (fast)    return Fastparse::fast_read_term(cin, cerr);
  else    return P.read_term(cin, cerr);
}


void Ioutil::term_writer(Term t, bool fast)
{
  ParseContainer P;
  if (fast)
    Fastparse::fast_fwrite_term_nl(cout, t);
  else
    P.fwrite_term_nl(cout, t);
  cout<<flush;
}  /* term_writer */

/*************
 *
 *   clause_reader()
 *
 *************/

/* DOCUMENTATION
If your program has optional fast parsing/writing, you
can use this routine to save a few lines of code.
The flag "fast" says whether or not to use fast parse form.
A clause is read from stdin (NULL if there are none).  
Errors are fatal.
*/

/* PUBLIC */
Topform Ioutil::clause_reader(bool fast){
  if (fast)    return Fastparse::fast_read_clause(cin, cerr);
  else    return read_clause(cin, cerr);
}  /* clause_reader */

/* DOCUMENTATION
If your program has optional fast parsing/writing, you
can use this routine to save a few lines of code.
The flag "fast" says whether or not to use fast parse form.
The clause is written to stdout, with a period and newline.
*/

/* PUBLIC */
void Ioutil::clause_writer(Topform c, bool fast)
{
  if (fast)  Fastparse::fast_fwrite_clause(cout, c);
  else   fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_BARE);
  cout<<flush;
}  /* clause_writer */

/*************
 *
 *   term_to_topform2()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform Ioutil::term_to_topform2(Term t) {
  FormulaContainer F;
  Clausify C;
  AttributeContainer AT;
  TopformContainer TF;
  Formula f = F.term_to_formula(t);
  Topform tf;
  if (F.clausal_formula(f)) {
    tf = C.formula_to_clause(f);
    tf->attributes = AT.copy_attributes(f->attributes);
    F.zap_formula(f);
    TF.clause_set_variables(tf, MAX_VARS);
  }
  else {
    tf = TF.get_topform();
    tf->is_formula = true;
    tf->attributes = f->attributes;
    f->attributes = NULL;
    tf->formula = F.universal_closure(f);
  }
  return tf;
}  /* term_to_topform2 */

/*************
 *
 *   read_clause_or_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform Ioutil::read_clause_or_formula(istream &fin, ostream &fout)
{
  TermContainer T;
  ParseContainer P;
  Term t = P.read_term(fin, fout);
  if (t == NULL)
    return NULL;
  else {
    Topform tf = term_to_topform2(t);
    T.zap_term(t);
    return tf;
  }
}  /* read_clause_or_formula */

/*************
 *
 *   read_clause_or_formula_list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist Ioutil::read_clause_or_formula_list(istream &fin, ostream &fout)
{
  Plist lst = NULL;
  PlistContainer PL;
  TopformContainer TF;
  Topform tf = read_clause_or_formula(fin, fout);
  while (tf != NULL && !end_of_list_clause(tf)) {
    lst = PL.plist_prepend(tf);
    tf = read_clause_or_formula(fin, fout);
  }
  if (tf) TF.zap_topform(tf);
  return PL.reverse_plist();
} 



