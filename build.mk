include out/config.mk

ARCH          ?= x64
SUFFIX        ?= $(ARCH)
HOST_OS       ?= $(shell uname|tr '[A-Z]' '[a-z]')
BRAND         ?=
SHARED        ?=
OS            ?= $(HOST_OS)
BUILDTYPE     ?= Release
V             ?= 0
CXX           ?= g++
LINK          ?= g++
ANDROID_LIB   ?= $(ANDROID_SDK)/platforms/android-28/android.jar
ANDROID_JAR    = out/android.classs.quark.jar
JAVAC         ?= javac
JAR            = jar
ENV           ?=
QKMAKE         = ./libs/qkmake
QKMAKE_OUT     = out/qkmake
GYP            = $(QKMAKE)/gyp-next/gyp
OUTPUT        ?= $(OS).$(SUFFIX).$(BUILDTYPE)
LIBS_DIR       = out/$(OUTPUT)
BUILD_STYLE   ?= make
BUILD_FILE    ?= quark

#######################

STYLES		= make xcode msvs make-linux cmake-linux cmake
GYPFILES	= Makefile quark.gyp tools/common.gypi out/config.gypi trial/trial.gypi
GYP_ARGS	= -Goutput_dir="out" \
-Iout/var.gypi -Iout/config.gypi -Itools/common.gypi -S.$(OS).$(SUFFIX) --depth=.

ifeq ($(V), 1)
	V_ARG = "V=1"
endif

ifeq ($(OS), android)
	BUILD_STYLE = make-linux
endif

gen_project=\
	echo "{'variables':{'project':'$(1)'}}" > out/var.gypi; \
	$(ENV) GYP_GENERATORS=$(1) $(GYP) -f $(1) $(2) --generator-output="out/$(1)" $(GYP_ARGS)

make_compile=\
	$(ENV) $(1) -C "out/$(BUILD_STYLE)" -f Makefile.$(OS).$(SUFFIX) \
	CXX="$(CXX)" LINK="$(LINK)" $(V_ARG) BUILDTYPE=$(BUILDTYPE) \
	builddir="$(shell pwd)/$(LIBS_DIR)"

.PHONY: $(STYLES) all build test2 clean

.SECONDEXPANSION:

###################### Build ######################

all: build # compile

# GYP file generation targets.
$(STYLES): $(GYPFILES)
	@$(call gen_project,$@,$(BUILD_FILE).gyp)

build: $(BUILD_STYLE) # out/$(BUILD_STYLE)/Makefile.$(OS).$(SUFFIX)
	@$(call make_compile,$(MAKE))

test2: $(GYPFILES)
	@make -C test/2 -f test2.mk

$(ANDROID_JAR): android/org/quark/*.java
	@mkdir -p out/android.classs
	@rm -rf out/android.classs/*
	$(JAVAC) -classpath $(ANDROID_LIB) -d out/android.classs android/org/quark/*.java
	@cd out/android.classs; $(JAR) cfv quark.jar .
	@mkdir -p $(QKMAKE_OUT)/product/android/libs
	@cp out/android.classs/quark.jar $(QKMAKE_OUT)/product/android/libs

clean:
	@rm -rfv $(LIBS_DIR)
	@rm -rfv out/noproj/product/$(OS)
