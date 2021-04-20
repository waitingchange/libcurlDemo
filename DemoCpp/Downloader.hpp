//
//  Downloader.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/16/21.
//

#ifndef Downloader_hpp
#define Downloader_hpp

#include <stdio.h>
//引入libcurl库
#include <curl/curl.h>
//#pragma comment(lib,"libcurl.lib")
//文件操作库
#include <sys/stat.h>
#include <fstream>

char* mLocalFilePath;//下载到本地的文件

//获取已下载部分的大小，如果没有则返回0
curl_off_t getLocalFileLength()
{
    curl_off_t ret = 0;
    struct stat fileStat;
    ret = stat(mLocalFilePath, &fileStat);
    if (ret == 0)
    {
        return fileStat.st_size;//返回本地文件已下载的大小
    }
    else
    {
        return 0;
    }
}

//下载前先发送一次请求，获取文件的总大小
double getDownloadFileLength(const char * url)
{
    double rel = 0, downloadFileLenth = 0;
    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HEADER, 1);    //只需要header头
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1);    //不需要body
    if (curl_easy_perform(handle) == CURLE_OK) {
        curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
    }
    else {
        downloadFileLenth = -1;
    }
    rel = downloadFileLenth;
    curl_easy_cleanup(handle);
    return rel;
}

//文件下载
CURLcode downloadInternal(const char * url)
{
    //1. 获取本地已下载的大小，有则断点续传
    curl_off_t localFileLenth = getLocalFileLength();
    //2. 以追加的方式写入文件
    FILE *file = fopen(mLocalFilePath, "ab+");
    CURL* mHandler = curl_easy_init();
    if (mHandler && file)
    {
         //3. 设置url
        curl_easy_setopt(mHandler, CURLOPT_URL, url);
        //4. 设置请求头 Range 字段信息，localFileLength 不等于0时，值大小就表示从哪开始下载
        curl_easy_setopt(mHandler, CURLOPT_RESUME_FROM_LARGE, localFileLenth);
        
        //5. 设置接收数据的处理函数和存放变量
        curl_easy_setopt(mHandler, CURLOPT_WRITEFUNCTION, writeFile);
        curl_easy_setopt(mHandler, CURLOPT_WRITEDATA, file);
        // 6. 发起请求
        CURLcode rel = curl_easy_perform(mHandler);
        fclose(file);
        return rel;
    }
    curl_easy_cleanup(mHandler);
    return CURLE_FAILED_INIT;
}
#endif /* Downloader_hpp */
