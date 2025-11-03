#include "parser.h"
#include <tuple>

std::ostream& operator<<(std::ostream& os, token_t t) {
    switch (t) {
        case token_t::NEWLINE:    return os << "NEWLINE";
        case token_t::EQUAL:      return os << "EQUAL";
        case token_t::LEFT_PAREN: return os << "LEFT_PAREN";
        case token_t::RIGHT_PAREN:return os << "RIGHT_PAREN";
        case token_t::COMMA:      return os << "COMMA";
        case token_t::PR_SYM:     return os << "PR_SYM";
        case token_t::MIN_SYM:    return os << "MIN_SYM";
        case token_t::CONST:      return os << "CONST";
        case token_t::PROJ:       return os << "PROJ";
        case token_t::SUCC:       return os << "SUCC";
        case token_t::VARIABLE:   return os << "VARIABLE";
        case token_t::NUM:        return os << "NUM";
        case token_t::END:        return os << "END";
    }
    return os; // optional: silence compiler warning
}

std::unique_ptr<parser> parser::create(std::string input)
{
    auto res = std::unique_ptr<parser>(new parser(input));
    res->cache.pos = 0;
    res->next_token();
    return res;
}

void parser::set_input(const std::string &input)
{
    this->input = input;
    cache.pos = 0;
    next_token();
}

/****** Lexer ******/

void parser::next_token()
{
    // Skip whitespace (excluding newlines)
    while (cache.pos < input.size() && (input[cache.pos] == ' ' || input[cache.pos] == '\t' || input[cache.pos] == '\r')) {
        cache.pos++;
    }

    if(input[cache.pos] == ';')
    {
        // Skip comment until newline or end of input
        cache.pos++; // Skip the ';'
        while (cache.pos < input.size() && input[cache.pos] != '\n') {
            cache.pos++;
        }
        // After skipping comment, get the next cache.token
        next_token();
        return;
    }
    
    // Check for end of input
    if (cache.pos >= input.size()) {
        cache.token = token_t::END;
        return;
    }
    
    switch (input[cache.pos]) 
    {
    case '\n':
        cache.token = token_t::NEWLINE;
        cache.pos++;
        break;
        
    case '=':
        cache.token = token_t::EQUAL;
        cache.pos++;
        break;
        
    case '(':
        cache.token = token_t::LEFT_PAREN;
        cache.pos++;
        break;
        
    case ')':
        cache.token = token_t::RIGHT_PAREN;
        cache.pos++;
        break;
        
    case ',':
        cache.token = token_t::COMMA;
        cache.pos++;
        break;
        
    case '@':
        cache.token = token_t::PR_SYM;
        cache.pos++;
        break;
        
    case '$':
        cache.token = token_t::MIN_SYM;
        cache.pos++;
        break;
        
    case 'C':
        // CONST: C<num>_<num>
        cache.pos++;
        if (cache.pos >= input.size() || !isdigit(input[cache.pos])) {
            throw parse_error("Expected digit after 'C'");
        }
        // Read first number
        cache.num1 = 0;
        while (cache.pos < input.size() && isdigit(input[cache.pos])) {
            cache.num1 = cache.num1 * 10 + (input[cache.pos] - '0');
            cache.pos++;
        }
        if (cache.pos >= input.size() || input[cache.pos] != '_') {
            throw parse_error("Expected '_' in CONST cache.token");
        }
        cache.pos++; // Skip '_'
        if (cache.pos >= input.size() || !isdigit(input[cache.pos])) {
            throw parse_error("Expected digit after '_' in CONST cache.token");
        }
        // Read second number
        cache.num2 = 0;
        while (cache.pos < input.size() && isdigit(input[cache.pos])) {
            cache.num2 = cache.num2 * 10 + (input[cache.pos] - '0');
            cache.pos++;
        }
        cache.token = token_t::CONST;
        break;
        
    case 'P':
        // PROJ: P<num>_<num>
        cache.pos++;
        if (cache.pos >= input.size() || !isdigit(input[cache.pos])) {
            throw parse_error("Expected digit after 'P'");
        }
        // Read first number
        cache.num1 = 0;
        while (cache.pos < input.size() && isdigit(input[cache.pos])) {
            cache.num1 = cache.num1 * 10 + (input[cache.pos] - '0');
            cache.pos++;
        }
        if (cache.pos >= input.size() || input[cache.pos] != '_') {
            throw parse_error("Expected '_' in PROJ cache.token");
        }
        cache.pos++; // Skip '_'
        if (cache.pos >= input.size() || !isdigit(input[cache.pos])) {
            throw parse_error("Expected digit after '_' in PROJ cache.token");
        }
        // Read second number
        cache.num2 = 0;
        while (cache.pos < input.size() && isdigit(input[cache.pos])) {
            cache.num2 = cache.num2 * 10 + (input[cache.pos] - '0');
            cache.pos++;
        }
        if(cache.num1 == 0 || cache.num2 > cache.num1)
        {
            throw parse_error("Invalid projection indices: P" + std::to_string(cache.num2) + "_" + std::to_string(cache.num1));
        }
        cache.token = token_t::PROJ;
        break;
        
    case 'S':
        cache.token = token_t::SUCC;
        cache.pos++;
        break;
        
    default:
        // VARIABLE: starts with lowercase, followed by alphanumerics
        if (islower(input[cache.pos])) {
            cache.var_name.clear();
            cache.var_name += input[cache.pos];
            cache.pos++;
            while (cache.pos < input.size() && (isalnum(input[cache.pos]) || input[cache.pos] == '_')) {
                cache.var_name += input[cache.pos];
                cache.pos++;
            }
            cache.token = token_t::VARIABLE;
        }
        else if(isdigit(input[cache.pos])) {
            cache.num2 = cache.num1 = 0;
            while (cache.pos < input.size() && isdigit(input[cache.pos])) {
                cache.num2 = cache.num2 * 10 + (input[cache.pos] - '0');
                cache.pos++;
            }
            cache.token = token_t::NUM;
        }
        else {
            throw parse_error(std::string("Unexpected character: '") + input[cache.pos] + "'");
        }
        break;
    }
    dprint("token:", cache.token, "before", cache.pos);
}

