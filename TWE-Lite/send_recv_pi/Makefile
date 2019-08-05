LDFLAGS	= -lwiringPi

send_recv: send_recv.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f send_recv
	rm -f *.o
