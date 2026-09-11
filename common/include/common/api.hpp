#pragma once

#ifdef _WIN32
    #ifdef MS_COMMON_EXPORTS
        #define MSC_API __declspec(dllexport)
    #else
        #define MSC_API __declspec(dllimport)
    #endif
#else
    #ifdef MS_COMMON_EXPORTS
        #define MSC_API __attribute__((visibility("default")))
    #else
        #define MSC_API
    #endif
#endif