.PHONY: all clean

# Subdirectories
SUBDIRS = outer_space_display astronaut_client  astronaut_display_client game_server

# Path to the common folder
COMMON_PATH = common
COMMON_OBJ = $(COMMON_PATH)/utils.o

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -I$(COMMON_PATH)

# Build all targets
all: common  $(SUBDIRS)

common:
	$(MAKE) -C $(COMMON_PATH)  

# Invoke child Makefile from within its directory
$(SUBDIRS): $(COMMON_OBJ)
	$(MAKE) -C $@  COMMON_PATH=../$(COMMON_PATH) COMMON_OBJ=../$(COMMON_OBJ)

# Clean rule
clean:
	rm -f $(COMMON_OBJ)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
