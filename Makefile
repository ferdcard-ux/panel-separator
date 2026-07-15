PLUGIN_NAME = custom-separator
PLUGIN_FILE = libcustom-separator.so

CC = gcc
CFLAGS = -Wall -O2 -DGETTEXT_PACKAGE=\"separator\" -DLOCALEDIR=\"/usr/share/locale\" \
         $(shell pkg-config --cflags gtk+-3.0 libxfce4panel-2.0 libxfce4ui-2 libxfce4util-1.0 libxfconf-0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0 libxfce4panel-2.0 libxfce4ui-2 libxfce4util-1.0 libxfconf-0) \
          -shared -fPIC

PLUGIN_DIR = /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins
DESKTOP_DIR = /usr/share/xfce4/panel/plugins

all: $(PLUGIN_FILE)

$(PLUGIN_FILE): separator-plugin.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

install: $(PLUGIN_FILE)
	install -D -m 755 $(PLUGIN_FILE) $(DESTDIR)$(PLUGIN_DIR)/$(PLUGIN_FILE)
	install -D -m 644 debian/custom-separator.desktop $(DESTDIR)$(DESKTOP_DIR)/custom-separator.desktop

uninstall:
	rm -f $(DESTDIR)$(PLUGIN_DIR)/$(PLUGIN_FILE)
	rm -f $(DESTDIR)$(DESKTOP_DIR)/custom-separator.desktop

deb:
	dpkg-buildpackage -us -uc -b -d

clean:
	rm -f $(PLUGIN_FILE)

.PHONY: all install uninstall deb clean
