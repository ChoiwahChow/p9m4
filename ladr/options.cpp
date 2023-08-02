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

#include "options.h"
#include "memory.h"
#include "mystring.h"
#include "ladrvglobais.h"


#include <stdarg.h>  /* for variable argument lists */
#include <iomanip>



void GlobalOptions::Free_Mem(void) {
    Flags.Free_Mem();
    Parms.Free_Mem();
}





GlobalOptions::GlobalOptions() {
    Option_updates = 0;  /* Flag, Parm, Stringparm */
    Ignore_dependencies = false;
    Optdep_gets=0;
    Optdep_frees=0;
}

GlobalOptions::~GlobalOptions() {
    
}


//----------------------------------------------------Global Options----------------------------------------------------------------------

Optdep GlobalOptions::get_optdep(void) {
  Optdep p = (Optdep) Memory::memCNew(sizeof(Toptdep));
  LADR_GLOBAL_OPTIONS.Optdep_gets++;
  return(p);
}  



void GlobalOptions::free_optdep(Optdep p) {
  
  if (p) {
    Memory::memFree(p, sizeof(Toptdep));
    LADR_GLOBAL_OPTIONS.Optdep_frees++;
    }
} 


void GlobalOptions::free_depedencies(Optdep p) {
    if(p->sval) 
        delete (p->sval); //teremos de limpar a string
    if(!p->next)   free_optdep(p);
    else free_depedencies(p->next);
}


void GlobalOptions::fprintf_options_mem(ostream &o, bool heading) {
    int f=11;
    int n;
    if (heading)
        o<<" type (bytes each)          gets        frees      in use      bytes"<<endl;
     n = sizeof(Toptdep);
     o<<" optdep ("<<n<<")"<<"        "<< //bytes each
     setw(f)<<LADR_GLOBAL_OPTIONS.Optdep_gets<< //gets
     setw(f)<<LADR_GLOBAL_OPTIONS.Optdep_frees<< //frees
     setw(f)<<LADR_GLOBAL_OPTIONS.Optdep_gets-LADR_GLOBAL_OPTIONS.Optdep_frees<< //in use
     setw(f)<<((LADR_GLOBAL_OPTIONS.Optdep_gets-LADR_GLOBAL_OPTIONS.Optdep_frees) / n ) /1024<<"k"<<//kbytes
     endl;
}


void GlobalOptions::p_options_mem(void){
  fprintf_options_mem(cout, true);
} 


void GlobalOptions::enable_option_dependencies(){
    Ignore_dependencies=false;
}

void GlobalOptions::disable_option_dependencies(void) {
    Ignore_dependencies=true;
}

bool GlobalOptions::option_dependencies_state(void) {
        return !Ignore_dependencies;
}



void GlobalOptions::fprintf_options(ostream &o) {
    int i;
    o<<endl<<"------------------------------------options------------------------------"<<endl;
  
    for(i=0; i<Flags.Next_flag; i++) {
            o<<  (Flags.Flags[i].val ? "set(":"clear(");
            o<<","<<Flags.Flags[i].name<<endl;
    }
    o<<endl;
    
    for (i = 0; i < Parms.Next_parm; i++) {  /* print parms */
        o<<"assign(";
        o<<Parms.Parms[i].name << " " <<Parms.Parms[i].val<<")"<<endl;
    }  
  
    o<<endl;
  
  
    for (i = 0; i < Floatparms.Next_floatparm; i++) {  /* print parms */
       o<<"assign(";
       o<<Floatparms.FloatParms[i].name << " " << Floatparms.FloatParms[i].val<<")"<<endl;
    }
 
    o<<endl;
  
    for (i = 0; i < Stringparms.Next_stringparm; i++) {  /* print stringparms */
        o<<"assign(";
        o<<Stringparms.StringParms[i].name <<" "<<Stringparms.StringParms[i].val;
        o<<")"<<endl;
    }
  
    o<<endl;
  
}


void GlobalOptions::p_options() {
        fprintf_options(cout);
}

