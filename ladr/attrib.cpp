#include "attrib.h"
#include "fatal.h"


#include "attrib.h"
#include "memory.h"
#include"fatal.h"
#include "mystring.h"
#include "symbols.h"
#include "unify.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>






GlobalAttribute::GlobalAttribute() {
    Next_attribute_name=0;
    Attribute_gets=0;   
    Attribute_frees=0;
}    

GlobalAttribute::~GlobalAttribute() {

    
}

void GlobalAttribute::Free_Mem(void) {

    for (int i=0; i<Next_attribute_name; i++) 
      if(Attribute_names[i].name) 
          delete (Attribute_names[i].name);
      
      

         
      
      
}



AttributeContainer::AttributeContainer() {
    head=NULL;
}
AttributeContainer::~AttributeContainer() {
    head=NULL;
}

Attribute AttributeContainer::get_attribute(void) {
  Attribute p = (Attribute) Memory::memCNew(sizeof(struct attribute));
  LADR_GLOBAL_ATTRIBUTE.Attribute_gets++;
  
  return(p);
}

void AttributeContainer::free_attribute(Attribute p) {
  Memory::memFree((Attribute)p, sizeof(struct attribute));
  LADR_GLOBAL_ATTRIBUTE.Attribute_frees++;
}

void AttributeContainer::fprint_attrib_mem(ostream &o, bool heading) {
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct attribute);
  o<<"attribute   ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_ATTRIBUTE.Attribute_gets;
  o<<setw(11)<<LADR_GLOBAL_ATTRIBUTE.Attribute_frees;
  o<<setw(11)<<LADR_GLOBAL_ATTRIBUTE.Attribute_gets-LADR_GLOBAL_ATTRIBUTE.Attribute_frees;
  o<<setw(9)<<((LADR_GLOBAL_ATTRIBUTE.Attribute_gets-LADR_GLOBAL_ATTRIBUTE.Attribute_frees) *n) / 1024<<"K"<<endl;
}

/* PUBLIC */
void AttributeContainer::p_attrib_mem(void){
  fprint_attrib_mem(cout, true);
}


Attribute_type AttributeContainer::attribute_type(Attribute a) {
  return LADR_GLOBAL_ATTRIBUTE.Attribute_names[a->id].type;
}


string & AttributeContainer::attribute_name (Attribute a) {
  
    return *(LADR_GLOBAL_ATTRIBUTE.Attribute_names[a->id].name);
}


int AttributeContainer::register_attribute(const string &name, Attribute_type type){
  SymbolContainer S;
  int id = -1;
  if (LADR_GLOBAL_ATTRIBUTE.Next_attribute_name == MAX_ATTRIBUTE_NAMES)
    fatal::fatal_error("register_attribute: too many attribute names");
  else {
    
    id = S.str_to_sn(name, 1);  /* insert into symbol table */
    id = LADR_GLOBAL_ATTRIBUTE.Next_attribute_name++;
    LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].name =new string (name);
    LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type = type;
  }
  return id;
}


void AttributeContainer::declare_term_attribute_inheritable(int id) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::TERM_ATTRIBUTE)
    fatal::fatal_error("declare_term_attribute_inheritable, bad id");
  LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].inheritable = true;
}



bool AttributeContainer::inheritable(Attribute a) {
  return LADR_GLOBAL_ATTRIBUTE.Attribute_names[a->id].inheritable;
}


Attribute AttributeContainer::set_int_attribute(Attribute a, int id, int val) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::INT_ATTRIBUTE)    fatal::fatal_error("set_int_attribute, bad id");

  if (a == NULL) {
    Attribute b = get_attribute();
    b->id = id;
    b->u.i = val;
    return b;
  }
  else {
    a->next = set_int_attribute(a->next, id, val);
    return a;
  }
} 


int AttributeContainer::get_int_attribute(Attribute a, int id, int n) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::INT_ATTRIBUTE) fatal::fatal_error("get_int_attribute, bad id");
  if (a == NULL)   return INT_MAX;
  else if (a->id == id && n == 1)   return a->u.i;
  else if (a->id == id)    return get_int_attribute(a->next, id, n-1);
  else  return get_int_attribute(a->next, id, n);
}

bool AttributeContainer::exists_attribute(Attribute a, int id) {
  if (a == NULL)  return false;
  else if (a->id == id)  return true;
  else  return exists_attribute(a->next, id);
}


Attribute AttributeContainer::set_term_attribute(Attribute a, int id, Term val) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::TERM_ATTRIBUTE) fatal::fatal_error("set_term_attribute, bad ID");

  if (a == NULL) {
    Attribute b = get_attribute();
    b->id = id;
    b->u.t = val;
    return b;
  }
  else {
    a->next = set_term_attribute(a->next, id, val);
    return a;
  }
} 


