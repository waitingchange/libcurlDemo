//
//  SecurityMD5.cpp
//  xsdk_frame
//
//  Created by oswin on 2018/1/16.
//  Copyright © 2018年 cocos. All rights reserved.
//

#include <stdio.h>
#include <string>
#include "openssl/md5.h"
#include "XsdkSecurity.h"

#define SUCCESS_TAG 1

int manual_md5_en(lua_State *L)
{
    size_t text_size;
    const char* text_str = lua_tolstring(L, 1, &text_size);
    
    MD5_CTX ctx;
    unsigned char outmd[MD5_DIGEST_LENGTH];
    memset(outmd, 0, sizeof(outmd));
    
    int init_result = SUCCESS_TAG;
    
    do{
        init_result = MD5_Init(&ctx);
        if(init_result != SUCCESS_TAG)
            break;
        
        init_result = MD5_Update(&ctx, text_str, text_size);
        if(init_result != SUCCESS_TAG)
            break;
        
        init_result = MD5_Final(outmd, &ctx);
        if(init_result != SUCCESS_TAG)
            break;
    }while(false);
    
    
    if(init_result != SUCCESS_TAG){
        lua_pushnumber(L, SECURITY_FAIL_CODE);
        lua_pushstring(L, SECURITY_FAIL_TEXT);
        lua_pushstring(L, SECURITY_FAIL_TEXT);
        return 3;
    }
    
    // 转换成 16 进制
    std::string result((const char *)outmd, MD5_DIGEST_LENGTH);
    char output[33];
    for(int j = 0; j < MD5_DIGEST_LENGTH; j++ )
    {
        sprintf( output + j * 2, "%02x", ((unsigned char*)outmd)[j]);
    }
    
    lua_pushnumber(L, MD5_DIGEST_LENGTH);
    lua_pushlstring(L, result.c_str(), MD5_DIGEST_LENGTH);
    lua_pushlstring(L, output, 32);
    
    return 3;
}
