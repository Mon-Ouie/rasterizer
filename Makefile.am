AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -g

lib_LIBRARIES = librasterizer.a
librasterizer_a_SOURCES = src/color_buffer.c src/texture.c
librasterizer_a_CPPFlAGS = -I$(srcdir)

bin_PROGRAMS = rasterizer
rasterizer_SOURCES = main.c
rasterizer_LDADD = librasterizer.a

dist_doc_DATA = README.md