void AttributeContainer::replace_term_attribute(Attribute a, int id, Term val, int n) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::TERM_ATTRIBUTE) fatal::fatal_error("replace_term_attribute, bad ID");

  if (a == NULL) fatal::fatal_error("replace_term_attribute, attribute not found");
  else if (a->id == id && n == 1) {
    TermContainer T;  
    T.zap_term(a->u.t);
    a->u.t = val;
  }
  else if (a->id == id)  replace_term_attribute(a->next, id, val, n-1);
  else   replace_term_attribute(a->next, id, val, n);
}

void AttributeContainer::replace_int_attribute(Attribute a, int id, int val, int n) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::INT_ATTRIBUTE) fatal::fatal_error("replace_int_attribute, bad ID");

  if (a == NULL) fatal::fatal_error("replace_int_attribute, attribute not found");
  else if (a->id == id && n == 1) {
    a->u.i = val;
  }
  else if (a->id == id)  replace_int_attribute(a->next, id, val, n-1);
  else replace_int_attribute(a->next, id, val, n);
}


Term AttributeContainer::get_term_attribute(Attribute a, int id, int n) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::TERM_ATTRIBUTE) fatal::fatal_error("get_term_attribute, bad ID");
  if (a == NULL) return NULL;
  else if (a->id == id && n == 1)  return a->u.t;
  else if (a->id == id)  return get_term_attribute(a->next, id, n-1);
  else  return get_term_attribute(a->next, id, n);
} 


Term AttributeContainer::get_term_attributes(Attribute a, int id) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::TERM_ATTRIBUTE) fatal::fatal_error("get_term_attribute, bad ID");
  if (a == NULL)  return NULL;
  else {
    Term t = get_term_attributes(a->next, id);
    if (a->id == id) {
       TermContainer T; 
       SymbolContainer S;
       Term head = T.copy_term(a->u.t);
       if (t == NULL) t = head;
       else	t = T.build_binary_term_safe(S.attrib_sym(), head, t);
    }
    return t;
  }
} 





Attribute AttributeContainer::set_string_attribute(Attribute a, int id, const string  &val) {
  SymbolContainer S;
  unsigned sn;
  string s;
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::STRING_ATTRIBUTE) fatal::fatal_error("set_string_attribute, bad ID");
  
  if (a==NULL) {
    sn = S.str_to_sn(val, 0);   /* insert into symbol table */
    s = S.sn_to_str(sn); 
    Attribute b = get_attribute();
    b->id = id;
    b->u.s = new string (val);
    return b;
  }
  else {
    a->next = set_string_attribute(a->next, id, val);
    return a;
  }
}  


void AttributeContainer::delete_string_attribute(Attribute a, const string &s) {
        if(a==NULL) return;
        if(myString::str_ident(*(a->u.s),s)) {
            delete(a->u.s);
            free_attribute(a);
        }
        delete_string_attribute(a->next,s);
}

const string &AttributeContainer::get_string_attribute(Attribute a, int id, int n) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::STRING_ATTRIBUTE)    fatal::fatal_error("get_string_attribute, bad ID");
  if (a == NULL) return myString::null_string();
  else if (a->id == id && n == 1)  return *(a->u.s);
  else if (a->id == id)  return get_string_attribute(a->next, id, n-1);
  else  return get_string_attribute(a->next, id, n);
} 



bool AttributeContainer::string_attribute_member(Attribute a, int id, const string &s) {
  if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type != Attribute_type::STRING_ATTRIBUTE)  fatal::fatal_error("string_attribute_member, bad ID");

  if (a == NULL)   return false;
  else if (a->id == id && myString::str_ident(s, *a->u.s))   return true;
  else return string_attribute_member(a->next, id, s);
} 


void AttributeContainer::zap_attributes(Attribute a) {
  if (a != NULL) {
    zap_attributes(a->next);
    /* If there is any Memory:: associted with the attribure, free it here. */
    if (attribute_type(a) == Attribute_type::TERM_ATTRIBUTE) {
          TermContainer T; 
          T.zap_term(a->u.t); 
    }
    
    if (attribute_type(a)==Attribute_type::STRING_ATTRIBUTE) {
        delete a->u.s;
    }
    free_attribute(a);
  }
}



Attribute AttributeContainer::delete_attributes(Attribute a, int id) {
  if (a == NULL)  return NULL;
  else {
    a->next = delete_attributes(a->next, id);
    if (a->id == id) {
      Attribute b = a->next;
      /* If there is any Memory:: associted with the attribure, free it here. */
      if (attribute_type(a) == Attribute_type::TERM_ATTRIBUTE) {TermContainer T; T.zap_term(a->u.t);}
      free_attribute(a);
      return b;
    }
    else return a;
  }
} 

