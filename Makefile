SRCDIR=src
BINFILE=zjump

.PHONY: clean

all:
	$(MAKE) -C $(SRCDIR) $@
	cp $(SRCDIR)/$(BINFILE) .

clean:
	$(MAKE) -C $(SRCDIR) $@
	$(RM) $(BINFILE)

