TARGET_EXEC  = ccalc
BUILD_DIR    = ./bin/release
# No trailing forward slash in INSTALL_PATH!
INSTALL_PATH = /etc/ccalc
SRC_DIRS     = ./src

CFLAGS       = "-DINSTALL_PATH=\"$(INSTALL_PATH)\"" -MMD -MP -std=c99 -Wall -Wextra -Werror -pedantic -Werror=vla
LDFLAGS      = -lm

# Compile with readline if no opt-out and target is not test
ifeq (,$(filter $(MAKECMDGOALS),tests))
	ifeq ($(NOREADLINE),)
		CFLAGS  += -DUSE_READLINE
		LDFLAGS += -lreadline
	endif
endif

# Compile with debugging flags if target is debug or tests
ifneq (,$(filter $(MAKECMDGOALS),debug))
	BUILD_DIR =  ./bin/debug
	CFLAGS    += -DDEBUG -g3 -O0 -fsanitize=undefined
	LDFLAGS   += -fsanitize=undefined
endif

# Compile additional test sources 
ifneq (,$(filter $(MAKECMDGOALS),tests))
	TARGET_EXEC  =  test
	BUILD_DIR    =  ./bin/tests
	SRC_DIRS     += ./tests
	CFLAGS       += -g3 -O0
	SRCS = $(shell find $(SRC_DIRS) -name *.c ! -wholename "./src/client/main.c")
else
	SRCS = $(shell find $(SRC_DIRS) -name *.c)
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

all: $(BUILD_DIR)/$(TARGET_EXEC)

debug: $(BUILD_DIR)/$(TARGET_EXEC)

tests: $(BUILD_DIR)/$(TARGET_EXEC)
	@echo Running tests...
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
