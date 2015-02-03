SRC_DIR   = src
BUILD_DIR = bin

MKDIR     = mkdir -p
CD        = cd
CMAKE		  = cmake
RM			  = rm -rf

all: clean run

${BUILD_DIR}/Makefile: ${BUILD_DIR}
	@(${CD} $<; ${CMAKE} ../${SRC_DIR})

build: ${BUILD_DIR}/Makefile
	@(${CD} ${BUILD_DIR}; ${MAKE})

run: build
	@(${CD} ${BUILD_DIR}; ${MAKE} run)

${BUILD_DIR}:
	@${MKDIR} $@

clean:
	@${RM} ${BUILD_DIR}

.PHONY: all build run clean
