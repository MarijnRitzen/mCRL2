// Author(s): Marijn Ritzen
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file TermBDD.cpp

#include <string>
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_string.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/function_symbol.h"
#include <cmath>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <boost/functional/hash.hpp>

using namespace atermpp;



class TermBDD: public aterm_appl 
{
public:
    TermBDD(): aterm_appl()
    {}
    
    TermBDD(const std::string& variable_name, TermBDD& high, TermBDD& low)
        : aterm_appl(function_symbol(variable_name, 2), high, low) 
    {} 

protected:
    TermBDD(const std::string& variable_name)
        : aterm_appl(function_symbol(variable_name, 0)) 
    {}
};


class Zero: public TermBDD 
{
public:
    Zero() 
    : TermBDD("zero") 
    {}
};
class One: public TermBDD 
{
public:
    One() 
    : TermBDD("one") 
    {}
};

enum Operation { OR, AND, IMPLIES, BI_IMPLIES, XOR, NEG };

// Define the tuple type
using KeyType = std::tuple<TermBDD, Operation, TermBDD>;

struct KeyHash : public std::unary_function<KeyType, std::size_t>
{
    std::size_t operator()(const KeyType& k) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, &std::get<0>(k));
        boost::hash_combine(seed, std::get<1>(k));
        boost::hash_combine(seed, &std::get<2>(k));
        return seed;
    }
};

// Declare the global hash map using the custom hash function and key type
// Define the global hash map
std::unordered_map<KeyType, TermBDD, KeyHash> map;

TermBDD neg(const TermBDD& bdd) {
    if (bdd.empty()) {
        if (bdd.function().name() == "one") {
            return Zero();
        } else {
            return One();
        }
    } 

    std::tuple tuple = std::make_tuple(bdd, Operation::NEG, bdd);

    if (map.find(tuple) != map.end()) {
        return map[tuple];
    }

    TermBDD new_high = neg(vertical_cast<TermBDD>(bdd[0]));
    TermBDD new_low = neg(vertical_cast<TermBDD>(bdd[1]));
    TermBDD result = TermBDD(bdd.function().name(), new_high, new_low);
    map[tuple] = result;
    return result;
}
int total_found = 0;
int total_not_found = 0;

TermBDD apply(const TermBDD& left, Operation op, const TermBDD& right) {
    if (left.empty() && right.empty()) {
        int left_int = left.function().name() == "one" ? 1 : 0;
        int right_int = right.function().name() == "one" ? 1 : 0;
        int result;

        switch (op) {
            case OR:
                result = left_int || right_int;
                break;
            case XOR:
                result = left_int != right_int;
                break;
            case AND:
                result = left_int && right_int;
                break;
            case IMPLIES:
                result = !left_int || right_int;
                break;
            case BI_IMPLIES:
                result = left_int == right_int;
                break;
            default:
                throw std::invalid_argument("Invalid operation");
                break;
        }

        if (result) {
            return One();
        } else {
            return Zero();
        }
    }
    std::tuple tuple = std::make_tuple(left, op, right);

    if (map.find(tuple) != map.end()) {
        total_found++;
        return map[tuple];
    }
    total_not_found++;

    int result;
    if (right.empty()) 
        result = -1;
    else if (left.empty()) 
        result = 1;
    else 
        result = left.function().name().compare(right.function().name());

    if (result < 0) { 
        TermBDD new_high = apply(vertical_cast<TermBDD>(left[0]), op, right);
        TermBDD new_low = apply(vertical_cast<TermBDD>(left[1]), op, right);
        if (new_high == new_low) {
            map[tuple] = new_high;
            map[std::make_tuple(right, op, left)] = new_high;
            return new_high;
        } else {
            TermBDD result = TermBDD(left.function().name(), new_high, new_low);
            map[tuple] = result;
            map[std::make_tuple(right, op, left)] = result;
            return result;
        }
    } else if (result > 0) {
        TermBDD new_high = apply(left, op, vertical_cast<TermBDD>(right[0]));
        TermBDD new_low = apply(left, op, vertical_cast<TermBDD>(right[1]));
        if (new_high == new_low) {
            map[tuple] = new_high;
            map[std::make_tuple(right, op, left)] = new_high;
            return new_high;
        } else {
            TermBDD result = TermBDD(right.function().name(), new_high, new_low);
            map[tuple] = result;
            map[std::make_tuple(right, op, left)] = result;
            return result;
        }
    } else {
        TermBDD new_high = apply(vertical_cast<TermBDD>(left[0]), op, vertical_cast<TermBDD>(right[0]));
        TermBDD new_low = apply(vertical_cast<TermBDD>(left[1]), op, vertical_cast<TermBDD>(right[1]));
        if (new_high == new_low) {
            map[tuple] = new_high;
            map[std::make_tuple(right, op, left)] = new_high;
            return new_high;
        } else {
            TermBDD result = TermBDD(right.function().name(), new_high, new_low);
            map[tuple] = result;
            map[std::make_tuple(right, op, left)] = result;
            return result;
        }
    }
}


