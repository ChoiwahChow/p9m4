cmake_minimum_required(VERSION 2.6)
project(prover9)

find_package(ZLIB)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(${CMAKE_SOURCE_DIR}/mace4/inc)
link_directories(${CMAKE_SOURCE_DIR}/mace4/lib)

add_library(libladr 
               ./ladr/banner.cpp ./ladr/nonport.cpp ./ladr/clock.cpp ./ladr/memory.cpp ./ladr/fatal.cpp ./ladr/options.cpp ./ladr/mystring.cpp ./ladr/attrib.cpp ./ladr/strbuf.cpp
               ./ladr/order.cpp ./ladr/ibuffer.cpp ./ladr/hash.cpp ./ladr/glist.cpp ./ladr/avltree.cpp ./ladr/symbols.cpp ./ladr/term.cpp ./ladr/ladrvglobais.cpp ./ladr/listterm.cpp ./ladr/parse.cpp ./ladr/tlist.cpp ./ladr/termflag.cpp
               ./ladr/multiset.cpp ./ladr/unify.cpp ./ladr/flatterm.cpp ./ladr/discrim.cpp ./ladr/accanon.cpp ./ladr/termorder.cpp ./ladr/basic.cpp ./ladr/fpalist.cpp  ./ladr/fpa.cpp ./ladr/dioph.cpp  
               ./ladr/btu.cpp ./ladr/btm.cpp ./ladr/discrimb.cpp ./ladr/discrimw.cpp ./ladr/mindex.cpp ./ladr/formula.cpp ./ladr/literals.cpp ./ladr/maximal.cpp ./ladr/topform.cpp ./ladr/clauseid.cpp ./ladr/just.cpp   
               ./ladr/definitions.cpp ./ladr/clauses.cpp ./ladr/features.cpp ./ladr/clist.cpp ./ladr/cnf.cpp ./ladr/clausify.cpp  ./ladr/parautil.cpp  ./ladr/compress.cpp ./ladr/pindex.cpp ./ladr/lindex.cpp  ./ladr/weight2.cpp
               ./ladr/complex.cpp ./ladr/weight.cpp ./ladr/int_code.cpp ./ladr/di_tree.cpp ./ladr/fastparse.cpp ./ladr/random.cpp ./ladr/subsume.cpp ./ladr/clause_misc.cpp ./ladr/clause_eval.cpp ./ladr/dollar.cpp ./ladr/flatdemod.cpp
               ./ladr/demod.cpp ./ladr/clash.cpp ./ladr/resolve.cpp ./ladr/paramod.cpp ./ladr/backdemod.cpp ./ladr/hints.cpp ./ladr/ac_redun.cpp ./ladr/xproofs.cpp ./ladr/std_options.cpp ./ladr/ivy.cpp ./ladr/ioutil.cpp 
               ./ladr/top_input.cpp ./ladr/interp.cpp
              )

add_executable(prover9 ./main.cpp ./provers.cpp ./search.cpp
               ./utilities.cpp ./actions.cpp ./semantics.cpp ./pred_elim.cpp ./index_lits.cpp ./demodulate.cpp ./forward_subsume.cpp ./unfold.cpp ./white_black.cpp ./giv_select.cpp
               ./prover9vglobais.cpp
               )


add_executable(mace4c ./mace4/smallprime.cpp ./mace4/mace4globalvalues.cpp ./mace4/options.cpp  ./mace4/mstats.h ./mace4/syms.cpp ./mace4/cell.cpp ./mace4/estack.cpp ./mace4/mstate.cpp
               ./mace4/mstats.cpp ./mace4/ground.cpp  ./mace4/arithmetic.cpp  ./mace4/negpropindex.cpp ./mace4/propagate.cpp ./mace4/select.cpp ./mace4/mace4vglobais.cpp ./mace4/msearch.cpp
               ./mace4/cmdline.cpp ./mace4/partition.cpp ./mace4/cube.cpp ./mace4/mace4.cpp ./mace4/main.cpp 
              )
              
add_executable(clausefilter ./apps/inputs_util.cpp ./apps/clausefilter.cpp)
              
