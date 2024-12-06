
#include <ctime>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <unistd.h>
#include "syms.h"
#include "cube.h"


const std::string  Cube::steal_signal_file_path = "request_work.txt";
const std::string  Cube::steal_cube_file_path = "release_work.out";

Cube::~Cube() {
    // TODO Auto-generated destructor stub
}

bool
Cube::initialize_cube()
{
    // all_cubes is a vector of integers. Each 2 consective integers
    // are cell_id and cell_value
    initialized = false;
    if (all_cubes.empty())
        return initialized;

    std::istringstream config(all_cubes.front());
    int in_value;
    config >> in_value;
    std::cout << "Debug Cube::initialize_cube " << std::endl;
    while (config) {
        cube_cell_ids[max_pos] = in_value;
        config >> in_value;
        // std::cout << cube_cell_ids[max_pos] << "|" << in_value << " ";
        cell_values[cube_cell_ids[max_pos++]] = in_value;
        config >> in_value;
    }
    // std::cout << std::endl;
    all_cubes.erase(all_cubes.begin());  // first cube is to be processed now
    initialized = true;
    return initialized;
}

bool
Cube::reinitialize_cube()
{
    // return false when all cubes in the all_cubes store are done
    max_pos = 0;
    current_pos = 0;
    last_printed.clear();
    return initialize_cube();
}

bool
Cube::read_config_multi(const char* config_file_path) {
    ifstream config(config_file_path);
    if (!config.is_open()) {
        return false;
    }
    std::cout << "Debug Cube::read_config_multi " << std::endl;
    std::string line;
    while (std::getline(config, line))
    {
        std::string a_cube = line;
        // std::cout << "Debug cube line****" << line << "*****" << std::endl;
        all_cubes.push_back(a_cube);
    }
    return true;
}


Cube::Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells, int cubes_options, int sym_breaking):
        initialized(false), marked(false), order(domain_size), Cells(Cells), current_pos(0), max_pos(0), Ordered_cells(Ordered_cells),
        cut_off(5), mult_table_size(0), cubes_options(cubes_options), do_work_stealing(cubes_options & 1),
        last_check_time(0), current_time(0), sym_breaking_by_row(false), sym_breaking_concentric(false), sym_breaking(sym_breaking)
{
    switch (sym_breaking) {
    case 1:  sym_breaking_by_row = true; break;
    case 2:  sym_breaking_concentric = true; break;
    }
    while (Ordered_cells[mult_table_size]->get_symbol() != "=" ) {
        mult_table_size++;
    }
    cut_off = mult_table_size * 9/10;

    for (size_t idx=0; idx<mult_table_size; idx++) {
        cell_ids.push_back(Ordered_cells[idx]->get_id());
    }
    int max_id = *std::max_element(std::begin(cell_ids), std::end(cell_ids));
    cell_values.resize(max_id+1, -1);
    cube_cell_ids.resize(max_id+1, -1);

    read_config_multi("cube.config");
    //    return;
    // at the very beginning, no cubes from config file means to start from the root
    initialized = true;
    if (!all_cubes.empty())
        initialize_cube();

    std::cout << "\ndebug constructor Cube*********************** cubes_options " << cubes_options << " max_pos = " << max_pos << std::endl;
    std::cout << "Debug ******************** cut_off: " << cut_off << std::endl;
}


int
Cube::num_cells_filled(Cell Cells)
{
    size_t count = 0;
    for (size_t idx=0; idx<cell_ids.size(); idx++){
        if (Cells[cell_ids[idx]].has_value())
            count++;
    }
    // std::cout << "Debug Num cells filled " << count << std::endl;
    return count;
}

int
Cube::value_assignment(int pos, int& value) 
{
   // for the pos-th value-assignment in the cube,
   // return the "term" as function return value, and the value of the assignment in "value"
   if (initialized && pos < max_pos) {
        value = cell_values[cube_cell_ids[pos]];
        return cube_cell_ids[pos];
   }
   else {
       value = -1;
       return -1;
   }
}

