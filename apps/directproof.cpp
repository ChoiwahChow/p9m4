

/* Format proofs in various ways
 */
#include "../ladr/attrib.h"
#include "../ladr/banner.h"
#include "../ladr/clause_misc.h"
#include "../ladr/fatal.h"
#include "../ladr/ivy.h"
#include "../ladr/mystring.h"
#include "../ladr/literals.h"
#include "../ladr/random.h"
#include "../ladr/resolve.h"
#include "../ladr/subsume.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"
#include "../ladr/xproofs.h"

#include "../VERSION_DATE.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "directproof";

static constexpr char Help_string[]   = "";
static int  Transformations = 0;

// Is every clause in the Plist an equality unit (pos or neg)?
static
bool horn_eq_clauses(Plist l)
{
  AttributeContainer a_con;
  LiteralsContainer  l_con;
  for (Plist p = l; p; p = p->next) {
    Topform c = static_cast<Topform>(p->v);
    if (!a_con.string_attribute_member(c->attributes, a_con.label_att(), "non_clause") && c->literals) {
      if (!l_con.horn_clause(c->literals) || !l_con.only_eq(c->literals)) {
        return false;
      }
    }
  }
  return true;
}


// If it is a resolution (binary, hyper, ur), is it positive binary?
static
bool pos_bin_res(Topform c, Plist proof)
{
  if (!c->justification)
    return true;
  else if (c->justification->type == Just_type::HYPER_RES_JUST ||
           c->justification->type == Just_type::UR_RES_JUST)
    return false;
  else if (c->justification->type != Just_type::BINARY_RES_JUST)
    return true;
  else {
    JustContainer      j_con;
    LiteralsContainer  l_con;
    Ilist parents = j_con.get_parents(c->justification, false);
    Topform a = Xproofs::proof_id_to_clause(proof, parents->i);
    Topform b = Xproofs::proof_id_to_clause(proof, parents->next->i);
    bool ok = (l_con.positive_clause(a->literals) || l_con.positive_clause(b->literals));
    IlistContainer parents_con;
    parents_con.set_head(parents);
    parents_con.zap_ilist();
    return ok;
  }
}

static
bool all_resolutions_pos_binary(Plist proof)
{
  for (Plist p = proof; p; p = p->next) {
    if (!pos_bin_res(static_cast<Topform>(p->v), proof))
      return false;
  }
  return true;
}


static
bool all_paramodulations_unit(Plist proof)
{
  LiteralsContainer l_con;
  for (Plist p = proof; p; p = p->next) {
    Topform c = static_cast<Topform>(p->v);
    if (c->justification->type == Just_type::PARA_JUST &&
        l_con.number_of_literals(c->literals) != 1)
      return false;
  }
  return true;
}


static
Topform first_pos_parent(Topform c, Plist proof)
{
  JustContainer      j_con;
  LiteralsContainer  l_con;
  Ilist parents = j_con.get_parents(c->justification, false);
  for (Ilist p = parents; p; p = p->next) {
    Topform c = Xproofs::proof_id_to_clause(proof, p->i);
    if (l_con.positive_clause(c->literals))
      return c;
  }
  return nullptr;
}


static
Topform first_neg_parent(Topform c, Plist proof)
{
  JustContainer      j_con;
  LiteralsContainer  l_con;
  Ilist parents = j_con.get_parents(c->justification, false);
  for (Ilist p = parents; p; p = p->next) {
    Topform c = Xproofs::proof_id_to_clause(proof, p->i);
    if (l_con.negative_clause(c->literals))
      return c;
  }
  return nullptr;
}


static
bool bidirectional(Plist proof)
{
  PlistContainer proof_list;
  proof_list.set_head(proof);
  Topform e = static_cast<Topform>(proof_list.plist_last());
  Topform n1 = first_neg_parent(e, proof);
  if (n1 && !n1->is_formula) {
    Topform n2 = first_neg_parent(n1, proof);
    return n2 && !n1->is_formula;
  }
  else
    return false;
}


