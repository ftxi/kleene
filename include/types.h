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
    unsigned int _dim;
    expression() {}
    unsigned int dim() const
    {
        return _dim;
    }
    virtual natural eval(const std::vector<natural>&) const = 0;
    virtual std::string to_string() const = 0;
    virtual ~expression() = default;
};

struct identifer 
{
    identifer() {}
    virtual unsigned int dim() const = 0;
    virtual natural eval(const std::vector<natural>&) const = 0;
    virtual std::string to_string(bool show_type = false) const = 0;
    virtual ~identifer() = default;
};

struct variable : public identifer 
{
    const std::string name;
    const unsigned int _dim;
    std::unique_ptr<expression> defn;
    variable(const std::string& name, unsigned int dim, std::unique_ptr<expression>&& defn)
        : identifer(), name(name), _dim(dim), defn(std::move(defn)) {}
    unsigned int dim() const override 
    {
        return _dim;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        dprint("eval:", defn->to_string());
        natural res = defn->eval(operands);
        dprint("eval:", to_string(false), range_to_string(operands), "=>", res);
        return res;
    }
    std::string to_string(bool show_type=false) const override
    {
        if(show_type)
        {
            return name + " : N^" + std::to_string(_dim) + " -> N";
        }
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

struct constant : public identifer 
{
    const unsigned int n;
    natural k;
    constant(unsigned int n, unsigned int k)
        : identifer(), n{n}, k{k} {}
    unsigned int dim() const override 
    {
        return n;
    }
    natural eval(const std::vector<natural>&) const override
    {
        return k;
    }
    std::string to_string(bool) const override
    {
        return "C^" + std::to_string(n) + "_" + std::to_string(k);
    }
};

struct projection : public identifer 
{
    const unsigned int n;
    const unsigned int k;
    projection(unsigned int n, unsigned int k)
        : identifer(), n{n}, k{k} 
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
    std::string to_string(bool) const override
    {
        return "P^" + std::to_string(n) + "_" + std::to_string(k);
    }
};

struct successor : public identifer 
{
    successor()
        : identifer() {}
    unsigned int dim() const override 
    {
        return 1;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        return operands[0]+1;
    }
    std::string to_string(bool) const override
    {
        return "S";
    }
};

struct composition : public expression
{
    std::shared_ptr<identifer> f;
    std::vector<std::shared_ptr<identifer>> gs;
    composition(const std::shared_ptr<identifer>& f,
                const std::vector<std::shared_ptr<identifer>>& gs)
        : f(f), gs(gs)
    {
        // N^a --g_1,g_2,...,g_b--> N^b --f--> N
        unsigned int b = f->dim();
        if(gs.size() != b)
        {
            throw parse_error("Arity mismatch in composition: "+std::to_string(gs.size())+" identifers provided but "+std::to_string(b)+" expected by "+f->to_string(true));
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
                throw parse_error("Dimension mismatch in composition: "+g->to_string(true)+" does not match "+gs[0]->to_string(true));
            }
        }
        _dim = a;
    }
    natural eval(const std::vector<natural> &operands) const override
    {
        std::vector<natural> vs;
        vs.resize(operands.size());
        std::transform(gs.begin(), gs.end(), vs.begin(), [&operands](std::shared_ptr<identifer> g){
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
    std::shared_ptr<identifer> f;
    std::shared_ptr<identifer> g;
    primitive_recursion(const std::shared_ptr<identifer>& f,
                        const std::shared_ptr<identifer>& g)
        : f(f), g(g)
    {
        // N^a --f--> N
        // N^{a+2} --g--> N
        // overall: N^{a+1} --f@g--> N
        if(f->dim() + 2 != g->dim())
        {
            throw parse_error("Dimension mismatch in primitive recursion: "+g->to_string(true)+" does not match "+f->to_string(true));
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
    std::shared_ptr<identifer> f;
    minimization(const std::shared_ptr<identifer>& f)
        : f(f)
    {
        // N^{a+1} --f--> N^1
        if(f->dim() < 1)
        {
            throw parse_error("Dimension mismatch in minimization: "+f->to_string(true)+" has insufficient dimension");
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

#endif // TYPES_H
