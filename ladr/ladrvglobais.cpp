#include "ladrvglobais.h"




LadrVGlobais            LADR_V_GLOBAIS::ladr_v_globais;


LadrVGlobais::LadrVGlobais() {

}

LadrVGlobais::~LadrVGlobais() {

    
    
}

void LadrVGlobais::FREE_MEM() {
    
    //Free Globalsymbol MEM
    LADR_GLOBAL_OPTIONS.Free_Mem();
    LADR_GLOBAL_SYMBOL.Free_Mem();
    LADR_GLOBAL_PARSE.Free_Mem();
    LADR_GLOBAL_TOP_INPUT.Free_Mem();
    LADR_GLOBAL_FEATURES.Free_Mem();
    LADR_GLOBAL_TERM.Free_Mem();
    LADR_GLOBAL_ATTRIBUTE.Free_Mem();
 
}


