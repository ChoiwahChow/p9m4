#include "fatal.h"
#include <iostream>


int fatal::Fatal_exit_code = 1;
bool fatal::debug=false;



void fatal::bell(ostream &o) {
   o<<'\a'<<endl;
}

int fatal::get_fatal_exit_code() {
    return Fatal_exit_code;
}

void fatal::set_fatal_exit_code(int exit_code) {
    Fatal_exit_code=exit_code;
}


void fatal::set_debug(void) {
    debug=true;
}

void fatal::reset_debug(void) {
    debug=false;
}

void fatal::fatal_error(string message) {
    cout<<endl<<"Fatal error: "<<message<<endl<<endl;
    cerr<<endl<<"Fatal error: "<<message<<endl<<endl;
    exit(Fatal_exit_code);
}
