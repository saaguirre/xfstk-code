#ifndef XFSTKDLDRAPI_GLOBAL_H
#define XFSTKDLDRAPI_GLOBAL_H

#if defined XFSTK_OS_WIN
#else
#include <QtCore/qglobal.h>
#endif

#if defined XFSTK_OS_WIN

#if defined(XFSTKDLDRAPI_LIBRARY)
#  define XFSTKDLDRAPISHARED_EXPORT __declspec(dllexport)
#else
#  define XFSTKDLDRAPISHARED_EXPORT __declspec(dllimport)
#endif

#else

#if defined(XFSTKDLDRAPI_LIBRARY)
#  define XFSTKDLDRAPISHARED_EXPORT Q_DECL_EXPORT
#else
#  define XFSTKDLDRAPISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif

#endif // XFSTKDLDRAPI_GLOBAL_H
