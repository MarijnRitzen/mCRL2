// Author(s): Marijn Ritzen
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file bdd.cpp

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_string.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/function_symbol.h"
#include "./bdd.h"

using namespace atermpp;

BDD::BDD() {

}
    
BDD BDD::make(std::string&& variable_name, BDD* high, BDD* low) {
    function_symbol f(variable_name, 2);    // the function symbol f of arity 2.
    aterm_appl term(f, high->get_term(), low->get_term());        // represents f(x,y). These constructors exist up till arity 7.

    BDD bdd;
    bdd.term = term;
    bdd.high = high;
    bdd.low = low;
    return bdd;
} 


BDD* BDD::get_high() {
    return this->high;
} 

BDD* BDD::get_low() {
    return this->low;
} 

aterm BDD::get_term() {
    return this->term;
} 

BDD BDD::true_terminal() {
    aterm_int term(1);

    BDD bdd;
    bdd.term = term;
    return bdd;
} 

BDD BDD::false_terminal() {
    aterm_int term(0);

    BDD bdd;
    bdd.term = term;
    return bdd;
} 

std::string BDD::toString() const
{
    return pp(this->term);
}


BDD BDD::operator!() {
    // Leaf
    if (this->term.type_is_int()) {
        aterm_int new_term(1 - static_cast<aterm_int>(this->term).value());
        // std::cout << " old value is " << static_cast<aterm_int>(this->term).value() << std::endl;
        this->term = new_term;
        // std::cout << " new value is " << new_term.value() << std::endl;
        return *this;
    }

    // Node
    function_symbol function = (this->term).function();

    aterm_appl new_term(function, (!(*this->get_high())).get_term(), (!(*this->get_low())).get_term());
    this->term = new_term;
    
    return *this;
}

int main(int argc, char* argv[])
{
    BDD high = BDD::true_terminal();
    BDD low = BDD::false_terminal();
    BDD p = BDD::make("p", &high, &low);
        
    BDD other_low = BDD::false_terminal();
    BDD q = BDD::make("q", &p, &other_low);

    std::cout << (q).toString() << '\n';
    std::cout << (!q).toString() << '\n';
}
