
/* Transform and format proofs in various ways.
 */

#include "../ladr/banner.h"
#include "../ladr/clause_misc.h"
#include "../ladr/clock.h"
#include "../ladr/fatal.h"
#include "../ladr/glist.h"
#include "../ladr/ivy.h"
#include "../ladr/just.h"
#include "../ladr/ioutil.h"
#include "../ladr/mystring.h"
#include "../ladr/strbuf.h"
#include "../ladr/top_input.h"
#include "../ladr/xproofs.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "prooftrans";

static constexpr char Help_string[] =
"\nprooftrans              [expand] [renumber] [striplabels] [-f <file>]\n"
  "prooftrans parents_only [expand] [renumber] [striplabels] [-f <file>]\n"
  "prooftrans xml          [expand] [renumber] [striplabels] [-f <file>]\n"
  "prooftrans ivy                   [renumber]               [-f <file>]\n"
  "prooftrans hints [-label <label>]  [expand] [striplabels] [-f <file>]\n"
  "prooftrans tagged                                         [-f <file>]\n"
"\n";

enum {NO_TRANS, EXPAND_EQ, EXPAND, EXPAND_IVY};   /* proof transformations */



static
String_buf read_heading(std::istream& fin, isu::Output_format output_format)
{
  StrbufContainer strbuf_con;
  strbuf_con.new_string_buf();

  char line[isu::line_buf_size];  // the first line_buf_size-1 chars of the line
  int i = 0;
  bool read_ok = isu::read_line(fin, line);

  while (read_ok && !myString::substring("= end of head =", line)) {
    if (i != 0) {
      if (output_format == isu::Output_format::IVY)
        strbuf_con.sb_append(";; ");
      strbuf_con.sb_append(line);
      strbuf_con.sb_append("\n");
    }
    i++;
    read_ok = isu::read_line(fin, line);
  }

  if (!read_ok)
    fatal::fatal_error("read_heading, \"= end of head =\" not found");

  return strbuf_con.get_string_buf();
}

static
Plist add_to_hints(Plist hints, Plist proof)
{
  ClausesContainer c_con;
  for (Plist p = proof; p; p = p->next) {
    if (!c_con.clause_member_plist(hints, static_cast<Topform>(p->v))) {
      PlistContainer  p0;
      p0.set_head(hints);
      hints = p0.plist_prepend(p->v);
    }
  }
  return hints;
}


