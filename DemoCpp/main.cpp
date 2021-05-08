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
//#include "ResumeDownload.hpp"
//#include "LibcurlDownloader.hpp"
#include "MD5.hpp"
#include "DownloadManager.hpp"
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
    std::string resource = "https://gimg2.baidu.com/image_search/src=http%3A%2F%2Fyouimg1.c-ctrip.com%2Ftarget%2Ftg%2F035%2F063%2F726%2F3ea4031f045945e1843ae5156749d64c.jpg&refer=http%3A%2F%2Fyouimg1.c-ctrip.com&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1623036229&t=7471b8118168bd13f2e01a3551b8a40a";
    DownloadManager::getInstance()->download(resource,path,"student_h5_res_20110318018.png","sdadfafsafa");
//    resumeDownload(resource,"b8de9d41adbbb657fd25a37fe14ece3b");
   
    
//    download(2,resource,path,"course_sdk_202103251553.zip");
//    libcurldownload(10,resource,path,"course_sdk_202103251553.zip");
//    resumeDownload("http://cdn.vipthink.cn/app/pc/teacher/vipthink-teacher_win_1.11.0.exe");

    char c ;
    c = getchar();
    return 0;
}