static
void last_two_steps(Plist proof, Topform *e, Topform *p1, Topform *n1,
                    Topform *p2, Topform *n2)
{
  PlistContainer proof_list;
  proof_list.set_head(proof);
  *e = static_cast<Topform>(proof_list.plist_last());
  *p1 = first_pos_parent(*e, proof);
  *n1 = first_neg_parent(*e, proof);
  *p2 = first_pos_parent(*n1, proof);
  *n2 = first_neg_parent(*n1, proof);
}


static
Term xx_recurse(Term t, Ilist pos, int* n)
{
  TermContainer t_con;
  if (pos == nullptr) {
    Term v = t_con.get_variable_term(*n);
    (*n)++;
    return v;
  }
  else {
    Term newTerm = t_con.get_rigid_term_like(t);
    for (int i = 0; i < ARITY(t); i++) {
      Term arg;
      if (i == pos->i-1)
        arg = xx_recurse(ARG(t,i), pos->next, n);
      else {
        arg = t_con.get_variable_term(*n);
        (*n)++;
      }
      ARG(newTerm,i) = arg;
    }
    return newTerm;
  }
}


static
Topform xx_instance(Literals lits, Ilist pos)
{
  LiteralsContainer  l_con;
  Literals lit = l_con.ith_literal(lits, pos->i);
  int n = 0;
  Term t1 = xx_recurse(ARG(lit->atom, pos->next->i-1), pos->next->next, &n);
  TermContainer t_con;
  Term e = t_con.get_rigid_term_like(lit->atom);
  ARG(e,0) = t1;
  ARG(e,1) = t_con.copy_term(t1);

  Topform newTopform = Random::random_clause(1,1,1,1,1,1,1);   //TODO: make get_topform() public so that we get the new topform directly
  newTopform->literals = l_con.append_literal(newTopform->literals, l_con.new_literal(1, e));
  JustContainer  j_con;
  newTopform->justification = j_con.input_just();
  return newTopform;
}


static
Term instance_from_position_recurse(Term t1, Term t2, Ilist pos, int n)
{
  if (VARIABLE(t1)) {
    Term s = xx_recurse(t2, pos, &n);
    ListtermContainer  lt_con;
    TermContainer      t_con;
    return lt_con.listterm_cons(t_con.copy_term(t1), s);
  }
  else {
    return instance_from_position_recurse(ARG(t1,pos->i-1), ARG(t2,pos->i-1), pos->next, n);
  }
}


static
Term instance_from_position(Topform c1, Topform c2, Ilist pos)
{
  LiteralsContainer  l_con;
  Term t1 = l_con.ith_literal(c1->literals, pos->i)->atom;
  Term t2 = l_con.ith_literal(c2->literals, pos->i)->atom;
  int n = l_con.greatest_variable_in_clause(c1->literals) + 1;
  return instance_from_position_recurse(t1, t2, pos->next, n);
}


