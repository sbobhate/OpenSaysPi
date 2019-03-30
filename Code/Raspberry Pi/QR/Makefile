CXX ?= g++

CXXFLAGS += -c -Wall $(shell pkg-config --cflags opencv)
LDFLAGS += $(shell pkg-config --libs --static opencv) -lzbar

all: QR

QR: qr.o; $(CXX) $< -o $@ $(LDFLAGS)

%.o: %.cpp; $(CXX) $< -o $@ $(CXXFLAGS)

clean: ; rm -f qr.o QR
