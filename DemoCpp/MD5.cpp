//
//  MD5.cpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/22/21.
//

#include "MD5.hpp"
#include <stdio.h>
#include <stdlib.h>

bool getFileMD5(std::string filePath,std::string &outPutMd5Str)
{
    FILE *fh;
    long filesize;
    unsigned char *buf;
    
    fh = fopen(filePath.c_str(), "r");
    if(fh == nullptr)
    {
        return false;
    }
    fseek(fh, 0L, SEEK_END);
    filesize = ftell(fh);
    fseek(fh, 0L, SEEK_SET);
    buf = (unsigned char *) malloc(filesize);
    fread(buf, filesize, 1, fh);
    fclose(fh);
    unsigned char md5_result[MD5_DIGEST_LENGTH];
    MD5(buf, filesize, md5_result);
    
    char mdString[33];
    for (int i=0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(&mdString[i*2], "%02x", (unsigned int)md5_result[i]);
    }
    std::string md5Str = std::string(mdString);
    transform(md5Str.begin(),md5Str.end(),md5Str.begin(),::toupper);
    outPutMd5Str = md5Str;
    free(buf);
    return true;
}
