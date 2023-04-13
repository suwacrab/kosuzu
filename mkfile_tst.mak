# compiler ------------------------------------------------------------------@/
CC	:= gcc
CXX	:= g++

LIBS	:= -static-libgcc -static-libstdc++
LIBS	:= -lkosuzu
LDFLAGS	:= -s
LDFLAGS	+= -Lbin 
LDFLAGS	+= $(LIBS)

#CWINFLAGS	:= -Wl,-subsystem,windows
CWINFLAGS	:= 
CWARN		:= -Wshadow=local
CINCLUDE	:= -Iinclude

CFLAGS		:= -std=c99 $(CINCLUDE)
CFLAGS		+= -O2 -MD $(CWARN)
CFLAGS		+= -finput-charset=cp932 -fexec-charset=cp932
CFLAGS		+= $(CWINFLAGS)

CXXFLAGS	:= -std=c++17 $(CINCLUDE)
CXXFLAGS	+= -O2 -MD $(CWARN)
CXXFLAGS	+= -finput-charset=cp932 -fexec-charset=cp932
CXXFLAGS	+= $(CWINFLAGS)

# output --------------------------------------------------------------------@/
SRC_DIR	:= source
OBJ_DIR	:= build
SRCS_CPP	:= $(shell find $(SRC_DIR) -name *.cpp)
SRCS_C		:= $(shell find $(SRC_DIR) -name *.c)

SRCS_C		:= $(filter-out $(wildcard $(SRC_DIR)/kosuzu/*),$(SRCS_C))
SRCS_CPP	:= $(filter-out $(wildcard $(SRC_DIR)/kosuzu/*),$(SRCS_CPP))

OBJS		:= $(subst $(SRC_DIR),$(OBJ_DIR),$(SRCS_C:.c=.o))
OBJS		+= $(subst $(SRC_DIR),$(OBJ_DIR),$(SRCS_CPP:.cpp=.o))

DEPS	:= $(OBJS:.o=.d)
OUTPUT_SMP	:= bin/suzutest.exe

-include $(DEPS)

all: $(OUTPUT_SMP)

# building samples ----------------------------------------------------------@/
$(OUTPUT_SMP): $(OBJS)
	@echo linking tests...
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rfv $(OBJS)

