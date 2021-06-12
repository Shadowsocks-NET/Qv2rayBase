
#ifndef QV2RAYBASE_EXPORT_H
#define QV2RAYBASE_EXPORT_H

#ifdef QV2RAYBASE_STATIC_DEFINE
#  define QV2RAYBASE_EXPORT
#  define QV2RAYBASE_NO_EXPORT
#else
#  ifndef QV2RAYBASE_EXPORT
#    ifdef Qv2rayBase_EXPORTS
        /* We are building this library */
#      define QV2RAYBASE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define QV2RAYBASE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QV2RAYBASE_NO_EXPORT
#    define QV2RAYBASE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QV2RAYBASE_DEPRECATED
#  define QV2RAYBASE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QV2RAYBASE_DEPRECATED_EXPORT
#  define QV2RAYBASE_DEPRECATED_EXPORT QV2RAYBASE_EXPORT QV2RAYBASE_DEPRECATED
#endif

#ifndef QV2RAYBASE_DEPRECATED_NO_EXPORT
#  define QV2RAYBASE_DEPRECATED_NO_EXPORT QV2RAYBASE_NO_EXPORT QV2RAYBASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QV2RAYBASE_NO_DEPRECATED
#    define QV2RAYBASE_NO_DEPRECATED
#  endif
#endif

#endif /* QV2RAYBASE_EXPORT_H */
