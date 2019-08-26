LDFLAGS	= -lwiringPi

recv: recv.o
	$(CXX) -o $@ $^ $(LDFLAGS)

run:
	make
	./recv

clean:
	rm -f recv
	rm -f *.o
