
#ifndef MACE4_CMDLINE_H
#define MACE4_CMDLINE_H


#include <ostream>
#include <string>
#include "../ladr/glist.h"
#include "options.h"


class CmdLine {
public:
  static constexpr auto PROGRAM_NAME = "Mace4";

  CmdLine() = delete;
  CmdLine(const CmdLine&) = delete;
  CmdLine& operator=(const CmdLine&) = delete;

public:
  static bool  member_args(int argc, char **argv, const std::string& str);
  static void  command_line_parm(int id, char *optarg);
  static void  command_line_flag(int id, char *optarg);
  static void  process_command_line_args(int argc, char **argv, Mace_options opt);
  static void  usage_message(std::ostream& fp, Mace_options opt);
  static Plist process_distinct_terms(Plist distinct);
  static Plist read_mace4_input(int argc, char **argv, bool allow_unknown_things, Mace_options opt);

};



#endif

