AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -g

lib_LIBRARIES = librasterizer.a
librasterizer_a_SOURCES = src/color.c src/color_buffer.c src/texture.c \
	 src/framebuffer.c src/rasterizer.c src/index_array.c \
	src/renderer_state.c src/vector_math.c src/vertex_array.c
librasterizer_a_CPPFlAGS = -I$(srcdir)
librasterizer_a_LDFLAGS = -lm
librasterizer_a_CFLAGS = -O2

bin_PROGRAMS = rasterizer
rasterizer_SOURCES = main.c
rasterizer_LDADD = librasterizer.a -lm -lGLEW -lGL -lglfw
rasterizer_CFLAGS = -O2

dist_doc_DATA = README.md
