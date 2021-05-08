//
//  DownloadManager.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/23/21.
//

#include "DownloadManager.hpp"


DownloadManager *DownloadManager::md_instance = nullptr;

DownloadManager::DownloadManager():m_receiver(nullptr),m_fileInfo(nullptr),m_jsonDocument(nullptr)
{
    downloadThreadInfo.clear();
}

DownloadManager::~DownloadManager()
{
    cleanUp();
}

void DownloadManager::deleteNodeById(int id)
{
    unordered_map<int, downloadNode *>::iterator iter = downloadThreadInfo.find(id);
      if(iter != downloadThreadInfo.end())
     {
         delete iter->second;
         downloadThreadInfo.erase(iter);
     }
}

void DownloadManager::cleanUp()
{
    unordered_map<int, downloadNode *>::iterator iter;
    iter = downloadThreadInfo.begin();
    while (iter != downloadThreadInfo.end()) {
        delete iter->second;
        downloadThreadInfo.erase(iter++);
    }
    downloadThreadInfo.clear();
}


DownloadManager * DownloadManager::getInstance()
{
    if (!md_instance) {
        md_instance = new DownloadManager();
    }
    return md_instance;
}


// read 方式去查看本地文件是否存在
bool DownloadManager::checkLocalFileIsExist(std::string filePath)
{
    FILE *fh;
    fh = fopen(filePath.c_str(), "rb");
    if(fh == nullptr)
    {
        return false;
    }
    fclose(fh);
    return true;
}


void DownloadManager::getCurrentDownloadInfo()
{
    FILE* fp = fopen(m_fileInfo->outTmpFile.c_str(), "rb"); // non-Windows use "r"
    if (!fp)
    {
        cout << "open local downloadinfo file error!!! " << endl;
    }
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document jsond;
    jsond.ParseStream(is);
    fclose(fp);
    if(jsond.IsNull()){
        cout << "local download info does't has key downloadNodes !!!" << endl;
        updateTempFileJsonInfo();
        return;  
    }
    
    rapidjson::Document::AllocatorType &allocator = m_jsonDocument.GetAllocator();
    m_jsonDocument.CopyFrom(jsond, allocator);
    
//    check
    
    StringBuffer buffer;
    rapidjson::Writer<StringBuffer> writer(buffer);
    m_jsonDocument.Accept(writer);
    std::string bufferStr = buffer.GetString();
//    cout << "fuck .... " << bufferStr << endl;
    
}

std::string DownloadManager::updateTempFileJsonInfo()
{
    int threadNum =  m_fileInfo->threadNum;
    uint64_t fileTotalLen = m_fileInfo->totalFileLen;
    m_jsonDocument.SetObject();
    rapidjson::Document::AllocatorType &allocator = m_jsonDocument.GetAllocator();
    rapidjson::Value fileNameStr(rapidjson::kStringType);
    fileNameStr.SetString(m_fileInfo->fileName.c_str(), m_fileInfo->fileName.size());
    m_jsonDocument.AddMember("fileName", fileNameStr, allocator);
    m_jsonDocument.AddMember("fileTotalLen", fileTotalLen, allocator);
    m_jsonDocument.AddMember("fileCurrentLen", m_fileInfo->currentFileLen, allocator);
    m_jsonDocument.AddMember("threadNum", threadNum, allocator);
    rapidjson::Value md5Str(rapidjson::kStringType);
    md5Str.SetString(m_fileInfo->fileMd5.c_str(), m_fileInfo->fileMd5.size());
    m_jsonDocument.AddMember("md5", md5Str, allocator);
    m_jsonDocument.AddMember("isSuccess", m_fileInfo->isSuccess, allocator);
    rapidjson::Value ObjectArray(rapidjson::kArrayType);
    
    //暴力一点，有就干掉
    if(m_jsonDocument.HasMember("downloadNodes")){
        m_jsonDocument.RemoveMember("downloadNodes");
    }
    unordered_map<int, downloadNode *>::iterator iter;
    iter = downloadThreadInfo.begin();
    while (iter != downloadThreadInfo.end()) {
        downloadNode * node = iter->second;
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("index",iter->first, allocator);
        rapidjson::Value endPos , startPos;
        startPos.SetUint64(node->startPos);
        endPos.SetUint64(node->endPos);
        obj.AddMember("endPos", endPos , allocator);
        obj.AddMember("startPos", startPos, allocator);
        obj.AddMember("isSuccess", node->startPos == node->endPos, allocator);
        ObjectArray.PushBack(obj, allocator);
        iter++;
    }
    m_jsonDocument.AddMember("downloadNodes", ObjectArray, allocator);
    
    StringBuffer buffer;
    rapidjson::Writer<StringBuffer> writer(buffer);
    m_jsonDocument.Accept(writer);
    std::string bufferStr = buffer.GetString();
    return bufferStr;
}