/****** Parser ******/

#define PARSE_START(t) auto [old_cache, parse_type] =  std::tuple{cache,t}; dprint("parse:", t);
#define PARSE_FAIL { \
    cache = old_cache; \
    dprint("failed to parse", parse_type, "; fallback"); \
    return nullptr; \
}

std::shared_ptr<identifier> parser::parse_identifer()
{
    PARSE_START("identifer");
    std::shared_ptr<identifier> result = nullptr;
    switch(cache.token)
    {
        case token_t::CONST: // parse constant
        case token_t::NUM: // number is just a syntatic sugar for C^0_k
            result = std::make_shared<constant>(cache.num1, cache.num2);
            next_token();
            break;
        case token_t::PROJ: // parse projection
            result = std::make_shared<projection>(cache.num1, cache.num2);
            next_token();
            break;
        case token_t::SUCC: // parse successor
            result = std::make_shared<successor>();
            next_token();
            break;
        case token_t::VARIABLE: // parse variable
            result = get_variable(cache.var_name);
            if(result == nullptr)
            {
                throw parse_error("Undefined variable: " + cache.var_name);
            }
            next_token();
            break;
        default:
            PARSE_FAIL;
    }
    dprint("parsed identifer:", result->show_type());
    return result;
}

// <atomic-exp> '(' <expression> {',' <expression>}* ')'
std::unique_ptr<composition> parser::parse_composition()
{
    PARSE_START("<composition>");
    std::shared_ptr<expression> f = parse_atomic_exp();
    if(f == nullptr) PARSE_FAIL;
    std::vector<std::shared_ptr<expression>> gs;
    if(cache.token != token_t::LEFT_PAREN) PARSE_FAIL;
    next_token();
    // parse gs
    while(cache.token != token_t::RIGHT_PAREN)
    {
        std::shared_ptr<expression> g = parse_expression();
        if(g == nullptr) PARSE_FAIL;
        gs.push_back(g);
        if(cache.token == token_t::COMMA)
        {
            next_token();
        }
        else if(cache.token != token_t::RIGHT_PAREN)
        {
            throw parse_error("Expect ')' in compositon");
        }
    }
    next_token();
    return composition::create(f, gs);
}

std::unique_ptr<primitive_recursion> parser::parse_primitive_recursion()
{
    PARSE_START("<primitive-recursion>");
    std::shared_ptr<expression> f = parse_comp_exp();
    if(f == nullptr) PARSE_FAIL;
    if(cache.token != token_t::PR_SYM) PARSE_FAIL;
    next_token();
    std::shared_ptr<expression> g = parse_comp_exp();
    if(g == nullptr)
    {
        throw parse_error("Expect expression after '@'");
    }
    return primitive_recursion::create(f, g);
}

std::unique_ptr<minimization> parser::parse_minimization()
{
    PARSE_START("<minimization>");
    if(cache.token != token_t::MIN_SYM) PARSE_FAIL;
    next_token();
    std::shared_ptr<expression> f = parse_comp_exp();
    if(f == nullptr)
    {
        throw parse_error("Expect expression after '$'");
    }
    return minimization::create(f);
}

