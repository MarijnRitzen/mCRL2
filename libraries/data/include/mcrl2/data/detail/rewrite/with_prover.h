// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/rewrite/with_prover.h
/// \brief Rewriting combined with semantic simplification using a prover

#ifndef __REWR_PROVER_H
#define __REWR_PROVER_H

#include "mcrl2/aterm/aterm2.h"
#include <mcrl2/data/detail/bdd_prover.h>
#include "mcrl2/data/rewriter.h"

namespace mcrl2
{
namespace data
{
namespace detail
{

class RewriterProver: public Rewriter
{
  public:
    BDD_Prover* prover_obj;
    boost::shared_ptr<detail::Rewriter> rewr_obj; 

  public:
    RewriterProver(const data_specification& DataSpec, mcrl2::data::rewriter::strategy strat, const bool add_rewrite_rules);
    ~RewriterProver();

    mcrl2::data::detail::RewriteStrategy getStrategy();

    data_expression rewrite(
         const data_expression term,
         mutable_map_substitution<> &sigma);

    atermpp::aterm_appl rewrite_internal(
         const atermpp::aterm_appl term,
         mutable_map_substitution<atermpp::map < variable,atermpp::aterm_appl> > &sigma);

    atermpp::aterm_appl toRewriteFormat(const data_expression term);
    data_expression fromRewriteFormat(const atermpp::aterm_appl term);

    bool addRewriteRule(const data_equation rule);
    bool removeRewriteRule(const data_equation rule);

};

}
}
}

#endif
