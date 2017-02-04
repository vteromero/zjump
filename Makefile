SRCDIR=src

.PHONY: clean

all:
	$(MAKE) -C $(SRCDIR) $@

clean:
	$(MAKE) -C $(SRCDIR) $@