/*
<expression> ::= <comp-exp> '@' <comp-exp>
               | '$' <comp-exp>
               | <comp-exp>
*/
std::unique_ptr<expression> parser::parse_expression()
{
    PARSE_START("<expression>");
    std::unique_ptr<expression> expr = nullptr;
    // try primitive recursion
    expr = parse_primitive_recursion();
    if(expr == nullptr)
    {
        // try minimization
        expr = parse_minimization();
        if(expr == nullptr)
        {
            // try comp-exp
            expr = parse_comp_exp();
            if(expr == nullptr)
                PARSE_FAIL;
        }
    }
    
    return expr;
}

/*
<comp-exp>   ::= <atomic-exp> ['(' <expression> {',' <expression>}* ')']
*/
std::unique_ptr<expression> parser::parse_comp_exp()
{
    PARSE_START("<comp-exp>");
    std::unique_ptr<expression> expr = nullptr;
    // try composition
    expr = parse_composition();
    if(expr == nullptr)
    {
        //try atomic-exp
        expr = parse_atomic_exp();
        if(expr == nullptr)
            PARSE_FAIL;
    }
    return expr;
}

/*
<atomic-exp> ::= <identifer> | '(' <expression> ')'
*/
std::unique_ptr<expression> parser::parse_atomic_exp()
{
    PARSE_START("<atomic-exp>");
    if(cache.token == token_t::LEFT_PAREN)
    {
        next_token();
        std::unique_ptr<expression> expr = parse_expression();
        if(expr == nullptr)
        {
            throw parse_error("Expect expression between parenthesis");
        }
        if(cache.token != token_t::RIGHT_PAREN)
        {
            throw parse_error("Expect ')' after '('");
        }
        next_token();
        return expr;
    }
    else
    {
        std::shared_ptr<identifier> id = parse_identifer();
        if(id == nullptr) PARSE_FAIL;
        return std::make_unique<atomic_exp>(std::move(id));
    }
}

std::shared_ptr<variable> parser::parse_line()
{
    PARSE_START("<line>");
    // parse lvalue
    if(cache.token != token_t::VARIABLE) PARSE_FAIL;
    std::string var_name_local = std::move(cache.var_name);
    next_token();
    if(cache.token != token_t::EQUAL) PARSE_FAIL;
    next_token();
    // parse rvalue
    std::unique_ptr<expression> rvalue = parse_expression();
    if(rvalue == nullptr)
    {
        throw parse_error("Unknown expression");
    }
    // add variable to context
    auto var = std::make_shared<variable>(var_name_local, rvalue->dim(), std::move(rvalue));
    add_variable(var);
    return var;
}

void parser::parse()
{
    try{
        while(cache.token != token_t::END)
        {
            if(cache.token == token_t::NEWLINE)
            {
                next_token();
                continue;
            }
            parse_line();
            if(cache.token == token_t::NEWLINE)
            {
                next_token();
            }
            else if(cache.token != token_t::END)
            {
                throw parse_error("Expected end of line");
            }
        }
    }
    catch(const parse_error &err)
    {
        char var = input[cache.pos];
        if(cache.pos>0) cache.pos--;
        while(isspace(input[cache.pos]) && cache.pos > 0)
            cache.pos--;
        size_t line_num = std::count(input.begin(), input.begin() + std::min(cache.pos, input.size()), '\n');
        std::cerr << "In line " << line_num << ":\n";
        size_t start = input.rfind('\n', cache.pos);
        if (start == std::string::npos)
            start = 0;
        else
            start += start < cache.pos ? 1 : 0; // move past '\n'

        size_t end = input.find('\n', cache.pos);
        if (end == std::string::npos)
            end = input.size();
        
        std::cerr << input.substr(start, end - start) + "\n";
        std::cerr << std::string(cache.pos-start,' ')+"^ ";
        std::cerr << cache.token << " here\n";
        std::cerr << "parse error: " << err.what() << std::endl;
        exit(1);
    }
}

/****** interpreter ******/

natural parser::eval_var(std::shared_ptr<variable> v, std::vector<natural> operands)
{
    return v->eval(operands);
}

natural parser::eval_var(std::string s, std::vector<natural> operands)
{
    std::shared_ptr<variable> v = get_variable(s);
    if(v == nullptr)
    {
        throw parse_error("eval_var: Undefined variable: " + s);
    }
    return v->eval(operands);
}

/****** help funtions ******/

std::shared_ptr<variable> parser::get_variable(const std::string &name) noexcept
{
    auto it = context.find(name);
    if(it != context.end())
    {
        return program[it->second];
    }
    else
    {
        return nullptr;
    }
}

void parser::add_variable(const std::shared_ptr<variable> &var)
{
    if(context.find(var->name) != context.end())
    {
        throw parse_error("Redefinition of variable: " + var->name);
    }
    context[var->name] = program.size();
    program.push_back(var);
}

std::string parser::to_string() const
{
    std::string result;
    for(const auto& var : program)
    {
        //result += "; " + var->to_string(true) + "\n";
        result += var->to_string() + " = " + var->defn->to_string();
        result += "\n";
    }
    return result;
}
