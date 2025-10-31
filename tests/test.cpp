#include <iostream>
#include "parser.h"

std::string str = R"(
pred = C^0_0 @ P^2_1 ;; x ~> x-1
minus2 = pred(pred) ;; x ~> x-2
tmp = minus2(P^3_2) ;; (x,y,z) ~> y-2
am2b = P^1_1 @ tmp ;; (b,a) ~> a-2b
div2cell = $am2b ;; n ~> cell(n `div` 2)
)";

int main(int argc, char* argv[])
{
    parser p(str);
    p.parse();
    std::cout << p.to_string();
    std::cout << p.eval_var("div2cell", {15}) << std::endl;
    return 0;
}
