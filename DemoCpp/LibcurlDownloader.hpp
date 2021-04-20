//
//  LibcurlDownloader.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/20/21.
//

#ifndef LibcurlDownloader_hpp
#define LibcurlDownloader_hpp

#include <stdio.h>
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

bool libcurldownload(int threadNum, string url, string path, string fileName);



#endif /* LibcurlDownloader_hpp */
