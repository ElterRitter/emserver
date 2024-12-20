#ifndef CSERVER_GLOBAL_H
#define CSERVER_GLOBAL_H

#ifdef WIN32
# ifndef CSERVER_BUILD_STATIC
#  ifdef CSERVER_LIBRARY
#    define CSERVER_EXPORT __declspec(dllexport)
#  else
#    define CSERVER_EXPORT __declspec(dllimport)
#  endif
# else
#    define CSERVER_EXPORT
# endif
#else
#  if __GNUC__ >= 4
#    define CSERVER_EXPORT __attribute__ ((visibility ("default")))
#  else
#    define CSERVER_EXPORT
#  endif
#endif

#endif // CSERVER_GLOBAL_H
