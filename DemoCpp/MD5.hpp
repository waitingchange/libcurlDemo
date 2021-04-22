//
//  MD5.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/22/21.
//

#ifndef MD5_hpp
#define MD5_hpp

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "openssl/md5.h"


bool getFileMD5(std::string filePath,std::string &outPutMd5Str);

#endif /* MD5_hpp */
