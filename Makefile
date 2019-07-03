CLEANEXTS   = o so d
CXX = g++
LDFLAGS =
CPPFLAGS =

SRCDIR = src
SOURCES     = $(SRCDIR)/Stroke.cpp $(SRCDIR)/TactileBrush.cpp
HEADERS 		= $(SRCDIR)/Stroke.h $(SRCDIR)/TactileBrush.h
OUTPUTFILE  = libtactilebrush.so
INSTALLPREFIX = /usr
LIBDIR  = lib
INCDIR = include

.PHONY: all install clean

all: $(OUTPUTFILE)

$(OUTPUTFILE): $(subst .cpp,.o,$(SOURCES))
	$(CXX) -shared -fPIC $(LDFLAGS) -o $@ $^

install:
	mkdir -p $(LIBDIR) $(INCDIR)
	install -m 644 -o root -g root $(OUTPUTFILE) $(INSTALLPREFIX)/$(LIBDIR)
	install -m 644 -o root -g root $(HEADERS) $(INSTALLPREFIX)/$(INCDIR)

clean:
	for file in $(CLEANEXTS); do rm -f *.$$file $(SRCDIR)/*.$$file; done

include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	$(CXX) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
rm -f $@.$$$$
