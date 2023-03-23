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




#ifndef TP_OPTIONS_H
#define TP_OPTIONS_H

#include "string.h"
#include "fatal.h"
#include <iostream>
#include <vector>

/* INTRODUCTION
There are 3 types of option:
<UL>
<LI>Flags: Boolean
<LI>Parms: Integer
<LI>Stringparm: String
</UL>
<P>
To introduce a new option, choose an integer ID that is
unique for the type of option, and call the appropriate
initialization routine.  Then you can change the value
of the option and check its value as you like.
<P>
The routine read_commands() (in the "commands" package)
will parse the user's set, clear, and assign commands,
making the appropriate changes to the values of the options.
<P>
In most cases, the applications programmer will be using
the following routines.
<UL>
<LI>Flags: init_flag(), flag().
<LI>Parms: init_parm(), parm().
<LI>Stringparm: init_stringparm(), stringparm().
</UL>
*/

/* Public definitions */

#define MAX_FLAGS                100
#define MAX_PARMS                100
#define MAX_STRINGPARMS          100
#define MAX_FLOATPARMS           100

/* End of public definitions */

/* Public function prototypes from options.c */

enum class OptType { FLAGT, PARMT, FLOATPARMT, STRINGPARMT };

typedef class Toptdep * Optdep;

class Toptdep {                  /* these get attached to flags/parms that have dependents */

public:
        bool flag_trigger;    /* whether set or clear triggers action */
        OptType type;          /* type of dependent option */
        int id;               /* id of dependent option */
        int val;              /* value for dependent flag or parm */
        double dval;          /* value for dependent floatparm */
        string *sval;         /* value for dependent stringparm */
        bool multiply;        /* means val is multiplier instead of fixed value */
        Optdep next;          /* next dependent */
        
        
        static::Optdep append_dep(Optdep, Optdep);
        
        friend class TFlags;
        
        
};



class Tflag {  /* Flags are boolean valued options */
public:
        string name;
        bool val;
        bool default_val;
        Optdep dependencies; //list of dependencies
        
        
};



class Tparm {  /* Parms are integer valued options */
public:
        string name;
        int val;
        int default_val;
        int min, max;  /* minimum and maximum permissible values */
        Optdep dependencies;
};



class TParms {
                private:
                        Tparm Parms[MAX_PARMS];
                        int Next_parm;
                        

                public: TParms();
                        ~TParms();
                        int init_parm(string, int,int,int);
	                    int parm(int);
                        bool at_parm_limit(int, int);
                        bool over_parm_limit(int, int);
                        int str_to_parm_id(string);
                        string parm_id_to_str(int);
                        void parm_flag_dependency(int, int, int);
                        void parm_parm_dependency(int, int, int, bool);
                        int parm_default(int);
                        void Free_Mem(void);
    
                friend class GlobalOptions;
              
};



class TFloatParms; //for forward reference

class TStringParms; //for forward reference

class TFlags {  //classe que contém um array de flags
                private: 
                            Tflag Flags[MAX_FLAGS];
                            int Next_flag;  //apontador para o array de flags
                           
                private:    
                          
                            bool flag(int);
                            int init_flag(string, bool);
                            int str_to_flag_id(string);
                            string flag_id_to_str(int);
                            void flag_flag_dependency(int, bool, int, bool);
                            void flag_flag_dep_default(int, bool, int);
                            void flag_parm_dependency(int, bool, int, int);
                            void flag_floatparm_dependency(int, bool, int, double);
                            void flag_parm_dep_default(int, bool, int, TParms &);
                            void flag_floatparm_dep_default(int, bool, int, TFloatParms &);
                            void flag_stringparm_dependency(int, bool, int, string);
                            void flag_stringparm_dep_default(int, bool, int, TStringParms &);
                            int flag_default(int);
            
                            
                   public:  TFlags(); //construtor
                            ~TFlags();
                            void Free_Mem(void);
                           
                            
                friend class GlobalOptions;
                
};




class Tstringparm {
                        public:
                                string *name;
                                string val;
                                string default_val;
                                int n;
                                string *range;
};






class TStringParms {
                    private:
                               
