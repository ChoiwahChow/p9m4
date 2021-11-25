#include <iomanip>
#include "interp.h"
#include "ioutil.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include "mystring.h"




GlobalInterp::GlobalInterp() {
    Iso_checks=0;
    Iso_perms=0;
    Interp_gets=0;
    Interp_frees=0;
}

GlobalInterp::~GlobalInterp() {
}




Interp InterpContainer::get_interp(void){ 
  Interp p =(Interp) Memory::memCNew(sizeof(struct interp));
  LADR_GLOBAL_INTERP.Interp_gets++;
  return(p);
}
								

void InterpContainer::free_interp(Interp p) {
  Memory::memFree((void *)p, sizeof(struct interp));
  LADR_GLOBAL_INTERP.Interp_frees++;
} 								


void InterpContainer::fprint_interp_mem(ostream &o, bool heading) {
  int n;
  if (heading)
	o<<"  type (bytes each)        gets      frees      in use      bytes"<<endl;	
  n = sizeof(struct interp);
  o<< "interp ("<<setw(4)<<n<<")       ";
  o<<setw(11)<<LADR_GLOBAL_INTERP.Interp_gets;
  o<<setw(11)<<LADR_GLOBAL_INTERP.Interp_frees;
  o<<setw(11)<<LADR_GLOBAL_INTERP.Interp_gets-LADR_GLOBAL_INTERP.Interp_frees;
  o<<setw(9)<< ((LADR_GLOBAL_INTERP.Interp_gets-LADR_GLOBAL_INTERP.Interp_frees) * n ) /1024<<endl;
} 

void InterpContainer::p_interp_mem() {
  fprint_interp_mem(cout, true);
}



int * InterpContainer::trivial_permutation(int n) {
  int *p = (int *) malloc(sizeof(int) * n);
  int i;
  for (i = 0; i < n; i++)
    p[i] = i;
  return p;
} 



int InterpContainer::int_power(int n, int exp) {
  if (exp < 0)   return -1;
  else {
    int i;
    int r = 1;
    for (i = 0; i < exp; i++) {
      int x = r * n;
      if (x < r)
	return INT_MAX;  /* overflow */
      r = x;
    }
    return r;
  }
} 

Interp InterpContainer::compile_interp(Term t, bool allow_incomplete) {
  SymbolContainer S;
  ListtermContainer L;
  TermContainer T;
  Interp p;
  int number_of_ops, domain_size, arity;
  int i, j, n, symnum, val, max, rc;
  bool function = false;
  int *table; 
  Term comments = NULL;
  Term size = NULL;
  Term operations = NULL;

  if (ARITY(t) == 3) {  /* interpretation(size, comments, ops). */
    size = ARG(t,0);
    comments = ARG(t,1);
    operations = ARG(t,2);
  }
  else
    fatal::fatal_error("compile_interp, arity must be 3 (size, comments, ops).");

  rc = myString::str_to_int(S.sn_to_str(SYMNUM(size)), &domain_size);
  
  if (!rc || domain_size < 1)
    fatal::fatal_error("compile_interp, domain size out of range.");

  number_of_ops = L.listterm_length(operations);
  /*
  if (number_of_ops == 0)
    fatal_error("compile_interp, interpretation has no operations.");
  */

  /* Get the largest symnum, so we can get a table array big enough. */

  max = 0;
  for (i = 1; i <= number_of_ops; i++) {
    Term f = L.listterm_i(operations, i);
    if (ARITY(f) != 2 || VARIABLE(ARG(f,0)))
      fatal::fatal_error("compile_interp, bad operation.");
    symnum = SYMNUM(ARG(f,0));
    max = symnum > max ? symnum : max;
  }

  p = get_interp();
  p->t = T.copy_term(t);
  p->comments = T.copy_term(comments);
  p->size = domain_size;
  p->num_tables = max + 100 + 1;  /* allow 100 extra in case of new symbols */

  p->occurrences = (int *) malloc(domain_size * sizeof(int));
  p->blocks = (int *) malloc(domain_size * sizeof(int));
  p->profile = (int **) malloc(domain_size * sizeof(int *));
  for (i = 0; i < domain_size; i++) {
    p->occurrences[i] = 0;
    p->blocks[i] = -1;
    p->profile[i] = NULL;
  }

  p->tables  =(int **) malloc(p->num_tables * sizeof(int *));
  p->types   = (int *) malloc(p->num_tables * sizeof(int));
  p->arities = (int *) malloc(p->num_tables * sizeof(int));

  for (i = 0; i < p->num_tables; i++) {
    p->tables[i] = NULL;
    p->types[i] = UNDEFINED;
    p->arities[i] = -1;
  }

  for (i = 1; i <= number_of_ops; i++) {
    Term lst;
    Term f = L.listterm_i(operations, i);
    if (S.is_symbol(SYMNUM(f), "function", 2))
      function = true;
    else if (S.is_symbol(SYMNUM(f), "relation", 2) || 
	     S.is_symbol(SYMNUM(f), "predicate", 2)) {
      function = false;
    }
    else
      fatal::fatal_error("compile_interp, bad function/relation");

    symnum = SYMNUM(ARG(f,0));
    arity = ARITY(ARG(f,0));

    /* n = domain_size^arity */

    for (j = 0, n = 1; j < arity; j++)
      n = n * domain_size;

    lst = ARG(f,1);
    if (L.listterm_length(lst) != n)
      fatal::fatal_error("compile_interp, list of elements is wrong "
		  "length for arity/domain_size.");

    p->types[symnum] = (function ? FUNCTION : RELATION);
    p->arities[symnum] = arity;
    p->tables[symnum] =(int *) malloc(n * sizeof(int));
    table = p->tables[symnum];

    for (j = 0; j < n; j++, lst = ARG(lst,1)) {
      string str = S.sn_to_str(SYMNUM(ARG(lst,0)));
      rc = myString::str_to_int(str, &val);
      if (!rc) {
	if (allow_incomplete && myString::str_ident(str, "-")) {
	  table[j] = -1;
	  p->incomplete = true;
	}
	else
	  fatal::fatal_error("compile_interp, bad domain elemnt.");
      }
      else if (function && (val < 0 || val > domain_size-1))
			fatal::fatal_error("compile_interp, function element out of range.");
      else if (!function && (val < 0 || val > 1))
			fatal::fatal_error("compile_interp, relation element out of range.");
      else {
			table[j] = val;
			if (function)	p->occurrences[val]++;
	  }
    }
  }
  return p;
} 


void InterpContainer::transpose_binary(Term t) {
  SymbolContainer S;
  ListtermContainer L;
  int number_of_ops, n;
  int i, rc;
  Term operations;

  rc = myString::str_to_int(S.sn_to_str(SYMNUM(ARG(t,0))), &n);
  operations = ARG(t,2);
  number_of_ops = L.listterm_length(operations);

  for (i = 1; i <= number_of_ops; i++) {
    Term f = L.listterm_i(operations, i);  /* e.g., function(j(_,_), [0,1,1,0]) */
    if (ARITY(ARG(f,0)) == 2) {
      int j, k;
      Term lst = ARG(f,1);             /* e.g., [0,1,1,0] */
      for (j = 0; j < n; j++) {
		for (k = j+1; k < n; k++) {
			Term t1 = L.listterm_i(lst, j*n+k+1);
			Term t2 = L.listterm_i(lst, k*n+j+1);
			int tmp = t1->private_symbol;
			t1->private_symbol = t2->private_symbol;
			t2->private_symbol = tmp;
		}
      }
    }
  }
}


