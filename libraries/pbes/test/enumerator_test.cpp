// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file enumerator_test.cpp
/// \brief Add your file description here.

#define BOOST_TEST_MODULE enumerator_test
#include <boost/test/included/unit_test_framework.hpp>
#include "mcrl2/core/detail/print_utility.h"
#include "mcrl2/data/substitutions/mutable_map_substitution.h"
#include "mcrl2/pbes/enumerator.h"
#include "mcrl2/pbes/detail/parse.h"
#include "mcrl2/pbes/rewriters/simplify_rewriter.h"

using namespace mcrl2;
using namespace mcrl2::pbes_system;

  const std::string VARSPEC =
    "datavar         \n"
    "  m: Nat;       \n"
    "  n: Nat;       \n"
    "  b: Bool;      \n"
    "  c: Bool;      \n"
    "                \n"
    "predvar         \n"
    "  X: Bool, Pos; \n"
    "  Y: Nat;       \n"
    ;

BOOST_AUTO_TEST_CASE(test_enumerator)
{
  typedef pbes_system::simplify_data_rewriter<data::rewriter> pbes_rewriter;
  typedef data::enumerator_list_element<pbes_expression> enumerator_element;

  data::data_specification data_spec;
  data_spec.add_context_sort(data::sort_nat::nat());
  data::rewriter datar(data_spec);
  pbes_rewriter R(datar);

  data::variable_list v;
  v.push_front(data::variable("n", data::sort_nat::nat()));
  pbes_expression phi = parse_pbes_expression("val(n < 2)", VARSPEC);
  data::mutable_indexed_substitution<> sigma;
  data::enumerator_identifier_generator id_generator("x");
  data::enumerator_algorithm<pbes_rewriter> E(R, data_spec, datar, id_generator);
  std::set<pbes_system::pbes_expression> solutions;
  data::enumerator_queue<enumerator_element> P(enumerator_element(v, phi));
  E.enumerate_all(P, sigma, is_not_false(),
      [&](const enumerator_element& p)
      {
        solutions.insert(p.expression());
        return false; // do not interrupt
      }
  );
  std::clog << "solutions = " << core::detail::print_list(solutions) << std::endl;
  BOOST_CHECK(solutions.size() == 1);
}

BOOST_AUTO_TEST_CASE(test_enumerator_with_iterator)
{
  typedef pbes_system::simplify_data_rewriter<data::rewriter> pbes_rewriter;
  typedef data::enumerator_list_element<pbes_expression> enumerator_element;

  data::data_specification data_spec;
  data_spec.add_context_sort(data::sort_nat::nat());
  data::rewriter datar(data_spec);
  pbes_rewriter R(datar);

  data::variable_list v;
  v.push_front(data::variable("n", data::sort_nat::nat()));
  pbes_expression phi = parse_pbes_expression("val(n < 2)", VARSPEC);
  data::mutable_indexed_substitution<> sigma;
  data::enumerator_identifier_generator id_generator;
  data::enumerator_algorithm_with_iterator<pbes_rewriter, enumerator_element, pbes_system::is_not_true> E(R, data_spec, datar, id_generator, 20);
  std::vector<pbes_system::pbes_expression> solutions;

  data::enumerator_queue<enumerator_element> P;
  P.push_back(enumerator_element(v, phi));
  for (auto i = E.begin(sigma, P); i != E.end(); ++i)
  {
    solutions.push_back(i->expression());
  }
  std::clog << "solutions = " << core::detail::print_list(solutions) << std::endl;
  BOOST_CHECK(solutions.size() >= 1);
}

BOOST_AUTO_TEST_CASE(test_enumerator_with_substitutions)
{
  typedef pbes_system::simplify_data_rewriter<data::rewriter> pbes_rewriter;
  typedef data::enumerator_list_element_with_substitution<pbes_expression> enumerator_element;

  data::data_specification data_spec;
  data_spec.add_context_sort(data::sort_nat::nat());
  data::rewriter datar(data_spec);
  pbes_rewriter R(datar);

  data::variable_list v;
  v.push_front(data::variable("n", data::sort_nat::nat()));
  pbes_expression phi = parse_pbes_expression("val(n < 2)", VARSPEC);
  data::mutable_indexed_substitution<> sigma;
  data::enumerator_identifier_generator id_generator;
  data::enumerator_algorithm_with_iterator<pbes_rewriter, enumerator_element, pbes_system::is_not_false> E(R, data_spec, datar, id_generator);
  std::vector<pbes_system::pbes_expression> solutions;

  data::enumerator_queue<enumerator_element> P;
  P.push_back(enumerator_element(v, phi));
  for (auto i = E.begin(sigma, P); i != E.end(); ++i)
  {
    solutions.push_back(i->expression());
    data::mutable_map_substitution<> sigma;
    i->add_assignments(v, sigma, datar);
    std::clog << "  solutions " << i->expression() << " substitution = " << sigma << std::endl;
    BOOST_CHECK(R(phi, sigma) == i->expression());
  }
  std::clog << "solutions = " << core::detail::print_list(solutions) << std::endl;
  BOOST_CHECK(solutions.size() >= 1);
}

BOOST_AUTO_TEST_CASE(enumerate_callback)
{
  typedef pbes_system::simplify_data_rewriter<data::rewriter> pbes_rewriter;
  typedef data::enumerator_list_element<pbes_expression> enumerator_element;
  data::enumerator_identifier_generator id_generator;
  data::data_specification dataspec;
  dataspec.add_context_sort(data::sort_int::int_());
  std::size_t max_count = 10;
  bool throw_exceptions = true;
  data::rewriter r(dataspec);
  pbes_rewriter R(r);
  data::enumerator_algorithm<pbes_rewriter> E(R, dataspec, r, id_generator, max_count, throw_exceptions);

  auto enumerate = [&](const pbes_expression& x)
  {
    data::rewriter::substitution_type sigma;
    pbes_expression result;
    id_generator.clear();
    if (is_forall(x))
    {
      const auto& x_ = atermpp::down_cast<forall>(x);
      result = true_();
      data::enumerator_queue<enumerator_element> P(enumerator_element(x_.variables(), R(x_.body())));
      E.enumerate_all(P, sigma, is_not_true(),
                      [&](const enumerator_element& p)
                      {
                        std::cout << "solution: " << p << std::endl;
                        result = data::optimized_and(result, p.expression());
                        return is_false(result);
                      }
      );
    }
    else if (is_exists(x))
    {
      const auto& x_ = atermpp::down_cast<exists>(x);
      result = false_();
      data::enumerator_queue<enumerator_element> P(enumerator_element(x_.variables(), R(x_.body())));
      E.enumerate_all(P, sigma, is_not_false(),
                      [&](const enumerator_element& p)
                      {
                        std::cout << "solution: " << p << std::endl;
                        result = data::optimized_or(result, p.expression());
                        return is_true(result);
                      }
      );
    }
    return result;
  };

  BOOST_CHECK_EQUAL(enumerate(parse_pbes_expression("forall n: Nat. val(n < 2)")), false_());
  BOOST_CHECK_EQUAL(enumerate(parse_pbes_expression("exists n: Nat. val(n < 2)")), true_());
}
