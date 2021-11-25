#ifndef TP_IOUTIL_H
#define TP_IOUTIL_H

#include "parse.h"
#include "fastparse.h"
#include "ivy.h"
#include "clausify.h"


enum class Clause_print_format{ CL_FORM_STD,
								CL_FORM_BARE,
								CL_FORM_PARENTS,
								CL_FORM_XML,
								CL_FORM_TAGGED,
								CL_FORM_IVY
								};  /* clause print format */

class Ioutil {
	
private:
               
    
				public:
                    
                    
                    
                    
                        static void print_flagged_hints(ostream &, Clist, int);
						static  void fwrite_formula(ostream &, Formula);
						static Topform read_clause(istream &, ostream &);
						static Topform parse_clause_from_string(string);
						static bool end_of_list_clause(Topform);
						static Clist read_clause_clist(istream &, ostream &, string, bool);
						static Plist read_clause_list(istream &, ostream &, bool);
						static void sb_write_clause_jmap(String_buf,Topform, int, I3list);
						static void sb_write_clause(String_buf, Topform, int);
						static void sb_xml_write_clause_jmap(String_buf, Topform, I3list);
						static void sb_tagged_write_clause_jmap(String_buf, Topform, I3list);
						static void fwrite_clause_jmap(ostream &,Topform, int, I3list);
						static void fwrite_clause(ostream &, Topform , int);
						static void f_clause(Topform);
						static void fwrite_clause_clist(ostream &, Clist, int);
						static void fwrite_demod_clist(ostream &, Clist, int);
						static void fwrite_clause_list(ostream &, Plist , string, int);
						static void f_clauses(Plist);
						static Formula read_formula(istream &, ostream &);
						static bool end_of_list_formula(Formula f);
						static Plist read_formula_list(istream &, ostream &);
						static void fwrite_formula_list(ostream &, Plist, string name);
						static void zap_formula_list(Plist);
						static bool end_of_list_term(Term);
						static bool end_of_commands_term(Term);
						static Plist read_term_list(istream &, ostream &);
                        static Plist read_term_list(String_buf, ostream &, int &);
						static void fwrite_term_list(ostream &, Plist, string);
						static Term term_reader(bool);
						static void term_writer(Term t, bool);
						
						//static Term term_reader(bool);
						static Topform clause_reader(bool);
						static void clause_writer(Topform, bool);
						static Topform term_to_topform2(Term);
						static Topform read_clause_or_formula(istream &, ostream &);
						static Plist read_clause_or_formula_list(istream &, ostream &);
                        
};
								




#endif
