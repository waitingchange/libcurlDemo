//
//  main.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 1/22/21.
//

// 使用Boost::Asio库, 实现Http的下载
// 使用Boost::Asio库, 不但可以实现下载,
// 而且可以方便的实现断点续传.(以后会补充出来)
// 驳壳地址: http://blog.csdn.net/robertkun
 

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include "CurlDownloader.hpp"
#include "ResumeDownload.hpp"
//#include "Pinger.hpp"

using boost::asio::ip::tcp;


using boost::asio::ip::icmp;

 
struct HttpResponse
{
public:
    explicit HttpResponse(){ clear();}
    std::string http_version;        // 版本
    unsigned int status_code;        // 状态码
    std::string status_message;    // 状态
    std::string header;                    // HTTP包头
    std::string body;           // HTTP返回的内容
    std::string content_type;
    std::string modify_time;
    unsigned int content_length;
    unsigned int total_length;
    unsigned int offset;

    void clear()
    {
        http_version.clear();
        status_code = -1;
        status_message.clear();
        header.clear();
        content_type.clear();
        modify_time.clear();
        content_length = 0;
        total_length = 0;
        offset = 0;
        body.clear();
    }
};

struct DownloadInfo
{
    DownloadInfo()
    {
        id = 0;
        url.clear();
        filename.clear();
        md5.clear();
        writesize = 0;
        continued = false;
        lasterr = 0;
        trycount = 0;
    }
    int id;
    std::string url;
    std::string filename;
    std::string md5;
    int writesize;
    bool continued;
    int lasterr;
    int trycount;
};

int  GetTempFileRange( const std::string& fn );
bool GetHttpFile(const std::string& szHost, const std::string& szParam);
bool AnalyseHeader(HttpResponse& result, std::string& packetString, int nEndHeader);
bool WriteFile( const std::string& fn, int rangeStart, std::string& packetString, DownloadInfo* d_diCurrent );

std::ofstream f_ofsSave;

 
// 获取文件
bool GetHttpFile(const std::string& szHost, const std::string& szParam)
{
    // 创建下载信息
        DownloadInfo* d_diCurrent = new DownloadInfo();
        d_diCurrent->filename = "DownLoadFile";
        d_diCurrent->continued = true;

        if(d_diCurrent->continued) {
            d_diCurrent->filename = d_diCurrent->filename+std::string(".td");
        }

        try
        {
            boost::asio::io_service io_serv;

            // Get a list of endpoints corresponding to the server name.
            std::string szService ("http");
            std::string szIp = szHost;
            int i = szHost.find(":") ;
            if (i != -1)
            {
                szService = szHost.substr(i+1);
                szIp = szHost.substr(0, i);
            }
            tcp::resolver::query query(szIp, szService);

            tcp::resolver m_resolver(io_serv);
            // 创建SOCKET
            tcp::socket s_socket(io_serv);

            tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query), end_it;

            // Try each endpoint until we successfully establish a connection.
            tcp::resolver::iterator it = boost::asio::connect(s_socket, endpoint_iterator);

            if(it == end_it)
                return false;

            boost::asio::streambuf request;
            {
                // 封装请求HTTP GET
                std::ostream request_stream(&request);

                request_stream << "GET " ;
                request_stream << szParam << " HTTP/1.1\r\n";
                request_stream << "Host: " << szHost << "\r\n";

                request_stream << "Accept: */*\r\n";
                request_stream << "Pragma: no-cache\r\n";
                request_stream << "Cache-Control: no-cache\r\n";
                request_stream << "Connection: close\r\n";

                // 1. 是否开启断点续传, 如是则读取临时文件长度
                int rangeStart = 0;
                if (d_diCurrent->continued) {
                    rangeStart = GetTempFileRange(d_diCurrent->filename);
                    if (rangeStart) {
                        request_stream << "Range: bytes=" << rangeStart << "- \r\n";
                    }
                    request_stream << "\r\n";
                }

                boost::asio::write(s_socket, request);

                boost::asio::streambuf response;
                std::ostringstream packetStream;
                try
                {
                    // Read until EOF, writing data to output as we go.
                    bool hasReadHeader = false;

                    boost::system::error_code error;

                    HttpResponse result;
                    result.body.clear();

                    while (boost::asio::read(s_socket, response,
                        boost::asio::transfer_at_least(1), error))
                    {
                        packetStream.str("");
                        packetStream << &response;

                        std::string packetString = packetStream.str();

                        // 2. 是否已分析文件头
                        if (!hasReadHeader)
                        {
                            hasReadHeader = true;
                            // 取出http header
                            size_t nEndHeader = packetString.find("\r\n\r\n");
                            if(nEndHeader == std::string::npos)
                                continue;

                            if(!AnalyseHeader(result, packetString, nEndHeader)) {
                                return false;
                            }
                        }

                        // 3. 写入文件
                        WriteFile(d_diCurrent->filename, rangeStart, packetString, d_diCurrent);
                    }

                    // 4. 关闭文件
                    f_ofsSave.close();

                    // 5. 文件改名
                    std::string fn = "DownLoadFile.zip";
                    rename(d_diCurrent->filename.c_str(), fn.c_str());

                    if (error != boost::asio::error::eof)
                        throw boost::system::system_error(error);

                }
                catch (std::exception& e)
                {
                    std::cout << "Exception: " << e.what() << "\n";
                    return false;
                }
            }
        }
        catch(std::exception& e) {
            std::cout << "Exception: " << e.what() << "\n";
            return false;
        }

        return true;
}
 
