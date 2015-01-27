PROJECT   = temperature
APPS      = temperature merging map add lift
SRC_DIR   = src
BUILD_DIR = bin

MKDIR     = mkdir -p
CD        = cd
CMAKE		  = cmake
RM			  = rm -rf

SRCS      = $(wildcard ${SRC_DIR}/${PROJECT}/*.c) \
	 					$(wildcard ${SRC_DIR}/${PROJECT}/*.h)

BINS			= $(addprefix ${BUILD_DIR}/, ${APPS})

all: clean run

run: ${BINS}
	@for bin in $^; do echo "*** executing $${bin}"; ./$${bin}; done

${BINS}: ${BUILD_DIR}/Makefile ${SRCS}
	@(${CD} ${BUILD_DIR}; ${MAKE})

${BUILD_DIR}/Makefile: ${BUILD_DIR}
	@(${CD} ${BUILD_DIR}; ${CMAKE} ../${SRC_DIR}/${PROJECT})

${BUILD_DIR}:
	@${MKDIR} $@

clean:
	@${RM} ${BUILD_DIR}

.PHONY: all run clean