int
Cube::value(size_t depth, size_t id) {
    //if (initialized && current_pos < max_pos) {
    if (is_inside_input_cube()) {
        // std::cout << "Debug, Cube::value, incoming id " << id << " current_pos " << current_pos << " depth: " << depth << std::endl;
        while (id != cell_ids[current_pos] && current_pos < max_pos) {
            if ((Cells[cell_ids[current_pos]].get_value() != cell_values[cell_ids[current_pos]]) && (cell_values[cell_ids[current_pos]] != -1)) {
                std::cout << "Debug, Cube::value, Mis-matched cell id =" << cell_ids[current_pos] << ", cell value = " << cell_values[cell_ids[current_pos]]
                          << "  vs  " << Cells[cell_ids[current_pos]].get_value() << std::endl;
                current_pos++;
                return order+1;
            }
            // std::cout << "Debug, Cube::value, matched, cell id = "  << cell_ids[current_pos] << "  current pos " << current_pos << std::endl;
            current_pos++;
        }
        if (current_pos >= max_pos) {
            return -1;
        }
        if (id == cell_ids[current_pos]) {
            current_pos++;
        }
        // std::cout << "Debug Cube::value pos " << current_pos << " max position " << max_pos << " cell value: " << cell_values[id] << std::endl;
        return cell_values[id];
    }
    return -1;
}

inline bool
Cube::work_stealing_requested() {
    struct stat buffer;
    bool signal_exists = stat (steal_signal_file_path.c_str(), &buffer) == 0;
    if (!signal_exists)
        return false;
    bool pending_cubes = stat (steal_cube_file_path.c_str(), &buffer) == 0;
    // time_t now = time(0);
    // char* date_time = ctime(&now);
    // std::cout << "debug Current working directory: " << get_current_dir_name() << " " << date_time << std::endl;
    std::cout << "debug work_stealing_requested: " << steal_signal_file_path.c_str() << signal_exists << " and " << steal_cube_file_path.c_str() << pending_cubes << std::endl;
    return signal_exists && !pending_cubes;
}

bool
Cube::move_on(size_t id, std::vector<std::vector<int>>& all_nodes) {
    if (!initialized)
        return false;
    if (current_time - last_check_time > min_check_interval) {
        last_check_time = current_time;
        // cut_off++;
        if (work_stealing_requested()) {
            // std::cout << "debug move_on, work_stealing_requested" << std::endl;
            if (!all_cubes.empty())
                return print_unprocessed_cubes(all_nodes, -1, 0, 0);
            size_t first_pos = 0;
	    // Find shortest cubes to print out
            while (all_nodes[first_pos][1] == all_nodes[first_pos][2]) {
                first_pos ++;
                if (first_pos >= all_nodes.size())
                    return false;
            }
            // std::cout << "debug move_on, first_pos " << first_pos << " " << all_nodes[first_pos][1]+1 << " " << all_nodes[first_pos][2] << std::endl;
            if (first_pos > cut_off)
                return false;
            if (print_unprocessed_cubes(all_nodes, all_nodes[first_pos][0], all_nodes[first_pos][1]+1, all_nodes[first_pos][2])) {
                all_nodes[first_pos][2] = all_nodes[first_pos][1];
                return true;
            }
        }
    }
    return false;
}

bool
Cube::print_unprocessed_cubes(const std::vector<std::vector<int>>& all_nodes, int root_id, size_t from, size_t to)
{
    bool ret_value = true;
    std::stringstream buffer;
    if (all_cubes.empty()) {
	std::cout << "print_unprocessed_cubes " << root_id << " " << from << " " << to << std::endl;
        for (size_t idx=from; idx<=to; idx++) {
            size_t pos = 0;
            while (all_nodes[pos][0] != root_id) {
                buffer << all_nodes[pos][0] << " " << Cells[all_nodes[pos][0]].get_value() << " ";
                pos++;
            }
            buffer << root_id << " " << idx << std::endl;
        }
    }
    else {
        ret_value = false;
        size_t last = all_cubes.size() / 2;
        if (last == 0) {
            buffer << all_cubes[0] << std::endl;
            all_cubes.erase(all_cubes.begin());
        }
        else {
            for(std::vector<std::string>::iterator it = std::begin(all_cubes); it != std::begin(all_cubes)+last; ++it) {
                buffer << *it << std::endl;
            }
            all_cubes.erase(all_cubes.begin(), all_cubes.begin()+last);
        }
    }
    buffer << "End" << std::endl;
    std::cout << "debug print_unprocessed_cubes: " << from << " " << to << "\n" << buffer.str() << std::endl;
    ofstream cube_file;
    cube_file.open (steal_cube_file_path);
    cube_file << buffer.str();
    cube_file.close();

    if (remove(steal_signal_file_path.c_str()) != 0) {
        std::cout << "@@@@@@@@@@Cube::print_unprocessed_cubes@@@@@@@@@@@@@@@@@@failed to delete signal file " << steal_signal_file_path.c_str() << std::endl;
    }
    return ret_value;
}


