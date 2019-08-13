TARGET	= station
OBJS	= main.o twelite.o

CC		= gcc
CXX		= g++

LDFLAGS	= -lpthread

%.o: %.c
	$(CC) -c -o $@ $<

%.o: %.cc
	$(CXX) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

run:
	make
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o

clean_all:
	make clean
	rm -f log/*