void InterpContainer::zap_interp(Interp p) {
  TermContainer T;
  int i;
  free(p->occurrences);
  free(p->blocks);
  free(p->types);
  free(p->arities);

  if (p->discriminator_counts)
    free(p->discriminator_counts);

  for (i = 0; i < p->size; i++)
    if (p->profile[i])
      free(p->profile[i]);
  free(p->profile);

  for (i = 0; i < p->num_tables; i++)
    if (p->tables[i] != NULL)
      free(p->tables[i]);

  free(p->tables);

  if (p->t != NULL)
    T.zap_term(p->t);
  if (p->comments != NULL)
    T.zap_term(p->comments);
  free_interp(p);
} 

void InterpContainer::fprint_interp_tex(ostream &o, Interp p) {
  TermContainer T;
  SymbolContainer S;
  ParseContainer P;
  ListtermContainer LT;
  int n = p->size;
  int i;
  bool first = true;

  if (p->comments) {
    Term comment = p->comments;
    while (LT.cons_term(comment)) {
      o<<"% ";
      P.fwrite_term(o, ARG(comment,0));
      o<<"\n";
      comment = ARG(comment,1);
    }
  }
  
  o<<"\\begin{table}[H]  \\centering % size "<<p->size<<"\n";
  for (i = 0; i < p->num_tables; i++) {  /* arity 0 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    if (table != NULL && arity == 0) {
      if (first) first = false;
      else	o<<" \\hspace[.5cm}\n";
	  o<<S.sn_to_str(i)<<": "<<table[0];
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity 1 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    if (table != NULL && arity == 1) {
      int j;
      if (first)first = false;
      else o<<" \\hspace{.5cm}\n";
	  o<<"\\begin{tabular}{r|";
      
      for (j = 0; j < n; j++) o<<"r";
      o<<"}\n";
	  o<<S.sn_to_str(i)<<": & ";
      for (j = 0; j < n; j++)
		o<<j<<(j<n-1? " & " : "\\\\\n\\hline\n   & ");
      for (j = 0; j < n; j++) o<<table[j]<<(j<n-1? " & " : "\n");
		o<<"\\end{tabular}";
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity 2 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    if (table != NULL && arity == 2) {
      int j, k;
      if (first) first = false;
      else	o<<" \\hspace{.5cm}\n";
	  o<<"\\begin{tabular}{r|"; 	
      
      for (j = 0; j < n; j++) o<<"r";
	  o<<"}\n";
   	  o<<S.sn_to_str(i)<<": & ";
      for (j = 0; j < n; j++) o<<j<<(j<n-1? " & " : "\\\\\n\\hline\n");
	  for (j = 0; j < n; j++) {
		o<<"    "<<j<<" & ";
		for (k = 0; k < n; k++) {
			o<<table[(n*j) + k]<<(k < n-1 ? " & " : (j < n-1 ? " \\\\\n" : "\n"));
		}
      }
      o<<"\\end{tabular}";
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity > 2 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    if (table != NULL && arity > 2) {
      o<<"\n\n% table for arity "<<arity<<" "<< (p->types[i] == FUNCTION ? "function" : "relation");
	  o<<" "<<S.sn_to_str(i)<<" not printed\n\n";
    }
  }
  o<<"\n\\caption{ }\n";
  o<<"\\end{table}\n";
} 

void InterpContainer::compute_args(int *a, int arity, int n, int i) {
  int x = i;
  int r;
  for (r = arity-1; r >= 0; r--) {
    a[r] = x % n;
    x = x - a[r];
    x = x / n;
  }
}

void InterpContainer::fprint_interp_xml(ostream &o, Interp p) {
  SymbolContainer S;
  TermContainer T;
  ListtermContainer LT;
  int n = p->size;
  int i;
  o<<"\n  <interp size=\""<<n<<"\"";
  if (p->comments && !LT.nil_term(p->comments)) {
    Term comment = p->comments;
    while (LT.cons_term(comment)) {
      Term pair  = ARG(comment,0);
      string name  = S.sn_to_str(SYMNUM(ARG(pair,0)));
      string value = S.sn_to_str(SYMNUM(ARG(pair,1)));
      o<<" "<<name<<"=\""<<value<<"\"";
	  comment = ARG(comment,1);
    }
  }
  o<<">\n";

  for (i = 0; i < p->num_tables; i++) {  /* arity 0 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    string type = (p->types[i] == FUNCTION ? "function" : "relation");
    if (table != NULL && arity == 0) {
      o<<"\n    <op0 type=\""<<type<<"\">\n";
	  o<<"      <sym><![CDATA["<<S.sn_to_str(i)<<"]]>/sym\n";
      o<<"      <v>"<<table[0]<<">\v>\n";
      o<<"    </op0>\n";
    }
  }
  
  for (i = 0; i < p->num_tables; i++) {  /* arity 1 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i); 
    string type = (p->types[i] == FUNCTION ? "function" : "relation");
    if (table != NULL && arity == 1) {
      int j;
      o<<"\n    <op1 type=\""<<type<<"\"\n";
	  o<<"        <sym><![CADATA["<<S.sn_to_str(i)<<"]]></sym>\n";
      o<<"        <head>";
      
      for (j = 0; j < n; j++) o<<"<i>"<<j<<"</i>";
      o<<"</head\n";
	  o<<"        <row> "; 
      
      for (j = 0; j < n; j++) o<<"<v>"<<table[j]<<"</v>";
      o<<"</row>\n";
      o<<"    </op1>\n";
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity 2 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    string type = (p->types[i] == FUNCTION ? "function" : "relation");
    if (table != NULL && arity == 2) {
      int j, k;
      o<<"\n    <op2 type=\""<<type<<"\">\n";
	  o<<"        <sym><![CDATA["<<S.sn_to_str(i)<<"]]></sym>\n";
      o<<"        <head>        ";
      
      for (j = 0; j < n; j++) o<<"<i>"<<j<<"</i>";
	  o<<"</head\n";
      

      for (j = 0; j < n; j++) {
		o<<"        <row><i>"<<j<<"</i> ";
		for (k = 0; k < n; k++)
			o<<"<v>"<<table[(n*j)+k]<<"</v>";
		o<<"</row>\n";
		
      }
      o<<"    </op2>\n";
    }
  }
  for (i = 0; i < p->num_tables; i++) {  /* arity > 2 */
    int *table = p->tables[i];
    int arity = S.sn_to_arity(i);
    string type = (p->types[i] == FUNCTION ? "function" : "relation");
    if (table != NULL && arity > 2) {
      int *a = (int *) malloc(arity * sizeof(int));
      int m = int_power(p->size, arity);
      int j;
      o<<"\n    <opn type=\""<<type<<"\" arity=\""<<arity<<"\"\n";
	  o<<"      <sym><!CDATA["<<S.sn_to_str(i)<<"]]></sym>\n";
      
      for (j = 0; j < m; j++) {
		int k;
		compute_args(a, arity, p->size, j);
		o<<"      <tupval> <tup>";
		for (k = 0; k < arity; k++) o<<"<i>"<<a[k]<<"</i>";
		o<<"</tup>   <v>"<<table[j]<<"</v> </tupval>\n";
		
      }
      free(a);
      o<<"    </opn>\n";
    }
  }
  o<<"  </interp>\n";
} 


