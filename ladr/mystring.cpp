#include "mystring.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fatal.h"

string myString::nullstring=string();

bool myString::str_ident(const string &s1, const string &s2) {
    return s1==s2;
}


bool myString::str_ident(char *s1, char *s2) {
    return (strcmp(s1,s2)==0);
}



int myString::witch_string_member(const char* s,char *ss[], int n) {
   int i;
   for (i = 1; i < n; i++)
    if (str_ident(ss[i], s))    return i;
  return -1;
}



bool myString::string_member(const string &s, const string ss[], int n) {
    for(int i=0;i<n;i++) 
        if(str_ident(s, ss[i])) return true;
    return false;
}


bool myString::initial_substring(const string &x, const string &y) {
    int i=0;
    int j=0;
    
    while(i<x.length() && j<y.length() && x[i]==y[j]) {
        i++; j++;
    }
    return (i==x.length());
}

bool myString::substring(const string &x, const string &y) {
    return y.find(x)!=string::npos;
}

void myString::reverse_chars(string & s, int start, int end) {
    if(start<end) {
        char c=s[start]; s[start]=s[end]; s[end]=c;
        reverse_chars(s, start+1, end-1);
    }
}


bool myString::str_to_int(char *s, int *ip) {
    char *end;
    *ip=strtol(s, &end, 10);
    if(*end!='\0') 
        return false;
    return true;
}



bool myString::str_to_int(const string &s, int *ip) {
    return str_to_int((char *) &(s).at(0) ,ip);
} 


bool myString::str_to_int(const string &s, int &i) {
    return str_to_int((char *) &(s.at(0)), &i);
}


int myString::natural_string(char *s){
        int i;
        if(!str_to_int(s, &i)) return -1;
        else if (i<0) return -1;
        else return i;
}


int myString::natural_string(const string &s) {
    return natural_string((char *)(&s.at(0)));
}

int myString::char_occurrences(const string &s, char c) {
    int n=0;
    for(int i=0; i<s.length();i++)
        if (s[i]==c) n++;
    return n;
}



const string &myString::int_to_str(int n, const string &s, int size) {
    static string aux;
    aux=to_string(n);
    if (aux.length()>size)
        fatal::fatal_error("int to string, string too small");
    return aux;
}

bool myString::str_to_double(const string &s, double *dp) {
   if(s[0]!='\"') return false;
   else if (s[1]=='\"') return false;
   else {
            char *aux=(char *)&(s.at(0));
            char *end;
            double f=strtod(aux+1, &end);
            *dp=f;
            return (*end=='\"');
    }
    
}

bool myString::str_to_double(const string &s, double &dp) {
   if(s[0]!='\"') return false;
   else if (s[1]=='\"') return false;
   else {
            char *aux=(char *)&(s.at(0));
            char *end;
            double f=strtod(aux+1, &end);
            dp=f;
            return (*end=='\"');
    }
}


const string &myString::double_to_str(double d, int size) {
    static string aux=std::to_string(d);
    aux="\""+ aux+ "\"";
    return aux;
}



bool myString::string_of_repeated(char c, const string &s) {
    for (int i=0; i<s.length(); i++) 
        if(s[i]!=c) return false;
    return true;    
}


char * myString::string_to_new_char_pointer(const string &str) {
  int n=str.length()+1; //por causa do nulo
  char *s=(char *) malloc(n);
  strcpy(s,str.c_str());
  return s;
}

const string & myString::null_string(void){
    return nullstring;
}