                                Tstringparm StringParms[MAX_STRINGPARMS];
                                int Next_stringparm;
                                
                    public:            
                                int init_stringparm(string , int, va_list);
                                int init_stringparm_v2(const string& name, const vector<string>& arg);
                                bool stringparm(int, string);
                                string stringparm1(int);
                                int str_to_stringparm_id(string);
                                string stringparm_id_to_str(int);
                                string stringparm1_default(int);
                                
                                
                                TStringParms();
                                ~TStringParms();
                           
                    friend class GlobalOptions;
};



class Tfloatparm {
                    public:
                        string name;
                        double val;
                        double default_val;
                        double min, max;  /* minimum and maximum permissible values */
                        Optdep dependencies;
    
                     
};



class TFloatParms {
                        private:    Tfloatparm FloatParms[MAX_FLOATPARMS];
                                    int Next_floatparm;
                                    
                        public:     
                                    double floatparm(int);
                                    int init_floatparm(string, double, double, double);
                                    int str_to_floatparm_id(string);
		                            string floatparm_id_to_str(int);
                                    double floatparm_default(int);
                                    TFloatParms();
                                    ~TFloatParms();
    
                        friend class GlobalOptions;
                       
};



class GlobalOptions {

    
                        private:    
                                    unsigned Optdep_gets, Optdep_frees;
                                  
                                    TFlags Flags;
                                    TParms Parms;
                                    TStringParms Stringparms;     
                                    TFloatParms Floatparms;
                                    bool Ignore_dependencies;
                            
    
                        public:
                               GlobalOptions();
                               ~GlobalOptions();
                                
                               void Free_Mem(void);
                              
                                
                                int Option_updates;  /* Flag, Parm, Stringparm */
                               
                        
                               
                                
                                int init_flag(string, bool);
                                bool flag(int id);
                                void flag_parm_dependency(int, bool, int, int);
                                void flag_parm_dep_default(int, bool, int, TParms &);
                                void flag_flag_dependency(int, bool, int, bool);
                                void flag_stringparm_dependency(int, bool, int, string);
                                void flag_floatparm_dependency(int,bool, int, double);
                                void flag_flag_dep_default(int, bool, int);
                                void flag_floatparm_dep_default(int, bool, int, TFloatParms &);
                                void flag_stringparm_dep_default(int,bool, int, TStringParms &);
                                int  str_to_flag_id(string);
                                string flag_id_to_str(int);
                                
                                
                                
                                int init_parm(string, int,int,int);
                                void parm_parm_dependency(int, int, int, bool);
                                TParms & getParms(void);
                                int parm (int);
                                int str_to_parm_id(string);

                            
                                
                                string stringparm1(int id);
                                int init_stringparm(string name, int n, ...);
                                int init_stringparm_v2(const string& name, const vector<string>& arg);
                                TStringParms & getStringParms(void);
                                int str_to_stringparm_id(string name) ;
                                
                                int init_floatparm(string, double, double, double);
                                TFloatParms & getFloatParms(void);
                                int str_to_floatparm_id(string);
                                
                                
                                void fprintf_options(ostream &);
                                void p_options(void);
                                
                                
                                void fprintf_options_mem(ostream &, bool);
                                void p_options_mem(void);
                                
                                Optdep get_optdep(void);
                                void free_optdep(Optdep p);
                                
                                void free_depedencies(Optdep);
                                
                                void enable_option_dependencies(void);
                                void disable_option_dependencies(void);
                                bool option_dependencies_state(void);
                                
                                
                                
                                void update_flag(ostream &, int, bool, bool);
                                void set_flag(int, bool);
                                void clear_flag(int, bool);
                                

                                void assign_parm(int, int, bool);
                                void assign_floatparm(int, double, bool);
                                void assign_stringparm(int, string, bool);
                                
                                int option_updates(void);
                                string parm_id_to_str(int);
                                
                                int parm_default(int);
                                double floatparm(int);
                                bool stringparm(int, string);
                                bool over_parm_limit(int, int);
                                bool at_parm_limit(int,int);
                                
                                
                                friend class LadrVGlobais;
};



#endif  /* conditional compilation of whole file */