void fprint_interp_standard(ostream &o, Interp p) {
  TermContainer T;
  SymbolContainer S;
  ParseContainer P;
  int i;
  o<<"interpretation("<<p->size<<", ";
  if (p->comments) {
    P.fwrite_term(o, p->comments);
    o<<", [\n";
  }
  else
    o<<"[], [\n";
	
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j, n;
      int arity = S.sn_to_arity(i);
      
      o<<"    "<< (p->types[i] == FUNCTION ? "function" : "relation") <<"(";
	  if (arity == 0) o<<S.sn_to_str(i)<<", [";
      else {
		o<<S.sn_to_str(i)<<"(";
		for (j = 0; j < arity; j++)
			o<<"_"<<(j == arity-1 ? "), [" : ",");
      }

      for (j = 0, n = 1; j < arity; j++, n = n * p->size);
      for (j = 0; j < n; j++) {
		if (table[j] == -1) o<<"-"<< (j == n-1 ? "])" : ",");
	   else o<<table[j]<<(j == n-1 ? "])" : ",");
      }

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL) o<<",\n";
    }
  }
  o<<"]).\n";
}

void InterpContainer::fprint_interp_standard2(ostream &o, Interp p) {
  TermContainer T;
  SymbolContainer S;
  ParseContainer P;
  int i;
  o<<"interpretation( "<<p->size<<", ";
  

  if (p->comments) {
    P.fwrite_term(o, p->comments);
    o<<", [\n";
  }
  else o<<"[], [\n";
    
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j, n;
      int arity = S.sn_to_arity(i);
      bool w = (arity == 2 && p->size > 10);
      o<<"    "<< (p->types[i] == FUNCTION ? "function" : "relation") << "(";
	  if (arity == 0) o<<S.sn_to_str(i)<<", [";
	  else {
				o<<S.sn_to_str(i)<<"(";
				for (j = 0; j < arity; j++) o<<"_"<<(j == arity-1 ? "), " : ",");
				o<<"["<<(arity== 2 ? "\n        " : "");
	
      }

      for (j = 0, n = 1; j < arity; j++, n = n * p->size);
       for (j = 0; j < n; j++) {
	    if (table[j] == -1)	  {
			if (w) o<<" -";
			else o<<"-";
			o<<(j == n-1 ? "])" : ",");
		}
		else {
			if (w) o << setw(2);
			o << table[j]<<(j == n-1 ? "])" : ",");
		}
		if (arity == 2 && (j+1) % p->size == 0 && j != n-1)	 o<<"\n        ";
      }

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL) o<<",\n";
    }
  }
  o<<"]).\n";
}

void InterpContainer::portable_indent(ostream &o, int n) {
  int i;
  o<<"      ";
  for (i = 0; i < n; i++) o<<"  ";
}


void InterpContainer::portable_recurse(ostream &o,int arity, int domain_size,  int *table, int *idx_ptr, int depth)
{
  if (arity == 0) o << setw(2) << table[(*idx_ptr)++];
  else {
    int i;
    portable_indent(o, depth);
    o<<"["<<(arity > 1 ? "\n" : "");
	
    for (i = 0; i < domain_size; i++) {
      portable_recurse(o, arity-1, domain_size, table, idx_ptr, depth+1);
      if (i < domain_size-1) o<<",";
	  o<<(arity > 1 ? "\n" : ""); 	
    }
    if (arity > 1)
      portable_indent(o, depth);
	o<<"]";  
  }
} 


void InterpContainer::fprint_interp_portable(ostream &o, Interp p) {
  int i;
  Term t;
  TermContainer T;
  ListtermContainer LT;
  SymbolContainer S;
  o<<"  ["<<p->size<<",\n    [";
  for (t = p->comments; t && !LT.nil_term(t) ; t = ARG(t,1)) {
    o<<" \"";
	T.fprint_term(o, ARG(t,0));
    o<<"\""<<(LT.nil_term(ARG(t,1)) ? "":",")<<" ";
  }

  o<<"],\n";
  o<<"    [\n";
  
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j;
      int arity = S.sn_to_arity(i);
      int idx = 0;
      /* BOOL w = (arity == 2 && p->size > 10); */
      o<<"      [\""<<(p->types[i] == FUNCTION ? "function" : "relation");
	  o<<"\", \""<<S.sn_to_str(i)<<"\""<<", "<<arity<<",\n";
      if (arity == 0) portable_indent(o, 1);
      portable_recurse(o, arity, p->size, table, &idx, 1);
	  o<<"\n      ]";
      /* ugly: decide if there are any more symbols */
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL) o<<",\n";
	  else o<<"\n    ]\n  ]";
    }
  }
} 

void InterpContainer::fprint_interp_standard(ostream &o, Interp p) {
int i;
ParseContainer P;
SymbolContainer S;
  
  o<<"interpretation( "<<p->size<<", ";
  if (p->comments) {
    P.fwrite_term(o, p->comments);
    o<<", [\n";
  }
  else o<<"[], [\n";
    
  
  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    if (table != NULL) {
      int j, n;
      int arity = S.sn_to_arity(i);
      o<<"    "<<(p->types[i] == FUNCTION ? "function" : "relation")<<"(";
      if (arity == 0) o<<S.sn_to_str(i)<<", [";
      else {
            o<<S.sn_to_str(i)<<"(";    
            for (j = 0; j < arity; j++) o<<"_"<<(j == arity-1 ? "), [" : ",");
        }

      for (j = 0, n = 1; j < arity; j++, n = n * p->size);
      for (j = 0; j < n; j++) {
        if (table[j] == -1)	 o<<"-"<<(j == n-1 ? "])" : ","); 
        else o<< table[j]<<(j == n-1 ? "])" : ",");
      }

      /* ugly: decide if there are any more symbols */
      
      for (j = i+1; j < p->num_tables && p->tables[j] == NULL; j++);
      if (j < p->num_tables && p->tables[j] != NULL)	o<<",\n";
    }
  }
  o<<"]).\n";
    
}

void InterpContainer::p_interp(Interp p) {
  fprint_interp_standard(cout, p);
}  /* p_interp */


void InterpContainer::fprint_interp_cooked(ostream &o, Interp p) {
  TermContainer T;
  ListtermContainer LT;
  SymbolContainer S;
  ParseContainer P;
  int i;
  if (p->comments) {
    Term comment = p->comments;
    while (LT.cons_term(comment)) {
      o<<"% ";
	  P.fwrite_term(o, ARG(comment,0));
      o<<"\n";
      comment = ARG(comment,1);
    }
  }
  o<<"\n% Interpretation of size "<<p->size<<"\n";
  

  for (i = 0; i < p->num_tables; i++) {
    int *table = p->tables[i];
    bool function = (p->types[i] == FUNCTION);
    if (table != NULL) {
      int j, n;
      int arity = S.sn_to_arity(i);
	  o<<"\n";
      if (arity == 0) {
		if (table[0] == -1) o<<S.sn_to_str(i)<<" = -.\n";
		else o<<S.sn_to_str(i)<<" = "<<table[0]<<".\n";
      }
      else {
			int *a = (int *) malloc(arity * sizeof(int));
			n = int_power(p->size, arity);
			for (j = 0; j < n; j++) {
				int k;
				compute_args(a, arity, p->size, j);
				if (function) {
					o<<S.sn_to_str(i)<<"(";
                    for (k = 0; k < arity; k++)
					o<<a[k]<<(k == arity-1 ? "" : ",");
					if (table[j] == -1) o<<") = -.\n";
					else o<<") = " <<table[j]<<".\n";
				}
				else {
					 o<<(table[j] ? " " : S.not_sym())<<" "<<S.sn_to_str(i)<<"(";
					 for (k = 0; k < arity; k++) o<<a[k]<<(k == arity-1 ? "" : ","); 
					 o<<").\n";
				}
	 }
	free(a);
   }
  }
 }
} 



