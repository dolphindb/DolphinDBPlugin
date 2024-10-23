PYTHON ?= python
BUILDTYPE ?= Release
GYP=./tools/gyp-next/gyp
CPPLINT=./tools/cpplint/cpplint.py
CLANG_FORMAT=./tools/node_modules/.bin/clang-format
MAIN_GYP_FILE=./libencode.gyp

out/$(BUILDTYPE)/libencode.a: out/$(BUILDTYPE)/Makefile
	make -C out BUILDTYPE=$(BUILDTYPE) libencode

example: out/$(BUILDTYPE)/example

out/$(BUILDTYPE)/example: out/$(BUILDTYPE)/Makefile
	make -C out BUILDTYPE=$(BUILDTYPE) example

out/$(BUILDTYPE)/Makefile: $(MAIN_GYP_FILE)
	$(GYP) \
		--depth=. \
		--generator-output=./out \
		-Goutput_dir=. \
		-fmake \
		$(MAIN_GYP_FILE)
	$(GYP) \
		--depth=. \
		--generator-output=./out \
		-Goutput_dir=./out \
		-fcompile_commands_json \
		$(MAIN_GYP_FILE)

CPP_FILES=$(shell find src include -type f ! -name '*-data.cc' -name '*.cc' -or -name '*.h')
lint-cpp: $(CPP_FILES)
	$(CPPLINT) $(CPP_FILES) src/urlencode-data.cc
clang-format: $(CPP_FILES)
	$(CLANG_FORMAT) -i --style=file $(CPP_FILES)

.PHONY: out/$(BUILDTYPE)/libencode.a out/$(BUILDTYPE)/example lint-cpp clang-format
