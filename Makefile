# ################################################################
# LZ5 - Makefile
# Copyright (C) Yann Collet 2011-2015
# All rights reserved.
#
# BSD license
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# You can contact the author at :
#  - LZ5 source repository : https://github.com/inikep/lz5
# ################################################################

DESTDIR ?=
PREFIX  ?= /usr/local
VOID    := /dev/null

LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR=$(PREFIX)/include
PRGDIR  = programs
LZ5DIR  = lib
TESTDIR = tests


# Define nul output
ifneq (,$(filter Windows%,$(OS)))
EXT = .exe
else
EXT =
endif


.PHONY: default all lib lz5 clean test versionsTest examples

default: lz5

all: lib lz5

lib:
	@$(MAKE) -C $(LZ5DIR) all

lz5:
	@$(MAKE) -C $(PRGDIR)
	@cp $(PRGDIR)/lz5$(EXT) .

clean:
	@$(MAKE) -C $(PRGDIR) $@ > $(VOID)
	@$(MAKE) -C $(TESTDIR) $@ > $(VOID)
	@$(MAKE) -C $(LZ5DIR) $@ > $(VOID)
	@$(MAKE) -C examples $@ > $(VOID)
	@$(RM) lz5$(EXT)
	@echo Cleaning completed


#------------------------------------------------------------------------
#make install is validated only for Linux, OSX, kFreeBSD, Hurd and
#FreeBSD targets
ifneq (,$(filter $(shell uname),Linux Darwin GNU/kFreeBSD GNU FreeBSD))
HOST_OS = POSIX

install:
	@$(MAKE) -C $(LZ5DIR) $@
	@$(MAKE) -C $(PRGDIR) $@

uninstall:
	@$(MAKE) -C $(LZ5DIR) $@
	@$(MAKE) -C $(PRGDIR) $@

travis-install:
	$(MAKE) install PREFIX=~/install_test_dir

test:
	$(MAKE) -C $(TESTDIR) test

clangtest: clean
	clang -v
	@CFLAGS="-O3 -Werror -Wconversion -Wno-sign-conversion" $(MAKE) -C $(LZ5DIR)  all CC=clang
	@CFLAGS="-O3 -Werror -Wconversion -Wno-sign-conversion" $(MAKE) -C $(PRGDIR)  all CC=clang
	@CFLAGS="-O3 -Werror -Wconversion -Wno-sign-conversion" $(MAKE) -C $(TESTDIR) all CC=clang

clangtest-native: clean
	clang -v
	@CFLAGS="-O3 -Werror -Wconversion -Wno-sign-conversion" $(MAKE) -C $(LZ5DIR)  all    CC=clang
	@CFLAGS="-O3 -Werror -Wconversion -Wno-sign-conversion" $(MAKE) -C $(PRGDIR)  native CC=clang
	@CFLAGS="-O3 -Werror -Wconversion -Wno-sign-conversion" $(MAKE) -C $(TESTDIR) native CC=clang

sanitize: clean
	CFLAGS="-O3 -g -DLZ5_NO_HUFFMAN -fsanitize=undefined" $(MAKE) test CC=clang FUZZER_TIME="-T1mn" NB_LOOPS=-i1

staticAnalyze: clean
	CFLAGS=-g scan-build --status-bugs -v $(MAKE) all

platformTest: clean
	@echo "\n ---- test lz5 with $(CC) compiler ----"
	@$(CC) -v
	CFLAGS="-O3 -Werror"         $(MAKE) -C $(LZ5DIR) all
	CFLAGS="-O3 -Werror -static" $(MAKE) -C $(PRGDIR) native
	CFLAGS="-O3 -Werror -static" $(MAKE) -C $(TESTDIR) native
	$(MAKE) -C $(TESTDIR) test-platform

versionsTest: clean
	$(MAKE) -C $(TESTDIR) $@

examples:
	$(MAKE) -C $(LZ5DIR)
	$(MAKE) -C $(PRGDIR) lz5
	$(MAKE) -C examples test

endif


ifneq (,$(filter MSYS%,$(shell uname)))
HOST_OS = MSYS
CMAKE_PARAMS = -G"MSYS Makefiles"
endif


#------------------------------------------------------------------------
#make tests validated only for MSYS, Linux, OSX, kFreeBSD and Hurd targets
#------------------------------------------------------------------------
ifneq (,$(filter $(HOST_OS),MSYS POSIX))

cmake:
	@cd contrib/cmake_unofficial; cmake $(CMAKE_PARAMS) CMakeLists.txt; $(MAKE)

gpptest: clean
	g++ -v
	CC=g++ $(MAKE) -C $(LZ5DIR)  all CFLAGS="-O3 -Wall -Wextra -Wundef -Wshadow -Wcast-align -Werror"
	CC=g++ $(MAKE) -C $(PRGDIR)  all CFLAGS="-O3 -Wall -Wextra -Wundef -Wshadow -Wcast-align -Werror"
	CC=g++ $(MAKE) -C $(TESTDIR) all CFLAGS="-O3 -Wall -Wextra -Wundef -Wshadow -Wcast-align -Werror"

gpptest-native: clean
	g++ -v
	CC=g++ $(MAKE) -C $(LZ5DIR)  all    CFLAGS="-O3 -Wall -Wextra -Wundef -Wshadow -Wcast-align -Werror"
	CC=g++ $(MAKE) -C $(PRGDIR)  native CFLAGS="-O3 -Wall -Wextra -Wundef -Wshadow -Wcast-align -Werror"
	CC=g++ $(MAKE) -C $(TESTDIR) native CFLAGS="-O3 -Wall -Wextra -Wundef -Wshadow -Wcast-align -Werror"

c_standards: clean
	$(MAKE) all MOREFLAGS="-std=gnu90 -Werror"
	$(MAKE) clean
	$(MAKE) all MOREFLAGS="-std=c99 -Werror"
	$(MAKE) clean
	$(MAKE) all MOREFLAGS="-std=gnu99 -Werror"
	$(MAKE) clean
	$(MAKE) all MOREFLAGS="-std=c11 -Werror"
	$(MAKE) clean

endif
