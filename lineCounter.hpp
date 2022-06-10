#pragma once

#include <iostream>
#include <filesystem>
#include <functional>
#include <fstream>
#include <vector>
#include <set>
#include <condition_variable>
#include <thread>
#include <future>
#include <queue>


using namespace std;
namespace fs = filesystem;

namespace lineCounter
{
    struct File
    {
        fs::path mPathToFile;
        unsigned long int mCounter;

        File(string pathToFile) : mPathToFile(pathToFile), mCounter(0) {}
    };

    void GetPathsToFiles(const fs::path &pathToDir, vector<File> &filesInfo, const set<fs::path> &extensions);
    void GetFilesLineCount(vector<File> &filesInfo);
    size_t GetTotalLineCount(vector<File> &filesInfo);
    void ToDisplay(vector<lineCounter::File> &filesInfo, size_t &totalLines);

} // namespace lineCounter
