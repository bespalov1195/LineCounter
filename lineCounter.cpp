#include "lineCounter.hpp"

template <class Func, class Arg> 
static void Enqueue(Func task, Arg &iterator);

static void ThreadPool (int threadsCount);
static void FilesLineCounter(const vector<lineCounter::File>::iterator &fileAttributes);
static void WaitAll();  


namespace
{
    string errMsg("*** An error occurred while reading file. ***");
    string errNonExOrNoAccFileMsg(": The file does not exist or the executing process is not allowed to access the file.");
    string errDirMsg(": It is a directory.");
    char   qMarks('\'');

    vector<thread> mThreads;
    queue<future<void>> mTasks;

    condition_variable mEventVar;
    mutex mEventMutex;
    mutex mLineCounterMutex;

    bool mStopping = false;



    void ExceptionsConfig(ifstream &fin)
    {
        fin.exceptions(ifstream::badbit | ifstream::failbit | ifstream::eofbit);
    }

    void FileIOErrorHandling(fs::path &path, ifstream &fin)
    {
        if (!fin.eof() && fin.fail() && !fin.bad())
        {
            lock_guard<mutex> lock(mLineCounterMutex);
           
            cerr << errMsg <<  endl;
            cerr << qMarks << path << qMarks << errNonExOrNoAccFileMsg << endl << endl;
        }
        else if (!fin.eof() && fin.fail() && fin.bad())
        {
            lock_guard<mutex> lock(mLineCounterMutex);
            
            cerr << errMsg <<  endl;
            cerr << qMarks << path << qMarks << errDirMsg << endl << endl;
        }

        if (fin.is_open())
            fin.close();
        
        fin.clear();
    }

} //namespace


static void ThreadPool (int threadsCount)
{
    while(threadsCount--)
    {
        mThreads.emplace_back([=] ()
        {
            while(true)
            {
                unique_lock<mutex> lock(mEventMutex);
                mEventVar.wait(lock, [=] () {return mStopping || !mTasks.empty();});

                if (mStopping && mTasks.empty())
                    break;

                auto task = move(mTasks.front());
                mTasks.pop();

                lock.unlock();

                task.wait();
            }
        });
    }
}


template <class Func, class Arg> 
static void Enqueue(Func task, Arg &iterator)
{
    unique_lock<mutex> locker(mEventMutex);
    mTasks.emplace(async(launch::deferred, task, iterator));
    locker.unlock();

    mEventVar.notify_one();
}


static void WaitAll()
{
    while(!mTasks.empty());

    unique_lock<mutex> locker(mEventMutex);
    mStopping = true;
    locker.unlock();

    mEventVar.notify_all();

    for(auto &thread : mThreads)
        thread.join();
}


void lineCounter::GetPathsToFiles(const fs::path &pathToDir, vector<File> &filesInfo, const set<fs::path> &extensions)
{
    try
    {
        for(auto &elDirEntry : fs::recursive_directory_iterator(pathToDir,fs::directory_options::skip_permission_denied ))
        {
            if (elDirEntry.is_regular_file())
            {
                if (!extensions.empty())
                {
                    for(auto &elExtension : extensions)
                    {
                        if (elDirEntry.path().extension().string() == elExtension)
                        {
                            filesInfo.emplace_back(lineCounter::File(elDirEntry.path().string()));
                            break;
                        }
                    }
                }
                else
                {
                    filesInfo.emplace_back(elDirEntry.path().string());
                }
            }
        }
    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
    }
}


void lineCounter::GetFilesLineCount(vector<lineCounter::File> &filesInfo)
{
    auto pathsVectorIter = filesInfo.begin();

    auto concurrentThreadsCount = thread::hardware_concurrency();

    if (concurrentThreadsCount > filesInfo.size())
        concurrentThreadsCount = filesInfo.size();
    
    ThreadPool(concurrentThreadsCount);

    pathsVectorIter = filesInfo.begin();

    while (pathsVectorIter != filesInfo.end())
    {
        Enqueue(FilesLineCounter, pathsVectorIter);
        pathsVectorIter++;
    }

    WaitAll();
}


static void FilesLineCounter(const vector<lineCounter::File>::iterator &fileAttributes)
{
    unique_lock<mutex> locker(mLineCounterMutex);
    auto currentPathToFile = fileAttributes->mPathToFile;
    locker.unlock();
    
    ifstream fin;
    ExceptionsConfig(fin);
    unsigned int counter = 0;

    try
    {
        fin.open(currentPathToFile);
        
        string unused;

        while(getline(fin, unused))
            ++counter;

        fin.close();
    }
    catch(const ifstream::failure &ex)
    {
        if (fin.eof() && !fin.bad())
        {
            if(!fin.fail())
                ++counter;

            if (fin.is_open())
                fin.close();

            fin.clear();
        }
        else
        {
            unique_lock<mutex> locker(mLineCounterMutex);
            cerr << ex.what() << endl;
            cerr << ex.code() << endl;
            locker.unlock();

            FileIOErrorHandling(currentPathToFile, fin);
        }
    }
    catch(...)
    {
        unique_lock<mutex> locker(mLineCounterMutex);
        cerr << errMsg <<  endl;
        cerr << fileAttributes->mPathToFile << endl << endl;
        locker.unlock();
            
        counter = 0;

        if (fin.is_open())
            fin.close();
        
        fin.clear();
    }

    locker.lock();
    fileAttributes->mCounter = counter;
}


size_t lineCounter::GetTotalLineCount(vector<lineCounter::File> &filesInfo)
{
    size_t totalLineCounter = 0;

    for(auto &fileAttributes : filesInfo)
    {
        totalLineCounter += fileAttributes.mCounter;
    }

    return totalLineCounter;
}

void lineCounter::ToDisplay(vector<lineCounter::File> &filesInfo, size_t &totalLines)
{
    cout  << "==========EVENTUALLY==========" << endl;
    for(auto &fileAttributes : filesInfo)
    {
        cout <<"The path: "<<  fileAttributes.mPathToFile << "\t - contains  " << fileAttributes.mCounter << "\t lines." << endl; 
    }
    cout << endl;

    cout << "Total Line Count: " << totalLines << endl;
}