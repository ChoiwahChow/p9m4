
#include "banner.h"
#include "nonport.h"
#include "clock.h"
#include <iostream>

#include <ctype.h>  



/*
void banner::print_separator(ostream &fp, string str, bool initial_newline)
{
  int len = 80;  // total length of line 
  string first_half = "==============================";
  if (initial_newline) fp<<endl;
  fp<<first_half<<" "<<str;
  
  int n=len-(first_half.length() + str.length()+2);
  for (int i=0; i<n; i++) fp<<"=";
  fp<<endl;
}  
*/

void banner::print_separator(ostream &fp, string str, bool initial_newline)
{
   int len = 80;  /* total length of line */
   string first_half = "==============================";
   if (initial_newline) fp<<endl;
   fp<<first_half<<" "<<str<<" ";

   int n=len-(first_half.length() + str.length()+2);
   for (int i=0; i<n; i++) fp<<"=";
   fp<<endl;
}


void banner::print_banner(int argc, char *argv[], const string name, const string version, string date,const string in ,bool as_comments)
{
        
    string com="%";
    if (!as_comments) {
        com="";  
        print_separator(cout, name, false);
    }
    
   cout<<com<<name<<" ("<<nonport::get_bits()<<") version "<<version<<" "<<date<<endl;
   cout <<com<<in<<endl;
   cout<<com<<"Process "<<nonport::my_process_id()<<" was started by "<<nonport::username()<<" on "<<nonport::hostname()<<endl<<com<< myClock::get_date(); 

   cout<<com<<"The command was \""; 
   for (int i=0; i<argc; i++)
       cout<<argv[i]<<(i<argc-1 ? " " : "");
   cout<<"\""<<endl;
   
  if (!as_comments)
    print_separator(cout, "end of head", false);
  

  cout<<endl;
    
} 
