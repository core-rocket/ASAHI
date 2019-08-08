LDFLAGS	= -lwiringPi

send: send.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f send
	rm -f *.o