bool
Cube::check_sym(size_t base_id, size_t a, size_t b, size_t col, int val)
{
    // if the cube is to be suppressed, return true;
    // Case 1:
    // col > b > a, f(r, a) = a and f(r, b) = b for the row r of concern
    // Cells[base_id+a] = a and Cells[base_id+b] = b

    if (val != b)
        return false;

    for (size_t idx=0; idx < col; ++idx) {
	if (idx != a && idx != b && Cells[base_id+idx].get_value() == a) // the cube becomes lex bigger
            return false;
    }
    return true;    
}

bool
Cube::count_appearance(bool& counted, size_t base_id, size_t id, std::vector<size_t>& appear) 
{
    // count the number of times a domain element appears as a value in the cube, up to
    // but not including the cell with id
    if (!counted) {
        for (size_t ptr=0; ptr<id; ++ptr)
            appear[Cells[base_id + ptr].get_value()] += 1;
    }
    return true;
}


bool
Cube::has_value(size_t start_id, size_t end_id, int val)
{
    for (size_t ptr=0; ptr < end_id; ++ptr) {
        if (Cells[ptr].get_value() == val)
            return true;
    }
    return false;
}

int
Cube::sum_val(int& sum, bool& counted, size_t base_id, size_t id, std::vector<size_t>& appear)
{
    if (sum > -1)
        return sum;
    count_appearance(counted, base_id, id, appear);
    sum = accumulate(appear.begin()+1, appear.end(), 0);
    return sum;
}

bool
Cube::break_symmetries(int parent_id)
{
    // a model is found, make sure it is not a non-lexmin model
    // returns true if it is sure that it is not min lex

    return false;
}

bool
Cube::break_symmetries(int parent_id, int id, int val)
{
    if (sym_breaking == 1)
        return break_symmetries_by_row (parent_id, id, val);
    else if (sym_breaking == 2)
        return break_symmetries_concentric (parent_id, id, val);
    else
        return false;
}

bool
Cube::break_symmetries_by_row(int parent_id, int id, int val)
{
    // first, catch up on those cells whose values are propagated
    if (parent_id < 0)
        return break_symmetries_by_row(id, val);
    for (size_t iptr = parent_id+1; iptr <id; ++iptr) {
        if (break_symmetries(iptr, Cells[iptr].get_value()))
            return true;
    }
    return break_symmetries_by_row(id, val);
}

bool
Cube::no_change(size_t base_id, size_t row, size_t lcol, size_t rcol)
{
    // check that swapping the columns lcol and rcol does not change the values
    // in these 2 columns up to and including "row" - 1 rows.
    for (size_t rptr = 0; rptr < row; ++rptr) {
        if (Cells[base_id + rptr * order + lcol].get_value() != Cells[base_id + rptr * order + rcol].get_value()) {
            return false;
        }
    }
    return true;
}

