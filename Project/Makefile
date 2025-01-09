# Variables
COMMON_DIR = common
SUBDIRS = astronaut_client astronaut_display_client game_server outer_space_display
COMMON_OBJS = $(COMMON_DIR)/score_message.pb-c.o $(COMMON_DIR)/utils.o
CFLAGS_COMMON = -I../$(COMMON_DIR)

# Targets
all: common_all subdirs_all

# Build `common` first
common_all:
	$(MAKE) -C $(COMMON_DIR) all

# Build all subdirectories
subdirs_all: common_all
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) COMMON_OBJS="$(abspath $(COMMON_OBJS))" CFLAGS+="$(CFLAGS_COMMON)" all;)

# Clean everything
clean:
	$(MAKE) -C $(COMMON_DIR) clean
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) clean;)

.PHONY: all common_all subdirs_all clean
