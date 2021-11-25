#ifndef _MYSTRING_H
#define _MYSTRING_H

#include <string>

using namespace std;

#define Float_format "%.3f"



class myString {

    private:  static string nullstring;
    
    public:
                static bool str_ident(const string &, const string &);
                static bool str_ident(char*, char*);
                
                static bool string_member(const string &, const string []  , int );
                
                static int witch_string_member(const char *, char *[], int);
                static bool initial_substring(const string &, const string &);
                static bool substring(const string &, const string &);
                static void reverse_chars(string &, int , int );
                static int natural_string(char *);
                static int natural_string(const string &);
                static bool str_to_int(char *, int *);
                static bool str_to_int(const string &, int *);
                static bool str_to_int(const string &, int &);
                static int char_occurrences(const string &, char);
          
                static const string &int_to_str(int, const string &, int);
                static bool str_to_double(const string &, double &);
                static bool str_to_double(const string &, double *);
                static const string &double_to_str(double, int);
                static bool string_of_repeated(char , const string &);
               
                static char* string_to_new_char_pointer(const string &);
                static const string & null_string(void);

                
              
};

#endif