int get_nr_of_solutions(TermBDD bdd, int depth, int maxdepth) {
    if (bdd.function().name() == "zero") {
        // The BDD is not satisfiable, return 0
        return 0;
    } else if (bdd.function().name() == "one") {
        // The BDD is already satisfied, return 1
        return  pow(2, (maxdepth - depth));
    } else {
        // Recursively count the number of satisfying assignments for the high child and low child
        int nr_of_solutions = get_nr_of_solutions(vertical_cast<TermBDD>(bdd[0]), depth + 1, maxdepth) + get_nr_of_solutions(vertical_cast<TermBDD>(bdd[1]), depth + 1, maxdepth);
        return nr_of_solutions;
    }
}

void eight_queens_bdd(int board_size) {
    // Create a TermBDD for each square on the board
    std::vector<TermBDD> squares(board_size * board_size);
    for (int i = 0; i < board_size * board_size; i++) {
        One one;
        Zero zero;
        squares[i] = TermBDD("s" + std::to_string(i), one, zero);
    }

    // Create TermBDDs for the rows, columns, and diagonals
    std::vector<TermBDD> rows_min(board_size);
    std::vector<TermBDD> rows_max(board_size);
    for (int i = 0; i < board_size; i++) {
        rows_min[i] = squares[i * board_size];
        for (int j = 1; j < board_size; j++) {
            rows_min[i] = apply(rows_min[i], Operation::OR, squares[i * board_size + j]);
        }
        One at_most_one;
        rows_max[i] = at_most_one;
        for (int j = 0; j < board_size; j++) {
            for (int k = j + 1; k < board_size; k++) {
                rows_max[i] = apply(rows_max[i], Operation::AND, neg(apply(squares[i * board_size + j], Operation::AND, squares[i * board_size + k])));
            }
        }
    }    

    std::vector<TermBDD> cols_min(board_size);
    std::vector<TermBDD> cols_max(board_size);
    for (int j = 0; j < board_size; j++) {
        cols_min[j] = squares[j];
        for (int i = 1; i < board_size; i++) {
            cols_min[j] = apply(cols_min[j], Operation::OR, squares[i * board_size + j]);
        }
        One at_most_one;
        cols_max[j] = at_most_one;
        for (int k = 0; k < board_size; k++) {
            for (int l = k + 1; l < board_size; l++) {
                cols_max[j] = apply(cols_max[j], Operation::AND, neg(apply(squares[k * board_size + j], Operation::AND, squares[l * board_size + j])));
            }
        }
    }    

    // At most 2 in every diagonal
    std::vector<TermBDD> diags((board_size * board_size - board_size) / 2);
    int index = 0;
    for (int i = 0; i < board_size; i++) {
        for (int i_prime = i + 1; i_prime < board_size; i_prime++) {
            One one;
            diags[index] = one;
            for (int j = 0; j < board_size; j++) {
                for (int j_prime = 0; j_prime < board_size; j_prime++) {
                    if ((i + j == i_prime + j_prime) || (i - j == i_prime - j_prime)) {
                        diags[index] = apply(diags[index], Operation::AND, apply(neg(squares[i * board_size + j]), Operation::OR, neg(squares[i_prime * board_size + j_prime])));
                    }
                }
            }
            index += 1;
        }
    }

    std::vector<TermBDD> constraints;
    constraints.insert(constraints.end(), rows_min.begin(), rows_min.end());
    constraints.insert(constraints.end(), rows_max.begin(), rows_max.end());
    constraints.insert(constraints.end(), cols_min.begin(), cols_min.end());
    constraints.insert(constraints.end(), cols_max.begin(), cols_max.end());
    constraints.insert(constraints.end(), diags.begin(), diags.end());
    TermBDD constraints_bdd = constraints[0];
    for (int i = 1; i < constraints.size(); i++) {
        constraints_bdd = apply(constraints_bdd, Operation::AND, constraints[i]);
    }
    std::cout << "solution: " << pp(constraints_bdd) << std::endl;
    std::cout << "nr solutions: " << get_nr_of_solutions(constraints_bdd, 0, board_size * board_size) << std::endl;
}


int main(int argc, char* argv[])
{
    eight_queens_bdd(8);
    std::cout << total_found << std::endl;
    std::cout << total_not_found << std::endl;
    
    return 0;
}

