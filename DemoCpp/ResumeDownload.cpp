//
//  ResumeDownload.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/19/21.
//

#include "ResumeDownload.hpp"
//采用CURLOPT_RESUME_FROM_LARGE 实现文件断点续传功能

//这个函数为CURLOPT_HEADERFUNCTION参数构造
/* 从http头部获取文件size*/

int currThreadCnt;
int totalThreadNum;
uint64_t totalDownloadSize;
bool errorFlag;
map <int, uint64_t> downloadMap;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;



/* 保存下载文件 */
size_t wirtefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}


uint64_t getDownloadFileLenth(const char *url)
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
        std::cout << "We received Content-Type: " << ct  << std::endl;
    }
    else
    {
        std::cout << "curl getdownload error and code is: " << code << " and message is: " << curl_easy_strerror(code) << " error buffer : "<<curl_errbuf << std::endl;
        downloadFileLenth = -1;
    }
    return downloadFileLenth;
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
        map <int, uint64_t>::iterator it = downloadMap.begin();
        while (it != downloadMap.end())
        {
            size += it->second;
            ++it;
        }
        size = size - uint64_t(totalThreadNum) + 1L;  // 计算真实数据长度
        float precent = ((size * 100 )/ totalDownloadSize) ;
        
        //断点需要 除以剩下的size
        cout << "total download precent is " << precent <<  "% 。"<< endl;
        
        pthread_mutex_unlock (&g_mutex);
    }
    return 0;
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

 
// 下载 或者上传文件函数
int download(CURL *curlhandle, const char * remotepath, const char * localpath,
           uint64_t timeout, uint64_t tries)
{
    FILE *f;
    curl_off_t local_file_len = -1 ;


    downloadNode *pNode = new downloadNode();
    totalDownloadSize = getDownloadFileLenth(remotepath);
    CURLcode r = CURLE_GOT_NOTHING;
    struct stat file_info;
    int use_resume = 0;
    /* 得到本地文件大小 */
    //if(access(localpath,F_OK) ==0)
    totalThreadNum = 1;

    pNode->startPos = 0;
    pNode->endPos = totalDownloadSize ;
    pNode->index = 1;

    if(stat(localpath, &file_info) == 0)
    {
        local_file_len =  file_info.st_size;
        pNode->startPos = local_file_len;
        use_resume  = 1;
        //采用追加方式打开文件，便于实现文件断点续传工作
        f = fopen(localpath, "ab+");
    }else{
        f = fopen(localpath, "wb");
    }
    
    if (f == NULL) {
        perror(NULL);
        return 0;
    }
    

    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);
    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, timeout);  // 设置连接超时，单位秒
//    //设置http 头部处理函数
//    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
//    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);
    
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, (void *)pNode);
    // 设置文件续传的位置给libcurl
    curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);
    if (use_resume) {
        uint64_t file_left_len = totalDownloadSize - local_file_len;
        std::cout << "fuck download file left len is " << file_left_len << std::endl;
        curl_easy_setopt(curlhandle, CURLOPT_MAXFILESIZE_LARGE, file_left_len);
        char range[64] = { 0 };
#ifdef _WIN32
        _snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
#else
        snprintf(range, sizeof (range), "%ld-%ld", pNode->startPos, pNode->endPos);
#endif
        curl_easy_setopt(curlhandle, CURLOPT_RANGE, range);
    }

    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, wirtefunc);
    
    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSFUNCTION, progressFunction);
    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSDATA, (void *)pNode);;
    //curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
    //curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curlhandle, CURLOPT_FORBID_REUSE, 1L);


    r = curl_easy_perform(curlhandle);


    fclose(f);

    if (r == CURLE_OK)
            return 1;
    else {
            fprintf(stderr, "%s\n", curl_easy_strerror(r));
            return 0;
    }
}
 
void resumeDownload(std::string downloadUrl , std::string md5Str) {
    CURL *curlhandle = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curlhandle = curl_easy_init();

    download(curlhandle , downloadUrl.c_str(),"download.zip",1,3);
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();
}
