//
//  LibcurlDownloader.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/20/21.
//

#ifndef LibcurlDownloader_hpp
#define LibcurlDownloader_hpp

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>
#include "DownloadManager.hpp"


using namespace std;
#ifdef _WIN32
    #include <windows.h>
    #pragma comment(lib,"ws2_32.lib")
    #pragma comment(lib,"winmm.lib")
    #pragma comment(lib,"wldap32.lib")
    #pragma comment(lib,"Advapi32.lib")
#endif

bool libcurldownload();



#endif /* LibcurlDownloader_hpp */
