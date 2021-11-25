#ifndef _BANNER_H
#define _BANNER_H

#include <iostream>
#include <string>
#include "clock.h"

//public functions

using namespace std;



class banner {

                  public:  
                      
                            static void print_banner(int , char *[], string, string, string , const string , bool);
                            static void print_separator(ostream &, string , bool );
                           
};


#endif