bool
Cube::break_symmetries_by_row(int id, int val)
{
    return break_symmetries(id, val);
    if (Cells[id].get_arity() == 2) { // bin op or bin rel
        int row = Cells[id].get_index(0);
        int col = Cells[id].get_index(1);
        size_t offset = row * order + col;
        size_t base_id = id - offset;      // TODO optimize by storing this
        if ((col+1) % order == 0) {
            for (size_t dptr = col-1; dptr > row; --dptr) {
                if (compare_cubes(base_id, id, val, dptr, col) < 0)
                    return true;
            }
            // check for all cycles (dptr, row)
            for (size_t dptr = row - 1; dptr > 0; --dptr) {
                if (compare_cubes(base_id, id, val, dptr, row) < 0)
                    return true;
            }
        }
    }
 
    return false;
}

bool
Cube::break_symmetries(int id, int val)
{
    // if the cube is to be suppressed, return true

    // We need to make sure we find a smaller model and do not end up deleting itself
    // With very few exceptions, we cannot a populated cell with a unpopulated cell
    // return false;

    if (Cells[id].get_arity() == 2) { // bin op or bin rel
        int row = Cells[id].get_index(0);
        int col = Cells[id].get_index(1);
        size_t offset = row * order + col;
        size_t base_id = id - offset;      // TODO optimize by storing this

        // LNH-like symmetry

        std::vector<size_t> appear(order, 0);
        bool counted = false;

        // f(0, col) = v > max(col,row)+1, so (max(col,row)+1, val) reduces val to max(col,row)+1
        // and the cycle does not affect "row" and "col" so far.
        // Required: rows below "row" are not moved
        // Required: columns on the right of col is either not moved, or
        //            swapped values have the same value. 
        if (val > col + 1 && val > row + 1) {
            int max_r_c = max(col, row);
            bool good = no_change(base_id, row, max_r_c+1, val);
            /* 
            for (size_t rptr = 0; rptr < row; ++rptr) {
                // make sure the cycle (max_r_c+1, val) does not affect columns on the right
                if (Cells[base_id + rptr * order + max_r_c + 1].get_value() !=  
                    Cells[base_id + rptr * order + val].get_value() ) {
                    good = false;
                    break;
                }
            }
            */
            if (good && row > 0) {
                counted = count_appearance(counted, base_id, id, appear);
                if (appear[max_r_c+1] > 0)
                    good = false;
            }
            if (good) 
                return true;
        }

        /*
        // covered by square block check below
        // diagonal quick check
        // f(0,0)>0, f(r,r) = r, so the cycle (0, row) will make f(0,0) = 0, smaller
        if (row > 0 && row == col && val == row && Cells[base_id].get_value() > 0)
            return true;
        */

        // row 0 symmetries

        /* commented out, covered by the general case below
        
        // compare immediate left cell: consider cycle (col-1, val)  
        if (row == 0 && col > 1 && val == col) {
            if (Cells[base_id+col-1].get_value() > col - 1) {
                counted = count_appearance(counted, base_id, id, appear);
                if (appear[col-1] == 0)
                    return true;
            }
        }
        */

        // compare with all left cells: consider cycle (cptr, col)
        // f(row, col) = val = col, since cptr < col, so the cube is reduced if
        // Required: cptr does not appear as a value anywhere
        // Required: rows below "row" are not moved
        /* commented out, covered by the general case below
        if (row == 0 && col > row+1 && val == col) {
            for (size_t cptr = col-1; cptr > row; --cptr) {
                if (Cells[base_id+cptr].get_value() > cptr) {
                    counted = count_appearance(counted, base_id, id, appear);
                    if (appear[cptr] == 0)
                        return true;
                }
            }
        }
        */
        if (col > row+1 && val == col) {
            for (size_t cptr = col-1; cptr > row; --cptr) {
                if (Cells[base_id + order * row + cptr].get_value() > cptr) {
                    bool good = no_change(base_id, row, cptr, col);
                    /*
                    for (size_t rptr = 0; rptr < row; ++rptr) {
                        if (Cells[base_id + rptr * order + cptr].get_value() != Cells[base_id + rptr * order + col].get_value()) {
                            good = false;
                            break;
                        }
                    }
                    */
                    if (good) {
                        counted = count_appearance(counted, base_id, id, appear);
                        if (appear[cptr] == 0)
                            return true;
                    }
                }
            }
        }

        // inner cycles, or cycle high point
        // row 0
        // if f(0, val) = a < val and f(0, a) = val, then the cycle (a, val) reduces val to a
        //    0       a       val       col
        // 0          val     a         val
        // the cycle does not affect the columns "a" and "val", so if "a" does not appear
        // anywhere else, then the cube is reduced.
        // make sure the cycle does not move the rows that are already populated (so a > row)
        /* commented out, covered by the general case below
        if (row == 0 && col > row+1 && val < col && val > row) {
            // look for cycles with lower end pt "a"
            int a = Cells[base_id+val].get_value();
            if (a > row && a < val && Cells[base_id+a].get_value() == val) {
                counted = count_appearance(counted, base_id, id, appear);
                if (appear[val] == 1)
                    return true;
            }
        }
        */
        if (col > row+1 && val < col && val > row) {
            // look for cycles with lower end pt "a"
            int a = Cells[base_id+row*order+val].get_value();
            if (a > row && a < val && Cells[base_id + row * order + a].get_value() == val) {
                bool good = no_change(base_id, row, a, val);
                /*
                for (size_t rptr = 0; rptr < row; ++rptr) {
                    if (Cells[base_id + rptr * order + a].get_value() != Cells[base_id + rptr * order + val].get_value()) {
                        good = false;
                        break;
                    }
                }
                */
                if (good) {
                    counted = count_appearance(counted, base_id, id, appear);
                    if (appear[val] == 1)
                        return true;
                }
            }
        }


        // Equal-value swap
        // row 0
        //    0       a       val       col
        // 0          d       d         val
        // the cycle (a, val) does not affect the columns "a" and "val", so if "a" does not appear
        // anywhere else, then the cube is reduced.
        // "a" is cidx, "b" is val, "d" is common_val below
        /* commented out, covered by the general case below
        if (row == 0 && val <= col && val > row) {
            // special maneuver because Cells[base_id + col] is not assigned the value val yet.
            int b = val;   // when val = col
            int common_val = val;
            if (val < col) 
                common_val = Cells[base_id + b].get_value(); // f(0, b) = common_val;
          
            for (size_t cidx = b-1; cidx > row; --cidx) {
                // if f(0, cidx) = comm_val, then (cidx, val) is the cycle to move val down to cidx
                if (Cells[base_id + cidx].get_value() == common_val) {
                    // cidx cannot appear as a value, otherwise it becomes a high value
                    counted = count_appearance(counted, base_id, id, appear);
                    if (appear[cidx] == 0)
                        return true;
                }
            }
        }
        */
        if (val <= col && val > row) {
            // special maneuver because Cells[base_id + col] is not assigned the value val yet.
            int b = val;   // when val = col
            int common_val = val;
            if (val < col) 
                common_val = Cells[base_id + row * order + b].get_value(); // f(0, b) = common_val;
          
            for (size_t cidx = b-1; cidx > row; --cidx) {
                // if f(0, cidx) = comm_val, then (cidx, val) is the cycle to move val down to cidx
                if (Cells[base_id + row * order + cidx].get_value() == common_val) {
                    // cidx cannot appear as a value, otherwise it becomes a high value
                    bool good = no_change(base_id, row, cidx, val);
                    /*
                    for (size_t rptr = 0; rptr < row; ++rptr) {
                        if (Cells[base_id + rptr * order + cidx].get_value() != Cells[base_id + rptr * order + val].get_value()) {
                            good = false;
                            break;
                        }
                    }
                    */
                    if (good) {
                        counted = count_appearance(counted, base_id, id, appear);
                        if (appear[cidx] == 0)
                            return true;
                    }
                }
            }
        }


        /* commented out, covered by square block check below
        // f(1,0) = f(1,1) = 1 and (f(0,0) != 0 or f(0,1) != 0)
        // cycle (0, 1) makes f(0, 0) or f(0, 1) smaller
        if (row == 1 && 1 == col && val == row && Cells[base_id+order].get_value() == 1
            && (Cells[base_id].get_value() != 0 || Cells[base_id+1].get_value() != 0))
            return true;
        */

        // square block check
        // row >= 1, row = col, and f(r, r) = r 
        // consider the cycle (0, r): check if the first r columns of the first row is reduced.
        if (row >= 1 && row == col && val == row) {
            if (Cells[base_id].get_value() > 0)
                return true;
            
            bool is_smaller = false;
            bool is_bigger = false;
            for (size_t cptr = 1; cptr < col; ++cptr) {
                int v = swap_value(0, row, Cells[base_id+row*order+cptr].get_value());
                if (v == Cells[base_id+cptr].get_value()) 
                    continue;
                else {
                    if (v < Cells[base_id+cptr].get_value()) 
                        is_smaller = true;
                    else
                        is_bigger = true;
                    break;
                }
            }
            if (is_smaller)
                return true;

            if (!is_bigger) {
                int v = swap_value(0, row, Cells[base_id+row*order].get_value());
                if (v < Cells[base_id+col].get_value())
                    return true;
            }
        }

        // compare with the row before
        // Consider the cycle (row-1, row)
        // all col on the left must not be bigger than the row above, and
        // at least one column is less than that above, row must not appear as a cell value
        if (row > 0 && col > 1) {
            counted = count_appearance(counted, base_id, id, appear);
            if (no_change(base_id, row, row-1, row) && appear[row-1] == 0) {
                for (size_t cptr = 0; cptr < col; ++cptr) {
                    int v = swap_value(row-1, row, Cells[base_id+row*order+cptr].get_value());
                    if (cptr == row-1)  {
                        if (row != col)
                            v = swap_value(row-1, row, Cells[base_id+row*order+cptr+1].get_value()); 
                        else
                            v = swap_value(row-1, row, val); 
                    }
                    else if (cptr == row)
                        v = swap_value(row-1, row, Cells[base_id+row*order+cptr-1].get_value());
                    int v_1 = Cells[base_id+(row-1)*order+cptr].get_value();
                    if (v < v_1) 
                        return true;
                    else if (v > v_1) 
                        break;
                }
            }
        }

        // All zeroes in VA assignments so far
        // The state could be cached for efficiency
        int  sum = -1;
        sum_val(sum, counted, base_id, id, appear);
        if (sum == 0) {
            // the cycle (col+1, val)
            if (val > col+1 && val > row+1) 
                return true;

            // the cycle (row+1, val)
            if (val < col && val > row+1)
                return true;
        }
    }
    return false;
}