void InterpContainer::fprint_interp_tabular(ostream &o, Interp p) {
  TermContainer T;
  ParseContainer P;
  SymbolContainer S;
  ListtermContainer LT;
  int f, i, j;

  if (p->comments) {
    Term comment = p->comments;
    while (LT.cons_term(comment)) {
      o << "% ";
	  P.fwrite_term(o, ARG(comment,0));
      o << "\n";
	  comment = ARG(comment,1);
    }
  }
  o << "\n% Interpretation of size " << p->size << "\n";

  for (f = 0; f < p->num_tables; f++) {
    int *table = p->tables[f];
    if (table != NULL) {
      int n = p->size;
      int arity = S.sn_to_arity(f);
      o << "\n " << S.sn_to_str(f) << " : ";

      if (arity == 0)
    	  o << table[0] << "\n";
	  else if (arity == 1) {
		o << "\n        ";
		for (i = 0; i < n; i++)
		  o << std::setw(2) << i;
		o << "\n    ----";
		for (i = 0; i < n; i++)
		  o << "--";
		o << "\n        ";
		for (i = 0; i < n; i++) {
			if (table[i] == -1)
				o<< " -";
			else
				o << setw(2) << table[i];
		}
		o << "\n";
      }
      else if (arity == 2) {
		o << "\n       |";
		for (i = 0; i < n; i++)
			o << setw(2) << i;
		o << "\n    ---+";
		for (i = 0; i < n; i++)
			o << "--";
	  	for (i = 0; i < n; i++) {
			o << "\n    " << setw(2) << i << " |";
			for (j = 0; j < n; j++) {
				if (table[I2(n,i,j)] == -1)
					o << " -";
				else
					o << setw(2) << table[I2(n,i,j)];
			}
	}
	o << "\n";
	
   } 
   else {
	int m = int_power(n, arity);
	o << "[";
	for (i = 0; i < m; i++) {
	  if (table[i] == -1) o << "-" << (i == m-1 ? "]\n" : ",");
	  else o << table[i] << (i == m-1 ? "]\n" : ",");
	}
   }
  }
  }  /* for each function or relation */
} 


void InterpContainer::fprint_interp_raw(ostream &o, Interp p) {
  TermContainer T;
  ParseContainer P;
  SymbolContainer S;
  ListtermContainer LT;
  int f, i;

  if (p->comments) {
    Term comment = p->comments;
    while (LT.cons_term(comment)) {
	  o<<"% ";
      P.fwrite_term(o, ARG(comment,0));
      o<<"\n";
      comment = ARG(comment,1);
    }
  }
  o << "\n% Interpretation of size "<<p->size<<"\n";
  for (f = 0; f < p->num_tables; f++) {
    int *table = p->tables[f];
    if (table != NULL) {
      int n = p->size;
      int arity = S.sn_to_arity(f);
      int m = int_power(n, arity);
      bool function = (p->types[f] == FUNCTION);
      o << "\n% " << (function ? "Function" : "Relation") << " "  << S.sn_to_str(f) <<" / " << arity << " : \n\n";

      for (i = 0; i < m; i++) {
		if (table[i] == -1) o<<"  -";
		else o << " " << setw(2) << table[i];
		if (i % n == n-1)	o<<"\n";
      }
      if (arity == 0) o<<"\n";
    }
  }  /* for each function or relation */
} 

int InterpContainer::eval_term_ground(Term t, Interp p, int *vals) {
  TermContainer T;
  if (VARIABLE(t))   return vals[VARNUM(t)];
  else {
    int n = p->size;
    int sn = SYMNUM(t);
    int domain_element;

    if (CONSTANT(t) && T.term_to_int(t, &domain_element)) {
      if (domain_element < 0 || domain_element >= n) {
		cout<<"ready to abort, bad term: "; T.p_term(t);
		fatal::fatal_error("eval_term_ground, domain element out of range");
      }
      return domain_element;
    }
    else {
      int *table;
      int i, j, mult;

      if (sn >= p->num_tables || p->tables[sn] == NULL) {
		cout<<"ready to abort, bad term: ";
        T.p_term(p->t);
		fatal::fatal_error("eval_term_ground, symbol not in interpretation");
      }

      table = p->tables[sn];

      j = 0;     /* we'll build up the index with j */
      mult = 1;  
      for (i = ARITY(t)-1; i >= 0; i--) {
			int v = eval_term_ground(ARG(t,i), p, vals);
			j += v * mult;
			mult = mult * n;
      }
      return table[j];
    }
  }
} 


bool InterpContainer::eval_literals_ground(Literals lits, Interp p, int *vals) {
  SymbolContainer S;
  Literals lit;
  bool atom_val, true_literal;

  true_literal = false;
  for (lit = lits; lit && !true_literal; lit = lit->next) {

    if (S.is_eq_symbol(SYMNUM(lit->atom)))
      atom_val = (eval_term_ground(ARG(lit->atom,0), p, vals) ==
		  eval_term_ground(ARG(lit->atom,1), p, vals));
    else
      atom_val = eval_term_ground(lit->atom, p, vals);

    true_literal = (lit->sign ? atom_val : !atom_val);
  }
  return true_literal;
} 

bool InterpContainer::all_recurse(Literals lits, Interp p, int *vals, int nextvar, int nvars) {
  if (nextvar == nvars)    return eval_literals_ground(lits, p, vals);
  else if (vals[nextvar] >= 0)  return all_recurse(lits, p, vals, nextvar+1, nvars);
  else {
    int i, rc;
    for (i = 0; i < p->size; i++) {
      vals[nextvar] = i;
      rc = all_recurse(lits, p, vals, nextvar+1, nvars);
      if (!rc)	return false;
    }
    vals[nextvar] = -1;
    return true;
  }
}


bool InterpContainer::eval_literals(Literals lits, Interp p) {
  
  int vals[MAX_VARS_EVAL];
  int nvars, i;
  bool rc;

  nvars = LADRV_GLOBAIS_INST.Lit.greatest_variable_in_clause(lits) + 1;
  if (nvars > MAX_VARS_EVAL)
    fatal::fatal_error("eval_literals: too many variables");

  for (i = 0; i < nvars; i++)
    vals[i] = -1;

  rc = all_recurse(lits, p, vals, 0, nvars);

  return rc;
} 


int InterpContainer::all_recurse2(Literals lits, Interp p, int *vals, int nextvar, int nvars) {
  if (nextvar == nvars) {
    return eval_literals_ground(lits, p, vals) ? 1 : 0;
  }
  else if (vals[nextvar] >= 0)
    return all_recurse2(lits, p, vals, nextvar+1, nvars);
  else {
    int i;
    int true_instances = 0;
    for (i = 0; i < p->size; i++) {
      vals[nextvar] = i;
      true_instances += all_recurse2(lits, p, vals, nextvar+1, nvars);
    }
    vals[nextvar] = -1;
    return true_instances;
  }
} 

int InterpContainer::eval_literals_true_instances(Literals lits, Interp p) {
 
  int vals[MAX_VARS_EVAL];
  int nvars, i, true_instances;

  nvars = LADRV_GLOBAIS_INST.Lit.greatest_variable_in_clause(lits) + 1;
  if (nvars > MAX_VARS_EVAL)
    fatal::fatal_error("eval_literals_true_instances: too many variables");

  for (i = 0; i < nvars; i++)
    vals[i] = -1;

  true_instances = all_recurse2(lits, p, vals, 0, nvars);

  return true_instances;
}

