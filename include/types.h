#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "debug.h"

using natural = unsigned long long;

class parse_error : public std::runtime_error 
{
public:
    explicit parse_error(const std::string& message)
        : std::runtime_error(message) {}
};

class compile_error : public std::runtime_error 
{
public:
    explicit compile_error(const std::string& message)
        : std::runtime_error(message) {}
};

struct expression 
{
    expression() {}
    virtual unsigned int dim() const = 0;
    virtual natural eval(const std::vector<natural>&) const = 0;
    virtual std::string to_string() const = 0;
    std::string show_type()
    {
        return to_string()+"N^"+std::to_string(dim())+" -> N";
    }
    virtual ~expression() = default;
};

struct identifier
{
    identifier() {}
    virtual unsigned int dim() const = 0;
    virtual natural eval(const std::vector<natural>&) const = 0;
    virtual std::string to_string() const = 0;
    std::string show_type()
    {
        return to_string()+"N^"+std::to_string(dim())+" -> N";
    }
    ~identifier() = default;
};

struct variable : public identifier 
{
    const std::string name;
    const unsigned int _dim;
    std::unique_ptr<expression> defn;
    variable(const std::string& name, unsigned int dim, std::unique_ptr<expression>&& defn)
        : identifier(), name(name), _dim(dim), defn(std::move(defn)) {}
    unsigned int dim() const override 
    {
        return _dim;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        dprint("eval:", defn->to_string());
        natural res = defn->eval(operands);
        dprint("eval:", to_string(), range_to_string(operands), "=>", res);
        return res;
    }
    std::string to_string() const override
    {
        return name;
    }
    bool operator<(const variable& other) const
    {
        return name < other.name;
    }
    bool operator==(const variable& other) const
    {
        return name == other.name;
    }
};

struct constant : public identifier 
{
    const unsigned int n;
    natural k;
    constant(unsigned int n, unsigned int k)
        : identifier(), n{n}, k{k} {}
    unsigned int dim() const override 
    {
        return n;
    }
    natural eval(const std::vector<natural>&) const override
    {
        return k;
    }
    std::string to_string() const override
    {
        return "C^" + std::to_string(n) + "_" + std::to_string(k);
    }
};

struct projection : public identifier 
{
    const unsigned int n;
    const unsigned int k;
    projection(unsigned int n, unsigned int k)
        : identifier(), n{n}, k{k} 
    {
        if(k == 0 || k > n)
        {
            throw parse_error("Invalid projection indices: P^" + std::to_string(n) + "_" + std::to_string(k));
        }
    }
    unsigned int dim() const override
    {
        return n;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        return operands[k-1];
    }
    std::string to_string() const override
    {
        return "P^" + std::to_string(n) + "_" + std::to_string(k);
    }
};

struct successor : public identifier 
{
    successor()
        : identifier() {}
    unsigned int dim() const override 
    {
        return 1;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        return operands[0]+1;
    }
    std::string to_string() const override
    {
        return "S";
    }
};

struct composition : public expression
{
    std::shared_ptr<expression> f;
    std::vector<std::shared_ptr<expression>> gs;
    unsigned int _dim;
    unsigned int dim() const override
    {
        return _dim;
    }
    composition(const std::shared_ptr<expression>& f,
                const std::vector<std::shared_ptr<expression>>& gs)
        : f(f), gs(gs)
    {
        // N^a --g_1,g_2,...,g_b--> N^b --f--> N
        unsigned int b = f->dim();
        if(gs.size() != b)
        {
            throw parse_error("Arity mismatch in composition: "+std::to_string(gs.size())+" identifers provided but "+std::to_string(b)+" expected by "+f->show_type());
        }
        if(gs.size() == 0)
        {
            throw parse_error("No identifers in composition");
        }
        unsigned int a = gs[0]->dim();
        for(auto g : gs)
        {
            if(g->dim() != a)
            {
                throw parse_error("Dimension mismatch in composition: "+g->show_type()+" does not match "+gs[0]->show_type());
            }
        }
        _dim = a;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        std::vector<natural> vs;
        vs.resize(operands.size());
        std::transform(gs.begin(), gs.end(), vs.begin(), [&operands](std::shared_ptr<expression> g){
            return g->eval(operands);
        });
        return f->eval(vs);
    }
    std::string to_string() const override
    {
        std::string result = f->to_string() + "(";
        for(size_t i = 0; i < gs.size(); i++)
        {
            result += gs[i]->to_string();
            if(i + 1 < gs.size())
            {
                result += ", ";
            }
        }
        result += ")";
        return result;
    }
};

struct primitive_recursion : public expression
{
    std::shared_ptr<expression> f;
    std::shared_ptr<expression> g;
    unsigned int _dim;
    unsigned int dim() const override
    {
        return _dim;
    }
    primitive_recursion(const std::shared_ptr<expression>& f,
                        const std::shared_ptr<expression>& g)
        : f(f), g(g)
    {
        // N^a --f--> N
        // N^{a+2} --g--> N
        // overall: N^{a+1} --f@g--> N
        if(f->dim() + 2 != g->dim())
        {
            throw parse_error("Dimension mismatch in primitive recursion: "+g->show_type()+" does not match "+f->show_type());
        }
        _dim = f->dim() + 1;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        std::vector xs(operands.begin()+1, operands.end());
        std::vector<natural> ys = {0,f->eval(xs)};
        ys.insert(ys.end(), xs.begin(), xs.end());
        for(ys[0] = 0; ys[0] < operands[0]; ys[0]++)
        {
            ys[1] = g->eval(ys);
        }
        return ys[1];
    }
    std::string to_string() const override
    {
        return f->to_string() + " @ " + g->to_string();
    }
};

struct minimization : public expression
{
    std::shared_ptr<expression> f;
    unsigned int _dim;
    unsigned int dim() const override
    {
        return _dim;
    }
    minimization(const std::shared_ptr<expression>& f)
        : f(f)
    {
        // N^{a+1} --f--> N^1
        if(f->dim() < 1)
        {
            throw parse_error("Dimension mismatch in minimization: "+f->show_type()+" has insufficient dimension");
        }
        _dim = f->dim() - 1;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        std::vector<natural> xs = {0};
        xs.insert(xs.end(), operands.begin(), operands.end());
        while(f->eval(xs) != 0)
        {
            xs[0]++;
        }
        return xs[0];
    }
    std::string to_string() const override
    {
        return "$ " + f->to_string();
    }
};

struct atomic_exp : public expression
{
    std::shared_ptr<identifier> idt;
    atomic_exp(std::shared_ptr<identifier> idt)
        : idt(idt) {}
    unsigned int dim() const override
    {
        return idt->dim();
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        return idt->eval(operands);
    }
    std::string to_string() const override
    {
        return idt->to_string();
    }
};

#endif // TYPES_H