void
Cube::print_new_cube(ofstream& cubes_file, int cube_length, const std::vector<std::vector<int>>&  all_nodes, const std::string& cg) 
{
    bool same = true;
    if (last_printed.size() < cube_length)
        last_printed.insert(last_printed.end(), cube_length, -1);
    for (int idx = 0; idx < cube_length; ++idx) {
        if (last_printed[idx] != Cells[all_nodes[idx][0]].get_value()) {
            last_printed[idx] = Cells[all_nodes[idx][0]].get_value();
            same = false;
        }
    }
    if (same) {
        return;
    }
    /* debug print
      for (int idx = 0; idx < print_cubes; ++idx)
          std::cout << " " << splitter.cell_ids[idx] << "|" << Cells[splitter.cell_ids[idx]].get_symbol();
      std::cout << std::endl;
      */
    // std::cout << "cube";
    for (int idx = 0; idx < cube_length; ++idx)
        cubes_file << " " << all_nodes[idx][0] << " " << last_printed[idx];
    cubes_file << std::endl;
}

bool
Cube::is_end_of_circle(int id)
{
    // returns true if this is the last cell in the search of a concentric circle
    int id_pos = find_pos_in_search(id);
    if (id_pos > 0 && id_pos+1 < cell_ids.size()) {
        int next_id = cell_ids[id_pos+1];
        int next_row = Cells[next_id].get_index(0);
        int next_col = Cells[next_id].get_index(1);
        if (next_row == next_col) {
            return true;
        }
    }
    return false;
}


