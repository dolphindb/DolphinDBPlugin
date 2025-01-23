/**
 * @file         ama.h
 * @author       郭光葵
 * @mail         guoguangkui@archforce.com.cn
 * @created time Thu 21 Sep 2017 09:45:45 AM CST
 *
 * Copyright (c) 2018 Archforce Financial Technology.  All rights reserved.
 * Redistribution and use in source and binary forms, with or without  modification, are not permitted.
 * For more information about Archforce, welcome to archforce.cn.
 */

#ifndef __AMA_EXPORT_H__
#define __AMA_EXPORT_H__

#if defined (_WIN32)                                //Windows
    #if !defined (AMA_HAS_DLL)
        #define AMA_HAS_DLL 1
    #endif

    #if defined AMA_HAS_DLL && (AMA_HAS_DLL == 1)   //动态库
        #if defined (AMA_BUILD_DLL)
            #define AMA_EXPORT __declspec(dllexport)
        #else /* AMA_BUILD_DLL */
            #define AMA_EXPORT __declspec(dllimport)
        #endif /* AMA_BUILD_DLL */
    #else                                           // 静态库
        #define AMA_EXPORT
    #endif
#else /* !_WIN32 */                                 // linux
    #define AMA_EXPORT
#endif /* _WIN32 */

#endif // ! API_EXPORT
