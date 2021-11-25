#ifndef TP_STD_OPTIONS_H
#define TP_STD_OPTIONS_H

#include "options.h"
#include "symbols.h"
#include "clock.h"

class GlobalStdOptions {
					
					private:
							int Prolog_style_variables;      /* delayed effect */
							int Ignore_option_dependencies;   /* immediate effect */
							int Clocks;
					
					public:
                        
                            GlobalStdOptions();
                            ~GlobalStdOptions();
                            
                            void init_standard_options(void);
                            void process_standard_options(void);
                            int clocks_id(void);
                            int prolog_style_variables_id(void);
                            
                            
                            friend class LadrVGlobais;
};


#endif
