#include <fstream>
#include "parallel-scraper/FileIO.hpp"

namespace ps {
    std::vector<std::string> read_lines(const std::string& path){
        std::vector<std::string> lines;
        std::ifstream in(path);
        std::string s;
        while (std::getline(in,s)) if(!s.empty()) lines.push_back(s);
        return lines;
    }
}