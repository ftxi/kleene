#include <iostream>
#include "parser.h"

std::string str = R"(
pred = 0 @ P2_1 ;; x ~> x-1
minus2 = pred(pred) ;; x ~> x-2
minus3 = pred(pred(pred)) ;; x ~> x-3
id = P1_1
div3cell = $(id @ minus3(P3_2)) ;; n ~> cell(n `div` 2)
if = P2_1 @ P4_4 ;; if(p,a,b) = if p==0 then a else b

add = id @ S(P3_2)
mul = C1_0 @ add(P3_3, P3_2)

foo = P2_1 @ S
)";

int main(int argc, char* argv[])
{
    parser p(str);
    p.parse();
    std::cout << p.to_string();
    std::cout << p.eval_var("div3cell", {15}) << std::endl;
    std::cout << p.eval_var("if", {7,33,44}) << std::endl;
    std::cout << p.eval_var("minus3", {11}) << std::endl;
    std::cout << p.eval_var("mul", {7,8}) << std::endl;
    
    
    return 0;
}
