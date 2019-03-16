BUILDDIR = ./bin
CFLAGS = -lwiringPi
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
CONFDIR = /etc/coolpi
SERVICEDIR = /lib/systemd/system
SERVICEFILE = ./coolpi.service
CONFFILE = ./tools/sample.json

INSTALL = cp -a

target = coolpi
OBCS = $(shell find ./src -name "*.c")
DEPS = $(shell find ./src -name "*.h")
OBJS = $(OBCS:./src%.c=./bin%.o)

$(target): dirpre $(OBJS)
	$(CC) -o $(BUILDDIR)/$(target) $(OBJS) $(CFLAGS)

$(OBJS): ./bin%.o: ./src%.c $(DEPS)
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean install uninstall dirpre
clean:
	rm -rf $(BUILDDIR)

install:
	mkdir -p $(CONFDIR)
	$(INSTALL) $(BUILDDIR)/$(target) $(BINDIR)
	$(INSTALL) $(CONFFILE) $(CONFDIR)
	$(INSTALL) $(SERVICEFILE) $(SERVICEDIR)
	@echo "enable the service"
	@systemctl daemon-reload
	@systemctl enable coolpi.service
	@systemctl restart coolpi.service
	@echo "Done!"

uninstall:
	@echo "stopping the coolpi"
	@systemctl stop coolpi
	@echo Stopped
	rm -f $(BINDIR)/$(target)
	rm -rf $(CONFDIR)
	rm -f $(SERVICEDIR)/$(SERVICEFILE)
	@systemctl daemon-reload
	@echo "Done"

dirpre:
	@mkdir -p $(BUILDDIR)