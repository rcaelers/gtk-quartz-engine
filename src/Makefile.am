INCLUDES = $(GTK_CFLAGS) -xobjective-c -Wall -Werror

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines
engine_LTLIBRARIES = libquartz.la

libquartz_la_SOURCES = \
        quartz-theme-main.c	\
	quartz-style.c		\
	quartz-style.h		\
	quartz-rc-style.c	\
	quartz-rc-style.h	\
	quartz-draw.c		\
	quartz-draw.h

libquartz_la_LDFLAGS = -module -avoid-version -no-undefined -framework Carbon -framework AppKit
libquartz_la_LIBADD =  $(GTK_LIBS) -lobjc

noinst_PROGRAMS = test

test_SOURCES = test.c
test_LDADD = $(GTK_LIBS)
