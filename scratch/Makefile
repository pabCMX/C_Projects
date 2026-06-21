MAKEFLAGS += -s --no-print-directory

SUBDIRS := $(patsubst %/Makefile,%,$(wildcard */Makefile))

.PHONY: all clean $(SUBDIRS)

# Default target: build everything in all subdirectories
all: $(SUBDIRS)

# Run 'Make' in each subdir
$(SUBDIRS):
	$(MAKE) -C $@

# Clean: delegate to local Makefile as well
clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done
