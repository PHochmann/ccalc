TARGET_EXEC := ccalc
BUILD_DIR := ./bin/release
SRC_DIRS := ./src

CFLAGS := -MMD -MP -std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS := -lm

ifneq (,$(filter $(MAKECMDGOALS),debug tests))
    BUILD_DIR := ./bin/debug
	CFLAGS += -DDEBUG -Og -g3
endif

ifeq (,$(filter $(MAKECMDGOALS),noreadline))
	CFLAGS += -DUSE_READLINE
	LDFLAGS += -lreadline
endif

ifneq (,$(filter $(MAKECMDGOALS),tests))
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
