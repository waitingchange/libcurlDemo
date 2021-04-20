//
//  ResumeDownload.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/19/21.
//

#ifndef ResumeDownload_hpp
#define ResumeDownload_hpp

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <string.h>
#include <curl/curl.h>

void resumeDownload(std::string downloadUrl);

#endif /* ResumeDownload_hpp */
