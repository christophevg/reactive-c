SRC_DIR   = src
BUILD_DIR = bin

MKDIR     = mkdir -p
CD        = cd
CMAKE		  = cmake
RM			  = rm -rf

DOT=dot -Nfixedsize=False -Nfontname=Times-Roman

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

valgrind: ${BUILD_DIR}/Makefile
	@(${CD} ${BUILD_DIR}; make await)
	@echo "${RED}*** Verifying memory aspects using Valgrind...${NC}"
	@printf "${GREEN}press any key to continue..${NC}"
	@read
	@(${VALGRIND} ${BUILD_DIR}/examples/await)

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
	@(${CD} ${BUILD_DIR}/; make $@ )

cmake: ${BUILD_DIR}/Makefile

${BUILD_DIR}/Makefile: ${BUILD_DIR}
	@(${CD} $<; ${CMAKE} -DCMAKE_BUILD_TYPE=${TYPE} ../${SRC_DIR})

images/%.png: ${BUILD_DIR}/examples/%.dot
	@${MKDIR} images
	@${DOT} -Tpng -Gsize=5.5,5.5\! -Gdpi=100 -o $@ $<

${BUILD_DIR}/examples/%.dot: run_%
	@ # <- need a command so that make "thinks" it _can_ rebuild the file

${BUILD_DIR}:
	@${MKDIR} $@

clean:
	@${RM} ${BUILD_DIR}

.PHONY: all build run clean
.PRECIOUS: ${BUILD_DIR}/examples/%.dot