void GlobalOptions::update_flag(ostream &o, int id, bool val, bool echo)
{
  if (id < 0 || id >= Flags.Next_flag) {
    o<<"update_flag: flag id "<<id<<" is out of range."<<endl;
    
    cerr<<"update_flag: flag id "<<id<<" is out of range."<<endl;
    
    fatal::fatal_error("update_flag, flag out of range");
  }
  else {
    Optdep p;
    Flags.Flags[id].val = val;
    Option_updates++;
    /* special case */
    if (myString::str_ident(Flags.Flags[id].name, "ignore_option_dependencies"))
      Ignore_dependencies = val;
    

    if (!Ignore_dependencies) {
      for (p = Flags.Flags[id].dependencies; p; p = p->next) {
	   if (p->flag_trigger == val) {
	    if (p->type == OptType::FLAGT) {
	     if (echo) 
            o<<(val ? "% set(" : "% clear(") <<  Flags.Flags[id].name << ") -> "<< (p->val ? "set(":"clear(") <<Flags.Flags[p->id].name<<")"<<endl;
	    update_flag(o, p->id, p->val, echo);
	   }
	   else if (p->type == OptType::PARMT) {
	     if (echo) 
            o<< (val? "% set(":"% clear(")<< Flags.Flags[id].name<<") -> assign("<<Parms.Parms[p->id].name<<","<<p->val<<")"<<endl;
         assign_parm(p->id, p->val, echo);
	  }
	  else if (p->type == OptType::FLOATPARMT) {
	    if (echo) 
          o<<(val ? "% set(":"% clear(")<<Flags.Flags[id].name<<") -> assign("<<Floatparms.FloatParms[p->id].name<<","<<p->dval<<")"<<endl;
	    assign_floatparm(p->id, p->dval, echo);
	  }
	  else {
	    /* assume it's a stringparm */
	    if (echo) 
           o<<(val ? "% set(":"% clear(")<<Flags.Flags[id].name<<") -> assign("<<Stringparms.StringParms[p->id].name<<","<<p->dval<<")"<<endl;
 	    assign_stringparm(p->id, *p->sval, echo);
	   }
	  }
     }
    }
  }
}  /* update_flag */




void GlobalOptions::assign_parm(int id, int val, bool echo)
{
  if (id < 0 || id >= Parms.Next_parm) {
    cout << "assign_parm: "<< id << " is out of range"<<endl;
    cerr << "assign_parm: "<< id << " is out of range"<<endl;
    fatal::fatal_error("assign_parm");
  }
  else if (val < Parms.Parms[id].min || val > Parms.Parms[id].max) {
    cout <<"assign_parm: parm "<< Parms.Parms[id].name <<" , value "<<val<< " is out of range [" << Parms.Parms[id].min<<".."<<Parms.Parms[id].max<<"]"<<endl;
    cerr <<"assign_parm: parm "<< Parms.Parms[id].name <<" , value "<<val<< " is out of range [" << Parms.Parms[id].min<<".."<<Parms.Parms[id].max<<"]"<<endl;      
    fatal::fatal_error("assign_parm");
  }
  else {
    Optdep p;
    Parms.Parms[id].val = val;
    Option_updates++;
    
    if (!Ignore_dependencies) {
      for (p = Parms.Parms[id].dependencies; p; p = p->next) {
        if (p->type == OptType::PARMT) {
        /* parm -> parm */
        int dval = p->multiply ? p->val * val : p->val;
        if (echo) 
            cout<< "assign(" << Parms.Parms[id].name<<","<<val<<") -> assign("<<Parms.Parms[p->id].name<<","<<dval<<")"<<endl;
        assign_parm(p->id, dval, echo);
       }
       else {
	  /* parm -> flag */
        if (echo) 
            cout<<"assign("<<Parms.Parms[id].name<<","<<val<<") -> "<< (p->val ? "set(":"clear(" )<<LADR_GLOBAL_OPTIONS.Flags.Flags[p->id].name<<")"<<endl;    
        update_flag(cout, p->id, p->val, echo);
       }
      }
    }
  }
}  /* assign_parm */




