//
//  CurlDownloader.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/7/21.
//

#ifndef CurlDownloader_hpp
#define CurlDownloader_hpp

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>

using namespace std;
#ifdef _WIN32
    #include <windows.h>
    #pragma comment(lib,"ws2_32.lib")
    #pragma comment(lib,"winmm.lib")
    #pragma comment(lib,"wldap32.lib")
    #pragma comment(lib,"Advapi32.lib")
#endif

bool download(int threadNum, string url, string path, string fileName);

#endif /* CurlDownloader_hpp */
