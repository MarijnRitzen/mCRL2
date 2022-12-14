// Author(s): Marijn Ritzen
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file TermBDD.cpp

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_string.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/function_symbol.h"

using namespace atermpp;

class TermBDD: public aterm_appl 
{
public:
    TermBDD(const std::string& variable_name, TermBDD& high, TermBDD& low)
        : aterm_appl(function_symbol(variable_name, 2), high, low) 
    {} 

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

enum Operation { OR, AND, IMPLIES };

void apply(TermBDD& left, Operation op, TermBDD& right) {
    std::cout << pp(left);
    std::cout << pp(right);
}

int main(int argc, char* argv[])
{
    Zero zero;
    One one;
    TermBDD r("r", zero, one);
    TermBDD p("p", r, one);
    apply(r, Operation::OR, p);
}