void GlobalOptions::assign_floatparm(int id, double val, bool echo) {
  if (id < 0 || id >= Floatparms.Next_floatparm) {
    cout << "assign_floatparm: "<< id << " is out of range";
    cerr << "assign_floatparm: "<< id << " is out of range";
    fatal::fatal_error("assign_floatparm");
  }
  
  else if (val < Floatparms.FloatParms[id].min || val > Floatparms.FloatParms[id].max) {
    cout <<"assign_floatparm: parm "<< Floatparms.FloatParms[id].name <<" , value "<<val<< " is out of range [" << Floatparms.FloatParms[id].min<<".."<<Floatparms.FloatParms[id].max<<"]"<<endl;
    cerr <<"assign_floatparm: parm "<< Floatparms.FloatParms[id].name <<" , value "<<val<< " is out of range [" << Floatparms.FloatParms[id].min<<".."<<Floatparms.FloatParms[id].max<<"]"<<endl; 
    fatal::fatal_error("assign_floatparm");
  }
  
  else {
    Floatparms.FloatParms[id].val = val;
    Option_updates++;
    /* Currently, nothing depends on floatparms. */
  }
}

void GlobalOptions::assign_stringparm(int id, string name , bool echo) {
   
  if (id < 0 || id >= Stringparms.Next_stringparm) {
      cout << "assign_stringparm: "<< id << " is out of range";
      cerr << "assign_stringparm: "<< id << " is out of range";
      fatal::fatal_error("assign_stringparm");
  }
  else {
        Tstringparm *sp = &(Stringparms.StringParms[id]);
        
        int i = 0;
        while (i < sp->n && !myString::str_ident(sp->range[i], name))
        i++;
        if (i < sp->n) {
                sp->val = sp->range[i];
                Option_updates++;
        }
        else {
              cout <<"range of values for stringparm "<<sp->name<<endl;
              for (i = 0; i < sp->n; i++) cout << sp->range[i]<<endl;
              fatal::fatal_error("assign_stringparm, value out of range");
        }
  }
    
}

void GlobalOptions::set_flag(int id, bool echo) {
    update_flag(cout, id, true, echo);
}

void GlobalOptions::clear_flag(int id, bool echo) {
    update_flag(cout, id, false, echo);
}

int GlobalOptions::option_updates(void)
{
  return Option_updates;
}                        


string GlobalOptions::parm_id_to_str(int id) {
    return Parms.Parms[id].name;
}

int GlobalOptions::parm_default(int id)  {
    return Parms.parm_default(id);
}

double GlobalOptions::floatparm(int id) {
    return Floatparms.floatparm(id);
}

bool GlobalOptions::stringparm(int id, string s) {
    return Stringparms.stringparm(id,s);
}
                                

//-----------------------------------------------------End Global Options----------------------------------------------------------------------


//-----------------------------------------------------Flags-----------------------------------------------------------------------------------

TFlags::TFlags() {

    Next_flag=0;
}

TFlags::~TFlags() {

}


void TFlags::Free_Mem(void) {

    for(int i=0; i<MAX_FLAGS;i++) 
      if (Flags[i].dependencies)  
        LADR_GLOBAL_OPTIONS.free_depedencies(Flags[i].dependencies);
    
}



bool TFlags::flag(int id) {
    return Flags[id].val;
}


bool GlobalOptions::flag(int id) {
    return Flags.Flags[id].val;
}

int TFlags::str_to_flag_id(string name) {
  int i;
  for (i = 0; i < Next_flag; i++)
    if (myString::str_ident(name, Flags[i].name))
      return i;
  return -1;
}


int GlobalOptions::str_to_flag_id(string name) {
    return Flags.str_to_flag_id(name);
}

string TFlags::flag_id_to_str(int id) {
    if (id < 0 || id >= Next_flag) fatal::fatal_error("flag_id_to_str, bad id");
    return Flags[id].name;   
}


string GlobalOptions::flag_id_to_str(int id) {
    return Flags.flag_id_to_str(id);
}

//Exemplo de chamada
//LADR_GLOBAL_OPTIONS.Stringparms.init_parm("carlos",2,string("123"), string("23"));

int TFlags::init_flag(string name, bool default_value) {
  int id = -1;
  if (Next_flag == MAX_FLAGS) fatal::fatal_error("init_flag, too many flags");
  else 
    if (false && str_to_flag_id(name) != -1) {
        string s;
        s="init_flag, flag " + name + " already exists";
        fatal::fatal_error(s);
  }
  else {
    id = Next_flag++;
    Flags[id].name = name;
    Flags[id].val = default_value;
    Flags[id].default_val = default_value;
    Flags[id].dependencies = NULL;
  }
  return id;
}  


