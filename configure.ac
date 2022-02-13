#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([Skeldal], [0.1], [next_ghost@quick.cz])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([cztable.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_LANG(C++)
PKG_PROG_PKG_CONFIG([0.25])

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])
PKG_CHECK_MODULES([SDL], [sdl >= 1.2 SDL_mixer >= 1.2])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h climits cstddef cstdlib cstring unistd.h], [], AC_MSG_ERROR(Standard headers not found))
AC_CHECK_HEADER([SDL/SDL.h], [], AC_MSG_ERROR([libSDL header not found]))
AC_CHECK_HEADER([SDL/SDL_mixer.h], [], AC_MSG_ERROR([SDL_mixer header not found]))

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_CHECK_TYPES([int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t], [], AC_MSG_ERROR(Standard types not found))

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit floor getcwd memmove memset mkdir munmap pow setenv sqrt strcasecmp strchr strrchr], [], AC_MSG_ERROR(Standard functions not found))

AC_C_BIGENDIAN()

AC_CONFIG_FILES([
	Makefile
	tools/Makefile
	])
AC_OUTPUT
