CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AR = $(PREFIX)ar
RANLIB = $(PREFIX)ranlib
RM := rm -rf
MKDIR := mkdir -p

BIN_DIR ?= bin
LIB_DIR ?= lib
BUILD_DIR ?= build
DEP_DIR ?= $(BUILD_DIR)/dep

CFLAGS := -Wall -Wextra -Werror -DNDEBUG -std=c89 -fvisibility=hidden
CXXFLAGS := -Wall -Wextra -Werror -std=c++11
DEPFLAGS = -MMD -MP -MF $(@:$(BUILD_DIR)/%.o=$(DEP_DIR)/%.d)

DRIVER_SRCS := driver/fat16.c \
               driver/fat16_priv.c \
               driver/path.c \
               driver/rootdir.c \
               driver/subdir.c
DRIVER_OBJS := $(DRIVER_SRCS:%.c=$(BUILD_DIR)/%.o)
DRIVER_DEPS := $(DRIVER_OBJS:$(BUILD_DIR)/%.o=$(DEP_DIR)/%.d)

TEST_SRCS := test/AppendSmallFileTest.cpp \
             test/Common.cpp \
             test/DeleteDirectoryTest.cpp \
             test/DeleteFileTest.cpp \
             test/FilenameTest.cpp \
             test/linux_hal.cpp \
             test/LsTest.cpp \
             test/main.cpp \
             test/MkdirTest.cpp \
             test/ReadEmptyFileTest.cpp \
             test/ReadLargeFileTest.cpp \
             test/ReadSmallFileTest.cpp \
             test/RmdirTest.cpp \
             test/Test.cpp \
             test/WriteEraseContentTest.cpp \
             test/WriteLargeFileTest.cpp \
             test/WriteSmallFileTest.cpp
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJS:$(BUILD_DIR)/%.o=$(DEP_DIR)/%.d)

.PHONY: all
all: dynamic static test

.PHONY: dynamic
dynamic: $(LIB_DIR)/libfat16.so
dynamic: CFLAGS += -fPIC

.PHONY: static
static: $(LIB_DIR)/libfat16.a

.PHONY: test
test: $(BIN_DIR)/run_test

$(LIB_DIR)/libfat16.so: $(DRIVER_OBJS)
	@$(MKDIR) $(LIB_DIR)
	$(CC) -shared -o $@ $(DRIVER_OBJS)

$(LIB_DIR)/libfat16.a: $(DRIVER_OBJS)
	@$(MKDIR) $(LIB_DIR)
	$(AR) rc $@ $(DRIVER_OBJS)
	$(RANLIB) $@

$(BIN_DIR)/run_test: $(LIB_DIR)/libfat16.so $(TEST_OBJS)
	@$(MKDIR) $(BIN_DIR)
	$(CXX) -o $@ $(TEST_OBJS) -Wl,-rpath $(LIB_DIR) -L $(LIB_DIR) -lfat16

$(BUILD_DIR)/%.o: %.c
	@$(MKDIR) $(BUILD_DIR)/driver
	@$(MKDIR) $(DEP_DIR)/driver
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $(realpath $<) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@$(MKDIR) $(BUILD_DIR)/test
	@$(MKDIR) $(DEP_DIR)/test
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $(realpath $<) -o $@

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR)

-include $(DRIVER_DEPS)
-include $(TEST_DEPS)