Attribute AttributeContainer::cat_att(Attribute a, Attribute b) {
  if (a == NULL)  return b;
  else {
    a->next = cat_att(a->next, b);
    return a;
  }
} 


Term AttributeContainer::build_attr_term(Attribute a) {
  string name = attribute_name(a);
  Attribute_type type = attribute_type(a);
  TermContainer T;
  Term t = T.get_rigid_term(name, 1);  /* e.g., label(cl_32), answer(assoc) */
  
  switch (type) {
        case Attribute_type::INT_ATTRIBUTE: {
            string s;
            if (a->u.i < 0) {
                        ARG(t,0) = T.get_rigid_term("-", 1);
                        ARG(ARG(t,0),0) = T.get_rigid_term(myString::int_to_str(-(a->u.i), s, 25), 0);
            }
            else {
                    ARG(t,0) = T.get_rigid_term(myString::int_to_str(a->u.i, s, 25), 0);
            }
        break;
        }
        
        case Attribute_type::STRING_ATTRIBUTE:
                                            ARG(t,0) = T.get_rigid_term(*a->u.s, 0);
        break;
        
        case Attribute_type::TERM_ATTRIBUTE:
                                            ARG(t,0) = T.copy_term(a->u.t);
        break;
        default:
                fatal::fatal_error("build_attr_term: bad attribute type");
  }
  return t;
} 


Term AttributeContainer::attributes_to_term(Attribute a, const string & _operator) {
  if (a == NULL)      return NULL;  /* should happen only on top call */
  else if (a->next == NULL)   return build_attr_term(a);
  else {
                TermContainer T; SymbolContainer S;
                return T.build_binary_term(S.str_to_sn(_operator, 2), build_attr_term(a), attributes_to_term(a->next, _operator));
  }
} 


Attribute AttributeContainer::cat_attributes(Attribute a0, Attribute a1) {
  if (a0 == NULL)  return a1;
  else {
    a0->next = cat_attributes(a0->next, a1);
    return a0;
  }
} 


int AttributeContainer::attribute_name_to_id(const string &name) {
  int i;
  for (i = 0; i < MAX_ATTRIBUTE_NAMES; i++) {
    if (!LADR_GLOBAL_ATTRIBUTE.Attribute_names[i].name->empty() &&	myString::str_ident(*LADR_GLOBAL_ATTRIBUTE.Attribute_names[i].name, name)) return i;
  }
  return -1;
} 



Attribute AttributeContainer::term_to_attributes(Term t, const string & _operator) {
   TermContainer T;
   if (T.is_term(t, _operator, 2)) {
    Attribute a0 = term_to_attributes(ARG(t,0), _operator);
    Attribute a1 = term_to_attributes(ARG(t,1), _operator);
    return cat_attributes(a0, a1);
  }
  else {
    int id;
    Attribute a;
    SymbolContainer S;
    
    if (ARITY(t) != 1) fatal::fatal_error("term_to_attributes, arity not 1");
    id = attribute_name_to_id(S.sn_to_str(SYMNUM(t)));
    if (id == -1)  
        fatal::fatal_error("term_to_attributes, attribute name not found");
        
    a = get_attribute();
    a->id = id;
    switch (LADR_GLOBAL_ATTRIBUTE.Attribute_names[id].type) {
        case Attribute_type::INT_ATTRIBUTE: {
            int i;
            if (!T.term_to_int(ARG(t,0), &i)) fatal::fatal_error("term_to_attributes, bad integer");
            a->u.i = i;
            break;
       }
       case Attribute_type::STRING_ATTRIBUTE:
            if (!CONSTANT(ARG(t,0))) fatal::fatal_error("term_to_attributes, bad string");
            else 	
                a->u.s = new string(S.sn_to_str(SYMNUM(ARG(t,0))));
               
        break;
       case Attribute_type::TERM_ATTRIBUTE:
            a->u.t = T.copy_term(ARG(t,0));
        break;
    }
    return a;
  }
} 


Attribute AttributeContainer::inheritable_att_instances(Attribute a, Context subst) {
  if (a == NULL) return NULL;
  else if (!inheritable(a))
    return inheritable_att_instances(a->next, subst);
  else {
    TermContainer T;
    UnifyContainer U;
    Attribute novo = get_attribute();
    novo->id = a->id;
    novo->u.t = subst ? U.apply(a->u.t, subst) : T.copy_term(a->u.t);
    novo->next = inheritable_att_instances(a->next, subst);
    return novo;
  }
}



