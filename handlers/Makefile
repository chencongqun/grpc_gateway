OUTLIB = files.so
SRC_C = $(wildcard *.cc)
OBJS = $(patsubst %.cc,%.o,$(SRC_C))
CFLAGS = -g -I/usr/local/include -pthread -I../
LIBRARY =  -L/usr/local/lib -lgrpc++ -lgrpc -lcurl -lgrpc++_reflection -lprotobuf -lpthread -ldl

# Pattern rules
%.o : %.cc
	g++ -fPIC -std=c++0x $(CFLAGS) -c -o $@ $<

# Build rules
$(OUTLIB): $(OBJS)
	g++ -shared -o $@ $^ $(LIBRARY)
# Clean this project
clean:
	rm -f $(OUTLIB)
	rm -f $(OBJS)
