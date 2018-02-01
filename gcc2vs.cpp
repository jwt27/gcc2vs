/****************************** gcc2vs *******************************
Copyright (C) 2017-2018  J.W. Jagersma

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <cstdio>
#include <array>

std::string path { };

void convert_slash(std::string& s)
{
    std::size_t pos = 0;
    while ((pos = s.find('/', pos)) != std::string::npos)
        s.replace(pos, 1, 1, '\\');
}

std::string convert_path(auto s)
{
    convert_slash(s);
    if (s[1] == ':') return s;
    else return path + s;
}

// from http://stackoverflow.com/a/478960
std::string exec(std::string cmd) 
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get()))
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    return result;
}

void print_help()
{
    std::cout <<
        "gcc2vs - convert gcc error messages to Visual Studio format.\n"
        "\n"
        "Usage:\n"
        "    gcc [...] 2>&1 | gcc2vs [options]\n"
        "\n"
        "Options:\n"
        "    --help                 Display this message.\n"
        "    --path=<...>           Override displayed working directory.\n"
        "    --use-stderr=<type>    Output messages to stderr instead of stdout.\n"
        "                           <type> can be one of: errors, warnings, both.\n";
}

int main(int argc, char** argv)
{
    auto* err_out = &std::cout;
    auto* warn_out = &std::cout;

    for (auto i = 1; i < argc; ++i)
    {
        std::string_view arg { argv[i] };
        if (arg == "--help")
        {
            print_help();
            return 0;
        }
        else if (arg.compare("--path") >= 0)
        {
            if (arg[6] == '=') path = arg.substr(7);
            else if (++i < argc) path = argv[i];
        }
        else if (arg.compare("--use-stderr") >= 0)
        {
            if (arg[12] == '=') arg = arg.substr(13);
            else if (++i < argc) arg = argv[i];
            if (arg == "both" or arg == "warnings") warn_out = &std::cerr;
            if (arg == "both" or arg == "errors") err_out = &std::cerr;
        }
        else
        {
            std::cerr << "Unrecognized command line option: " << arg << " (try --help)\n";
            return 1;
        }
    }

    if (path.empty())
    {
        path = exec("cygpath -w \"" + exec("pwd") + '\"');
        if (path[1] != ':') path = std::getenv("MSYS2_ROOT") + path;
        convert_slash(path);
    }
    std::size_t pos { 0 };
    while ((pos = path.find('\n', pos)) != std::string::npos)
        path.erase(pos, 1);
    if (path.back() != '\\') path += '\\';

    int errors { 0 };
    std::string s;
    while (std::cin.good())
    {
        try
        {
            std::smatch r { };
            std::getline(std::cin, s);
            if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+): (fatal )?error:(.*)")))   // gcc error
            {
                *err_out << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): error : " << r[5] << '\n';
                ++errors;
            } 
            else if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+): warning:(.*)"))) // gcc warning
            {
                *warn_out << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): warning : " << r[4] << '\n';
            } 
            else if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+):(.*)")))  // gcc note
            {
                *warn_out << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): " << r[4] << '\n';
            } 
            else if(std::regex_search(s, r, std::regex("((.*)included from )([^ ]+):([0-9]+):([0-9]+)(.*)")))   // first "in file included from"
            {
                *warn_out << "In file included from:\n";
                *warn_out << "                       " << convert_path(std::string(r[3])) << '(' << r[4] <<',' << r[5] << "):\n";
            } 
            else if(std::regex_search(s, r, std::regex("((.*)from )([^ ]+):([0-9]+)(.*)"))) // subsequent "in file included from"
            {
                *warn_out << "                       " << convert_path(std::string(r[3])) << '(' << r[4] <<",1):\n";
            }
            else if(std::regex_search(s, r, std::regex("(.+): (In .*)")))   // in function/instantiation/constructor/etc
            {
                *warn_out << convert_path(std::string(r[1])) << ": " << r[2] << '\n';
            }
            else if(std::regex_search(s, r, std::regex("(make\\[[0-9]+\\]: \\[)(.*):([0-9]+):(.*)(\\(ignored\\))")))    // ignored error
            {
                *warn_out << r[1] << convert_path(std::string(r[2])) << '(' << r[3] << ",1): error : " << r[4] << r[5] << '\n';
            }
            else if(std::regex_search(s, r, std::regex("( *)([^ ]+):([0-9]+):(.*)")))   // linker error
            {
                *err_out << convert_path(std::string(r[2])) << '(' << r[3] << ",1): error : " << r[4] << '\n';
                ++errors;
            }
            else if(std::regex_search(s, r, std::regex("( *)([^ ]+):(.*) Stop.")))  // make error
            {
                *err_out << "make: error : " << r[3] << '\n';
                ++errors;
            }
            else std::cout << s << '\n';
        }
        catch (const std::exception& e)
        {
            std::cerr << " *** GCC2VS EXCEPTION: " << e.what() << '\n';
            std::cerr << " ***  CAUSED BY THIS : " << s << '\n';
        }
    } 
    std::cout << std::flush;
    return errors;
}
