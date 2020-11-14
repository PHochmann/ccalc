bin_PROGRAMS = ccalc
BUILD_DIR    = ./bin/release
SRC_DIRS     = ./src

CC           = gcc
CFLAGS       = -MMD -MP -std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS      = -lm

ifeq (,$(filter $(MAKECMDGOALS),tests))
	ifeq ($(NOREADLINE),)
		CFLAGS += -DUSE_READLINE
		LDFLAGS += -lreadline
	endif
endif

ifneq (,$(filter $(MAKECMDGOALS),debug))
	BUILD_DIR := ./bin/debug
	CFLAGS += -DDEBUG -g3 -O0
endif

ifneq (,$(filter $(MAKECMDGOALS),noreadline))
	BUILD_DIR := ./bin/noreadline
endif

ifneq (,$(filter $(MAKECMDGOALS),tests))
	BUILD_DIR := ./bin/tests
	SRC_DIRS += ./tests
	SRCS = $(shell find $(SRC_DIRS) -name *.c ! -wholename "./src/client/main.c")
	bin_PROGRAMS := test
else
	SRCS := $(shell find $(SRC_DIRS) -name *.c)
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

all: $(BUILD_DIR)/$(bin_PROGRAMS)

debug: $(BUILD_DIR)/$(bin_PROGRAMS)

tests: $(BUILD_DIR)/$(bin_PROGRAMS)
	@./$(BUILD_DIR)/$(bin_PROGRAMS)

$(BUILD_DIR)/$(bin_PROGRAMS): $(OBJS)
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo Done. Placed executable at $(BUILD_DIR)/$(bin_PROGRAMS)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo Compiling $<
	@$(CC) $(INC_FLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r ./bin

-include $(DEPS)
