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
<program>     ::= <line> {'\n'+ <line>}*
<line>        ::= <variable> '=' <expression> [';' <comment>]
<expression>  ::= <comp-exp> '@' <comp-exp>
                | '$' <comp-exp>
                | <comp-exp>
<comp-exp>    ::= <primary-exp> ['(' <expression> {',' <expression>}* ')']
<primary-exp> ::= <atomic-exp> | '(' <expression> ')'
<atomic-exp>  ::= <identifer>
<identifer>   ::= 'C'<num>'_'<num> | 'P'<num>'_'<num> | 'S' | <variable>
<variable> ::= {'a' | ... | 'z'}{'A' | ... | 'Z' | 'a' | ... | 'z' 
                               | '0' | '1' | ... | '9' | '_'}*
<comment>  ::= {any character except newline}*

White space (spaces and tabs) can appear between any two tokens and should be ignored.
*/


enum class token_t {
    NEWLINE,
    EQUAL,
    LEFT_PAREN, RIGHT_PAREN,
    COMMA, PR_SYM, MIN_SYM,
    CONST, PROJ, SUCC,
    VARIABLE, NUM,
    END
};
std::ostream& operator<<(std::ostream& os, token_t t);


class parser 
{
    std::string input;
    //cache for token values
    struct {
        token_t token;
        size_t pos;
        int num1, num2;
        std::string var_name;
    } cache;
    std::vector<std::shared_ptr<variable>> program;
    std::map<std::string, size_t> context;
    parser(const std::string &input) noexcept : input(input) {}
public:
    static std::unique_ptr<parser> create(std::string input);
    void set_input(const std::string &input);
    // Lexer
    void next_token();
    // Parser
    std::shared_ptr<identifier> parse_identifer();
    std::unique_ptr<composition> parse_composition();
    std::unique_ptr<primitive_recursion> parse_primitive_recursion();
    std::unique_ptr<minimization> parse_minimization();
    std::unique_ptr<expression> parse_expression();
    std::unique_ptr<expression> parse_comp_exp();
    std::unique_ptr<expression> parse_atomic_exp();
    std::shared_ptr<variable> parse_line();
    void parse();
    // Interpreter
    natural eval_var(std::shared_ptr<variable> v, std::vector<natural> operands);
    natural eval_var(std::string s, std::vector<natural> operands);
    // help functions
    std::shared_ptr<variable> get_variable(const std::string& name) noexcept;
    void add_variable(const std::shared_ptr<variable>& var);
    std::string to_string() const;
};

#endif // PARSER_H
