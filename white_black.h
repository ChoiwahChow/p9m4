#ifndef TP_WHITE_BLACK_H
#define TP_WHITE_BLACK_H

#include <limits>
#include "search_structures.h"

#define DBL_LARGE   std::numeric_limits<double>::max()

class WhiteBlack {

	private:
				static Plist White_rules;
				static Plist Black_rules;
				static bool  Rule_needs_semantics;
				
				static Term new_rule_int(string, OrderType, int);
				static Term new_rule_double(string, OrderType order, double);


	public:
				static void init_white_black(Plist, Plist);
				static Plist delete_rules_from_options(Prover_options);
				static bool black_tests(Topform);
				static bool white_tests(Topform);

};


#endif
