#ifndef _PROVERS_H
#define _PROVERS_H


#include "./ladr/top_input.h"
#include "search_structures.h"

static char Help_string[] = 
"\nUsage: prover9 [-h] [-x] [-p] [-t <n>] [-f <files>]\n"
"\n"
"  -h         Help.  Also see http://www.cs.unm.edu/~mccune/prover9/\n"
"  -x         set(auto2).  (enhanced auto mode)\n"
"  -p         Fully parenthesize output.\n"
"  -t n       assign(max_seconds, n).  (overrides ordinary input)\n"
"  -f files   Take input from files instead of from standard input.\n"
"\n";

struct arg_options {
                                bool parenthesize_output;
                                bool auto2;
                                int  max_seconds;
                                bool files;
                                bool strings;
};
                                


class Targ_options {

                        private: 
                                arg_options ag;
                        public:
                                Targ_options();
                                ~Targ_options();
                                void get_command_line_args(int, char *[]);
                                struct arg_options get_arg_options();
                        
};


class Provers {

    
public:
                
    
                Prover_input std_prover_int_and_input(int , char *[], bool , bool, unknown_actions);
                static void prover_sig_handler(int);
                void process_command_line_args_1(struct arg_options command_opt, Prover_options prover_opt);
                void process_command_line_args_2(struct arg_options command_opt, Prover_options prover_opt);
                static void max_megs_exit(void);
                
    
};



#endif
