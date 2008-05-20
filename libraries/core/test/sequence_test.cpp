#include <numeric>
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
#include <vector>
#include <boost/test/minimal.hpp>
#include "mcrl2/core/sequence.h"

using namespace mcrl2;

struct f
{
  std::vector<int>& v_;
  int& sum_;

  f(std::vector<int>& v, int& sum)
    : v_(v), sum_(sum)
  {}

  // Adds the sum of the elements of v_ to sum_.
  void operator()() const
  {
    for (std::vector<int>::const_iterator i = v_.begin(); i != v_.end(); ++i)
    {
      std::cout << *i << " ";
    }
    std::cout << std::endl;
    sum_ += std::accumulate(v_.begin(), v_.end(), 0);
  }
};

void test_sequence()
{
  std::vector<int> a;
  std::vector<int> b;
  std::vector<int> c;

  a.push_back(1);
  a.push_back(2);
  b.push_back(3);
  c.push_back(2);
  c.push_back(5);
  
  std::vector<std::vector<int> > v;
  v.push_back(a);
  v.push_back(b);
  v.push_back(c);
  
  int sum = 0;
  
  // w will hold a sequence
  std::vector<int> w(v.size());

  core::foreach_sequence(v, w.begin(), f(w, sum));
  std::cout << "sum == " << sum << std::endl;
  BOOST_CHECK(sum == 32);
}

int test_main(int, char*[])
{
  test_sequence();
  
  return 0;
}
