#pragma once

#include <iostream>
#include <vector>
#include <filesystem>
#include <functional>
#include <string>
#include <set>
#include <algorithm>


using namespace std;
namespace fs = filesystem;

namespace argParse
{
    fs::path GetPath();
    set<fs::path> GetExtensions();
    void ArgumentParse(const int &argc, char *argv[]);

} // namespace optionParse