int InterpContainer::eval_literals_false_instances(Literals lits, Interp p) {

  int true_instances = eval_literals_true_instances(lits, p);
  int nvars = LADRV_GLOBAIS_INST.Lit.greatest_variable_in_clause(lits) + 1;
  return int_power(p->size, nvars) - true_instances;
} 

int InterpContainer::eval_fterm_ground(Term t, Interp p, int *vals) {
  TermContainer T;
  if (VARIABLE(t))
    fatal::fatal_error("eval_fterm_ground, VARIABLE encountered.");

  if (vals[SYMNUM(t)] != -1)
    return vals[SYMNUM(t)];

  else {
    int n = p->size;
    int sn = SYMNUM(t);
    int domain_element;

    if (CONSTANT(t) && T.term_to_int(t, &domain_element)) {
      if (domain_element < 0 || domain_element >= n) {
		cout<<"ready to abort, bad term:"; T.p_term(t);
		fatal::fatal_error("eval_fterm_ground, domain element out of range");
	  } 
      return domain_element;
    }
    else {
      int *table;
      int i, j, mult;

      if (sn >= p->num_tables || p->tables[sn] == NULL) {
		cout<<"ready to abort, bad term: "; T.p_term(t);
		fatal::fatal_error("eval_fterm_ground, symbol not in interpretation");
      }

      table = p->tables[sn];

      j = 0;     /* we'll build up the index with j */
      mult = 1;  
      for (i = ARITY(t)-1; i >= 0; i--) {
		int v = eval_fterm_ground(ARG(t,i), p, vals);
		j += v * mult;
		mult = mult * n;
      }
      return table[j];
    }
  }
} 


bool InterpContainer::eval_form(Formula f, Interp p, int vals[]) {
  SymbolContainer S;
  if (f->type == Ftype::ATOM_FORM) {
    if (S.is_eq_symbol(SYMNUM(f->atom)))      return (eval_fterm_ground(ARG(f->atom,0), p, vals) ==    eval_fterm_ground(ARG(f->atom,1), p, vals));
    else    return eval_fterm_ground(f->atom, p, vals);
  }
  else if (f->type == Ftype::ALL_FORM) {
    /* ok if true for every element of domain */
    int i;
    bool ok = true;
    int sn = S.str_to_sn(*(f->qvar), 0);
    int saved_value = vals[sn];  /* in case in scope of this variable */
    for (i = 0; i < p->size && ok; i++) {
      vals[sn] = i;
      if (!eval_form(f->kids[0], p, vals)) ok = false;
	}
    vals[sn] = saved_value;
    return ok;
  }
  else if (f->type == Ftype::EXISTS_FORM) {
    /* ok if true for any element of domain */
    int i;
    bool ok = false;
    int sn = S.str_to_sn(*(f->qvar), 0);
    int saved_value = vals[sn];  /* in case in scope of this variable */
    for (i = 0; i < p->size && !ok; i++) {
      vals[sn] = i;
      if (eval_form(f->kids[0], p, vals)) ok = true;
    }
    vals[sn] = saved_value;
    return ok;
  }
  else if (f->type == Ftype::AND_FORM) {
    int i;
    bool ok = true;
    for (i = 0; i < f->arity && ok; i++)
      if (!eval_form(f->kids[i], p, vals))
	ok = false;
    return ok;
  }
  else if (f->type == Ftype::OR_FORM) {
    int i;
    bool ok = false;
    for (i = 0; i < f->arity && !ok; i++)
      if (eval_form(f->kids[i], p, vals))
	ok = true;
    return ok;
  }
  else if (f->type == Ftype::NOT_FORM) {
    return !eval_form(f->kids[0], p, vals);
  }
  else if (f->type == Ftype::IFF_FORM) {
    return (eval_form(f->kids[0], p, vals) == eval_form(f->kids[1], p, vals));
  }
  else if (f->type == Ftype::IMP_FORM) {
    return (!eval_form(f->kids[0], p, vals) || eval_form(f->kids[1], p, vals));
  }
  else if (f->type == Ftype::IMPBY_FORM) {
    return (eval_form(f->kids[0], p, vals) || !eval_form(f->kids[1], p, vals));
  }
  else {
    fatal::fatal_error("eval_form, bad formula.");
    return 0;  /* to please the compiler */
  }
}  


bool InterpContainer::eval_formula(Formula f, Interp p) {
  FormulaContainer F;
  int a[MAX_VARS_EVAL], *vals;
  int nsyms, i;
  bool rc;

  nsyms = F.greatest_symnum_in_formula(f) + 1;
  if (nsyms > MAX_VARS_EVAL)
    vals = (int *) malloc((nsyms * sizeof(int)));
  else
    vals = a;

  for (i = 0; i < nsyms; i++)
    vals[i] = -1;

  rc = eval_form(f, p, vals);

  if (nsyms > MAX_VARS_EVAL)    free(vals);

#if 0
  if (rc) cout<<"Formula is TRUE in this interpretation."<<endl;
    
  else  cout <<"Formula is FALSE in this interpretations."<<endl;

  
#endif
  return rc;
} 


Term InterpContainer::interp_remove_constants_recurse(Term ops) {
  TermContainer T;
  SymbolContainer S;
  ListtermContainer LT;
  if (LT.nil_term(ops))   return ops;
  else {
    if (S.sn_to_arity(SYMNUM(ARG(ARG(ops,0),0))) == 0) {
      T.zap_term(ARG(ops,0));  /* deep */
      T.free_term(ops);        /* shallow */
      return interp_remove_constants_recurse(ARG(ops,1));
    }
    else {
      ARG(ops,1) = interp_remove_constants_recurse(ARG(ops,1));
      return ops;
    }
  }
} 

void InterpContainer::interp_remove_constants(Term t) {
  ARG(t,2) = interp_remove_constants_recurse(ARG(t,2));
} 


Term InterpContainer::interp_remove_others_recurse(Term ops, Plist keepers) {
  TermContainer T;
  SymbolContainer S;
  PlistContainer P;
  ListtermContainer LT;
  if (LT.nil_term(ops))   return ops;
  else {
      P.set_head(keepers);
	  if (!P.string_member_plist(S.sn_to_str(SYMNUM(ARG(ARG(ops,0),0))))) {
      T.zap_term(ARG(ops,0));  /* deep */
      T.free_term(ops);        /* shallow */
      return interp_remove_others_recurse(ARG(ops,1), keepers);
    }
    else {
      ARG(ops,1) = interp_remove_others_recurse(ARG(ops,1), keepers);
      return ops;
    }
  }
}

void InterpContainer::interp_remove_others(Term t, Plist keepers) {
  ARG(t,2) = interp_remove_others_recurse(ARG(t,2), keepers);
} 


