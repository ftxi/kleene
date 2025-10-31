#include "parser.h"
#include <utility>

/****** Lexer ******/

void parser::next_token()
 {
    // Skip whitespace (excluding newlines)
    while (pos < input.size() && (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\r')) {
        pos++;
    }

    if(input[pos] == ';')
    {
        // Skip comment until newline or end of input
        pos++; // Skip the ';'
        while (pos < input.size() && input[pos] != '\n') {
            pos++;
        }
        // After skipping comment, get the next token
        next_token();
        return;
    }
    
    // Check for end of input
    if (pos >= input.size()) {
        token = token_t::END;
        return;
    }
    
    switch (input[pos]) 
    {
    case '\n':
        token = token_t::NEWLINE;
        pos++;
        break;
        
    case '=':
        token = token_t::EQUAL;
        pos++;
        break;
        
    case '(':
        token = token_t::LEFT_PAREN;
        pos++;
        break;
        
    case ')':
        token = token_t::RIGHT_PAREN;
        pos++;
        break;
        
    case ',':
        token = token_t::COMMA;
        pos++;
        break;
        
    case '@':
        token = token_t::PR_SYM;
        pos++;
        break;
        
    case '$':
        token = token_t::MIN_SYM;
        pos++;
        break;
        
    case 'C':
        // CONST: C^<num>_<num>
        if (pos + 1 < input.size() && input[pos + 1] == '^') {
            pos += 2; // Skip 'C^'
            if (pos >= input.size() || !isdigit(input[pos])) {
                throw parse_error("Expected digit after 'C^'");
            }
            // Read first number
            num1 = 0;
            while (pos < input.size() && isdigit(input[pos])) {
                num1 = num1 * 10 + (input[pos] - '0');
                pos++;
            }
            if (pos >= input.size() || input[pos] != '_') {
                throw parse_error("Expected '_' in CONST token");
            }
            pos++; // Skip '_'
            if (pos >= input.size() || !isdigit(input[pos])) {
                throw parse_error("Expected digit after '_' in CONST token");
            }
            // Read second number
            num2 = 0;
            while (pos < input.size() && isdigit(input[pos])) {
                num2 = num2 * 10 + (input[pos] - '0');
                pos++;
            }
            token = token_t::CONST;
        } else {
            throw parse_error("Expected '^' after 'C'");
        }
        break;
        
    case 'P':
        // PROJ: P^<num>_<num>
        if (pos + 1 < input.size() && input[pos + 1] == '^') {
            pos += 2; // Skip 'P^'
            if (pos >= input.size() || !isdigit(input[pos])) {
                throw parse_error("Expected digit after 'P^'");
            }
            // Read first number
            num1 = 0;
            while (pos < input.size() && isdigit(input[pos])) {
                num1 = num1 * 10 + (input[pos] - '0');
                pos++;
            }
            if (pos >= input.size() || input[pos] != '_') {
                throw parse_error("Expected '_' in PROJ token");
            }
            pos++; // Skip '_'
            if (pos >= input.size() || !isdigit(input[pos])) {
                throw parse_error("Expected digit after '_' in PROJ token");
            }
            // Read second number
            num2 = 0;
            while (pos < input.size() && isdigit(input[pos])) {
                num2 = num2 * 10 + (input[pos] - '0');
                pos++;
            }
            token = token_t::PROJ;
        } else {
            throw parse_error("Expected '^' after 'P'");
        }
        break;
        
    case 'S':
        token = token_t::SUCC;
        pos++;
        break;
        
    default:
        // VARIABLE: starts with lowercase, followed by alphanumerics
        if (islower(input[pos])) {
            var_name.clear();
            var_name += input[pos];
            pos++;
            while (pos < input.size() && isalnum(input[pos])) {
                var_name += input[pos];
                pos++;
            }
            token = token_t::VARIABLE;
        } else {
            throw parse_error(std::string("Unexpected character: '") + input[pos] + "'");
        }
        break;
    }
}

/****** Parser ******/

#define PARSE_START auto [fallback_token, start_pos] = std::pair{token,pos};
#define PARSE_FAIL {pos = start_pos; token = fallback_token; return nullptr;}

std::shared_ptr<identifer> parser::parse_identifer()
{
    PARSE_START;
    std::shared_ptr<identifer> result = nullptr;
    switch(token)
    {
        case token_t::CONST: // parse constant
            result = std::make_shared<constant>(num1, num2);
            next_token();
            return result;
        case token_t::PROJ: // parse projection
            result = std::make_shared<projection>(num1, num2);
            next_token();
            return result;
        case token_t::SUCC: // parse successor
            result = std::make_shared<successor>();
            next_token();
            return result;
        case token_t::VARIABLE: // parse variable
            result = get_variable(var_name);
            next_token();
            return result;
        default:
            PARSE_FAIL;
    }
    return result;
}

std::unique_ptr<composition> parser::parse_composition()
{
    PARSE_START;
    std::shared_ptr<identifer> f = parse_identifer();
    if(f == nullptr) PARSE_FAIL;
    std::vector<std::shared_ptr<identifer>> gs;
    if(token != token_t::LEFT_PAREN) PARSE_FAIL;
    next_token();
    // parse gs
    while(token != token_t::RIGHT_PAREN)
    {
        std::shared_ptr<identifer> g = parse_identifer();
        if(g == nullptr) PARSE_FAIL;
        gs.push_back(g);
        if(token == token_t::COMMA)
        {
            next_token();
        }
        else if(token != token_t::RIGHT_PAREN)
        {
            PARSE_FAIL;
        }
    }
    next_token();
    return std::make_unique<composition>(f, gs);
}

std::unique_ptr<primitive_recursion> parser::parse_primitive_recursion()
{
    PARSE_START;
    std::shared_ptr<identifer> f = parse_identifer();
    if(f == nullptr) PARSE_FAIL;
    if(token != token_t::PR_SYM) PARSE_FAIL;
    next_token();
    std::shared_ptr<identifer> g = parse_identifer();
    if(g == nullptr) PARSE_FAIL;
    return std::make_unique<primitive_recursion>(f, g);
}

std::unique_ptr<minimization> parser::parse_minimization()
{
    PARSE_START;
    if(token != token_t::MIN_SYM) PARSE_FAIL;
    next_token();
    std::shared_ptr<identifer> f = parse_identifer();
    if(f == nullptr) PARSE_FAIL;
    return std::make_unique<minimization>(f);
}

#undef PARSE_FAIL
#define PARSE_FAIL {pos = start_pos; return;}

void parser::parse_line()
{
    PARSE_START;
    // parse lvalue
    if(token != token_t::VARIABLE) PARSE_FAIL;
    std::string var_name_local = std::move(var_name);
    next_token();
    if(token != token_t::EQUAL) PARSE_FAIL;
    next_token();
    // parse rvalue
    std::unique_ptr<expression> rvalue = nullptr;
    // try composition
    rvalue = parse_composition();
    if(rvalue == nullptr)
    {
        // try primitive recursion
        rvalue = parse_primitive_recursion();
        if(rvalue == nullptr)
        {
            // try minimization
            rvalue = parse_minimization();
            if(rvalue == nullptr)
            {
                PARSE_FAIL;
            }
        }
    }
    // add variable to context
    auto var = std::make_shared<variable>(var_name_local, rvalue->dim(), std::move(rvalue));
    add_variable(var);
}

void parser::parse()
{
    while(token != token_t::END)
    {
        if(token == token_t::NEWLINE)
        {
            next_token();
            continue;
        }
        parse_line();
        if(token == token_t::NEWLINE)
        {
            next_token();
        }
        else if(token != token_t::END)
        {
            throw parse_error("Expected end of line");
        }
    }
}

/****** interpreter ******/

natural parser::eval_var(std::string s, std::vector<natural> operands)
{
    std::shared_ptr<variable> v = get_variable(s);
    return v->eval(operands);
}

/****** help funtions ******/

std::shared_ptr<variable> parser::get_variable(const std::string &name)
{
    auto it = context.find(name);
    if(it != context.end())
    {
        return program[it->second];
    }
    else
    {
        throw parse_error("Undefined variable: " + name);
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
        result += "; " + var->to_string(true) + "\n";
        result += var->to_string() + " = " + var->defn->to_string();
        result += "\n";
    }
    return result;
}