int main(int argc, const char *argv[])
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  int transformation = NO_TRANS;
  if (isu::which_member_arg(argc, argv, "expand") > 0)
    transformation = EXPAND;
  else if (isu::which_member_arg(argc, argv, "expand_eq") > 0)
    transformation = EXPAND_EQ;

  isu::Output_format output_format = isu::Output_format::ORDINARY;
  if (isu::which_member_arg(argc, argv, "parents_only") > 0)
    output_format = isu::Output_format::PARENTS_ONLY;
  else if (isu::which_member_arg(argc, argv,"xml") > 0 ||
           isu::which_member_arg(argc, argv,"XML") > 0)
    output_format = isu::Output_format::XML;
  else if (isu::which_member_arg(argc, argv, "hints") > 0)
    output_format = isu::Output_format::HINTS;

  if (isu::which_member_arg(argc, argv, "ivy") > 0 ||
      isu::which_member_arg(argc, argv, "IVY") > 0) {
    transformation = EXPAND_IVY;
    output_format = isu::Output_format::IVY;
  }

  /* BV(2007-aug-20): recognize tagged proof option */
  if (isu::which_member_arg(argc, argv, "tagged") > 0 ||
      isu::which_member_arg(argc, argv, "TAGGED") > 0) {
    transformation = NO_TRANS;
    output_format = isu::Output_format::TAGGED;
  }

  bool striplabels = (isu::which_member_arg(argc, argv, "striplabels") > 0 ||
                      isu::which_member_arg(argc, argv, "-striplabels") > 0);

  bool renumber_first = false;
  bool renumber_last = false;
  if (isu::which_member_arg(argc, argv,"renumber") > 0) {
    if (isu::which_member_arg(argc, argv,"expand") > 0) {
      if (isu::which_member_arg(argc, argv,"renumber") <
          isu::which_member_arg(argc, argv,"expand"))
	    renumber_first = true;
      else
        renumber_last = true;
    }
    else
      renumber_last = true;
  }

  std::ifstream fp;
  const char* filename = nullptr;
  std::istream* fin = isu::get_fin(argc, argv, fp, &filename);

  int n = isu::which_member_arg(argc, argv, "-label");
  const char *label = nullptr;
  if (n == -1)
    label = nullptr;
  else if (n+1 >= argc)
    label = "";
  else
    label = argv[n+1];

  TopInput topInput;
  topInput.init_standard_ladr();
  AttributeContainer a_con;
  int label_attr  = a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);
  int answer_attr = a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);
  int props_attr = a_con.register_attribute("props", Attribute_type::TERM_ATTRIBUTE);
  a_con.declare_term_attribute_inheritable(answer_attr);

  Clause_print_format clause_format;
  if (output_format == isu::Output_format::XML)
    clause_format = Clause_print_format::CL_FORM_XML;
  else if (output_format == isu::Output_format::IVY)
    clause_format = Clause_print_format::CL_FORM_IVY;
  else if (output_format == isu::Output_format::PARENTS_ONLY)
    clause_format = Clause_print_format::CL_FORM_PARENTS;
  else if (output_format == isu::Output_format::HINTS)
    clause_format = Clause_print_format::CL_FORM_BARE;
  else if (output_format == isu::Output_format::TAGGED)
    clause_format = Clause_print_format::CL_FORM_TAGGED;
  else
    clause_format = Clause_print_format::CL_FORM_STD;

  /* Ok, start reading the input. */

  String_buf heading = read_heading(*fin, output_format);  /* first few lines of the file */
  StrbufContainer strbuf_con;

  if (output_format != isu::Output_format::XML && output_format != isu::Output_format::HINTS) {
    if (output_format == isu::Output_format::IVY)
      std::cout << ";; ";
    banner::print_separator(std::cout, PROGRAM_NAME, false);
    strbuf_con.set_string_buf(heading);
    strbuf_con.fprint_sb(std::cout);
    if (output_format == isu::Output_format::IVY)
      std::cout << ";; ";
    banner::print_separator(std::cout, "end of head", false);
  }

  isu::read_program_input(*fin, topInput, output_format);

  if (output_format != isu::Output_format::XML && output_format != isu::Output_format::HINTS && output_format != isu::Output_format::IVY)
    banner::print_separator(std::cout, "end of input", true);

  bool found = isu::read_to_line(*fin, "=== PROOF");  /* finishes line */

  Plist proofs = nullptr;      /* all of the proofs */
  PlistContainer    proofs_con;
  proofs_con.set_head(proofs);

  Plist comments = nullptr;    /* the corresponding comments */
  PlistContainer    comments_con;
  comments_con.set_head(comments);

  Plist hints = nullptr;
  TopformContainer  tf_con;
  ParseContainer    parse_con;
  JustContainer     j_con;
  int number_of_proofs = 0;

  while (found) {
    /* ok, we have a proof */
    StrbufContainer  comment_con;
    comment_con.new_string_buf();

    char line[isu::line_buf_size];
    Plist proof = nullptr;
    PlistContainer  proof_con;
    proof_con.set_head(proof);

    number_of_proofs++;

    //rc = fscanf(fin, "%s", line);         /* just read the beginning of the line to get "%" or clause id */
    isu::beginning_of_line(*fin, line);
    while (myString::str_ident(line, (char*)"%")) {
      isu::read_line(*fin, line);
      /* in case prooftrans is run multiple times */
      if (!myString::substring("Comments from original proof", line)) {
        comment_con.sb_append("%");
        comment_con.sb_append(line);
      }
      /* in case there are any long comment lines (answers) */
      if (line[strlen(line)] != '\n') {
        char c;
        do {
          fin->get(c); //fgetc(fin);
          comment_con.sb_append_char(c);
        } while (c != '\n');
      }
      //rc = fscanf(fin, "%s", line);       /* "%" or clause id */
      isu::beginning_of_line(*fin, line);
    }

    while (!myString::substring("==========", line)) {  /* separator at end of proof */
      Term clause_term = parse_con.read_term(*fin, std::cerr);
      Term just_term = parse_con.read_term(*fin, std::cerr);
      int id;
      Topform cl = tf_con.term_to_clause(clause_term);
      tf_con.clause_set_variables(cl, MAX_VARS);
      cl->justification = j_con.term_to_just(just_term);
      if (striplabels)
        cl->attributes = a_con.delete_attributes(cl->attributes, label_attr);
        if (myString::str_to_int(line, &id))
        cl->id = id;
      else
        fatal::fatal_error("clause id is not an integer");

      proof = proof_con.plist_prepend(cl);

      //rc = fscanf(fin, "%s", line);         /* clause id */
      isu::beginning_of_line(*fin, line);
    }
    proof = proof_con.reverse_plist();
    proofs = proofs_con.plist_append(proof);
    comments = comments_con.plist_append(comment_con.get_string_buf());
    found = isu::read_to_line(*fin, "= PROOF =");
  }

  if (output_format == isu::Output_format::XML) {
    std::cout << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
    std::cout << "\n<!DOCTYPE proofs SYSTEM \"proof3.dtd\">\n";
    std::cout << "\n<?xml-stylesheet type=\"text/xsl\" href=\"proof3.xsl\"?>\n";
    std::cout << "\n<proofs number_of_proofs=\"" << number_of_proofs << "\">\n";
    if (fin != &std::cin)
      std::cout << "\n<source>" << filename << "</source>\n";
    std::cout << "\n<heading><![CDATA[\n";
    strbuf_con.set_string_buf(heading);
    strbuf_con.fprint_sb(std::cout);
    std::cout <<"]]></heading>\n";
  }

  Plist c;
  Plist p;
  for (p = proofs, c = comments, n = 1; p; p = p->next, c = c->next, n++) {
    I3list jmap = nullptr;
    Plist proof = static_cast<Plist>(p->v);
    String_buf comment = static_cast<String_buf>(c->v);

    if (renumber_first) {
      Plist proof2 = Xproofs::copy_and_renumber_proof(proof, 1);
      ClauseMisc::delete_clauses(proof);
      proof = proof2;
    }

    if (transformation == EXPAND) {
      Plist proof2 = Xproofs::expand_proof(proof, &jmap);
      ClauseMisc::delete_clauses(proof);
      proof = proof2;
    }

    else if (transformation == EXPAND_IVY) {
      Plist proof2 = Xproofs::expand_proof(proof, &jmap);
      Plist proof3 = Ivy::expand_proof_ivy(proof2);
      ClauseMisc::delete_clauses(proof);
      ClauseMisc::delete_clauses(proof2);
      proof = proof3;
    }

    if (renumber_last) {
      Plist proof2 = Xproofs::copy_and_renumber_proof(proof, 1);
      ClauseMisc::delete_clauses(proof);
      proof = proof2;
      I3listContainer i3_jmap;
      i3_jmap.set_head(jmap);
      i3_jmap.zap_i3list();
      jmap = nullptr;
    }

    if (output_format == isu::Output_format::HINTS)
      hints = add_to_hints(hints, proof);  /* add (without dups) to hints */
    else
      isu::print_proof(std::cout, proof, comment, clause_format, jmap, n);
  }

  if (output_format == isu::Output_format::XML)
    std::cout << "\n</proofs>\n";

  else if (output_format == isu::Output_format::HINTS) {
    PlistContainer hints_list;
    hints_list.set_head(hints);
    hints = hints_list.reverse_plist();

    PlistContainer proofs_list;
    proofs_list.set_head(proofs);

    std::cout << "\nformulas(hints).\n\n";
    std::cout << "% " << hints_list.plist_count() << " hints from " << proofs_list.plist_count() << " proof(s) in file "
    		  << (filename ? filename : "stdin") << ", " << myClock::get_date() << "\n";

    AttributeContainer  a_con;
    int n = 0;
    for (Plist p = hints; p; p = p->next) {
      Topform c = static_cast<Topform>(p->v);
      if (label) {
        char s[128];
        /* quote it only if necessary */
        const char *q = (parse_con.ordinary_constant_string(label) ? "" : "\"");
        sprintf(s, "%s%s_%d%s", q, label, ++n, q);
        c->attributes = a_con.set_string_attribute(c->attributes, label_attr, s);
      }
      Ioutil::fwrite_clause(std::cout, c, (int)Clause_print_format::CL_FORM_BARE);
    }
    std::cout << "end_of_list.\n";
  }

  if (number_of_proofs > 0)
    exit(0);
  else
    exit(2);
}
