#ifndef TP_RANDOM_H
#define TP_RANDOM_H

#include "topform.h"
#include "ladrvglobais.h"




class Random {
	
	private: 
						static int initialized;
						static int symnum[13];
			
	public:
				
		static Term random_term(int, int , int , int , int , int );
		static Term random_nonvariable_term(int, int , int , int , int ,int);
		static Term random_complex_term(int , int , int , int , int , int);
		static Ilist random_path(int, int);
		static void random_permutation(int *, int);
		static Topform random_clause(int, int , int , int , int ,int , int );
		static Term random_op_term(int);
};




#endif