Attribute AttributeContainer::copy_attributes(Attribute a) {
  if (a == NULL)  return NULL;
  else {
    TermContainer T;
    Attribute novo = get_attribute();
    novo->id = a->id;
    
    switch (attribute_type(a)) {
        case Attribute_type::INT_ATTRIBUTE: novo->u.i = a->u.i; break;
        case Attribute_type::STRING_ATTRIBUTE: {
                                                    novo->u.s=new string(*a->u.s);
                                                    break;
        }
        case Attribute_type::TERM_ATTRIBUTE: novo->u.t = T.copy_term(a->u.t); break;
        default: fatal::fatal_error("copy_attribute: unknown attribute");
    }
    novo->next = copy_attributes(a->next);
    return novo;
  }
}


void AttributeContainer::instantiate_inheritable_attributes(Attribute a, Context subst) {
  Attribute b;
  TermContainer T;
  UnifyContainer U;
  for (b = a; b; b = b->next) {
    if (attribute_type(b) == Attribute_type::TERM_ATTRIBUTE && inheritable(b)) {
      Term t = U.apply(b->u.t, subst);
      T.zap_term(b->u.t);
      b->u.t = t;
    }
  }
}


void AttributeContainer::renumber_vars_attributes(Attribute attrs, int vmap[], int max_vars){
  Attribute a;
  TermContainer T; 
  for (a = attrs; a; a = a->next) {
    if (inheritable(a)) {
      a->u.t = T.renum_vars_recurse(a->u.t, vmap, max_vars);
    }
  }
} 

void AttributeContainer::set_vars_attributes(Attribute attrs, string vnames[], int max_vars) {
  Attribute a;   TermContainer T;  
  for (a = attrs; a; a = a->next) {
    if (inheritable(a)) {
      a->u.t = T.set_vars_recurse(a->u.t, vnames, max_vars);
    }
  }
}



Plist AttributeContainer::vars_in_attributes(Attribute attrs) {
  Plist vars = NULL;
  Attribute a; 
  TermContainer T;  
  for (a = attrs; a; a = a->next) {
    if (inheritable(a)) {
      vars = T.set_of_vars(a->u.t, vars);
    }
  }
  return vars;
} 

int AttributeContainer::label_att(void) {
  int i;
  for (i = 0; i < MAX_ATTRIBUTE_NAMES; i++) {
    if (LADR_GLOBAL_ATTRIBUTE.Attribute_names[i].type == Attribute_type::STRING_ATTRIBUTE && myString::str_ident(*(LADR_GLOBAL_ATTRIBUTE.Attribute_names[i].name), "label"))    
        return i;
  }
  return -1;
} 


bool AttributeContainer::attributes_contain_variables(Attribute a) {
  if (a == NULL)    return false;
  else if (attributes_contain_variables(a->next)) return true;
  else if (attribute_type(a) != Attribute_type::TERM_ATTRIBUTE)   return false;
  else {
    TermContainer T;
    PlistContainer P;
    Term t = a->u.t;
    Plist p = T.free_vars_term(t, NULL);
    bool contains_vars = (p != NULL);
    P.set_head(p);
    P.zap_plist();
    return contains_vars;
  }
} 

Attribute AttributeContainer::copy_int_attribute(Attribute source, Attribute dest, int attr_id) {
  int i = 1;
  int val;
  while ((val = get_int_attribute(source, attr_id, i)) != INT_MAX) {
    dest = set_int_attribute(dest, attr_id, val);
    i++;
  }
  return dest;
}  



Attribute AttributeContainer::copy_string_attribute(Attribute source, Attribute dest, int attr_id) {
  int i = 1;
  string val;
  while ( !(val=get_string_attribute(source, attr_id, i)).empty() ) {
    dest = set_string_attribute(dest, attr_id, val);
    i++;
  }
  return dest;
}

void AttributeContainer::p_label_attribute(Attribute a)
{
    
 
    
}


void AttributeContainer::p_attribute(Attribute a) {
    TermContainer T;
    cout<<"Formula attributes---------------------------"<<endl;
    cout<<"Name:"<<attribute_name(a)<<endl;
    cout<<"Id:"<<a->id<<endl;

    switch(attribute_type(a)) {
        
        case Attribute_type::INT_ATTRIBUTE:
                                    cout<<"Type: INT, Value:"<< a->u.i;   
                                break;

        case Attribute_type::STRING_ATTRIBUTE:
                                    cout<<"Type: STRING, Value:"<< *(a->u.s)<<endl;                       
                                break;
            
        case Attribute_type::TERM_ATTRIBUTE:
                                 T.p_term(a->u.t);
                                break;
                            
        
    }
    
    cout<<"-----------------------------------------------"<<endl;
    
}


Attribute AttributeContainer::copy_term_attribute(Attribute source, Attribute dest, int attr_id) {
  int i = 1;
  Term val;
  TermContainer T;
  while ((val = get_term_attribute(source, attr_id, i))) {
    dest = set_term_attribute(dest, attr_id, T.copy_term(val));
    i++;
  }
  return dest;
}
