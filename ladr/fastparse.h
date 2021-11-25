#ifndef TP_FASTPARSE_H
#define TP_FASTPARSE_H

#include "topform.h"


#define MAX_LINE 1000

class Fastparse {

					private:
							static int Arity[256];  
							static int Symnum[256]; 
							static int Pos;
							
							static Term fast_parse(string);
							static void fast_fwrite_term(ostream &, Term);
                          
							
					
					public:
							static void fast_set_symbol(char, int);
							static void fast_set_defaults(void);
							static Term fast_read_term(ifstream &, ofstream&);
							static void fast_fwrite_term_nl(ostream &, Term);
							static Topform fast_read_clause(istream &, ostream &);
							static void fast_fwrite_clause(ostream &, Topform);
                            static Term fast_read_term(istream &, ostream &);
				
					

};




#endif

