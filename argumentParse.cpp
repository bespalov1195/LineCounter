#include "argumentParse.hpp"

static void ArgcEqualTwo(const vector<fs::path> &args);
static void ArgcGreaterThanTwo(const vector<fs::path> &args);
static void ArgcGreaterThanTwo(const vector<fs::path> &args);

namespace
{
    string mHelpMsg = "\r Usage: lineCounter [path] <.extension, ...> \n \
                                \r    or: lineCounter [path] \n \
                                \r    or: lineCounter <.extension, ...> \n\n \
                                \r Specify: the path to working directory with extension(s)\n \
                                \r          or the path to working directory without extension(s), \n \
                                \r          or only extension(s).";


    string mErrMsg = "lineCounter: *** Invalid command line parameters. Stop. \n \
                            \rTry 'lineCounter --help' for more information. ";

    fs::path mAbsolutePath;
    set<fs::path> mExtensions;
    function<void(const vector<fs::path>)> parseArgs;

} // namespace


fs::path argParse::GetPath() {return mAbsolutePath;}


set<fs::path> argParse::GetExtensions() {return mExtensions;}


static void ArgcEqualTwo(const vector<fs::path> &args)
{
    auto arg = args.begin();

    fs::directory_entry dirEntry(*arg);

    if (*arg == "-h" || *arg == "--help")
    {
        throw runtime_error(mHelpMsg);
    }

    if (arg->has_root_directory() && dirEntry.exists() && fs::is_directory(*arg))
    {
        mAbsolutePath = *arg;
    }
    else if (arg->has_relative_path() && dirEntry.exists() && fs::is_directory(*arg))
    {
        mAbsolutePath = fs::current_path() / arg->relative_path();
    }
    else if (string(*arg).front() == '.' && *(++string(*arg).cbegin()) != '/')
    {
        mAbsolutePath = fs::current_path();
        mExtensions.insert(*arg);
    }
    else
    {
        throw runtime_error(mErrMsg);
    }
}


static void ArgcGreaterThanTwo(const vector<fs::path> &args)
{
    auto arg = args.begin();

    fs::directory_entry dirEntry(*arg);

    if (arg->has_root_directory() && dirEntry.exists() && fs::is_directory(*arg))
    {
        mAbsolutePath = *arg;
    }
    else if (arg->has_relative_path() && dirEntry.exists() && fs::is_directory(*arg))
    {
        mAbsolutePath = fs::current_path() / arg->relative_path();
    }
    else if (string(*arg).front() == '.')
    {
        mAbsolutePath = fs::current_path();
        mExtensions.insert(*arg);
    }

    advance(arg, 1);

    for_each(arg, args.end(), [](fs::path argnt)
    {
        if (argnt.string().front() == '.')
        {
            mExtensions.insert(argnt);
        }
        else
        {
            throw runtime_error(mErrMsg);
        }

    });
}


void argParse::ArgumentParse(const int &argc, char *argv[])
{
    switch(argc)
    {
        case 1:
            throw runtime_error(mErrMsg);
            break;

        case 2:
            parseArgs = ArgcEqualTwo;
            break;

        default:
            if (argc > 20)
            {
                throw runtime_error("main: *** Too many input parameters!  Stop.");
                break;
            }
            parseArgs = ArgcGreaterThanTwo;
    }

    vector<fs::path> mArgsVector(argv+1, argv + argc);
    
    parseArgs(mArgsVector);
}