ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=computelibrary

#$(INSTALL_ROOT_$(OS)) is pointing to $QNX_TARGET
#by default, unless it was manually re-routed to
#a staging area by setting both INSTALL_ROOT_nto
#and USE_INSTALL_ROOT
computelibrary_INSTALL_ROOT ?= $(INSTALL_ROOT_$(OS))

#override 'all' target to bypass the default QNX build system
ALL_DEPENDENCIES = computelibrary_all
.PHONY: computelibrary_all install check clean

include $(MKFILES_ROOT)/qtargets.mk

SCONS_ARGS = -Q \
             debug=1 \
             arch=armv8a \
             os=qnx \
             build_dir=arm64nowerror \
             standalone=0 \
             opencl=0 \
             openmp=0 \
             validation_tests=1 \
             neon=1 \
             toolchain_prefix=" " \
             cppthreads=1 \
             compiler_prefix="" \
             extra_cxx_flags="-D_QNX_SOURCE=1" \
             Werror=0 \
             reference_openmp=0 \
             build_dir=$(PROJECT_ROOT)/nto-aarch64-le/build \
             install_dir=$(computelibrary_INSTALL_ROOT) \
            -j 12

ifndef NO_TARGET_OVERRIDE
computelibrary_all:
	@cd $(PRODUCT_ROOT)/../ && scons $(SCONS_ARGS)

install check: computelibrary_all
	@cd $(PRODUCT_ROOT)/../ && scons $(SCONS_ARGS)

clean iclean spotless:
	rm -rf $(PROJECT_ROOT)/nto-aarch64-le/build

uninstall:
endif
