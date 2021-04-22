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
 

#include <iostream>
#include <fstream>
#include "CurlDownloader.hpp"
#include "ResumeDownload.hpp"
#include "LibcurlDownloader.hpp"
#include "MD5.hpp"
//#include "Pinger.hpp"


int main(int argc, const char * argv[])
{
    std::string path = "/Users/change/Desktop/demo/测试/";
    std::string md5Str;
//    bool isSuccess = getFileMD5("/Users/change/Desktop/demo/测试/demo.zip", md5Str);
//    if (isSuccess) {
//        cout << "file md5 is " << md5Str << endl;
//    }else{
//        cout << "fuck file is not exist" << endl;
//    }
    
   
    std::string resource = "http://cdn.vipthink.cn/app/pc/teacher/vipthink-teacher_win_1.11.0.exe";
//    download(2,resource,path,"course_sdk_202103251553.zip");
    libcurldownload(10,resource,path,"course_sdk_202103251553.zip");
//    resumeDownload("http://cdn.vipthink.cn/app/pc/teacher/vipthink-teacher_win_1.11.0.exe");

    char c ;
    c = getchar();
    return 0;
}
