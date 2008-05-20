// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file filter_template.cpp
/// \brief Add your file description here.

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib> // EXIT_SUCCESS
#include <boost/program_options.hpp>
#include "mcrl2/lps/detail/filter_program.h"

using namespace std;
using namespace mcrl2::lps;
namespace po = boost::program_options;

/// The version of the program.
///
const char* program_version = "0.1";

/// The extension of the input file.
///
const char* input_extension = ".lps";

/// The extension of the output file.
///
const char* output_extension = ".lps";

/// The filter program.
///
class my_program: public filter_program
{
 public:
    my_program(const std::string& name,
               const std::string& version,
               const std::string& input_extension,
               const std::string& output_extension,
               const std::string& input_file,
               const std::string& output_file,
               bool quiet,
               bool verbose,
               bool debug
              )
      : filter_program(name, version, input_extension, output_extension, input_file, output_file, quiet, verbose, debug)
    {}

    /// Run the program.
    ///
    void run()
    {
      std::cerr << "name:    " << name() << "\n"
                << "version: " << version()<< "\n"
                << "input:   " << input_file() << "\n"
                << "output:  " << output_file() << "\n"
                << "quiet:   " << (quiet() ? "true" : "false") << "\n"
                << "verbose: " << (verbose() ? "true" : "false") << "\n"
                << "debug:   " << (debug() ? "true" : "false") << std::endl;

      specification spec = read_specification();
      if (!spec)
      {
        throw std::runtime_error("failed to read specification from " + input_file());
      }
      if (!write_specification(spec))
      {
        throw std::runtime_error("failed to write specification to " + output_file());
      }
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  std::string input_filename;
  std::string output_filename;

  try {
    //--- program options ---------
    po::options_description program_options("Allowed options");
    program_options.add_options()
      ("help,h",    "display help information")
      ("version",   "display version information")
      ("quiet,q",   "do not display warning messages")
      ("verbose,v", "turn on the display of short intermediate messages")
      ("debug,d",   "turn on the display of detailed intermediate messages")
    ;

    //--- hidden options ---------
    po::options_description hidden_options;
    hidden_options.add_options()
      ("input-file", po::value<std::string>(&input_filename), "input file")
      ("output-file", po::value<std::string>(&output_filename), "output file")
    ;

    //--- positional options ---------
    po::positional_options_description positional_options;
    positional_options.add("input-file", 1);
    positional_options.add("output-file", 2);

    //--- command line options ---------
    po::options_description cmdline_options;
    cmdline_options.add(program_options).add(hidden_options);

    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv).
        options(cmdline_options).positional(positional_options).run(), var_map);
    po::notify(var_map);    

    my_program program(argv[0],
                       program_version,
                       input_extension,
                       output_extension,
                       input_filename,
                       output_filename,
                       var_map.count("quiet") != 0,
                       var_map.count("verbose") != 0,
                       var_map.count("debug") != 0
                      );

    if (var_map.count("help"))
    {
      std::cerr << "Usage: "<< program.name() << " [OPTION]... INFILE [OUTFILE] \n\n"
                << program_options;
      return EXIT_SUCCESS;
    }
        
    if (var_map.count("version"))
    {
      std::cerr << program.name() << " " << program.version() << std::endl;
      return EXIT_SUCCESS;
    }
    
    program.run();
  }
  catch(std::runtime_error e)
  {
    std::cerr << "parse error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
  catch(...) {
    std::cerr << "exception of unknown type!\n";
    return EXIT_FAILURE;
  }   
    
  return EXIT_SUCCESS;
}
