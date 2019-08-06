LDFLAGS	= -lwiringPi

recv: recv.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f recv
	rm -f *.o
