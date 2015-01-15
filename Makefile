PROJECT   = observer
SRC_DIR   = src
BUILD_DIR = bin

MKDIR     = mkdir -p
CD        = cd
CMAKE		  = cmake
RM			  = rm -rf

all: run

run: ${BUILD_DIR}/${PROJECT}
	@${BUILD_DIR}/${PROJECT}

${BUILD_DIR}/${PROJECT}: ${BUILD_DIR}/Makefile
	@(${CD} ${BUILD_DIR}; ${MAKE})

${BUILD_DIR}/Makefile: ${BUILD_DIR}
	@(${CD} ${BUILD_DIR}; ${CMAKE} ../${SRC_DIR})

${BUILD_DIR}:
	@${MKDIR} $@

clean:
	@${RM} ${BUILD_DIR}

.PHONY: all run clean
