#ifndef MACE4_MACE4GLOBAIS_H
#define MACE4_MACE4GLOBAIS_H

#include "arithmetic.h"
#include "options.h"



class Mace4VGlobais {

//public properties
public:
  arithmetic    arith;
  Arithmetic    Arith;
  mace_options  opt;
  Mace_options  Opt;
  mace_local_options m_opts;
    
                                
//public methods
public:
  Mace4VGlobais();
  ~Mace4VGlobais();
};


#endif
