

#ifndef APPS_INPUTREADER_H
#define APPS_INPUTREADER_H

#include <istream>
#include <fstream>
#include "../ladr/clause_misc.h"
#include "../ladr/glist.h"
#include "../ladr/ioutil.h"
#include "../ladr/strbuf.h"
#include "../ladr/top_input.h"


namespace isu {
  enum class Output_format : int {ORDINARY, PARENTS_ONLY, XML, HINTS, IVY, TAGGED};

  static constexpr int line_buf_size = 1000;

  bool read_line(std::istream& fin, char* line);
  int  which_member_arg(int argc, const char *argv[], const char* str);
  bool read_to_line(std::istream& fin, const char *str);
  std::istream* get_fin(int argc, const char *argv[], std::ifstream& fin, const char** filename);
  void handle_help(int argc, const char *argv[], const char* Help_string, const char* PROGRAM_NAME);

  String_buf read_heading(std::istream& fin);
  char*      beginning_of_line(std::istream& fin, char* line);
  void       read_program_input(std::istream& fin, TopInput& topInput, Output_format output_format = Output_format::ORDINARY);
  void       print_proof(std::ostream& fp, Plist proof, String_buf comment, Clause_print_format format, I3list jmap, int number);
}



#endif
