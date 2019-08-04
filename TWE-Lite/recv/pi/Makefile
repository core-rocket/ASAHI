TARGET	= recv
OBJS	= recv.o

LDFLAGS = -lwiringPi

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
