#ifndef CLIENT_GLOBAL_H
#define CLIENT_GLOBAL_H

#ifdef WIN32
# ifndef CLIENT_BUILD_STATIC
#  ifdef CLIENT_LIBRARY
#    define CLIENT_EXPORT __declspec(dllexport)
#  else
#    define CLIENT_EXPORT __declspec(dllimport)
#  endif
# else
#    define CLIENT_EXPORT
# endif
#else
#  if __GNUC__ >= 4
#    define CLIENT_EXPORT __attribute__ ((visibility ("default")))
#  else
#    define CLIENT_EXPORT
#  endif
#endif

#endif // CLIENT_GLOBAL_H