//cria uma dependencia
void TFlags::flag_flag_dependency(int id, bool val, int dep_id, bool dep_val) {
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::FLAGT;
  dep->id = dep_id;
  dep->val = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
}


void GlobalOptions::flag_flag_dependency(int id, bool val, int dep_id, bool dep_val) {
    return Flags.flag_flag_dependency(id,val,dep_id, dep_val);
}




void TFlags::flag_flag_dep_default(int id, bool val, int dep_id){
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::FLAGT;
  dep->id = dep_id;
  dep->val = Flags[dep_id].default_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
}  
  
  
void GlobalOptions::flag_flag_dep_default(int id, bool val, int dep_id) {
Flags.flag_flag_dep_default(id,val,dep_id);
}

void TFlags::flag_parm_dependency(int id, bool val, int dep_id, int dep_val) {
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::PARMT;
  dep->id = dep_id;
  dep->val = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
} 


void GlobalOptions::flag_parm_dependency(int id, bool val, int dep_id, int dep_val) {
    Flags.flag_parm_dependency(id, val, dep_id, dep_val);
}




void TFlags::flag_floatparm_dependency(int id, bool val, int dep_id, double dep_val)
{
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::FLOATPARMT;
  dep->id = dep_id;
  dep->dval = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
} 


void GlobalOptions::flag_floatparm_dependency(int id, bool val, int dep_id, double dep_val) {
    Flags.flag_floatparm_dependency(id,val,dep_id,dep_val);
}

void TFlags::flag_parm_dep_default(int id, bool val, int dep_id, TParms &p) 
{
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::PARMT;
  dep->id = dep_id;
  dep->val =p.parm_default(dep_id);
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
}

void GlobalOptions::flag_parm_dep_default(int id, bool val, int dep_id, TParms &p) {
    Flags.flag_parm_dep_default(id,val,dep_id,p);
}


void TFlags::flag_floatparm_dep_default(int id, bool val, int dep_id, TFloatParms &fp) {
  Optdep dep =LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type =OptType::FLOATPARMT;
  dep->id = dep_id;
  dep->dval = fp.floatparm_default(dep_id);
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
}


void GlobalOptions::flag_floatparm_dep_default(int id, bool val, int dep_id, TFloatParms &fp) {
    Flags.flag_floatparm_dep_default(id,val,dep_id, fp);
}

void TFlags::flag_stringparm_dependency(int id, bool val, int dep_id, string dep_val)
{
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::STRINGPARMT;
  dep->id = dep_id;
  dep->sval = new string(dep_val);
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
} 

void GlobalOptions::flag_stringparm_dependency(int id, bool val, int dep_id, string dep_val) {
    Flags.flag_stringparm_dependency(id,val,dep_id,dep_val);
}




void TFlags::flag_stringparm_dep_default(int id,bool val, int dep_id, TStringParms& sp)
{
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::STRINGPARMT;
  dep->id = dep_id;
  dep->sval =new string(sp.stringparm1_default(dep_id));
  dep->flag_trigger = val;
  Flags[id].dependencies = Toptdep::append_dep(Flags[id].dependencies, dep);
}  


void GlobalOptions::flag_stringparm_dep_default(int id,bool val, int dep_id, TStringParms& sp) {
    Flags.flag_stringparm_dep_default(id, val,  dep_id, sp);
}


int TFlags::flag_default(int flag_id)
{
  return Flags[flag_id].default_val;
} 




//------------------------------------------------End--Flags-----------------------------------------------------------------------------------


//------------------------------------------------Parms----------------------------------------------------------------------------------------

TParms::TParms(){
    Next_parm=0;
  
}

TParms::~TParms(){
  
 
}


void TParms::Free_Mem(void) {
     for(int i=0; i<MAX_PARMS;i++) 
      if (Parms[i].dependencies)  
        LADR_GLOBAL_OPTIONS.free_depedencies(Parms[i].dependencies);
}

int TParms::init_parm(string name, int default_value,int min_value ,int max_value) {
 int id = -1;
  if (Next_parm == MAX_PARMS)
    fatal::fatal_error("init_parm: too many parms");
  else {
    id = Next_parm++;
    Parms[id].name = name;
    Parms[id].val = default_value;
    Parms[id].default_val = default_value;
    Parms[id].min = min_value;
    Parms[id].max = max_value;
  }
  return id;
}


