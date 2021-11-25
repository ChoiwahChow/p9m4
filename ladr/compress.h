#ifndef TP_COMPRESS_H
#define TP_COMPRESS_H

#include "parautil.h"
#include "strbuf.h"

class Compress {
					
					private:
							static void compress_term_recurse(String_buf, Term);
					public:
							static Term uncompress_term(string, int *);
							static char * compress_term(Term);
							static void compress_clause(Topform);
							static void uncompress_clause(Topform);
							static void uncompress_clauses(Plist);
};



#endif