bool
Cube::break_symmetries_concentric(int parent_id, int id, int val)
{
    if (Cells[id].get_arity() != 2)  // bin op or bin rel
        return false;

    // do catchup first
    int last_checked_pos = find_pos_in_search(parent_id);
    int current_pos = find_pos_in_search(id);
    for (size_t pos = last_checked_pos+1; pos < current_pos; ++pos)
        if (break_symmetries_concentric(cell_ids[pos], Cells[cell_ids[pos]].get_value()))
            return true;

    // do symmetries breaking for current cell
    return break_symmetries_concentric(id, val); 
}


bool
Cube::break_symmetries_concentric(int id, int val)
{
    int row = Cells[id].get_index(0);
    int col = Cells[id].get_index(1);
    size_t offset = row * order + col;
    size_t base_id = id - offset;      // TODO optimize by storing this

    // LNH-like
    std::vector<size_t> appear(order, 0);
    bool counted = false;

    if (is_end_of_circle(id)) {
        // check for all cycles (dptr, row)
        for (size_t dptr = row - 1; dptr > 0; --dptr) {
            if (compare_cubes(base_id, id, val, dptr, row) < 0)
                return true;
        }
    }

    return false;
}

size_t
Cube::find_pos_in_search(int id)
{
    // TODO optimize - no need to re-traverse everytime
    for (size_t pos=0; pos < cell_ids.size(); ++pos) {
        if (cell_ids[pos] == id)
            return pos;
    }
    return -1;
}


