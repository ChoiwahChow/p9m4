#ifndef _NON_PORT_H
#define _NON_PORT_H

#include <string>

using namespace std;

class nonport{

                public: 
                        static int get_bits(void);
                        static string username(void);
                        static string hostname(void);
                        static int my_process_id(void);
   
};


#endif