Interp InterpContainer::copy_interp(Interp p) {
  TermContainer T;
  int i, j;
  Interp q = get_interp();

  q->t = T.copy_term(p->t);
  q->comments = T.copy_term(p->comments);
  q->size = p->size;
  q->incomplete = p->incomplete;
  q->num_tables = p->num_tables;

  /* discriminators */

  q->num_discriminators = p->num_discriminators;
  q->discriminator_counts = (int *) malloc(sizeof(int) * q->num_discriminators);
  for (i = 0; i < q->num_discriminators; i++)
    q->discriminator_counts = p->discriminator_counts;
  
  /* occurrences */

  q->occurrences =(int *) malloc(q->size * sizeof(int));
  for (i = 0; i < q->size; i++)
    q->occurrences[i] = p->occurrences[i];

  /* blocks */

  q->blocks = (int *) malloc(q->size * sizeof(int));
  for (i = 0; i < q->size; i++)
    q->blocks[i] = p->blocks[i];

  /* profile */

  q->num_profile_components = p->num_profile_components;
  q->profile = (int **) malloc(sizeof(int *) * q->size);
  for (i = 0; i < q->size; i++) {
    q->profile[i] = (int *) malloc(sizeof(int) * q->num_profile_components);
    for (j = 0; j < q->num_profile_components; j++)
      q->profile[i][j] = p->profile[i][j];
  }

  /* types, arities */

  q->types =(int *)    malloc(q->num_tables * sizeof(int));
  q->arities = (int *) malloc(q->num_tables * sizeof(int));
  for (i = 0; i < q->num_tables; i++) {
    q->types[i] = p->types[i];
    q->arities[i] = p->arities[i];
  }

  /* tables */

  q->tables =(int **) malloc(q->num_tables * sizeof(int *));
  for (i = 0; i < q->num_tables; i++)
    q->tables[i] = NULL;

  for (i = 0; i < q->num_tables; i++)
    if (p->tables[i] != NULL) {
      int arity = p->arities[i];
      int n = 1;
      int *ptable, *qtable, j;
      for (j = 0; j < arity; j++)	n = n * p->size;
      q->tables[i] = (int *) malloc(n * sizeof(int));
      ptable = p->tables[i];
      qtable = q->tables[i];
      for (j = 0; j < n; j++)	qtable[j] = ptable[j];
    }

  return q;
} 


Interp InterpContainer::permute_interp(Interp source, int *p) {
  TermContainer T;
  Interp dest = copy_interp(source);
  int n = source->size;
  int f;
  for (f = 0; f < source->num_tables; f++) {
    if (source->tables[f] != NULL) {
      int *st = source->tables[f];
      int *dt =   dest->tables[f];
      int arity = source->arities[f];
      bool function = (source->types[f] == FUNCTION);
      if (arity == 0)	dt[0] = (function ? p[st[0]] : st[0]);
      else if (arity == 1) {
		int i;
		for (i = 0; i < n; i++)
		dt[p[i]] = (function ? p[st[i]] : st[i]);
      }
      else if (arity == 2) {
		int i, j;
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)	dt[I2(n,p[i],p[j])] = (function  ? p[st[I2(n,i,j)]]  : st[I2(n,i,j)]);
      }
      else if (arity == 3) {
		int i, j, k;
		for (i = 0; i < n; i++)
		 for (j = 0; j < n; j++)
		  for (k = 0; k < n; k++)   dt[I3(n,p[i],p[j],p[k])] = (function ? p[st[I3(n,i,j,k)]] : st[I3(n,i,j,k)]);
      }
      else
		fatal::fatal_error("permute_interp: arity > 3");
    }
  }
  {
    int i;
    for (i = 0; i < n; i++) {
      dest->occurrences[p[i]] = source->occurrences[i];
      myOrder::copy_vec(source->profile[i], dest->profile[p[i]], source->num_profile_components);
    }
  }
  /* The term representation is no longer correct. */
  if (dest->t)
    T.zap_term(dest->t);
  dest->t = NULL;
  return dest;
}  


bool InterpContainer::ident_interp_perm(Interp a, Interp b, int *p) {
  int n = a->size;
  int f;
  for (f = 0; f < a->num_tables; f++) {
    if (a->tables[f] != NULL) {
      int *at =   a->tables[f];
      int *bt =   b->tables[f];
      int arity = a->arities[f];
      bool function = (a->types[f] == FUNCTION);
      if (arity == 0) {
		if (bt[0] != (function ? p[at[0]] : at[0])) return false;
      }
      else if (arity == 1) {
		int i;
		for (i = 0; i < n; i++) {
			if (bt[p[i]] != (function ? p[at[i]] : at[i]))    return false;
		}
      } 
      else if (arity == 2) {
		int i, j;
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++) {
				if (bt[I2(n,p[i],p[j])] != (function
					? p[at[I2(n,i,j)]]
					: at[I2(n,i,j)])) return false;
		    }
      }
      else if (arity == 3) {
		int i, j, k;
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)
				for (k = 0; k < n; k++) {
					if (bt[I3(n,p[i],p[j],p[k])] != (function
					       ? p[at[I3(n,i,j,k)]]
					       : at[I3(n,i,j,k)])) return false;
	    }
      }
      else fatal::fatal_error("ident_interp_perm: arity > 3");
    }
  }
  return true;
} 



Interp InterpContainer::normal_interp(Interp a) {
  int i;
  int *occ  =(int *) malloc(sizeof(int) * a->size);  /* remember to free this */
  int *perm =(int *)malloc(sizeof(int) * a->size);  /* remember to free this */
  int size = a->size;
  Interp can;

  /* Determine the permutation we'll use to normalize
     the interpretation. */

  for (i = 0; i < size; i++)   occ[i] = a->occurrences[i];

  for (i = 0; i < size; i++) {
    int max = -1;
    int index_of_max = -1;
    int j;
    for (j = 0; j < size; j++) {
      if (occ[j] > max) {
		index_of_max = j;
		max = occ[j];
      }
    }
    perm[index_of_max] = i;
    occ[index_of_max] = -1;
  }

  free(occ);  /* This is now useless (all members are -1). */

  /* Apply the permutation to the interpretation. */
  
  can = permute_interp(a, perm);
  free(perm);
  return can;
} 



bool InterpContainer::iso_interp_recurse(int *p, int k, int n,Interp a, Interp b, bool normal) {
  int i;
  if (k == n) {
    /* We have a permutation. */
    LADR_GLOBAL_INTERP.Iso_perms++;
    return ident_interp_perm(a, b, p);
  }
  else {
    /* Continue building permutations. */
    if (iso_interp_recurse(p, k+1, n, a, b, normal))  return true;
    for (i = k+1; i < n; i++) {
      /* If normal, and if i and k are in the same block,
	 don't swap them. */
      if (!normal || a->blocks[i] == a->blocks[k]) {
		ISWAP(p[k], p[i]);
		if (iso_interp_recurse(p, k+1, n, a, b, normal)) return true;
		ISWAP(p[k], p[i]);
      }
    }
    return false;
  }
} 


bool InterpContainer::isomorphic_interps(Interp a, Interp b, bool normal) {
  bool isomorphic;
  int *perm;
  if (a->size != b->size)  return false;

  /* If normal, make sure the interps have the same profiles. */

  if (normal) {
    if (!same_profiles(a, b)) return false;
  }

  if (!same_discriminator_counts(a, b)) return false;
  LADR_GLOBAL_INTERP.Iso_checks++;
  perm = trivial_permutation(a->size);  /* remember to free this */
  isomorphic = iso_interp_recurse(perm, 0, a->size, a, b, normal);
  free(perm);
  return isomorphic;
} 



int InterpContainer::interp_size(Interp a) {
  return a->size;
}


Term InterpContainer::interp_comments(Interp a) {
  return a->comments;
}


int * InterpContainer::interp_table(Interp p, string sym, int arity) {
  SymbolContainer S;
  int f;
  for (f = 0; f < p->num_tables; f++)
    if (S.is_symbol(f, sym, arity))
      return p->tables[f];
  return NULL;
} 


long unsigned InterpContainer::iso_checks(void) {
  return LADR_GLOBAL_INTERP.Iso_checks;
}

