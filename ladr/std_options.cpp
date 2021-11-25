#include "ladrvglobais.h"
#include "std_options.h"





GlobalStdOptions::GlobalStdOptions() {
    Prolog_style_variables = -1;       /* delayed effect */
    Ignore_option_dependencies = -1;   /* immediate effect */
    Clocks = -1; 
}


GlobalStdOptions::~GlobalStdOptions() {

    
}


void GlobalStdOptions::init_standard_options(void) {
  
  /* Flags */
  Prolog_style_variables     = LADR_GLOBAL_OPTIONS.init_flag("prolog_style_variables",      false);
  Ignore_option_dependencies = LADR_GLOBAL_OPTIONS.init_flag("ignore_option_dependencies",  false);
  Clocks                     = LADR_GLOBAL_OPTIONS.init_flag("clocks",                      false);

}

void GlobalStdOptions::process_standard_options(void) {
  SymbolContainer S;
  if (LADR_GLOBAL_OPTIONS.flag(Clocks))  myClock::enable_clocks();
  else myClock::disable_clocks();

  if (LADR_GLOBAL_OPTIONS.flag(Prolog_style_variables))   S.set_variable_style(Variable_Style::PROLOG_STYLE);
  else   S.set_variable_style(Variable_Style::STANDARD_STYLE);

  /* Flag gnore_option_dependencies is handled internally by
     the options package.  It takes effect immediately.
   */
}


int GlobalStdOptions::clocks_id(void) {
  return Clocks;
} 

int GlobalStdOptions::prolog_style_variables_id(void) {
  return Prolog_style_variables;
}
