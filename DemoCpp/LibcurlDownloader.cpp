//
//  LibcurlDownloader.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/20/21.
//

#include "LibcurlDownloader.hpp"

#include "json/rapidjson.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

using namespace rapidjson;


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


struct downloadInfo
{
    string fileName;
    long totalFileLen;
    long currentFileLen;
    int threadNum;
    string fileMd5;
    uint32_t success;
};

void initTmpJson(int threadNum)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
//    object.AddMember("clientTime", UtilityTools::getCurrentMilliseconds(), allocator);
    document.AddMember("fileName", "Hello.zip", allocator);
    document.AddMember("fileTotalLen", 50000, allocator);
    document.AddMember("fileCurrentLen", 30000, allocator);
    document.AddMember("md5", "adfafagagsd", allocator);
    document.AddMember("isSuccess", 0, allocator);
    rapidjson::Value ObjectArray(rapidjson::kArrayType);

    for(int i = 1; i < threadNum; i++)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("index",i, allocator);//注：常量是没有问题的
        obj.AddMember("startPos", 0, allocator);
        obj.AddMember("endPosPos", 0, allocator);
        obj.AddMember("isSuccess", 0, allocator);
        ObjectArray.PushBack(obj, allocator);
    }
    document.AddMember("threadNodes", ObjectArray, allocator);

    StringBuffer buffer;
    rapidjson::Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    std::string bufferStr = buffer.GetString();
    cout << "fuck buffer str is " << bufferStr << endl;
}

static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    downloadNode *node = (downloadNode *)userdata;
    size_t written = 0;
    pthread_mutex_lock (&g_mutex);
    if (node->startPos + size * nmemb <= node->endPos)
    {
        fseek(node->fp, node->startPos, SEEK_SET);
        written = fwrite(ptr, size, nmemb, node->fp);
        node->startPos += size * nmemb;
    }
    else
    {
        fseek(node->fp, node->startPos, SEEK_SET);
        written = fwrite(ptr, 1, node->endPos - node->startPos + 1, node->fp);
        node->startPos = node->endPos;
    }
    pthread_mutex_unlock (&g_mutex);
    return written;
}

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



int progressFunction(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    if (totalToDownload > 0 && nowDownloaded >0)
    {
        pthread_mutex_lock (&g_mutex);
        long size = 0L;
        downloadNode *pNode = (downloadNode*)ptr;
        cout << "Current thread is : " << pNode->index << " , and download precent is "<< nowDownloaded * 100 / totalToDownload << "%"<< endl;
        
        downloadMap[pNode->index] = nowDownloaded;
        map <int, long>::iterator i = downloadMap.begin();
        while (i != downloadMap.end())
        {
            size += i->second;
            ++i;
        }
        size = size - long(totalThreadNum) + 1L;  // 计算真实数据长度
        float precent = ((size * 100 )/ totalDownloadSize) ;
        
        cout << "total download precent is " << precent <<  "% 。"<< endl;
        
        pthread_mutex_unlock (&g_mutex);
    }
    return 0;
}

void downloadFinish(FILE *fp)
{
    pthread_mutex_lock (&g_mutex);
    currThreadCnt--;
    pthread_mutex_unlock (&g_mutex);
    if (currThreadCnt == 0)
    {
        fclose(fp);
        cout << "download succed......" << endl;
        curl_global_cleanup();
    }
}

void downloadError(string errorString)
{
    if (errorFlag)
        return;
    errorFlag = true;
    cout << "download error " << errorString << endl;
}


void *workThread(void *pData)
{
    downloadNode *pNode = (downloadNode *)pData;
    CURLcode curlcode = curl_easy_perform(pNode->curl);
    if (curlcode == CURLE_OK)
    {
        cout << "####thread " << pNode->index << "  ###Downlaod ok" << endl;
        downloadFinish(pNode->fp);
    }
    else
    {
        cout << "####thread " << pNode->index << "  #########Downlaod error,Error code:" << curlcode << endl;
        downloadError("Downlaod error, Error code : " + curlcode);
    }
    curl_easy_cleanup(pNode->curl);

    delete pNode;
    return NULL;
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
    // 先清除掉本次下载信息
    
    
    initTmpJson(threadNum);
    downloadMap.clear();
    errorFlag = false;
    const string outFileName = path + fileName;
    FILE *fp = fopen(outFileName.c_str(), "wb");
    if (!fp)
    {
        return false;
    }
    
    //根据线程来进行分片
    for (int i = 0; i < threadNum; i++)
    {
        downloadNode *pNode = new downloadNode();
        pNode->startPos = fileLength * i / threadNum;
        pNode->endPos = fileLength * (i + 1) / threadNum;
        CURL *curl = curl_easy_init();

        pNode->curl = curl;
        pNode->fp = fp;
        pNode->index = i + 1;
        char range[64] = { 0 };
        
#ifdef _WIN32
        _snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
#else
        snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
#endif
        cout << "fuck  range is : " << range << endl;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pNode);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunction);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void *)pNode);;
        curl_easy_setopt(curl, CURLOPT_RANGE, range);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
//        // 设置重定位URL
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//
        pthread_mutex_lock (&g_mutex);
        currThreadCnt++;
        pthread_mutex_unlock (&g_mutex);
        std::thread thread(workThread, pNode);
        thread.detach();
    }
    
    return true;
}