long unsigned InterpContainer::iso_perms(void) {
  return LADR_GLOBAL_INTERP.Iso_perms;
} 

bool InterpContainer::evaluable_term(Term t, Interp p) {
  TermContainer T;
  if (VARIABLE(t))    return true;
  else {
    int domain_element;
    if (CONSTANT(t) && T.term_to_int(t, &domain_element))
      return domain_element >= 0 && domain_element < p->size;
    else if (SYMNUM(t) >= p->num_tables || p->tables[SYMNUM(t)] == NULL) {   return false;  }
    else {
      int i;
      bool ok;
      for (i = 0, ok = true; i < ARITY(t) && ok; i++) ok = evaluable_term(ARG(t,i), p);
      return ok;
    }
  }
} 


bool InterpContainer::evaluable_atom(Term a, Interp p) {
  SymbolContainer S;
  TermContainer T;
  if (S.is_eq_symbol(SYMNUM(a)))
    return evaluable_term(ARG(a,0), p) && evaluable_term(ARG(a,1), p);
  else {
    int b;
    if (CONSTANT(a) && T.term_to_int(a, &b))
      return b == 0 || b == 1;
    else if (SYMNUM(a) >= p->num_tables || p->tables[SYMNUM(a)] == NULL) {
		return false;
    }
    else {
      int i;
      bool ok;
      for (i = 0, ok = true; i < ARITY(a) && ok; i++) ok = evaluable_term(ARG(a,i), p);
      return ok;
    }
  }
} 



bool InterpContainer::evaluable_literals(Literals lits, Interp p){
  if (lits == NULL)   return true;
  else   return (evaluable_atom(lits->atom, p)  &&   evaluable_literals(lits->next, p));
}


bool InterpContainer::evaluable_formula(Formula f, Interp p) {
  if (f->type == Ftype::ATOM_FORM) return evaluable_atom(f->atom, p);
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (!evaluable_formula(f->kids[i], p))
	return false;
    return true;
  }
}

bool InterpContainer::evaluable_topform(Topform tf, Interp p)
{
  if (tf->is_formula)
    return evaluable_formula(tf->formula, p);
  else
    return evaluable_literals(tf->literals, p);
} 

void InterpContainer::update_interp_with_constant(Interp p, Term constant, int val) {
  int sn = SYMNUM(constant);
  cout<<"NOTE: sn="<<sn<<", num_tables="<<p->num_tables<<endl;
  if (sn >= p->num_tables)
    fatal::fatal_error("update_interp_with_constat, not enough tables");
  else if (p->tables[sn] != NULL)
    fatal::fatal_error("update_interp_with_constat, table not NULL");
  else {
    p->tables[sn] = (int *) malloc(sizeof(int));
    p->tables[sn][0] = val;
  }
}



bool InterpContainer::eval_topform(Topform tf, Interp p) {
  if (tf->is_formula)
    return eval_formula(tf->formula, p);
  else
    return eval_literals(tf->literals, p);
} 


OrderType InterpContainer::compare_interp(Interp a, Interp b) {
  int f;
  for (f = 0; f < a->num_tables; f++) {
    if (a->tables[f] != NULL) {
      int *at =   a->tables[f];
      int *bt =   b->tables[f];
      int n = int_power(a->size, a->arities[f]);
      int i;
      for (i = 0; i < n; i++) {
	if (at[i] < bt[i])
	  return OrderType::LESS_THAN;
	else if (at[i] > bt[i])
	  return OrderType::GREATER_THAN;
      }
    }
  }
  return OrderType::SAME_AS;
}


bool InterpContainer::ident_interp(Interp a, Interp b) {
  return compare_interp(a,b) == OrderType::SAME_AS;
} 


OrderType InterpContainer::compare_ints(int a, int b) {
  if (a < b)
    return OrderType::LESS_THAN;
  else if (a > b)
    return OrderType::GREATER_THAN;
  else
    return OrderType::SAME_AS;
} 

void InterpContainer::invert_perm(int *a, int *b, int n) {
  int i;
  for (i = 0; i < n; i++)
    b[a[i]] = i;
}

void InterpContainer::copy_perm(int *a, int *b, int n) {
  int i;
  for (i = 0; i < n; i++)
    b[i] = a[i];
} 


OrderType InterpContainer::compare_permed_interps(int *x, int *y, int *xx, int *yy, Interp a) {
  int n = a->size;
  int f;
  for (f = 0; f < a->num_tables; f++) {
    if (a->tables[f] != NULL) {
      int *t =   a->tables[f];
      int arity = a->arities[f];
      bool function = (a->types[f] == FUNCTION);
      if (arity == 0) {
		OrderType result;
		if (function)  result = compare_ints(x[t[0]], y[t[0]]);
		else  result = OrderType::SAME_AS;
		if (result != OrderType::SAME_AS)	  return result;
      }
      else if (arity == 1) {
		int i;
		for (i = 0; i < n; i++) {
			OrderType result;
			if (function)   result = compare_ints(x[t[xx[i]]], y[t[yy[i]]]);
			else    result = compare_ints(t[xx[i]], t[yy[i]]);
			if (result != OrderType::SAME_AS)    return result;
		}
      }
      else if (arity == 2) {
		int i, j;
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++) {
			OrderType result;
			if (function)     result = compare_ints(x[t[I2(n,xx[i],xx[j])]], y[t[I2(n,yy[i],yy[j])]]);
			else    result = compare_ints(t[I2(n,xx[i],xx[j])], t[I2(n,yy[i],yy[j])]);
			if (result != OrderType::SAME_AS)     return result;
	   }
      }
      else if (arity == 3) {
		int i, j, k;
		for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			for (k = 0; k < n; k++) {
				OrderType result;
				if (function) result = compare_ints(x[t[I3(n,xx[i],xx[j],xx[k])]],y[t[I3(n,yy[i],yy[j],yy[k])]]);
				else result = compare_ints(t[I3(n,xx[i],xx[j],xx[k])],t[I3(n,yy[i],yy[j],yy[k])]);
				if (result != OrderType::SAME_AS)	return result;
	    }
      }
      else
	   fatal::fatal_error("compare_permed_interps: arity > 3");
    }
  }
  return OrderType::SAME_AS;
} 

void InterpContainer::canon_recurse(int k, int *perm, int *best, int *perm1, int *best1,  Interp a) {
  /* 
     k: current position in working permutation
     perm:  working permutation
     best:  best permutation so far
     perm1: inverse of perm
     best1: inverse of best
     a: base interp
   */
  int n = a->size;
  if (k == n) {
    /* We have a permutation. */
    LADR_GLOBAL_INTERP.Iso_perms++;
    invert_perm(perm, perm1, a->size);
    if (compare_permed_interps(perm, best, perm1, best1, a) == OrderType::LESS_THAN) {
      /* copy working permutation to best-so-far. */
      copy_perm(perm, best, a->size);
      copy_perm(perm1, best1, a->size);
    }
  }
  else {
    /* Continue building permutations. */
    int i;
    canon_recurse(k+1, perm, best, perm1, best1, a);
    for (i = k+1; i < n; i++) {
      /* If i and k are in different blocks, don't swap them. */
      if (a->blocks[i] == a->blocks[k]) {
		ISWAP(perm[k], perm[i]);
		canon_recurse(k+1, perm, best, perm1, best1, a);
		ISWAP(perm[k], perm[i]);
      }
    }
  }
} 


