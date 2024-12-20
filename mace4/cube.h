
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <sys/stat.h>
#include <fstream>
#include <string>
#include <vector>
#include "cell.h"


class permutation {
public:
   size_t a;
   size_t b;    // a < b

   permutation(size_t a, size_t b): a(a), b(b) {};
   size_t apply(size_t c) { if (c==a) return b; else if (c==b) return a; else return c;};
   // bool operator==(const permutation& x) const { return a==x.a && b == x.b;};
};

struct Breaker{
    int     max_last;

    Breaker(): max_last(-1){};
};

class Cube {
public:
    static const std::string  steal_signal_file_path;
    static const std::string  steal_cube_file_path;
    static const int min_check_interval = 10;
    std::vector<size_t> cell_ids;
    int                 cubes_options;  // lsb for do work stealing, next for outputing num cells filled
private:
    Cell*             Ordered_cells;

    std::vector<int>  cell_values;
    size_t            order;
    Cell              Cells;
    bool              initialized;
    bool              marked;       // root id marked for the process
    size_t            current_pos;  // current position of the search in Ordered_cells
    size_t            max_pos;      // max value of current position
    size_t            cut_off;      // must be less than cut_off from the start cube position to release cubes for work stealing
    size_t            num_new_cubes_printed;
    size_t            num_input_cubes_processed;
    size_t            early_cut_off;
    size_t            top_cut_off;
    size_t            mult_table_size;    // total size of multiplication tables
    std::vector<int>  last_printed;
    bool              do_work_stealing;
    std::vector<int>  cube_cell_ids;
    std::vector<std::string> all_cubes;
    size_t            last_check_time;
    size_t            current_time;

private:
    bool print_unprocessed_cubes(const std::vector<std::vector<int>>& all_nodes, int root_id, size_t from, size_t to);
    inline bool work_stealing_requested ();
    //bool read_config(const char* config_file_path);
    bool read_config_multi(const char* config_file_path);
    bool initialize_cube();
    bool count_appearance(bool& counted, size_t base_id, size_t id, std::vector<size_t>& appear);
    int  sum_val(int& sum, bool& counted, size_t base_id, size_t id, std::vector<size_t>& appear);
    bool has_value(size_t start_id, size_t end_id, int val);
    int  swap_value(size_t a, size_t b, int v) const { if (v==a) return b; else if (v==b) return a; return v;};

    size_t find_pos_in_search(int id);
    int    change_in_cell_value(size_t low_el, size_t high_el, int val);
    bool   is_end_of_circle(int id);
    int    moved_to_cell(int orig_id, size_t low_el, size_t high_el);
    size_t pi(size_t a, size_t b, size_t c) { if (c==a) return b; else if (c==b) return a; else return c;};
    int    check_binop(size_t row, size_t col,
                  const std::vector<size_t>& row_idx, const std::vector<size_t>& col_idx,
                  const std::vector<size_t>& binop);
    
public:
    Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells, int cube_options);

    virtual ~Cube();

    bool reinitialize_cube();
    void set_time(size_t seconds) {current_time = seconds;}
    int  value(size_t depth, size_t id);
    void print_new_cube(ofstream& cubes_file, int cube_length, const std::vector<std::vector<int>>&  all_nodes, const std::string& cg);
    // size_t mark_root(size_t id, size_t from_index, size_t last);
    bool move_on(size_t id, std::vector<std::vector<int>>& all_nodes);
    size_t real_depth(size_t depth, size_t id);
    int  num_cells_filled(Cell Cells);
    // void print_ordered_cells(int number_of_cells) const;
    bool is_inside_input_cube() { return initialized && current_pos < max_pos; };
    int  value_assignment(int pos, int& value);
    size_t get_num_new_cubes_printed() { return num_new_cubes_printed; };

};


#endif
