//
//  LibcurlDownloader.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/20/21.
//

#include "LibcurlDownloader.hpp"


int currThreadCnt;
int totalThreadNum;
uint64_t totalDownloadSize;
bool errorFlag;
map <int, uint64_t> downloadMap;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;


FILE * tmpf;


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




int progressFunction(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    if (totalToDownload > 0 && nowDownloaded >0)
    {
        pthread_mutex_lock (&g_mutex);
        uint64_t size = 0L;
        downloadNode *pNode = (downloadNode*)ptr;
        cout << "Current thread is : " << pNode->index << " , and download precent is "<< nowDownloaded * 100 / totalToDownload << "%"<< endl;
        
        downloadMap[pNode->index] = nowDownloaded;
        map <int, uint64_t>::iterator i = downloadMap.begin();
        while (i != downloadMap.end())
        {
            size += i->second;
            ++i;
        }
        size = size - uint64_t(totalThreadNum) + 1L;  // 计算真实数据长度
        float precent = ((size * 100 ) / totalDownloadSize);
        downloadFileInfo * info = DownloadManager::getInstance()->m_fileInfo;
        info->currentFileLen = size;
        cout << "total download precent is " << precent <<  "% 。"<< endl;
    
      
        std::string jsonInfo = DownloadManager::getInstance()->updateTempFileJsonInfo();
        fseek(tmpf, 0, SEEK_SET);
        fwrite(jsonInfo.c_str(), sizeof(char), jsonInfo.length(), tmpf);
        fflush(tmpf);
        
        
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
        fclose(tmpf);
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
//    DownloadManager::getInstance()->deleteNodeById(pNode->index);
    return NULL;
}




bool libcurldownload()
{
    downloadFileInfo * info = DownloadManager::getInstance()->m_fileInfo;
    unordered_map <int, downloadNode *> & downloadNodes = DownloadManager::getInstance()->downloadThreadInfo;
    
    totalThreadNum = info->threadNum;
    totalDownloadSize = info->totalFileLen;
    const string outFileName = info->outPutName;
    const string tmpFileName = info->outTmpFile;
    errorFlag = false;
    FILE *fp;
    tmpf = fopen(tmpFileName.c_str(), "wb");
    
    if(!tmpf)
    {
        cout << "open local info file error !" << endl;
        return false;
    }
    if (info->isExist) {
        fp = fopen(outFileName.c_str(), "ab");
    }else{
        // 先清除掉本次下载信息
        downloadMap.clear();
        fp = fopen(outFileName.c_str(), "wb");
    }
    if (!fp)
    {
        return false;
    }
    
    unordered_map<int, downloadNode *>::iterator iter;
    iter = downloadNodes.begin();
    while(iter != downloadNodes.end()) {
        downloadNode *pNode = iter->second;
        CURL *curl = curl_easy_init();
        pNode->curl = curl;
        pNode->fp = fp;
        pNode->index = iter->first + 1;
        char range[64] = { 0 };
        
#ifdef _WIN32
        _snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
#else
        snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
#endif
        
        cout << "fuck range " << range << endl;
        
        curl_easy_setopt(curl, CURLOPT_URL, info->url.c_str());
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
        iter++;
    }
 
    return true;
}
