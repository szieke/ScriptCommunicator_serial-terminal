#ifndef TESTDLL_GLOBAL_H
#define TESTDLL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TESTDLL_LIBRARY)
#  define TESTDLLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TESTDLLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TESTDLL_GLOBAL_H
