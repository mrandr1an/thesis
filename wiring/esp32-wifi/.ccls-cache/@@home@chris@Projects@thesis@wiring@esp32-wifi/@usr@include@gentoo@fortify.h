#ifdef __clang__
# pragma clang system_header
#endif
#ifndef _FORTIFY_SOURCE
# if defined(__has_feature)
#  define __GENTOO_HAS_FEATURE(x) __has_feature(x)
# else
#  define __GENTOO_HAS_FEATURE(x) 0
# endif
#
# if defined(__STDC_HOSTED__) && __STDC_HOSTED__ == 1
#  define __GENTOO_NOT_FREESTANDING 1
# else
#  define __GENTOO_NOT_FREESTANDING 0
# endif
#
# if defined(__OPTIMIZE__) && __OPTIMIZE__ > 0 && __GENTOO_NOT_FREESTANDING > 0
#  if !defined(__SANITIZE_ADDRESS__) && !__GENTOO_HAS_FEATURE(address_sanitizer) && !__GENTOO_HAS_FEATURE(memory_sanitizer)
#   define _FORTIFY_SOURCE 2
#  endif
# endif
# undef __GENTOO_HAS_FEATURE
# undef __GENTOO_NOT_FREESTANDING
#endif
