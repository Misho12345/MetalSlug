#pragma once

#ifdef _WIN32
    #ifdef MSE_EXPORTS
        #define MSE_API __declspec(dllexport)
    #else
        #define MSE_API __declspec(dllimport)
    #endif
#else
    #ifdef MSE_EXPORTS
        #define MSE_API __attribute__((visibility("default")))
    #else
        #define MSE_API
    #endif
#endif