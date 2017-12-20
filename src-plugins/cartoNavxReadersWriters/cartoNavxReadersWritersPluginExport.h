#pragma once

#ifdef WIN32
    #ifdef cartoNavxReadersWritersPlugin_EXPORTS
        #define CARTONAVXREADERSWRITERSPLUGIN_EXPORT __declspec(dllexport)
    #else
        #define CARTONAVXREADERSWRITERSPLUGIN_EXPORT __declspec(dllimport)
    #endif
#else
    #define CARTONAVXREADERSWRITERSPLUGIN_EXPORT
#endif
