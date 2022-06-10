#include "argumentParse.hpp"
#include "lineCounter.hpp"

int main(int argc, char *argv[])
{   
    try
    {
        argParse::ArgumentParse(argc, argv);
    }
    catch(const runtime_error& error)
    {
        cerr << error.what() << endl;
        return EXIT_FAILURE;
    }

    fs::path currentPath = argParse::GetPath();

    cout << "Current Path: " << currentPath << endl;

    set<fs::path> extensions = argParse::GetExtensions();

    if (!extensions.empty())
    {
        cout << "Extension(s): ";
        for(auto &el : extensions)
        {
            cout << el << "  ";
        }
        cout << endl << endl;
    }
    else
    {
        cout << "Extension(s): for all extensions!" << endl << endl;;
    }

    vector<lineCounter::File> filesInfo;

    lineCounter::GetPathsToFiles(currentPath, filesInfo, extensions);

    GetFilesLineCount(filesInfo);

    size_t totalLIneCount = lineCounter::GetTotalLineCount(filesInfo);

    lineCounter::ToDisplay(filesInfo, totalLIneCount);


    return EXIT_SUCCESS;
}