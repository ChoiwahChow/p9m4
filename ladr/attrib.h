#ifndef TP_ATTRIB_H
#define TP_ATTRIB_H

#include "unify.h"
#include <iostream>
#define MAX_ATTRIBUTE_NAMES 50



enum class Attribute_type {
                            INT_ATTRIBUTE,
                            STRING_ATTRIBUTE,
                            TERM_ATTRIBUTE
};





struct attribute {   /*  to form lists of attributes */
                    int id;            /* attribute ID (index into Attribute_names array) */
               
                    union {            /* attribute value */
                            int i;          //for int attribute
                            string *s;      //for string attribute
                            Term t;         //for term attribute
                          } u;
                    attribute *next; //linked list
};

typedef struct attribute * Attribute;


typedef  struct {  /* array, indexed by attribute id */
                    string *name;         /* name of attribute, e.g., label, answer */
                    Attribute_type type;  /* INT_ATTRIBUTE STRING_ATTRIBUTE TERM_ATTRIBUTE etc */
                    bool inheritable;     /* child gets instance (for term attributes only) */
                    
                   
} TAttributeNames;




class GlobalAttribute {
                        private:
                                    TAttributeNames Attribute_names[MAX_ATTRIBUTE_NAMES];
                                    int Next_attribute_name;
                                    unsigned Attribute_gets;
                                    unsigned Attribute_frees;
                                 
                                   
                            
                                    
                        public:            
                                    GlobalAttribute();
                                    ~GlobalAttribute();
                                    void Free_Mem(void);
                                    friend class AttributeContainer;
                                    friend class LadrVGlobais;
                            

};


class AttributeContainer {
                             private:
                                        Attribute head;
                                        
                                        void free_attribute(Attribute);
                                        Attribute cat_attributes(Attribute, Attribute);
                                        
                             public:
                                        AttributeContainer();
                                        ~AttributeContainer();
                                        Attribute get_attribute(void);
                                        void fprint_attrib_mem(ostream &, bool);
                                        void p_attrib_mem(void);
                                        Attribute_type attribute_type(Attribute);
                                        string & attribute_name(Attribute);
                                        int register_attribute(const string &, Attribute_type);
                                        void declare_term_attribute_inheritable(int); 
                                        bool inheritable(Attribute);
                                        Attribute set_int_attribute(Attribute, int, int);
                                        int get_int_attribute(Attribute, int , int); 
                                        
                                        bool exists_attribute(Attribute, int);
                                        Attribute set_term_attribute(Attribute, int, Term);
                                        void replace_term_attribute(Attribute, int, Term, int);
                                        void replace_int_attribute(Attribute, int, int, int);
                                        Term get_term_attribute(Attribute, int, int);
                                        Term get_term_attributes(Attribute, int);
                                        bool is_string_attribute_name_defined(const string &);
                                        Attribute set_string_attribute(Attribute, int, const string &);
                                        const string & get_string_attribute(Attribute, int, int);
                                        bool string_attribute_member(Attribute, int, const string &);
                                        void zap_attributes(Attribute);
                                        Attribute delete_attributes(Attribute, int);
                                        Attribute cat_att(Attribute, Attribute);
                                        Term build_attr_term(Attribute);
                                        Term attributes_to_term(Attribute, const string &);
                                        int attribute_name_to_id(const string &);
                                        Attribute term_to_attributes(Term, const string &);
                                        Attribute inheritable_att_instances(Attribute, Context);
                                        Attribute copy_attributes(Attribute);
                                        void instantiate_inheritable_attributes(Attribute, Context);
                                        void renumber_vars_attributes(Attribute, int [], int);
                                        void set_vars_attributes(Attribute attrs, string [], int);
                                        Plist vars_in_attributes(Attribute);
                                        int label_att(void); 
                                        bool attributes_contain_variables(Attribute);
                                        Attribute copy_int_attribute(Attribute, Attribute, int);
                                        Attribute copy_string_attribute(Attribute, Attribute, int);
                                        Attribute copy_term_attribute(Attribute, Attribute, int);
                                        void p_attribute(Attribute);
                                        void p_label_attribute(Attribute);
                                        void delete_string_attribute(Attribute, const string &);
 
};





#endif
