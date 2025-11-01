#include <iostream>
#include "parser.h"

std::string str = R"(
pred = C^0_0 @ P^2_1 ;; x ~> x-1
minus2 = pred(pred) ;; x ~> x-2
minus3 = pred(pred(pred)) ;; x ~> x-3
am2b = P^1_1 @ minus2(P^3_2) ;; (b,a) ~> a-2b
div2cell = $am2b ;; n ~> cell(n `div` 2)
if = P^2_1 @ P^4_4 ;; if(p,a,b) = if p==0 then a else b
)";

int main(int argc, char* argv[])
{
    parser p(str);
    p.parse();
    std::cout << p.to_string();
    std::cout << p.eval_var("div2cell", {15}) << std::endl;
    std::cout << p.eval_var("if", {7,33,44}) << std::endl;
    std::cout << p.eval_var("minus3", {11}) << std::endl;
    
    return 0;
}
