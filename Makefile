SRC_DIR   = src
BUILD_DIR = bin

MKDIR     = mkdir -p
CD        = cd
CMAKE		  = cmake
RM			  = rm -rf

DOT=dot -Nfixedsize=False -Nfontname=Times-Roman -Nshape=rectangle

TYPE=Release

GCC=gcc-mp-4.6
VALGRIND=valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all
SCANBUILD=scan-build-mp-3.5 -o report

all: clean test

RED   = \033[0;31m
GREEN = \033[0;32m
NC  	= \033[0m 		# NoColor

verify: gcc valgrind scan-build

gcc:
	@echo "${RED}*** Verifying GCC...${NC}"
	@printf "${GREEN}press any key to start..${NC}"
	@read
	@(CC=${GCC} make TYPE=Debug)

valgrind: ${BUILD_DIR}/examples/await
	@echo "${RED}*** Verifying memory aspects using Valgrind...${NC}"
	@printf "${GREEN}press any key to continue..${NC}"
	@read
	@(${VALGRIND} $<)

scan-build:
	@echo "${RED}*** Verifying static code using scan-build...${NC}"
	@printf "${GREEN}press any key to continue..${NC}"
	@read
	@(${SCANBUILD} make)

examples: cmake
	@(${CD} ${BUILD_DIR}; ${MAKE} examples)

test: cmake
	@(${CD} ${BUILD_DIR}; ${MAKE} check)

perf: cmake
	@(${CD} ${BUILD_DIR}; ${MAKE} perf)

run_%: ${BUILD_DIR}/Makefile
	@(cd ${BUILD_DIR}/; make $@ )

cmake: ${BUILD_DIR}/Makefile

${BUILD_DIR}/Makefile: ${BUILD_DIR}
	@(${CD} $<; ${CMAKE} -DCMAKE_BUILD_TYPE=${TYPE} ../${SRC_DIR})

images/%.png: ${BUILD_DIR}/examples/%.dot
	@${MKDIR} images
	@${DOT} -Tpng -Gsize=5.5,5.5\! -Gdpi=100 -o $@ $<

${BUILD_DIR}/examples/await.dot: ${BUILD_DIR}/examples/dot
	@(cd ${BUILD_DIR}/examples/; ./dot)

${BUILD_DIR}/examples/await: ${BUILD_DIR}/Makefile
	@(cd ${BUILD_DIR}; make await)

${BUILD_DIR}/examples/dot: ${BUILD_DIR}/Makefile
	@(cd ${BUILD_DIR}; make dot)

${BUILD_DIR}:
	@${MKDIR} $@

clean:
	@${RM} ${BUILD_DIR}

.PHONY: all build run clean
