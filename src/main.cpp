#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include "parser.h"

std::string version_str = "Kleene interpreter, version 0.2.0";

std::string help_str = R"(
usage: kleene [option] file [arg] ...
Options: 
  -h,    : print this help message and exit (also --help)
  -e var : the entry point. If not specified, entry point is 'main'
  -i     : interactive mode; will run script first if entry point
           is valid (also --interactive)
Arguments:
  file   : program read from script file. The entry point function 
           will be evaluated with the arguments passed
  arg... : arguments to be passed to the program
)";


void show_help()
{
    std::cout << version_str << "\n";
    std::cout << help_str.substr(1);
}

void repl(std::unique_ptr<parser> p)
{
    std::string line;
    while(true)
    {
        std::cout << ">>> ";
        if(!std::getline(std::cin, line))
        {
            if (std::cin.eof())
            {
                std::cout << std::endl;
                exit(0);
            }
            else
            {
                std::cerr << "Kleene REPL: Input error\n";
                exit(1);
            }
        }
        p->set_input(line);
        try
        {
            dprint("repl: try parse as line");
            auto v = p->parse_line();
            if(v != nullptr)
            {
                dprint("current context:");
                dprint(p->to_string());
            }
            else
            {
                dprint("repl: try parse as expression");
                std::unique_ptr<expression> expr = p->parse_expression();
                if(expr->dim() == 0)
                {
                    dprint("repl: evaluating", expr->to_string());
                    natural ans = expr->eval({});
                    std::cout << ans << std::endl;
                }
                else
                {
                    std::cout << "Function N^" << expr->dim() << " -> N" << std::endl;
                }
            }
        }
        catch(const parse_error &e)
        {
            std::cerr << "Error: ";
            std::cerr << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) 
{
    std::string entry_point = "main";
    std::string filename = "";
    bool interactive = false;
    std::vector<std::string> args;
    std::unique_ptr<parser> p = nullptr;
    bool numeric_args = true;
    // phase 1: parse argv
    if(argc <= 1)
    {
        interactive = true;
    }
    for (int i = 1; i < argc; i++)
    {
        std::string current_arg = argv[i];
        if(current_arg[0] == '-')
        {
            if(current_arg == "-h" || current_arg == "--help")
            {
                show_help();
                return 0;
            }
            else if(current_arg == "-e")
            {
                if (i + 1 < argc)
                {
                    i++;
                    entry_point = argv[i];
                }
                else
                {
                    std::cerr << "Argument expected by -e option\n";
                    std::cerr << "Try `kleene -h` for more information." << std::endl;
                    return 2;
                }
            }
            else if(current_arg == "-i" || current_arg == "--interactive")
            {
                interactive = true;
            }
            else
            {
                std::cerr << "unrecognised flag: " << current_arg << "\n";
                std::cerr << "Try `kleene -h` for more information." << std::endl;
                return 2;
            }
        }
        else
        {
            filename = current_arg;
            args.reserve(argc-i);
            for(i++; i < argc; i++)
            {
                args.push_back(argv[i]);
            }
        }
    }
    // phase 2: open source file and parse
    if(filename.empty())
    {
        if(interactive)
        {
            p = parser::create("");
        }
        else
        {
            std::cerr << "Expect input file after option(s)\n";
            std::cerr << "Try `kleene -h` for more information." << std::endl;
            return 2;
        }
    }
    else
    {
        std::ifstream file(filename);
        if(file.good())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string code = buffer.str();
            p = parser::create(code);
        }
        else
        {
            std::cerr << "Cannot open file: " + filename << std::endl;
            return 2;
        }
    }
    // phase 3: transform arguments
    std::vector<natural> operands(args.size());
    if(numeric_args)
    {
        try {
            std::transform(args.begin(), args.end(), operands.begin(), [](std::string s){
                return stoull(s);
            });
        }
        catch (std::invalid_argument)
        {
            for(auto a:args)
                std::cerr << " " << a;
            std::cerr << "\n";
            std::cerr << "Found non-numerical arguments with --numeric_args" << std::endl;
            exit(2);
        }
    }
    // phase 4: evaluate entry point
    p->parse();
    auto v = p->get_variable(entry_point);
    if(v == nullptr && !interactive)
    {
        std::cerr << "Entry point '" << entry_point << "' not found; abort\n";
        std::cerr << "Try `kleene -h` for more information." << std::endl;
        return 2;
    }
    else if(v != nullptr)
    {
        if(v->dim() != operands.size())
        {
            if(!interactive || operands.size() > 0)
            {
                std::cerr << "Entry point '" << entry_point << "' expects " << v->dim() << " arguments,";
                std::cerr << " but " << operands.size() << " provided; abort" << std::endl;
                return 2;
            }
            else
            {
                std::cout << entry_point << ": N^" << v->dim() << " -> N" << std::endl;
            }
        }
        else
        {
            natural ans = p->eval_var(v, operands);
            std::cout << ans << std::endl;
        }
    }
    // phase 5: repl
    if(interactive)
    {
        repl(std::move(p));
    }
    return 0;
}
