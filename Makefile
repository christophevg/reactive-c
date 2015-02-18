SRC_DIR   = src
BUILD_DIR = bin

MKDIR     = mkdir -p
CD        = cd
CMAKE		  = cmake
RM			  = rm -rf

DOT=dot -Nfixedsize=False -Nfontname=Times-Roman -Nshape=rectangle

TYPE=Release

GCC=gcc-mp-4.6
SCANBUILD=scan-build-mp-3.5 -o report

all: clean run

verify:
	@(CC=${GCC} make)
	@(${SCANBUILD} make)

${BUILD_DIR}/Makefile: ${BUILD_DIR}
	@(${CD} $<; ${CMAKE} -DCMAKE_BUILD_TYPE=${TYPE} ../${SRC_DIR})

build: ${BUILD_DIR}/Makefile
	@(${CD} ${BUILD_DIR}; ${MAKE})

run: build
	@(${CD} ${BUILD_DIR}; ${MAKE} run)

images/await.png: bin/await.dot
	@${MKDIR} images
	@${DOT} -Tpng -Gsize=5.5,5.5\! -Gdpi=100 -o $@ $<

${BUILD_DIR}:
	@${MKDIR} $@

clean:
	@${RM} ${BUILD_DIR}

.PHONY: all build run clean