add_executable(clausetester ./apps/inputs_util.cpp ./apps/clausetester.cpp)
              
add_executable(complex ./apps/complex.cpp)
              
add_executable(directproof ./apps/inputs_util.cpp ./apps/directproof.cpp)
              
add_executable(dprofiles ./apps/inputs_util.cpp ./apps/dprofiles.cpp)
              
add_executable(genterms ./apps/inputs_util.cpp ./apps/genterms.cpp)
              
add_executable(idfilter ./apps/inputs_util.cpp ./apps/idfilter.cpp)
              
add_executable(interpfilter ./apps/inputs_util.cpp ./apps/interpfilter.cpp)
              
add_executable(interpformat ./apps/inputs_util.cpp ./apps/interpformat.cpp)
              
add_executable(isofilter ./apps/inputs_util.cpp ./apps/isofilter.cpp)
              
add_executable(isofilter0 ./apps/inputs_util.cpp ./apps/isofilter0.cpp)
              
add_executable(isofilter2 ./apps/inputs_util.cpp ./apps/isofilter2.cpp)
              
add_executable(latfilter ./apps/inputs_util.cpp ./apps/latfilter.cpp)
              
add_executable(miniscope ./apps/inputs_util.cpp ./apps/miniscope.cpp)
              
add_executable(mirror-flip ./apps/inputs_util.cpp ./apps/mirror-flip.cpp)
              
add_executable(olfilter ./apps/inputs_util.cpp ./apps/olfilter.cpp)
              
add_executable(perm3 ./apps/inputs_util.cpp ./apps/perm3.cpp)

add_executable(prooftrans ./apps/inputs_util.cpp ./apps/prooftrans.cpp)
              
add_executable(renamer ./apps/inputs_util.cpp ./apps/renamer.cpp)
              
add_executable(rewriter ./apps/inputs_util.cpp ./apps/rewriter.cpp)
              
add_executable(sigtest ./apps/inputs_util.cpp ./apps/sigtest.cpp)
              
add_executable(test_complex ./apps/inputs_util.cpp ./apps/test_complex.cpp)
              
add_executable(test_clause_eval ./apps/inputs_util.cpp ./apps/test_clause_eval.cpp)
              
add_executable(unfast ./apps/inputs_util.cpp ./apps/unfast.cpp)
              
add_executable(upper-covers ./apps/inputs_util.cpp ./apps/upper-covers.cpp)

target_link_libraries(prover9 libladr)
target_link_libraries(mace4c libladr libmlex.a libisonaut.a -l:nauty.a libminisat.a)
target_link_libraries(mace4c PUBLIC ZLIB::ZLIB)
target_link_libraries(clausefilter libladr)
target_link_libraries(clausetester libladr)
target_link_libraries(complex libladr)
target_link_libraries(directproof libladr)
target_link_libraries(dprofiles libladr)
target_link_libraries(genterms libladr)
target_link_libraries(idfilter libladr)
target_link_libraries(interpfilter libladr)
target_link_libraries(interpformat libladr)
target_link_libraries(isofilter libladr)
target_link_libraries(isofilter0 libladr)
target_link_libraries(isofilter2 libladr)
target_link_libraries(latfilter libladr)
target_link_libraries(miniscope libladr)
target_link_libraries(mirror-flip libladr)
target_link_libraries(olfilter libladr)
target_link_libraries(perm3 libladr)
target_link_libraries(prooftrans libladr)
target_link_libraries(renamer libladr)
target_link_libraries(rewriter libladr)
target_link_libraries(sigtest libladr)
target_link_libraries(test_complex libladr)
target_link_libraries(test_clause_eval libladr)
target_link_libraries(unfast libladr)
target_link_libraries(upper-covers libladr)

install(TARGETS libladr DESTINATION lib)
install(TARGETS prover9 mace4c clausefilter clausetester complex directproof dprofiles genterms idfilter interpfilter interpformat 
        isofilter isofilter0 isofilter2 latfilter miniscope mirror-flip olfilter perm3 prooftrans renamer rewriter sigtest 
        test_clause_eval test_complex unfast upper-covers RUNTIME DESTINATION bin)
