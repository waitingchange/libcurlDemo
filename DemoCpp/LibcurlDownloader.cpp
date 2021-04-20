//
//  LibcurlDownloader.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/20/21.
//

#include "LibcurlDownloader.hpp"

int currThreadCnt;
int totalThreadNum;
long totalDownloadSize;
bool errorFlag;
map <int, long> downloadMap;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

struct downloadNode
{
    FILE *fp;
    long startPos;
    long endPos;
    CURL *curl;
    int index;
};


long getDownloadFileLenth(const char *url)
{
    double downloadFileLenth = 0;
    char curl_errbuf[CURL_ERROR_SIZE];
    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url);
    // 输出详细信息
    //curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, curl_errbuf);
    CURLcode code = curl_easy_perform(handle);
    if ( code == CURLE_OK)
    {
        char *ct;
        /* ask for the content-type */
        curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &ct);
        curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
        cout << "We received Content-Type: " << ct  << endl;
    }
    else
    {
        cout << "curl getdownload error and code is: " << code << " and message is: " << curl_easy_strerror(code) << " error buffer : "<<curl_errbuf << endl;
        downloadFileLenth = -1;
    }
    return downloadFileLenth;
}



bool libcurldownload(int threadNum, string url, string path, string fileName)
{
    totalThreadNum = threadNum;
    long fileLength = getDownloadFileLenth(url.c_str());
    if (fileLength <= 0)
    {
        cout<<"get the file length error...";
        return false;
    }
    totalDownloadSize = fileLength;
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>totalDownloadsize: " << totalDownloadSize << endl;
    downloadMap.clear();
    errorFlag = false;
    const string outFileName = path + fileName;
    FILE *fp = fopen(outFileName.c_str(), "wb");
    if (!fp)
    {
        return false;
    }
    
//    struct data config;
//    config.trace_ascii = 1; /* enable ascii tracing */
//
//    for (int i = 0; i < threadNum; i++)
//    {
//        downloadNode *pNode = new downloadNode();
//        pNode->startPos = fileLength * i / threadNum;
//        pNode->endPos = fileLength * (i + 1) / threadNum;
//        CURL *curl = curl_easy_init();
//
//        pNode->curl = curl;
//        pNode->fp = fp;
//        pNode->index = i + 1;
//
//        char range[64] = { 0 };
//#ifdef _WIN32
//        _snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
//#else
//        snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
//#endif
//
//        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
//        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
//        /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
////        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//
//        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pNode);
//        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
//        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunction);
//        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void *)pNode);;
//        curl_easy_setopt(curl, CURLOPT_RANGE, range);
//        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
//        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
//        // 设置重定位URL
//        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//
//        pthread_mutex_lock (&g_mutex);
//        currThreadCnt++;
//        pthread_mutex_unlock (&g_mutex);
//        std::thread thread(workThread, pNode);
//        thread.detach();
//    }
//    while (currThreadCnt > 0)
//    {
//        usleep(1000000L);
//    }
    return true;

}
