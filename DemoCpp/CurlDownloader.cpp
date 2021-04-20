//
//  CurlDownloader.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/7/21.
//

#include "CurlDownloader.hpp"

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
        long size = 0L;
        downloadNode *pNode = (downloadNode*)ptr;
        downloadMap[pNode->index] = nowDownloaded;
        map <int, long>::iterator i = downloadMap.begin();
        while (i != downloadMap.end())
        {
            size += i->second;
            ++i;
        }
        size = size - long(totalThreadNum) + 1L;
        float precent = ((size * 100 )/ totalDownloadSize) ;
        cout << "download precent is " << precent <<  "% 。"<< endl;
//        cout <<"currentDownloadSize: "<<size << " tototalSize:"<<totalDownloadSize<<endl;
        pthread_mutex_unlock (&g_mutex);
    }
    return 0;
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
        cout << "####thread " << pNode->index << "  #########Downlaod error,Error code:" + curlcode << endl;
        downloadError("Downlaod error, Error code : " + curlcode);
    }
    curl_easy_cleanup(pNode->curl);

    delete pNode;
        
    return NULL;
}

struct data {
  char trace_ascii; /* 1 or 0 */
};

static void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size,
          char nohex)
{
  size_t i;
  size_t c;

  unsigned int width = 0x10;

  if(nohex)
    /* without the hex output, we can fit more on screen */
    width = 0x40;

  fprintf(stream, "%s, %10.10lu bytes (0x%8.8lx)\n",
          text, (unsigned long)size, (unsigned long)size);

  for(i = 0; i<size; i += width) {

    fprintf(stream, "%4.4lx: ", (unsigned long)i);

    if(!nohex) {
      /* hex not disabled, show it */
      for(c = 0; c < width; c++)
        if(i + c < size)
          fprintf(stream, "%02x ", ptr[i + c]);
        else
          fputs("   ", stream);
    }

    for(c = 0; (c < width) && (i + c < size); c++) {
      /* check for 0D0A; if found, skip past and start a new line of output */
      if(nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
         ptr[i + c + 1] == 0x0A) {
        i += (c + 2 - width);
        break;
      }
      fprintf(stream, "%c",
              (ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)?ptr[i + c]:'.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */
      if(nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
         ptr[i + c + 2] == 0x0A) {
        i += (c + 3 - width);
        break;
      }
    }
    fputc('\n', stream); /* newline */
  }
  fflush(stream);
}

static int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  struct data *config = (struct data *)userp;
  const char *text;
  (void)handle; /* prevent compiler warning */

  switch(type) {
  case CURLINFO_TEXT:
    fprintf(stderr, "== Info: %s", data);
    /* FALLTHROUGH */
  default: /* in case a new one is introduced to shock us */
    return 0;

  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }

  dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
  return 0;
}

bool download(int threadNum, string url, string path, string fileName)
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
    
    struct data config;
    config.trace_ascii = 1; /* enable ascii tracing */

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
        
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
        /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pNode);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunction);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void *)pNode);;
        curl_easy_setopt(curl, CURLOPT_RANGE, range);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
        // 设置重定位URL
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        pthread_mutex_lock (&g_mutex);
        currThreadCnt++;
        pthread_mutex_unlock (&g_mutex);
        std::thread thread(workThread, pNode);
        thread.detach();
    }
//    while (currThreadCnt > 0)
//    {
//        usleep(1000000L);
//    }
    return true;

}
