// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/regular_formula.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_REGULAR_FORMULA_H
#define MCRL2_MODAL_REGULAR_FORMULA_H

#include <iostream> // for debugging

#include <string>
#include <cassert>
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/atermpp.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/soundness_checks.h"

namespace mcrl2 {

namespace modal {

///////////////////////////////////////////////////////////////////////////////
// regular_formula
/// \brief regular formula expression.
class regular_formula: public atermpp::aterm_appl
{
  public:
    /// \brief Constructor
    regular_formula()
      : atermpp::aterm_appl(mcrl2::core::detail::constructRegFrm())
    {}

    /// \brief Constructor
    /// \param t A term
    regular_formula(ATermAppl t)
      : atermpp::aterm_appl(atermpp::aterm_appl(t))
    {
      assert(mcrl2::core::detail::check_rule_RegFrm(m_term));
    }

    /// \brief Constructor
    /// \param t A term
    regular_formula(atermpp::aterm_appl t)
      : atermpp::aterm_appl(t)
    {
      assert(mcrl2::core::detail::check_rule_RegFrm(m_term));
    }

    /// \brief Applies a low level substitution function to this term and returns the result.
    /// \param f A
    /// The function <tt>f</tt> must supply the method <tt>aterm operator()(aterm)</tt>.
    /// This function is applied to all <tt>aterm</tt> noded appearing in this term.
    /// \deprecated
    /// \return The substitution result.
    template <typename Substitution>
    regular_formula substitute(Substitution f) const
    {
      return regular_formula(f(atermpp::aterm(*this)));
    }
};

///////////////////////////////////////////////////////////////////////////////
// regular_formula_list
/// \brief Read-only singly linked list of regular expressions
typedef atermpp::term_list<regular_formula> regular_formula_list;

} // namespace modal

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::modal::regular_formula)
/// \endcond

#endif // MCRL2_MODAL_REGULAR_FORMULA_H