Interp InterpContainer::canon_interp(Interp a) {
  Interp canon;

  int *perm  = trivial_permutation(a->size);  /* remember to free this */
  int *best  = trivial_permutation(a->size);  /* remember to free this */
  int *perm1 = trivial_permutation(a->size);  /* remember to free this */
  int *best1 = trivial_permutation(a->size);  /* remember to free this */

  invert_perm(best, best1, a->size);  /* let best1 be the inverse of best */
  /* perm gets inverted when needed */

  canon_recurse(0, perm, best, perm1, best1, a);

  canon = permute_interp(a, best);  /* makes new copy */

  free(perm);
  free(best);
  free(perm1);
  free(best1);

  return canon;
}



void InterpContainer::assign_discriminator_counts(Interp a, Plist discriminators) {
  PlistContainer P;
  P.set_head(discriminators);
  int n = P.plist_count();
  int *counts = (int *) malloc(sizeof(int) * n);
  int i;
  Plist p;
  for (p = discriminators, i = 0; p; p = p->next, i++) {
    Topform c = (Topform) p->v;
    counts[i] = eval_literals_true_instances(c->literals, a);
  }
  a->discriminator_counts = counts;
  a->num_discriminators = n;
} 

bool InterpContainer::same_discriminator_counts(Interp a, Interp b){
  if (a->num_discriminators != b->num_discriminators)
    fatal::fatal_error("different number of discriminators");
  return (myOrder::compare_vecs(a->discriminator_counts,b->discriminator_counts,a->num_discriminators) == OrderType::SAME_AS);
} 


void InterpContainer::update_profile(Topform c, Interp a, int *next)
     /* vecs[domain_element][profile_component] */
{

  int vals[MAX_VARS_EVAL];
  int nvars, v, i, true_instances;

  nvars = LADRV_GLOBAIS_INST.Lit.greatest_variable_in_clause(c->literals) + 1;
  if (nvars > MAX_VARS_EVAL)
    fatal::fatal_error("update_profile: too many variables");

  for (v = 0; v < nvars; v++)
    vals[v] = -1;

  for (v = 0; v < nvars; v++) {
    for (i = 0; i < a->size; i++) {
      vals[v] = i;
      true_instances = all_recurse2(c->literals, a, vals, 0, nvars);
      a->profile[i][*next] = true_instances;
    }
    vals[v] = -1;
    (*next)++;
  }
} 

void InterpContainer::create_profile(Interp a, Plist discriminators) {
 
  int i, next;
  Plist p;

  a->num_profile_components = 1;  /* first is occurrences */

  for (p = discriminators; p; p = p->next) {
    Topform c =(Topform) p->v;
    int nvars = LADRV_GLOBAIS_INST.Lit.greatest_variable_in_clause(c->literals) + 1;
    a->num_profile_components += nvars;
  }
  for (i = 0; i < a->size; i++) {
    a->profile[i] = (int *) malloc(sizeof(int) * a->num_profile_components);
    a->profile[i][0] = a->occurrences[i];
  }

  next = 1;
  for (p = discriminators; p; p = p->next)
    update_profile((Topform)p->v, a, &next);

  if (next != a->num_profile_components)
    fatal::fatal_error("create_profile, counts do not match");
} 



void InterpContainer::p_interp_profile(Interp a, Plist discriminators) {

  SymbolContainer S;
  
  int i, j, k;
  string str;
  Plist p;
  cout<<"\n========================== PROFILE\n";
  fprint_interp_standard2(cout, a);
  if (discriminators) {
    cout<<"\n     blocks:             ";
	for (k = 0; k < a->size; k++)
      cout<<" "<<setw(3)<<'A'+a->blocks[k];
    cout<<"\n";
    cout<<"Permutaitons: "<<perms_required(a)<<"\n";
	cout<<"occurrences:             ";
    
    for (k = 0; k < a->size; k++)
      cout<<" "<<setw(3)<<a->profile[k][0];
	cout<<"\n";
    i = 1;
    for (p = discriminators; p; p = p->next) {
      Topform c = (Topform) p->v;
      int nvars = LADRV_GLOBAIS_INST.Lit.greatest_variable_in_clause(c->literals) + 1;
      Ioutil::fwrite_clause(cout, c, (int) Clause_print_format::CL_FORM_BARE);
      for (j = 0; j < nvars; j++, i++) {
		S.symbol_for_variable(str, j);
		cout<< "          "<<str<<":             ";      
		for (k = 0; k < a->size; k++) {
			cout<<" "<<setw(3)<<a->profile[k][i];
		}
		cout <<"\n";
      }
    }
  }
  else {
    for (i = 0; i < a->num_profile_components; i++) {
      for (k = 0; k < a->size; k++)
		cout<<" "<<setw(2)<<a->profile[k][i];
	  cout<<"\n";
    }
  }
} 

Interp InterpContainer::normal3_interp(Interp a, Plist discriminators) {
  int i, b;
  int **prof = (int **)malloc(sizeof(int *) * a->size);  /* remember to free this */
  int *perm  = (int *) malloc(sizeof(int) * a->size);  /* remember to free this */
  int size = a->size;
  Interp norm;

  create_profile(a, discriminators);

  /* Determine the permutation we'll use to normalize
     the interpretation. */

  for (i = 0; i < size; i++)
    prof[i] = a->profile[i];

  for (i = 0; i < size; i++) {
    int *max = NULL;
    int index_of_max = -1;
    int j;
    for (j = 0; j < size; j++) {
      if (prof[j] != NULL) {
	if (max == NULL ||
	    myOrder::compare_vecs(prof[j],
			 max, a->num_profile_components) == OrderType::GREATER_THAN) {
	  index_of_max = j;
	  max = prof[j];
	}
      }
    }
    perm[index_of_max] = i;
    prof[index_of_max] = NULL;
  }

  free(prof);  /* This is now useless (all members are NULL). */

  /* Apply the permutation to the interpretation. */

  norm = permute_interp(a, perm);
  free(perm);

  /* Set up blocks of identical profile components.  Note that
     identical profile components are alreay adjacent.
     The blocks are specified by a vector of integers, e.g.,
     [1,1,1,1,2,3,3,4] says there are 4 blocks (domain size 8),
     with the first 4 identical, etc.
  */

  b = 0;  /* block counter */

  norm->blocks[0] = 0;
  for (i = 1; i < norm->size; i++) {
    if (myOrder::compare_vecs(norm->profile[i-1], norm->profile[i],
		     norm->num_profile_components) != OrderType::SAME_AS)
      b++;
    norm->blocks[i] = b;
  }
  return norm;
}

bool InterpContainer::same_profiles(Interp a, Interp b)
{
  int i, j;
  for (i = 0; i < a->size; i++)
    for (j = 0; j < a->num_profile_components; j++)
      if (a->profile[i][j] != b->profile[i][j])
	return false;
  return true;
} 


long unsigned InterpContainer::perms_required(Interp a)
{
  int i, n;
  long unsigned p, r;
  p = 1;
  i = 0;
  while (i < a->size) {
    int c = a->blocks[i];
    n = 0;
    while (i < a->size && a->blocks[i] == c) {
      i++;
      n++;
    }
    r = factorial(n);
    if (r < 1 || r == ULONG_MAX)
      return 0;
    r = p * r;
    if (r < p || r == ULONG_MAX)
      return 0;
    p = r;
  }
  return p;
} 

long unsigned InterpContainer::factorial(int n) {
  long unsigned f, x;
  int i;
  f = 1;
  for (i = 1; i <= n; i++) {
    x = f * i;
    if (x == ULONG_MAX || x < f)
      return 0;
    f = x;
  }
  return f;
} 
