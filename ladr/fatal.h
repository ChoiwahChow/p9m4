/*  Copyright (C) 2006, 2007 William McCune

    This file is part of the LADR Deduction Library.

    The LADR Deduction Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License,
    version 2.

    The LADR Deduction Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the LADR Deduction Library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _FATAL_H
#define _FATAL_H


#include <iostream>

using namespace std;

class fatal {
                private:
                        static int Fatal_exit_code;
                        static bool debug;
                public:
                        static void bell(ostream &);

                        static int get_fatal_exit_code();

                        static void set_fatal_exit_code(int exit_code);

                        static void fatal_error(string);
                        
                        static void set_debug(void);
                        static void reset_debug(void);
    
    
};



#endif  /* conditional compilation of whole file */
