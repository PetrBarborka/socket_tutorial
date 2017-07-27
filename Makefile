CC=clang
CFLAGS=-g -O3 -Isrc -Ilib/header -Wall -Wextra -fsanitize=undefined -DNDEBUG $(OPTFLAGS)
LDLIBS=$(LIB) -ldl -lbsd $(OPTLIBS)  
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
EXECUTABLES=$(patsubst %.c, %, $(SOURCES))

LIB_SOURCES=$(wildcard lib/*.c)
LIB_OBJECTS=$(patsubst %.c, %.o, $(LIB_SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c, %, $(TEST_SRC))

LIB=build/libsockethelper.a
SO_LIB=$(patsubst %.a, %.so, $(LIB))

$(LIB): CFLAGS+=-fPIC
$(LIB): build $(LIB_OBJECTS)
	ar rcs $@ $(LIB_OBJECTS)
	ranlib $@

$(SO_LIB): $(LIB) 
	$(CC) -shared -o $@ $(LIB_OBJECTS)

# The Target Build
all: build $(SO_LIB) $(EXECUTABLES) tests

dev: CFLAGS=-g -O0 -Ilib/header -Isrc -Wall -Wextra -fsanitize=undefined $(OPTFLAGS)
dev: all

build:
	@mkdir -p bin
	@mkdir -p build

# The Unit Tests
.PHONY: tests
tests: $(TESTS)
	######@#$(foreach T, $(TESTS), $(CC) $(CFLAGS) $(T).c $(TARGET) -o $(T))
	sh ./tests/runtests.sh

# The Cleaner 
clean:
	rm -rf build $(LIB_OBJECTS) $(OBJECTS) $(EXECUTABLES) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Checker
check:
	@echo Files with potentially dangerous functions
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|dup|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(SOURCES) || true
