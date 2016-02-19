##### Available defines for CJSON_CFLAGS #####
##
## USE_INTERNAL_ISINF:      Workaround for Solaris platforms missing isinf().
## DISABLE_INVALID_NUMBERS: Permanently disable invalid JSON numbers:
##                          NaN, Infinity, hex.
##
## Optional built-in number conversion uses the following defines:
## USE_INTERNAL_FPCONV:     Use builtin strtod/dtoa for numeric conversions.
## IEEE_BIG_ENDIAN:         Required on big endian architectures.
## MULTIPLE_THREADS:        Must be set when Lua CJSON may be used in a
##                          multi-threaded application. Requries _pthreads_.

##### Build defaults #####
LUA_VERSION =       5.1
TARGET =            ac.so
PREFIX =            /usr/local
#CFLAGS =            -g -Wall -pedantic -fno-inline
CFLAGS =            -O3 -Wall -DNDEBUG
LUA_CFLAGS =      -fpic
LUA_LDFLAGS =     -shared
LUA_INCLUDE_DIR =   /usr/include/luajit-2.0/
LUA_CMODULE_DIR =   $(PREFIX)/lib/lua/$(LUA_VERSION)
LUA_MODULE_DIR =    $(PREFIX)/share/lua/$(LUA_VERSION)
LUA_BIN_DIR =       $(PREFIX)/bin


BUILD_CFLAGS =      -I$(LUA_INCLUDE_DIR) $(CJSON_CFLAGS)
OBJS =              ac.o ac_slow.o ac_fast.o ac_lua.o

.PHONY: all clean install install-extra

.SUFFIXES: .html .txt

.c.o:
	$(CC) -c $(CFLAGS) $(LUA_CFLAGS) $(CPPFLAGS) $(BUILD_CFLAGS) -o $@ $<

.txt.html:
	$(ASCIIDOC) -n -a toc $<

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(LUA_LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o $(TARGET)