int Cube::moved_to_cell(int orig_id, size_t low_el, size_t high_el)
{
    size_t arity = Cells[orig_id].get_arity();
    size_t f_idx = Cells[orig_id].get_index(0);
    size_t f_offset = 0;
    if (f_idx == low_el)
        f_offset = high_el - low_el;
    else if (f_idx == high_el)
        f_offset = low_el - high_el;
    if (arity == 1)
        return orig_id + f_offset;

    size_t s_idx = Cells[orig_id].get_index(1);
    size_t s_offset = 0;
    if (s_idx == low_el)
        s_offset =  high_el - low_el;
    else if (s_idx == high_el)
        s_offset = low_el - high_el;
    if (arity == 2) 
        return orig_id + f_offset * order + s_offset;
    else    
        return -1; 
}

int
Cube::compare_cubes(size_t base_id, size_t end_id, int val, size_t low_el, size_t high_el)
{
    // This is called only when a concentric circle is complete

    // The cycle is (low_el, high_el) where low_el < high_el.
    // The last id in the cube is end_id, with value "val" which is not filled in Cells yet.
    // we go in the concentric order
    // if the cube is decreased by the cycle, return -1
    // if the cube is increased by the cycle, return 1
    // else continue until the end, which is the cell (high_el, high_el-1), return 0    
    // When a smaller prefix of the cube is seen, we still need to continue to make sure
    // LNH will continue to the end_id and generate a cube.

    size_t base_pos = find_pos_in_search(base_id);
    size_t end_pos = find_pos_in_search(end_id);
    bool   seen_smaller = false;
    int    max_value_so_far = -1;

    // Goes down (0, 0), (1, 1), (0,1), (1,0) ... 
    // before (low_el-1, low_el-1), cells are not moved and cycle applies only to val 
    size_t ptr = base_pos;
    for (size_t ptr = base_pos; ptr <= end_pos; ++ptr) {
        size_t cur_id = cell_ids[ptr];
        int    cur_val;
        int    new_cell_id = moved_to_cell(cur_id, low_el, high_el);
        int    sigma_val;
        // cell for the end_id has not been set in the Cells array yet.
        if (new_cell_id == end_id) 
            sigma_val = swap_value(low_el, high_el, val);
        else 
            sigma_val = swap_value(low_el, high_el, Cells[new_cell_id].get_value()); 
        if (cur_id == end_id)
            cur_val = val;
        else
            cur_val = Cells[cur_id].get_value();

        // if the cell value have moved up to high_val by the cycle  
        // LNH prevents this value be assigned to this cell, so this is not a lexmin cube of its class
        int mdn_plus1 = max(max_value_so_far, Cells[cur_id].get_max_index()) + 1;
        if (sigma_val > mdn_plus1) 
            return 1;
     
        int comp = sigma_val - cur_val;
        if (comp > 0) {
            if (!seen_smaller) 
                return 1;
        }
        else if (comp < 0)
            seen_smaller = true;
        max_value_so_far = max (max_value_so_far, sigma_val);
    }

    if (seen_smaller)
        return -1;
    else
        return 0;
}