static
Plist forward_proof(Plist proof)
{
  if (!horn_eq_clauses(proof))
    fatal::fatal_error("forward_proof, all clauses must be Horn equality");
  else if (!all_paramodulations_unit(proof))
    fatal::fatal_error("forward_proof, all paramodulations must be unit");
  else if (!all_resolutions_pos_binary(proof))
    fatal::fatal_error("forward_proof, all resolutions must be positive binary");

  int next_id = Xproofs::greatest_id_in_proof(proof) + 1;

  TermContainer t_con;
  JustContainer j_con;
  Parautil      p_util;
  LiteralsContainer  l_con;
  TopformContainer   tf_con;
  while (bidirectional(proof)) {
    Topform e, p1, n1, p2, n2;
    Transformations++;
    /* print_proof(std::cout, proof, nullptr, CL_FORM_STD, nullptr, 0); */
    last_two_steps(proof, &e, &p1, &n1, &p2, &n2);
    if (!p1 && j_con.has_copy_flip_just(n1)) {
      /* CASE 0: flip neg clause, then conflict with x=x. */
      /* We can simply remove the flip inference. */
      PlistContainer  proof_con;
      proof_con.set_head(proof);
      proof = proof_con.plist_remove(n1);
      /* Change justification on empty clause. */
      e->justification->u.id = n2->id;
    }
    else if (p1 && j_con.has_copy_flip_just(n1)) {
      /* CASE 1: flip negative clause, then unit conflict. */
      if (e->justification->u.lst->next->i == -1 ||
        e->justification->u.lst->next->next->next->i == -1) {
        /* CASE 1a: Unit conflict has an implicit flip.
           double flips cancel, so we can just remove n1. */
        Topform empty = Subsume::try_unit_conflict(p1, n2);
        if (!empty)
          fatal::fatal_error("forward_proof, case 1a failed");
        empty->id = next_id++;
        PlistContainer  proof_con;
        proof_con.set_head(proof);
        proof_con.plist_remove(n1);
        proof_con.plist_remove(e);
        ClauseMisc::delete_clause(n1);
        ClauseMisc::delete_clause(e);
        proof = proof_con.plist_append(empty);
      }
      else {
        /* CASE 1b: change flip from neg to pos clause. */
        Topform flipped_pos, empty;
        int n = n1->justification->next->u.id;  /* flipped literal */
        flipped_pos = Resolve::copy_inference(p1);
        p_util.flip_eq(flipped_pos->literals->atom, n);
        flipped_pos->id = next_id++;
        /* Now, flipped_pos and n2 should give unit conflict. */
        empty = Subsume::try_unit_conflict(flipped_pos, n2);
        if (!empty)
          fatal::fatal_error("forward_proof, case 1b failed");
        empty->id = next_id++;
        /* Remove 2 clauses and append 2 clauses. */
        PlistContainer  proof_con;
        proof_con.set_head(proof);
        proof_con.plist_remove(n1);
        proof_con.plist_remove(e);
        ClauseMisc::delete_clause(n1);
        ClauseMisc::delete_clause(e);
        proof_con.plist_append(flipped_pos);
        proof = proof_con.plist_append(empty);
      }
    }
    else if (p1 && n1->justification->type == Just_type::PARA_JUST &&
             n1->justification->next == nullptr) {
      /* CASE 3: para into negative clause, then unit conflict. */
      Ilist p2_pos;
      Ilist n2_pos;
      Term target;
      Ilist from_pos;
      if (e->justification->u.lst->next->i == -1 ||
          e->justification->u.lst->next->next->next->i == -1) {
        /* The unit conflict involved an implicit flip.  We'll handle that by
           explicitly flipping the positive equality first. */
        Topform flipped_pos = Resolve::copy_inference(p1);
        p_util.flip_eq(flipped_pos->literals->atom, 1);
        flipped_pos->id = next_id++;
        PlistContainer  proof_con;
        proof_con.set_head(proof);
        proof = proof_con.plist_append(flipped_pos);
        p1 = flipped_pos;
      }

      p2_pos = n1->justification->u.para->from_pos;
      n2_pos = n1->justification->u.para->into_pos;
      target = l_con.term_at_position(p1->literals, n2_pos);
      if (!target) {
        /* Into_term does not exist, so we have to instantiate p1. */
        Term pair = instance_from_position(p1, n1, n2_pos);
        Topform p1i = tf_con.copy_clause(p1);
        p1i->literals->atom = t_con.subst_term(p1i->literals->atom, ARG(pair,0), ARG(pair,1));
        tf_con.upward_clause_links(p1i);
        PlistContainer  pair_con;
        pair_con.set_head(nullptr);
        p1i->justification = j_con.instance_just(p1, pair_con.plist_append(pair));
        p1i->id = next_id++;
        PlistContainer  proof_con;
        proof_con.set_head(proof);
        proof = proof_con.plist_append(p1i);
        p1 = p1i;
      }
      IlistContainer p2_pos_con;
      p2_pos_con.set_head(p2_pos);
      from_pos = p2_pos_con.copy_ilist();
      from_pos->next->i = (p2_pos->next->i == 1 ? 2 : 1);
      Topform new1 = Paramodulation::para_pos2(p2, from_pos, p1, n2_pos);
      IlistContainer from_pos_con;
      from_pos_con.set_head(from_pos);
      from_pos_con.zap_ilist();
      new1->id = next_id++;
      Topform empty = Subsume::try_unit_conflict(new1, n2);
      if (!empty) {
        fatal::fatal_error("directproof failed (unit conflict case 3)");
      }
      empty->id = next_id++;
      PlistContainer  proof_con;
      proof_con.set_head(proof);
      proof = proof_con.plist_remove(n1);
      proof = proof_con.plist_remove(e);
      ClauseMisc::delete_clause(n1);
      ClauseMisc::delete_clause(e);
      proof = proof_con.plist_append(new1);
      proof = proof_con.plist_append(empty);
    }  /* case 3 */
    else if (!p1 &&
             e->justification->type == Just_type::COPY_JUST &&
             e->justification->next &&
             e->justification->next->type == Just_type::XX_JUST) {
      /* CASE 2: conflict with x=x. */
      Ilist p2_pos = n1->justification->u.para->from_pos;
      Ilist n2_pos = n1->justification->u.para->into_pos;
      IlistContainer n2_pos_con;
      n2_pos_con.set_head(n2_pos);
      if (n2_pos_con.ilist_count() > 2) {
        /* into subterm */
        Topform xxi = xx_instance(n2->literals,
                                  n1->justification->u.para->into_pos);
        IlistContainer p2_pos_con;
        p2_pos_con.set_head(p2_pos);
        Ilist from_pos = p2_pos_con.copy_ilist();
        from_pos->next->i = (p2_pos->next->i == 1 ? 2 : 1);
        xxi->id = next_id++;
        Topform new1 = Paramodulation::para_pos2(p2, from_pos, xxi, n2_pos);
        IlistContainer from_pos_con;
        from_pos_con.set_head(from_pos);
        from_pos_con.zap_ilist();
        new1->id = next_id++;
        Topform empty = Subsume::try_unit_conflict(new1, n2);
        if (!empty)
          fatal::fatal_error("directproof failed (unit conflict case 3a)");
        empty->id = next_id++;
        PlistContainer  proof_con;
        proof_con.set_head(proof);
        proof = proof_con.plist_remove(n1);
        proof = proof_con.plist_remove(e);
        ClauseMisc::delete_clause(n1);
        ClauseMisc::delete_clause(e);
        proof = proof_con.plist_append(xxi);
        proof = proof_con.plist_append(new1);
        proof = proof_con.plist_append(empty);
      }
      else {
        /* into top */
        Topform positive;
        if (p2_pos->next->i != n2_pos->next->i) {
          /* left->right or right->left, so we flip the positive parent. */
          Topform flipped_pos = Resolve::copy_inference(p2);
          p_util.flip_eq(flipped_pos->literals->atom, 1);
          flipped_pos->id = next_id++;
          PlistContainer  proof_con;
          proof_con.set_head(proof);
          proof = proof_con.plist_append(flipped_pos);
          positive = flipped_pos;
        }
        else
          positive = p2;
        /* Now, positive and n2 should give unit conflict. */
        Topform empty = Subsume::try_unit_conflict(positive, n2);
        if (!empty)
          fatal::fatal_error("directproof failed (unit conflict case 3b)");
        empty->id = next_id++;
        /* replace empty. */
        PlistContainer  proof_con;
        proof_con.set_head(proof);
        proof = proof_con.plist_remove(n1);
        proof = proof_con.plist_remove(e);
        ClauseMisc::delete_clause(n1);
        ClauseMisc::delete_clause(e);
        proof = proof_con.plist_append(empty);
      }
    }  /* case 2 */
    else {
      std::cerr << "CASE not handled, transformation incomplete.\n";
      std::cout << "CASE not handled:\n";
      if (p2)
        { std::cout << "p2: "; Ioutil::f_clause(p2); }
      if (n2)
        { std::cout << "n2: "; Ioutil::f_clause(n2); }
      if (p1)
        { std::cout << "p1: "; Ioutil::f_clause(p1); }
      if (n1)
        { std::cout << "n1: "; Ioutil::f_clause(n1); }
      if (e)
        { std::cout << "e:  "; Ioutil::f_clause(e); }
      fatal::fatal_error("forward_proof, case not handled");
    }
  }
  return proof;
}

