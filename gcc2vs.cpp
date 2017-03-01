#include <iostream>
#include <string>
#include <regex>
#include <cstdio>
#include <array>

std::string path;

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
std::string exec(const char* cmd) 
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get()))
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    return result;
}

int main()
{
    path = exec("pwd -P"); // this is a different pwd than the one in /usr/bin/pwd? (which has a -W option)
    if (path[1] != ':') path = std::getenv("MSYS2_ROOT") + path;
    convert_slash(path);
    std::size_t pos = 0;
    while ((pos = path.find('\n', pos)) != std::string::npos)
        path.erase(pos, 1);
        
    if (path.back() != '\\') path += '\\';

    int error = 0;
    std::string s;
    while (std::cin.good())
    {
        try
        {
            std::smatch r { };
            std::getline(std::cin, s);
            if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+): (fatal )?error:(.*)")))           // gcc error
            {
                std::cerr << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): error : " << r[5] << '\n';
                ++error;
            } 
            else if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+): warning:(.*)")))             // gcc warning
            {
                std::cout << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): warning : " << r[4] << '\n';
            } 
            else if(std::regex_search(s, r, std::regex("([^ ]+):([0-9]+):([0-9]+):(.*)")))                      // gcc note
            {
                std::cout << convert_path(std::string(r[1])) << '(' << r[2] << ',' << r[3] << "): " << r[4] << '\n';
            } 
            else if(std::regex_search(s, r, std::regex("((.*)included from )([^ ]+):([0-9]+):([0-9]+)(.*)")))   // first "in file included from"
            {
                std::cout << "In file included from:\n";
                std::cout << "                       " << convert_path(std::string(r[3])) << '(' << r[4] <<',' << r[5] << "):\n";
            } 
            else if(std::regex_search(s, r, std::regex("((.*)from )([^ ]+):([0-9]+)(.*)")))                     // subsequent "in file included from"
            {
                std::cout << "                       " << convert_path(std::string(r[3])) << '(' << r[4] <<",1):\n";
            }
            else if(std::regex_search(s, r, std::regex("(.+): (In .*)")))                                       // In function/instantiation/constructor/etc
            {
                std::cout << convert_path(std::string(r[1])) << ": " << r[2] << '\n';
            }/*
            else if(std::regex_search(s, r, std::regex("( *)([^ ]+):([0-9]+):(.*)")))
            {
                std::cerr << convert_path(std::string(r[2])) << '(' << r[3] << ",1): error: " << r[4] << '\n';
                ++error;
            } */
            else if(std::regex_search(s, r, std::regex("( *)([^ ]+):(.*) Stop.")))                              // make error
            {
                std::cerr << "make: error : " << r[3] << '\n';
                ++error;
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
