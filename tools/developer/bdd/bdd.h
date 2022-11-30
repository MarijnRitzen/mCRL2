#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/function_symbol.h"

using namespace atermpp;

class BDD {
    
public:
    aterm term;
    BDD* high;
    BDD* low;

    BDD(); 
    static BDD from(BDD&);
    static BDD make(std::string&&, BDD*, BDD*);
    static BDD true_terminal();
    static BDD false_terminal();

    aterm get_term();
    BDD* get_high();
    BDD* get_low();

    std::string toString() const;
    BDD operator!();
};