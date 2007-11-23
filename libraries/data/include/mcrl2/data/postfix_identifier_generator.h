// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/postfix_identifier_generator.h
/// \brief Add your file description here.

#ifndef MCRL2_DATA_POSTFIX_IDENTIFIER_GENERATOR_H
#define MCRL2_DATA_POSTFIX_IDENTIFIER_GENERATOR_H

#include "mcrl2/data/set_identifier_generator.h"

namespace lps {

/// Uses the given postfix as a hint for creating new names.
class postfix_identifier_generator: public set_identifier_generator
{
  protected:
    std::string m_postfix;

  public:
    postfix_identifier_generator(std::string postfix)
      : m_postfix(postfix)
    {}

    /// Returns a unique identifier, with the given hint as prefix.
    /// The returned identifier is added to the context.
    virtual identifier_string operator()(const std::string& hint)
    {
      identifier_string id(hint);
      int index = 0;
      while (has_identifier(id))
      {   
        std::string name = str(boost::format(hint + m_postfix + "%02d") % index++);
        id = identifier_string(name);
      }
      add_to_context(id);
      return id;
    }   
};

} // namespace lps

#endif // MCRL2_DATA_POSTFIX_IDENTIFIER_GENERATOR_H
