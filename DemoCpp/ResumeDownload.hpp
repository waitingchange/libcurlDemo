//
//  ResumeDownload.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/19/21.
//

#ifndef ResumeDownload_hpp
#define ResumeDownload_hpp

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <curl/curl.h>

using namespace std;

struct downloadNode
{
    FILE *fp;
    long startPos;
    long endPos;
    CURL *curl;
    int index;
};


void resumeDownload(std::string downloadUrl,std::string md5Str);

#endif /* ResumeDownload_hpp */
