// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps2torx.cpp

#define NAME "lps2torx"
#define AUTHOR "Muck van Weerdenburg"

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <climits>
#include <cstring>
#include <cassert>
#include <iostream>
#include <sstream>
#include <aterm2.h>
#include "mcrl2/core/struct.h"
#include "mcrl2/core/print.h"
#include "mcrl2/lps/nextstate.h"
#include "mcrl2/data/enum.h"
#include "mcrl2/data/rewrite.h"
#include "mcrl2/lps/dataelm.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // after messaging.h and rewrite.h

using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace mcrl2::data;
using namespace std;

#define is_tau(x) ATisEmpty((ATermList) ATgetArgument(x,0))
          
void print_torx_action(ostream &os, ATermAppl mact)
{
  if ( is_tau(mact) )
  {
    os << "tau";
  } else {
    ATermAppl act = (ATermAppl) ATgetFirst((ATermList) ATgetArgument(mact,0));
    PrintPart_CXX(cout,ATgetArgument(act,0), ppDefault);
    ATermList dl = (ATermList) ATgetArgument(act,1);
    for (; !ATisEmpty(dl); dl=ATgetNext(dl))
    {
      cout << "!";
      PrintPart_CXX(cout,ATgetFirst(dl), ppDefault);
    }
  }
}
  
typedef struct {
  int action;
  int state;
} index_pair;

class torx_data
{
  private:
    ATermIndexedSet stateactions;
    ATermTable state_indices;
    AFun fun_trip;
    unsigned int num_indices;

    ATerm triple(ATerm one, ATerm two, ATerm three)
    {
      return (ATerm) ATmakeAppl3(fun_trip,one,two,three);
    }

    ATerm third(ATerm trip)
    {
      return ATgetArgument((ATermAppl) trip,2);
    }

  public:
    torx_data(unsigned int initial_size)
    {
      stateactions = ATindexedSetCreate(initial_size,50);
      state_indices = ATtableCreate(initial_size,50);
      fun_trip = ATmakeAFun("@trip@",2,ATfalse);
      ATprotectAFun(fun_trip);
      num_indices = 0;
    }

    ~torx_data()
    {
      ATunprotectAFun(fun_trip);
      ATtableDestroy(state_indices);
      ATindexedSetDestroy(stateactions);
    }

    index_pair add_action_state(ATerm from, ATerm action, ATerm to)
    {
      ATbool is_new;
      index_pair p;

      p.action = ATindexedSetPut(stateactions,triple(from,action,to),&is_new);
      if ( is_new == ATtrue )
      {
        num_indices = num_indices + 1;
      }

      ATerm i;
      if ( (i = ATtableGet(state_indices,to)) == NULL )
      {
        ATtablePut(state_indices,to,(ATerm) ATmakeInt(p.action));
        p.state = p.action;
      } else {
        p.state = ATgetInt((ATermInt) i);
      }

      return p;
    }

    ATerm get_state(unsigned int index)
    {
      if ( index < num_indices )
      {
        return third(ATindexedSetGetElem(stateactions,index));
      } else {
        return NULL;
      }
    }
};

struct tool_options_type {
  bool            usedummies;
  bool            removeunused; 
  RewriteStrategy strategy;
  int             stateformat;
  std::string     name_for_input;
};

tool_options_type parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... INFILE\n"
    "Provide a TorX explorer interface to the LPS in INFILE. "
    "\n\n"
    "The LPS can be explored using TorX as described in torx_explorer(5).");

  clinterface.add_rewriting_options();

  clinterface.
    add_option("freevar",
      "do not replace free variables in the LPS with dummy values", 'f').
    add_option("dummy",
      "replace free variables in the LPS with dummy values (default)", 'y').
    add_option("unused-data",
      "do not remove unused parts of the data specification", 'u').
    add_option("vector",
      "store state in a vector (fastest, default)", 'c').
    add_option("tree",
      "store state in a tree (for memory efficiency)", 'T');

  command_line_parser parser(clinterface, ac, av);

  tool_options_type options;

  options.usedummies   = parser.options.count("freevar") == 0;
  options.usedummies   = 0 < parser.options.count("dummy");
  options.removeunused = parser.options.count("unused-data") == 0;
  options.strategy     = parser.option_argument_as< RewriteStrategy >("rewriter");
  options.stateformat  = GS_STATE_VECTOR;

  if (parser.options.count("vector")) {
    options.stateformat = GS_STATE_VECTOR; 
  }
  if (parser.options.count("tree")) {
    options.stateformat = GS_STATE_TREE; 
  }

  if (parser.arguments.size() == 0) {
    parser.error("no INFILE specified");
  } else if (parser.arguments.size() == 1) {
    options.name_for_input = parser.arguments[0];
  } else {
    //parser.arguments.size() > 1
    parser.error("too many file arguments");
  }

  return options;
}

