#ifndef PARSER_H
#define PARSER_H


#include <cctype>
#include <vector>
#include <variant>
#include <memory>
#include <stdexcept>
#include <map>
#include "types.h"

/*
<program> = <line> {'\n'+ <line>}*
<line> = <variable> '=' <expression> [';' <comment>]
<expression> = <identifer> '(' <identifer> {',' <identifer>}* ')' 
             | <identifer> '@' <identifer>
             | '$' <identifer>
<identifer> = 'C^'<num>'_'<num> | 'P^'<num>'_'<num> | 'S' | <variable>
<variable> = {'a' | ... | 'z'}{'A' | ... | 'Z' | 'a' | ... | 'z' | '0' | '1' | ... | '9'}*
<comment> = {any character except newline}*

White space (spaces and tabs) can appear between any two tokens and should be ignored.
*/


// struct line 
// {
//     std::shared_ptr<variable> lvalue;
//     std::unique_ptr<expression> rvalue;
//     line(const std::shared_ptr<variable>& lvalue,
//          std::unique_ptr<expression>&& rvalue)
//         : lvalue(lvalue), rvalue(std::move(rvalue)) {}
//     std::string to_string() const 
//     {
//         return lvalue->to_string(true) + " = " + rvalue->to_string() + "\n";
//     }
// };

enum class token_t {
    NEWLINE,
    EQUAL,
    LEFT_PAREN, RIGHT_PAREN,
    COMMA, PR_SYM, MIN_SYM,
    CONST, PROJ, SUCC,
    VARIABLE,
    END
};


class parser 
{
    //cache for token values
    token_t token;
    std::string input;
    size_t pos;
    int num1, num2;
    std::string var_name;
    
    std::vector<std::shared_ptr<variable>> program;
    std::map<std::string, size_t> context;

public:
    parser(const std::string &input) : input(input), pos(0) 
    {
        next_token();
    }
    // Lexer
    void next_token();
    // Parser
    std::shared_ptr<identifer> parse_identifer();
    std::unique_ptr<composition> parse_composition();
    std::unique_ptr<primitive_recursion> parse_primitive_recursion();
    std::unique_ptr<minimization> parse_minimization();
    void parse_line();
    void parse();
    // Interpreter
    natural eval_var(std::string s, std::vector<natural> operands);
    // help functions
    std::shared_ptr<variable> get_variable(const std::string& name);
    void add_variable(const std::shared_ptr<variable>& var);
    std::string to_string() const;
};

#endif // PARSER_H
