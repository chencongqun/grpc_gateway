OUTFILE = grpc_gateway
SRC_C = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC_C))
CFLAGS = -D_GNU_SOURCE -I. -I/usr/local/include/ -g
LIBRARY = -ljson -lfcgi -lcurl -ldl
CC = gcc


# Pattern rules
%.o : %.c
	$(CC) -fPIC $(CFLAGS) -c -o $@ $<

# Build rules
all: $(OUTFILE) 

$(OUTFILE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBRARY) -o $@

# Clean this project
clean:
	rm -f $(OUTFILE)
	rm -f $(OBJS)
