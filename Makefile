# Nonnegative Matrix Factorization Makefile

# Figure out app path
ADMM_DIR := $(shell readlink $(dir $(lastword $(MAKEFILE_LIST))) -f)
PETUUM_ROOT = $(ADMM_DIR)/../../

include $(PETUUM_ROOT)/defns.mk

PETUUM_INCFLAGS+= ${HAS_HDFS}

# Define macros in src/matrix_loader.hpp
# Element in matrix whose absolute value is smaller than INFINITESIMAL is 
# considered 0 at output
PETUUM_CXXFLAGS += -DINFINITESIMAL=0.00001
# Element in matrix is set to MAXELEVAL if its value exceeds MAXELEVAL
# This bound is to prevent numeric overflow
PETUUM_CXXFLAGS += -DMAXELEVAL=100000
# Element in matrix is set to MINELEVAL if its value is lower than MINELEVAL
# This bound is to prevent numeric overflow
PETUUM_CXXFLAGS += -DMINELEVAL=-100000

PETUUM_CXXFLAGS += -static-libstdc++

ADMM_SRC = $(wildcard $(ADMM_DIR)/src/*.cpp)
ADMM_HDR = $(wildcard $(ADMM_DIR)/src/*.hpp)
ADMM_BIN = $(ADMM_DIR)/bin
ADMM_OBJ = $(ADMM_SRC:.cpp=.o)
UTIL_SRC = $(wildcard $(ADMM_DIR)/src/util/*.cpp)
UTIL_HDR = $(wildcard $(ADMM_DIR)/src/util/*.hpp)
UTIL_OBJ = $(UTIL_SRC:.cpp=.o)

all: linearRegression_main 

linearRegression_main: $(ADMM_BIN)/linearRegression_main

$(ADMM_BIN):
	mkdir -p $(ADMM_BIN)

$(ADMM_BIN)/linearRegression_main: $(ADMM_OBJ) $(UTIL_OBJ) $(PETUUM_PS_LIB) $(PETUUM_ML_LIB) $(ADMM_BIN)
	$(PETUUM_CXX) $(PETUUM_CXXFLAGS) $(PETUUM_INCFLAGS) \
	$(ADMM_OBJ) $(UTIL_OBJ) $(PETUUM_PS_LIB) $(PETUUM_ML_LIB) $(PETUUM_LDFLAGS) -o $@

$(ADMM_OBJ): %.o: %.cpp $(ADMM_HDR) $(UTIL_HDR)
	$(PETUUM_CXX) $(NDEBUG) $(PETUUM_CXXFLAGS) -Wno-unused-result $(PETUUM_INCFLAGS) -c $< -o $@

$(UTIL_OBJ): %.o: %.cpp $(UTIL_HDR)
	$(PETUUM_CXX) $(NDEBUG) $(PETUUM_CXXFLAGS) -Wno-unused-result $(PETUUM_INCFLAGS) -c $< -o $@

clean:
	rm -rf $(ADMM_OBJ)
	rm -rf $(UTIL_OBJ)
	rm -rf $(ADMM_BIN)

.PHONY: clean linearRegression_main
