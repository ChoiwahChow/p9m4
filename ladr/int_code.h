#ifndef TP_INT_CODE_H
#define TP_INT_CODE_H

#include "just.h"
#include "ibuffer.h"
#include "ladrvglobais.h"


class IntCode {

					private:
							static void put_ilist_to_ibuf(Ibuffer, Ilist);
							static Ilist get_ilist_from_ibuf(Ibuffer);
						    static void put_i3list_to_ibuf(Ibuffer, I3list);
							static I3list get_i3list_from_ibuf(Ibuffer);
							static void put_term_to_ibuf(Ibuffer, Term);
							static Term get_term_from_ibuf(Ibuffer);
							static void put_just_to_ibuf(Ibuffer, Just);
							static Just get_just_from_ibuf(Ibuffer);


					public:
							static void put_clause_to_ibuf(Ibuffer ibuf, Topform c);
							static Topform get_clause_from_ibuf(Ibuffer ibuf);
							static void check_ibuf_clause(Topform c);

};


#endif