int main(int argc, const char *argv[])
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  std::ifstream fp;
  const char* filename = nullptr;
  std::istream* fin = isu::get_fin(argc, argv, fp, &filename);

  TopInput topInput;
  topInput.init_standard_ladr();
  AttributeContainer a_con;
  int label_attr  = a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);
  int answer_attr = a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);
  int props_attr  = a_con.register_attribute("props", Attribute_type::TERM_ATTRIBUTE);
  a_con.declare_term_attribute_inheritable(answer_attr);

  /* Ok, start reading the input. */

  String_buf heading = isu::read_heading(*fin);  /* first few lines of the file */

  banner::print_separator(std::cout, PROGRAM_NAME, false);
  StrbufContainer sb_con;
  sb_con.set_string_buf(heading);
  sb_con.fprint_sb(std::cout);
  banner::print_separator(std::cout, "end of head", false);

  isu::read_program_input(*fin, topInput);

  banner::print_separator(std::cout, "end of input", true);

  bool found = isu::read_to_line(*fin, "=== PROOF");  /* finishes line */

  int number_of_proofs = 0;
  Plist proofs = nullptr;      /* all of the proofs */
  PlistContainer  proofs_con;
  proofs_con.set_head(proofs);
  Plist comments = nullptr;    /* the corresponding comments */
  PlistContainer  comments_con;
  comments_con.set_head(comments);
  ParseContainer  p_con;

  while (found) {
    /* ok, we have a proof */
    StrbufContainer comment_con;
    comment_con.new_string_buf();

    char line[isu::line_buf_size];
    Plist proof = nullptr;
    PlistContainer  proof_con;
    proof_con.set_head(proof);
    number_of_proofs++;

    //rc = fscanf(*fin, "%s", s);         /* "%" or clause id */
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
          fin->get(c);
          comment_con.sb_append_char(c);
        } while (c != '\n');
      }
      //rc = fscanf(*fin, "%s", s);       /* "%" or clause id */
      isu::beginning_of_line(*fin, line);
    }

    JustContainer     j_con;
    TopformContainer  tf_con;
    while (!myString::substring("==========", line)) {  /* separator at end of proof */
      Term clause_term = p_con.read_term(*fin, std::cerr);
      Term just_term = p_con.read_term(*fin, std::cerr);
      Topform cl = tf_con.term_to_clause(clause_term);
      tf_con.clause_set_variables(cl, MAX_VARS);
      cl->justification = j_con.term_to_just(just_term);
      int id;
      if (myString::str_to_int(line, &id))
        cl->id = id;
      else
        fatal::fatal_error("clause id is not an integer");

      proof = proof_con.plist_prepend(cl);

      //rc = fscanf(fin, "%s", s);         /* clause id */
      isu::beginning_of_line(*fin, line);
    }
    proof = proof_con.reverse_plist();
    proofs = proofs_con.plist_append(proof);
    String_buf comment = comment_con.get_string_buf();
    comments = comments_con.plist_append(comment);
    found = isu::read_to_line(*fin, "=== PROOF");
  }

  Plist p, c;
  int   n;
  for (p = proofs, c = comments, n = 1; p; p = p->next, c = c->next, n++) {
    I3list jmap = nullptr;
    Plist proof = static_cast<Plist>(p->v);
    String_buf comment = static_cast<String_buf>(c->v);

    Plist xproof = Xproofs::expand_proof(proof, &jmap);
    /* ClauseMisc::delete_clauses(xproof); */

    I3listContainer i3_jmap;
    i3_jmap.set_head(jmap);
    i3_jmap.zap_i3list();
    /* ClauseMisc::delete_clauses(proof); */

    Plist xnproof = Xproofs::copy_and_renumber_proof(xproof, 1);
    /* ClauseMisc::delete_clauses(xproof); */

    Plist xnfproof = forward_proof(xnproof);
    /* ClauseMisc::delete_clauses(xnproof); */

    isu::print_proof(std::cout, xnfproof, comment, Clause_print_format::CL_FORM_STD, nullptr, n);
    /* ClauseMisc::delete_clauses(xnfproof); */
  }

  if (number_of_proofs > 0) {
	  std::cout << "\n% Directproof did " << Transformations << " transformation(s) on " << number_of_proofs << " proof(s).\n";
    exit(0);
  }
  else
    exit(2);
}