// *****************************************************
// 分析文件头
// *****************************************************
bool AnalyseHeader(HttpResponse& result, std::string& packetString, int nEndHeader)
{
    result.header = packetString.substr(0, nEndHeader);

    // Check that response is OK.
    std::istringstream response_stream(result.header);
    response_stream >> result.http_version;
    response_stream >> result.status_code;

    std::string strLine;
    std::getline(response_stream, strLine);
    while (!strLine.empty())
    {
        if (strLine.find("Content-Type:") != std::string::npos)
        {
            result.content_type = strLine.substr(strlen("Content-Type:"));
            result.content_type.erase(0, result.content_type.find_first_not_of(" "));
        }
        if (strLine.find("Content-Length:") != std::string::npos)
        {
            result.content_length = atoi(strLine.substr(strlen("Content-Length:")).c_str());
            result.total_length = result.content_length;
        }
        if (strLine.find("Last-Modified:") != std::string::npos)
        {
            result.modify_time = strLine.substr(strlen("Last-Modified:"));
            result.modify_time.erase(0, result.modify_time.find_first_not_of(" "));
        }
        if (strLine.find("Content-Range: bytes") != std::string::npos)
        {
            std::string tmp = strLine.substr(strlen("Content-Range: bytes"));
            result.offset = atoi(tmp.substr(0, tmp.find('-')).c_str());
            int ipos = tmp.find('/');
            int ivalue = 0;
            if (ipos != std::string::npos)
            {
                ivalue = atoi(tmp.substr(ipos+1).c_str());
            }
            if (ivalue)
                result.total_length = ivalue;
        }
        strLine.clear();
        std::getline(response_stream, strLine);
    }

    if ( result.http_version.substr(0, 5) != "HTTP/")
    {
        std::cout << "Invalid response\n";
        return false;
    }
    if (result.status_code != 200 && result.status_code != 206)
    {
        std::cout << "Response returned with status code "
            << result.status_code << "\n";
        return false;
    }

    packetString.erase(0, nEndHeader + 4);

    return true;
}

// **************************************************************
// 获取临时文件大小
// **************************************************************
int GetTempFileRange( const std::string& fn )
{
    int rangeStart = 0;
    std::ifstream ifs;
    ifs.open(fn, std::ios_base::in | std::ios_base::binary );
    if (ifs.is_open()) {
        ifs.seekg(0, std::ios::end);
        rangeStart = ifs.tellg();
    }
    ifs.close();

    return rangeStart;
}

// **************************************************************
// 写入文件
// **************************************************************
bool WriteFile( const std::string& fn, int rangeStart, std::string& packetString, DownloadInfo* d_diCurrent )
{
    if (!f_ofsSave.is_open())
    {
        if (d_diCurrent->continued)
        {
            f_ofsSave.open(fn, std::ios_base::out | std::ios_base::binary | std::ios_base::app );
            if (f_ofsSave.is_open())
            {
                f_ofsSave.seekp(rangeStart);
                d_diCurrent->writesize += rangeStart;
                //int range = f_ofsSave.tellp();
            }
        }
        else
        {
            f_ofsSave.open(fn, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
        }

        if (!f_ofsSave.is_open())
        {
            return false;
        }

        d_diCurrent->writesize = 0;
    }

    try {
        f_ofsSave.write(packetString.c_str(), packetString.length());
        d_diCurrent->writesize += packetString.length();
    }
    catch (std::exception &e)
    {
        return false;
    }

    std::cout << " write size = "<<d_diCurrent->writesize<<"\n";

    return true;
}


int main(int argc, const char * argv[])
{
    std::string path = "/Users/change/Desktop/demo/测试/";
    std::string resource = "https://game-oss.vipthink.cn/video/class/course_sdk_202103251553.zip";
    download(2,resource,path,"course_sdk_202103251553.zip");
//    resumeDownload("http://cdn.vipthink.cn/app/pc/teacher/vipthink-teacher_win_1.11.0.exe");

    char c ;
    c = getchar();
    return 0;
}