int main(int argc, char **argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  try {
    tool_options_type options = parse_command_line(argc, argv);

    ATermAppl Spec;

    if (options.name_for_input.empty()) {
      gsVerboseMsg("reading LPS from stdin\n");

      Spec = (ATermAppl) ATreadFromFile(stdin);

      if (Spec == 0) {
        throw std::runtime_error("error: could not read LPS from stdin");
      }
      if (!gsIsSpecV1(Spec)) {
        throw std::runtime_error("error: stdin does not contain an LPS");
      }
    }
    else {
      gsVerboseMsg("reading LPS from '%s'\n", options.name_for_input.c_str());

      FILE *in_stream = fopen(options.name_for_input.c_str(), "rb");

      if (in_stream == 0) {
        throw std::runtime_error("error: could not open input file '" + options.name_for_input + "' for reading");
      }

      Spec = (ATermAppl) ATreadFromFile(in_stream);

      fclose(in_stream);

      if (Spec == 0) {
        throw std::runtime_error("error: could not read LPS from '" + options.name_for_input + "'");
      }
      if (!gsIsSpecV1(Spec)) {
        throw std::runtime_error("error: '" + options.name_for_input + "' does not contain an LPS");
      }
    }
 
    assert(gsIsSpecV1(Spec));
 
    if ( options.removeunused )
    {
      gsVerboseMsg("removing unused parts of the data specification.\n");
      Spec = removeUnusedData(Spec);
    }
 
    gsVerboseMsg("initialising...\n");
    torx_data td(10000);
 
    NextState *nstate = createNextState(
      Spec,
      !options.usedummies,
      options.stateformat,
      createEnumerator(
        data_specification(ATAgetArgument(Spec,0)),
        createRewriter(data_specification(ATAgetArgument(Spec,0)),options.strategy),
        true
      ),
      true
    );
 
    ATerm initial_state = nstate->getInitialState();
 
    ATerm dummy_action = (ATerm) ATmakeAppl0(ATmakeAFun("@dummy_action@",0,ATfalse));
    td.add_action_state(initial_state,dummy_action,initial_state);
 
    gsVerboseMsg("generating state space...\n");
 
    NextStateGenerator *nsgen = NULL;
    bool err = false;
    bool notdone = true;
    while ( notdone && !cin.eof() )
    {
      string s;
 
      cin >> s;
      if ( s.size() != 1 )
      {
              cout << "A_ERROR UnknownCommand: unknown or unimplemented command '" << s << "'" << endl;
              continue;
      }
 
      switch ( s[0] )
      {
        case 'r': // Reset
          // R event TAB solved TAB preds TAB freevars TAB identical
          cout << "R 0\t1\t\t\t" << endl;
          break;
        case 'e': // Expand
          {
          int index;
          ATerm state;
          
          cin >> index;
          state = td.get_state( index );
          if ( state == NULL )
          {
          	cout << "E0 value " << index << " not valid" << endl;
          	break;
          }
      
          cout << "EB" << endl;
          nsgen = nstate->getNextStates(state,nsgen);
          ATermAppl Transition;
          ATerm NewState;
          while ( nsgen->next(&Transition,&NewState) )
          {
            index_pair p;
      
            p = td.add_action_state(state,(ATerm) Transition,NewState);
 
            // Ee event TAB visible TAB solved TAB label TAB preds TAB freevars TAB identical
            cout << "Ee " << p.action << "\t" << (is_tau(Transition)?0:1) << "\t1\t";
            print_torx_action(cout,Transition);
            cout << "\t\t\t";
            if ( p.action != p.state )
            {
              cout << p.state;
            }
            cout << endl;
          }
          cout << "EE" << endl;
 
          if ( nsgen->errorOccurred() )
          {
            err = true;
            notdone = false;
          }
          break;
          }
        case 'q': // Quit
          cout << "Q" << endl;
          notdone = false;
          break;
        default:
          cout << "A_ERROR UnknownCommand: unknown or unimplemented command '" << s << "'" << endl;
          break;
      }
    }
    delete nsgen;
    delete nstate;
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_FAILURE;
}
