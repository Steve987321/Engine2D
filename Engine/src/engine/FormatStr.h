#include <EngineCore.h>
#include <string>
#include <vector>

namespace Toad
{
    template<typename T>
    std::string stringify(T&& val)
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    template <typename ...TArgs>
    std::string format_str(std::string_view format, TArgs&& ... args)
    {
        std::vector<std::string> args_as_str = {};

        (args_as_str.push_back(stringify(args)),...);

        int curr_args_as_str_index = 0;
        bool get_index = false;
        std::string index_str = "";
        std::string result = "";

        for (const auto c : format)
        {
            if (c == '{')
            {
                get_index = true;
                index_str.clear();
                continue;
            }

            if (get_index && c != '}')
            {
                index_str += c;
            }
            else if (get_index && c == '}')
            {
                try
                {
                    if (index_str.empty())
                    {
                        if (curr_args_as_str_index + 1 > args_as_str.size())
                        {
                            result += "{}";
                            continue;
                        }

                        result += args_as_str[curr_args_as_str_index++];
                        continue;
                    }
                    int index = std::stoi(index_str);
                    if (index >= args_as_str.size())  // bru
                    {
                        result += "{INVALID}";
                        continue;
                    }
                    result += args_as_str[index];
                }
                catch (std::out_of_range& e)
                {
                    std::cout << "Error formatting: " << e.what() << std::endl;
                }
                catch (std::invalid_argument& e)
                {
                    std::string args;
                    for (const auto& s : args_as_str)
                        args += s + ' ';

                    std::cout << "Error formatting: " << format << ' ' << args << ' ' << e.what() << std::endl;
                }
            }
            else
            {
                result += c;
            }
        }
        return result;
    }

}