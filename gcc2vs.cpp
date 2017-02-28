#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>

std::string path;

void convert_slash(std::string& s)
{
    for (auto i = s.begin(); i != s.end(); ++i)
        *i = *i == '/' ? '\\' : *i;
}

std::string convert_path(auto s)
{
    convert_slash(s);
    return path + s;
}

int main(int argc, char** argv)
{
    if (argc < 2) return 2;
    if (argv[1][1] == ':') path = argv[1];
    else
    {
        path = std::getenv("MSYS2_ROOT");
        path += argv[1];
    }
    convert_slash(path);
    if (path.back() != '\\') path += '\\';

    bool error = false;
    std::string s;
    while (std::cin.good())
    {
        try
        {
            std::smatch r { };
            std::getline(std::cin, s);
            if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+): error:(.*)")))
            {
                std::cerr << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): " << r[4] << '\n';
                error = true;
            } 
            if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+): (warning|note):(.*)")))
            {
                std::cout << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): " << r[4] << '\n';
            } 
            else if(std::regex_search(s, r, std::regex("((.*)included from )([^ ]+):([0-9]+)(.*)")))
            {
                std::cout << "In file included from:\n";
                std::cout << "                       " << convert_path(std::string(r[3])) << '(' << r[4] <<",1):\n";
            } 
            else if(std::regex_search(s, r, std::regex("((.*)from )([^ ]+):([0-9]+)(.*)")))
            {
                std::cout << "                       " << convert_path(std::string(r[3])) << '(' << r[4] <<",1):\n";
            }
            else if(std::regex_search(s, r, std::regex("( *)([^ ]+):([0-9]+):(.*)")))
            {
                std::cerr << convert_path(std::string(r[2])) << '(' << r[3] << ",1): error: " << r[4] << '\n';
                error = true;
            }
            else if(std::regex_search(s, r, std::regex("( *)([^ ]+):(.*) Stop.")))
            {
                std::cerr << "make: error: " << r[3] << '\n';
                error = true;
            }
            else std::cout << s << '\n';
        }
        catch (const std::exception& e)
        {
            std::cerr << " *** GCC2VS EXCEPTION: " << e.what() << '\n';
        }
    } 
    std::cout << std::flush;
    return error;
}