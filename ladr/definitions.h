#ifndef TP_DEFINITIONS_H
#define TP_DEFINITIONS_H

#include "formula.h"
#include "topform.h"
#include "clauseid.h"
#include "just.h"

class DefinitionsContainer {
					
						private:
									Term subst_terms(Term, Plist, Plist , Plist);
									void subst_free_vars(Formula, Plist, Plist, Plist);
									Formula subst_atomic_formula(Formula, Formula, Formula);
									Formula strip_quantifiers(Formula);
									bool definition_applies(Formula, Formula);
									Topform first_definition(Plist);
						
						public:
								bool is_definition(Formula);
								Formula expand_with_definition(Formula, Formula);
								void process_definitions(Plist, Plist *,Plist *,Plist *);
								void expand_with_definitions(Plist,Plist,Plist *,Plist *);
								void separate_definitions(Plist,Plist *,Plist *);
};


#endif