uint64_t DownloadManager::getDownloadFileLenth(const char *url)
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






// 从manager 中选择下载
bool DownloadManager::download(std::string url, std::string path, std::string fileName , std::string md5)
{
    const string outFileName = path + fileName;
    const string outTmpFile = outFileName + ".json";
    uint64_t fileLength = getDownloadFileLenth(url.c_str());
    if (fileLength <= 0)
    {
        return false;
    }
    
    m_fileInfo = new downloadFileInfo();
    m_fileInfo->url = url;
    m_fileInfo->outPutName = outFileName;
    m_fileInfo->threadNum = 2; // 等下想一个算法
    m_fileInfo->fileName = fileName;
    m_fileInfo->totalFileLen = fileLength;
    m_fileInfo->isSuccess = false;
    m_fileInfo->fileMd5 = md5;
    m_fileInfo->outTmpFile = outTmpFile;
    m_fileInfo->isExist = false; // 默认tmpfile 不存在
    
    bool hasTemp = checkLocalFileIsExist(outTmpFile);
    if (!hasTemp)
    {
        // 获取要下载文件长度
        m_fileInfo->currentFileLen = 0;
        for(int i = 0; i < m_fileInfo->threadNum; i++)
        {
            uint64_t startPos = fileLength * i / m_fileInfo->threadNum;
            uint64_t endPos = fileLength * (i + 1) / m_fileInfo->threadNum;
            
            downloadNode *pNode = new downloadNode();
            pNode->startPos = startPos;
            pNode->endPos = endPos;
            pNode->index = i + 1;
            downloadThreadInfo[i] = pNode;
        }
        
        FILE *fp = fopen(outTmpFile.c_str(), "wb");
        if (!fp)
        {
            return false;
        }
        std::string jsonInfo = updateTempFileJsonInfo();
        fwrite(jsonInfo.c_str(), sizeof(char), jsonInfo.length(), fp);
        fclose(fp);
    }else
    {
        m_fileInfo->isExist = true;
        getCurrentDownloadInfo();
        if (!m_jsonDocument.IsNull() && m_jsonDocument.HasMember("downloadNodes")) {
            rapidjson::Value& nodeArray = m_jsonDocument["downloadNodes"];
            size_t len = nodeArray.Size();
            for (int i = 0; i < len; i++) {
                uint64_t startPos,endPos;
                rapidjson::Value &data = nodeArray[i];
                startPos = data["startPos"].GetUint64();
                endPos = data["endPos"].GetUint64();
                int index = data["index"].GetInt();
                downloadNode *pNode = new downloadNode();
                pNode->startPos = startPos;
                pNode->endPos = endPos;
                pNode->index = index;
                downloadThreadInfo[i] = pNode;
//                cout << "fuck startPos is " << startPos << endl;
            }
        }
    }
    
    bool isOn = libcurldownload();
    return isOn;
}

void DownloadManager::setProgressReceiver(ProgressReceiver * receiver)
{
    if (m_receiver) {
        return;
    }
    m_receiver = receiver;
}

ProgressReceiver * DownloadManager::getProgressReceiver()
{
    if (m_receiver) {
        return m_receiver;
    }
    return nullptr;
}