int GlobalOptions::init_parm(string name, int default_value,int min_value ,int max_value) {
    return Parms.init_parm(name, default_value, min_value, max_value);
}


int TParms::parm(int id) {
    return Parms[id].val;
}




int GlobalOptions::parm(int id) {
    return Parms.parm(id);
}



bool TParms::at_parm_limit(int value, int parm_id) {
      int limit = Parms[parm_id].val;
      if (limit == -1)
            return false;  /* no limit */
      else
        return value >= limit;
}

bool TParms::over_parm_limit(int value, int parm_id)
{
  int limit = Parms[parm_id].val;
  if (limit == -1)
    return false;  /* no limit */
  else
    return value > limit;
} 

int TParms::str_to_parm_id(string name) {
  int i;
  for (i = 0; i < Next_parm; i++)
    if (myString::str_ident(name, Parms[i].name))
      return i;
  return -1;
}

int GlobalOptions::str_to_parm_id(string name) {
    return Parms.str_to_parm_id(name);
}

int GlobalOptions::str_to_floatparm_id(string name) {
    return Floatparms.str_to_floatparm_id(name);
}

string TParms::parm_id_to_str(int id) {
    if (id < 0 || id >= Next_parm) fatal::fatal_error("parm_id_to_str, bad id");
    return Parms[id].name;   
}





void TParms::parm_flag_dependency(int id, int dep_id, int dep_val){
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::FLAGT;
  dep->id = dep_id;
  dep->val = dep_val;
  Parms[id].dependencies = Toptdep::append_dep(Parms[id].dependencies, dep);
}

void TParms::parm_parm_dependency(int id, int dep_id, int dep_val, bool multiply)
{
  Optdep dep = LADR_GLOBAL_OPTIONS.get_optdep();
  dep->type = OptType::PARMT;
  dep->id = dep_id;
  dep->val = dep_val;
  dep->multiply = multiply;
  Parms[id].dependencies = Toptdep::append_dep(Parms[id].dependencies, dep);
} 



void GlobalOptions::parm_parm_dependency(int id, int dep_id, int dep_val, bool multiply) {
    Parms.parm_parm_dependency(id,dep_id,dep_val,multiply);
}


TParms & GlobalOptions::getParms() {
    return Parms;
}

TFloatParms & GlobalOptions::getFloatParms(void) {
        return Floatparms;
}

TStringParms & GlobalOptions::getStringParms(void) {
return Stringparms;
}

int TParms::parm_default(int parm_id)
{
  return Parms[parm_id].default_val;
} 

//--------------------------------------------End-Parms----------------------------------------------------------------------------------------




//------------------------------------------string parms---------------------------------------------------------------------------------------

int TStringParms::init_stringparm(string name, int n, va_list arg) {
    
    if(Next_stringparm==MAX_STRINGPARMS) {
        fatal::fatal_error("init_string parm: too many stringparms");
        return -1;   //To appease the compiler g++ 9.2.1, fatal_error above would have exited the program
    }
    else {
            int id=-1;
            
            id=Next_stringparm++;
            
            StringParms[id].range= new string[n];//cria um array de strings
            for (int i=0; i<n; i++) 
                StringParms[id].range[i]=va_arg(arg, const char*);
            
            StringParms[id].name=new string (name);
            StringParms[id].n=n;
            StringParms[id].val = StringParms[id].range[0];
            StringParms[id].default_val = StringParms[id].range[0];
            return id;
    }
}


//------------------------------------------string parms---------------------------------------------------------------------------------------

int TStringParms::init_stringparm_v2(const string& name, const vector<string>& arg) {
    
    if(Next_stringparm==MAX_STRINGPARMS) {
        fatal::fatal_error("init_string parm: too many stringparms");
        return -1;   //To appease the compiler g++ 9.2.1, fatal_error above would have exited the program
    }
    else {
            int id=-1;
            
            id=Next_stringparm++;
            
            int n = arg.size();
            StringParms[id].range= new string[n];//cria um array de strings
            for (int i=0; i<n; i++) 
                StringParms[id].range[i]=arg[i];
            
            StringParms[id].name=new string (name);
            StringParms[id].n=n;
            StringParms[id].val = StringParms[id].range[0];
            StringParms[id].default_val = StringParms[id].range[0];
            return id;
    }
}

