TARGET	= station
OBJS	= main.o

CC		= gcc
CXX		= g++

LDFLAGS	= -lwiringPi

%.o: %.c
	$(CC) -c -o $@ $<

%.o: %.cc
	$(CXX) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

run:
	make
	./$(TARGET)
