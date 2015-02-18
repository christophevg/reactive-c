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

all: clean run

RED   = \033[0;31m
GREEN = \033[0;32m
NC  	= \033[0m 		# NoColor

verify:
	@echo "${RED}*** Verifying GCC...${NC}"
	@printf "${GREEN}press any key to start..${NC}"
	@read
	@(CC=${GCC} make TYPE=Debug)
	@echo "${RED}*** Verifying memory aspects using Valgrind...${NC}"
	@printf "${GREEN}press any key to continue..${NC}"
	@read
	@(${VALGRIND} bin/await)
	@echo "${RED}*** Verifying static code using scan-build...${NC}"
	@printf "${GREEN}press any key to continue..${NC}"
	@read
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