int
Cube::check_binop(size_t row, size_t col,
                  const std::vector<size_t>& row_idx, const std::vector<size_t>& col_idx, 
                  const std::vector<size_t>& binop)
{
    // check whether binop is made lex smaller when permuted by the permutation
    // of rows and columns (represented by pi1 and pi2).

    size_t col_end = order;
    // Check each cell, row by row and column by column starting with row zero
    for (size_t ridx=0; ridx <= row; ++ridx) {
        if (row == ridx)
            col_end = col + 1;
        for (size_t cidx = 0; cidx < col_end; ++cidx) {
            int diff = row_idx[col_idx[Cells[binop[row_idx[col_idx[ridx]]] + row_idx[col_idx[cidx]]].get_value()]] - 
                       Cells[binop[ridx] + cidx].get_value();
            if (diff != 0)
                return diff;
        }
    }
    return 0;
}


int
Cube::check_lexmin(size_t id, bool is_model)
{
    if (!is_model) {
        while (Cells[id+1].has_value())
            ++id;
    }
//    else  return 0;
    std::vector<size_t> rows(order, 0);
    size_t row = Cells[id].get_index(0);
    size_t col = Cells[id].get_index(1);
    
    if ((row == col && col != order - 1) || col == row+1)
        return 0;

    size_t offset = row * order + col;
    size_t base_id = id - offset;      // TODO optimize by storing this
    if (is_model) {
        row = order - 1;
        col = order - 1;
    }

    for (size_t ridx=0; ridx < order; ++ridx)
        rows[ridx] = base_id + ridx*order;

    return check_lexmin(row, col, rows);
}

int
Cube::check_lexmin(size_t row, size_t col, const std::vector<size_t>& binop)
{
    // return -1 if it is not lexmin, 0 if it is not proven lexmin, 1 if it is lexmin
    // the cell to add or just added is (row, col).
    // We only check for permutations that move the cell (row, col), assuming
    // the previous cube is lexmin
    // Note that row permutation and column permutation are disjooint

    std::vector<size_t> row_idx(order);
    std::iota(row_idx.begin(), row_idx.end(), 0);
    std::vector<size_t> col_idx(order);
    std::iota(col_idx.begin(), col_idx.end(), 0);

    size_t start_col = 0;
    if (col > row+1)
        start_col = row+1;
    size_t start_row = row;  // no row permutation
    if (col == order - 1 && row < order - 1)
        start_row = 0;
    size_t end_col = col;
    if (row > 0 && col == order - 1)
        end_col++;
    for (size_t ridx = start_row; ridx <= row; ++ridx) {
        row_idx[ridx] = row;
        row_idx[row] = ridx;
        for (size_t cidx = start_col; cidx < end_col; ++cidx) {
            col_idx[cidx] = col;
            col_idx[col] = cidx;
            int check = check_binop(row, col, row_idx, col_idx, binop);
            if (check == -1)
                return -1;
            col_idx[cidx] = cidx;
            col_idx[col] = col;
        }
        row_idx[row] = row;
        row_idx[ridx] = ridx;
    }

    size_t val = Cells[binop[row] + col].get_value();
// std::cerr << val << std::endl;
    size_t start_val = val;
    size_t end_val = val;
    if (val < col && (val < row || (val == row && col == order - 1))) {
        start_val = 0;
    }
    else if (val > row && val > col+1) {
        start_val = std::max(row, col) + 1;
    }
// std::cerr << "  start_val: " << start_val << " row: " << row << " col: " << col << std::endl;
    for (size_t cidx = start_val; cidx < end_val; ++cidx) {
        col_idx[cidx] = val;
        col_idx[val] = cidx;
        int check = check_binop(row, col, row_idx, col_idx, binop);
        if (check == -1)
            return -1;
        col_idx[cidx] = cidx;
        col_idx[val] = val;
    }

    return 0;
}

