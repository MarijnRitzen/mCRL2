// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/traverse.h
/// \brief add your file description here.

#ifndef MCRL2_LPS_TRAVERSE_H
#define MCRL2_LPS_TRAVERSE_H

#include "mcrl2/lps/detail/lps_sort_traverser.h"
#include "mcrl2/lps/detail/lps_variable_traverser.h"

namespace mcrl2 {

namespace lps {

  /// \brief Traverses all sort expressions in the data type o, and writes them
  /// to the output iterator dest.
  template <typename Object, typename OutIter>
  void traverse_sort_expressions(const Object& o, OutIter dest)
  {
    lps::detail::lps_sort_traverser<OutIter> r(dest);
    r(o);
  }

  /// \brief Traverses all variables in the data type o, and writes them
  /// to the output iterator dest.
  template <typename Object, typename OutIter>
  void traverse_variables(const Object& o, OutIter dest)
  {
    lps::detail::lps_variable_traverser<OutIter> r(dest);
    r(o);
  }

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_TRAVERSE_H