int GlobalOptions::init_stringparm(string name, int n, ...) {
    int i;
    va_list args;
    va_start(args,n);
    i= Stringparms.init_stringparm(name,n,args);
    va_end(args);
    return i;
}

int GlobalOptions::init_stringparm_v2(const string& name, const vector<string>& args) {
    return Stringparms.init_stringparm_v2(name, args);
}

bool TStringParms::stringparm(int id, string s) {
    return myString::str_ident(StringParms[id].val,s);
}

string TStringParms::stringparm1(int id) {
  return StringParms[id].val;
} 



string GlobalOptions::stringparm1(int id) {
    return Stringparms.stringparm1(id);
}

TStringParms::TStringParms() {
    
    Next_stringparm=0;
}


int TStringParms::str_to_stringparm_id(string name) {
  int i;
  for (i = 0; i < Next_stringparm; i++)
    if (myString::str_ident(name, *StringParms[i].name))
      return i;
  return -1;
}


int GlobalOptions::str_to_stringparm_id(string name) {
    return Stringparms.str_to_stringparm_id(name);
}

string TStringParms::stringparm_id_to_str(int id) {
    
    if (id < 0 || id >= Next_stringparm) fatal::fatal_error("stringparm_id_to_str, bad id");
    return *StringParms[id].name;   
    
}



TStringParms::~TStringParms() {
  
    for(int id=0; id<MAX_STRINGPARMS;id++) {
        if (StringParms[id].range)
            delete [] StringParms[id].range;
        
        if(StringParms[id].name)
            delete StringParms[id].name;
    }
        
    
           
}

string TStringParms::stringparm1_default(int id)
{
  return StringParms[id].default_val;
} 

//--------------------------------------End-string parms---------------------------------------------------------------------------------------



//--------------------------------------Float Parms--------------------------------------------------------------------------------------------
int TFloatParms::init_floatparm(string name, double default_value, double min_value, double max_value) {
  int id = -1;
  if (Next_floatparm == MAX_FLOATPARMS)
    fatal::fatal_error("init_floatparm: too many parms");
  else {
    id = Next_floatparm++;
    FloatParms[id].name = name;
    FloatParms[id].val = default_value;
    FloatParms[id].default_val = default_value;
    FloatParms[id].min = min_value;
    FloatParms[id].max = max_value;
  }
  return id;
    
}


int GlobalOptions::init_floatparm(string name, double default_value, double min_value, double max_value) {
        return Floatparms.init_floatparm(name, default_value, min_value, max_value);
}

double TFloatParms::floatparm(int float_parm_id) {
    return FloatParms[float_parm_id].val;
}


int TFloatParms::str_to_floatparm_id(string name) {
  int i;
  for (i = 0; i < Next_floatparm; i++)
    if (myString::str_ident(name, FloatParms[i].name))
      return i;
  return -1;
}  /* str_to_floatparm_id */

string TFloatParms::floatparm_id_to_str(int id){
    if (id < 0 || id >= Next_floatparm) fatal::fatal_error("floatparm_id_to_str, bad id");
    return FloatParms[id].name;   
}



double TFloatParms::floatparm_default(int floatparm_id)
{
  return FloatParms[floatparm_id].default_val;
}



TFloatParms::TFloatParms() {
    Next_floatparm=0;
}

TFloatParms::~TFloatParms() {
   for(int i=0; i<MAX_FLOATPARMS;i++) 
      if (FloatParms[i].dependencies)  
        LADR_GLOBAL_OPTIONS.free_depedencies(FloatParms[i].dependencies);   
}


//----------------------------------End-Float Parms--------------------------------------------------------------------------------------------






//----------------------------------Toptdep-----------------------------------------------------------------------------------------------------
Optdep Toptdep::append_dep(Optdep d1, Optdep d2) {
 if(d1==NULL) return d2;
 else {
        d1->next=append_dep(d1->next,d2);
        return d1;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------


int GlobalOptions::init_flag(string s, bool b) {
    return Flags.init_flag(s,b);
}


bool GlobalOptions::over_parm_limit(int value, int parm_id) {
        return Parms.over_parm_limit(value, parm_id);
}

bool GlobalOptions::at_parm_limit(int value,int parm_id) {
    return Parms.at_parm_limit(value, parm_id);
}
