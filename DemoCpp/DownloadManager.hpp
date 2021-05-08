//
//  DownloadManager.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/23/21.
//

#ifndef DownloadManager_hpp
#define DownloadManager_hpp

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
#ifdef _WIN32
    #include <windows.h>
    #pragma comment(lib,"ws2_32.lib")
    #pragma comment(lib,"winmm.lib")
    #pragma comment(lib,"wldap32.lib")
    #pragma comment(lib,"Advapi32.lib")
#endif

#include <unordered_map>

#include "json/rapidjson.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "json/filereadstream.h"
#include <cstdio>
#include <curl/curl.h>
#include "LibcurlDownloader.hpp"
using namespace rapidjson;


struct downloadFileInfo
{
    string url;
    string fileName;
    uint64_t totalFileLen;
    uint64_t currentFileLen;
    int threadNum;
    string fileMd5;
    string outPutName;
    bool isSuccess;
    bool isExist;
    string outTmpFile;
};


struct downloadNode
{
    FILE *fp;
    uint64_t startPos;
    uint64_t endPos;
    CURL *curl;
    int index;
};


class ProgressReceiver {
public:
    ProgressReceiver();
    virtual ~ProgressReceiver();
    virtual void onProgress() = 0;
};



class DownloadManager {
public:
    DownloadManager();
    ~DownloadManager();
    bool checkLocalFileIsExist(std::string filePath);
    uint64_t getDownloadFileLenth(const char *url);
    static DownloadManager * getInstance();
    void setProgressReceiver(ProgressReceiver * receiver);
    std::string updateTempFileJsonInfo();
    void getCurrentDownloadInfo();
    bool download(std::string url, std::string path, std::string fileName, std::string md5);
    ProgressReceiver * getProgressReceiver();
    void deleteNodeById(int id);
    
    void cleanUp();
    
    downloadFileInfo *m_fileInfo;
    unordered_map <int, downloadNode *> downloadThreadInfo;
private:
    static DownloadManager *md_instance;
    ProgressReceiver * m_receiver;
    rapidjson::Document m_jsonDocument;
};




#endif /* DownloadManager_hpp */
