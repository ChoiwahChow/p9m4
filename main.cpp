#include "./ladr/ladrvglobais.h"
#include "./ladr/ioutil.h"
#include "./ladr/banner.h"
#include "./ladr/fatal.h"
#include "provers.h"
#include "search.h"
#include "utilities.h"
#include "search.h"
#include "VERSION_DATE.h"

//g++ main.cpp 



#include <iostream>
#include <ext/stdio_filebuf.h>
#define PROVER_NAME     "Prover9"

using namespace std;





int main(int argc, char *argv[]) {

    
    Provers PRV;
    
    
    
    fatal::set_debug();
    
    LADR_GLOBAL_TOP_INPUT.set_program_name(PROVER_NAME);
    banner::print_banner(argc, argv, PROVER_NAME,PROGRAM_VERSION, PROGRAM_DATE,INSTITUTION, false);
    
   
    Search::input = PRV.std_prover_int_and_input(argc, argv, true, true, unknown_actions::KILL_UNKNOWN); //até aqui, inclusivé  tudo bem 02-01-2019
    Search::results = Search::search(Search::input); //free any residual memory
    
    if (Search::results->return_code == exitCodes::MAX_PROOFS_EXIT) {
        cout<<endl<<"THEOREM PROVED"<<endl;
        
    if (!LADR_GLOBAL_OPTIONS.flag(Search::input->options->quiet))
        cerr<<endl<<"THEOREM PROVED"<<endl;
        
    }
    else {
    // Note that we fail if we found less than max_proofs proofs.
       cout<<endl<<"SEARCH FAILED"<<endl;
       if (!LADR_GLOBAL_OPTIONS.flag(Search::input->options->quiet))
         cerr<<endl<<"SEARCH FAILED"<<endl;
    }
    
    Search::fprint_all_stats(cout,"all");
    Search::exit_with_message(cout,(int) Search::results->return_code);
    
    
    
    
}
