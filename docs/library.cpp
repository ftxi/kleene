
#include <emscripten.h>
#include <sstream>
#include <string>
#include <exception>
#include "parser.h"

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* run_program(const char* code, const char* entry, const char* input) 
    {
        static std::string result;
        result = "[NO OUTPUT]";
        try {
            std::vector<natural> operands;
            std::istringstream iss(input);
            natural x;
            while (iss >> x)
                operands.push_back(x);
            auto p = parser::create(std::string(code));
            auto errmsg = p->try_parse();
            if(errmsg)
            {
                result = *errmsg;
            }
            else
            {
                std::string entry_point = entry;
                if(entry_point.empty())
                    entry_point = "main";
                auto v = p->get_variable(entry_point);
                if(v == nullptr)
                {
                    result = "Entry point '" + entry_point + "' not found\n";
                }
                else if(v != nullptr)
                {
                    if(v->dim() != operands.size())
                    {
                        result = "Entry point '" + entry_point + "' expects " + std::to_string(v->dim()) + " arguments,";
                        result += " but " + std::to_string(operands.size()) + " provided\n";
                    }
                    else
                    {
                        natural output = p->eval_var(v, operands);
                        result = std::to_string(output);
                    }
                }
            }
        } catch (const std::invalid_argument& e) {
            result = std::string("Invalid argument: ") + e.what();
        } catch (const std::out_of_range& e) {
            result = std::string("Out of range: ") + e.what();
        } catch (const std::runtime_error& e) {
            result = std::string("Runtime error: ") + e.what();
        } catch (const std::exception& e) {
            result = std::string("Exception: ") + e.what();
        } catch (...) {
            result = "Unknown exception";
        }
        return result.c_str();
    }
}


