
#ifndef KITEMMODELS_EXPORT_H
#define KITEMMODELS_EXPORT_H

#ifdef KITEMMODELS_STATIC_DEFINE
#  define KITEMMODELS_EXPORT
#  define KITEMMODELS_NO_EXPORT
#else
#  ifndef KITEMMODELS_EXPORT
#    ifdef KF5ItemModels_EXPORTS
        /* We are building this library */
#      define KITEMMODELS_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KITEMMODELS_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KITEMMODELS_NO_EXPORT
#    define KITEMMODELS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KITEMMODELS_DEPRECATED
#  define KITEMMODELS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KITEMMODELS_DEPRECATED_EXPORT
#  define KITEMMODELS_DEPRECATED_EXPORT KITEMMODELS_EXPORT KITEMMODELS_DEPRECATED
#endif

#ifndef KITEMMODELS_DEPRECATED_NO_EXPORT
#  define KITEMMODELS_DEPRECATED_NO_EXPORT KITEMMODELS_NO_EXPORT KITEMMODELS_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define KITEMMODELS_NO_DEPRECATED
#endif

#endif
