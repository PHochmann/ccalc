TARGET_EXEC := ccalc
BUILD_DIR := ./bin/release
SRC_DIRS := ./src

ifeq ($(filter debug,$(MAKECMDGOALS)),debug)
    BUILD_DIR := ./bin/debug
endif

ifeq ($(filter tests,$(MAKECMDGOALS)),tests)
	SRC_DIRS += ./tests
	SRCS = $(shell find $(SRC_DIRS) -name *.c ! -wholename "./src/main.c")
	TARGET_EXEC := test
else
	SRCS := $(shell find $(SRC_DIRS) -name *.c)
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS := -std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS := -lm

ifneq ($(filter noreadline,$(MAKECMDGOALS)),noreadline)
	CFLAGS += -DUSE_READLINE
	LDFLAGS += -lreadline
endif

ifeq ($(filter debug,$(MAKECMDGOALS)),debug)
	CFLAGS += -Og -g3
endif

all: $(BUILD_DIR)/$(TARGET_EXEC)

debug: $(BUILD_DIR)/$(TARGET_EXEC)

noreadline: $(BUILD_DIR)/$(TARGET_EXEC)

tests: $(BUILD_DIR)/$(TARGET_EXEC)
	@./$(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo Done. Placed executable at $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo Compiling $<
	@$(CC) $(INC_FLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r ./bin

-include $(DEPS)