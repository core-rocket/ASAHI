#ifndef GPS_H_
#define GPS_H_

#include <SoftwareSerial.h>

class GPS {
public:
	static constexpr size_t default_brate = 9600;

	explicit GPS(const uint8_t rx, const uint8_t tx) : rx(rx), tx(tx), brate(default_brate), serial(nullptr) {}

	void init(const size_t brate=default_brate){
		serial = new SoftwareSerial(rx, tx);
		// brate setting
		if(brate != default_brate)
			change_brate(brate);
		else
			serial->begin(brate);
	}

	int read() const {
		serial->listen();
		return serial->read();
	}
private:
	void send_cmd(const char *cmd, SoftwareSerial *s=nullptr){
		uint8_t checksum = 'P' ^ 'M' ^ 'T' ^ 'K';
		for(size_t i=0;;i++){
			if(cmd[i] == '\0') break;
			checksum = checksum ^ cmd[i];
		}
		char buf[100];
		sprintf(buf, "$PMTK%s*%X\r\n", cmd, checksum);
		if(s == nullptr)
			s = this->serial;
		s->write(buf);
		Serial.write(buf);
	}

	// SoftwareSerial.begin()を複数回呼べないので初期化時にのみ呼ぶべき
	void change_brate(const size_t &brate){
		SoftwareSerial tmpserial(rx, tx);
		tmpserial.begin(this->brate);
		tmpserial.listen();

		char cmd[10];
		sprintf(cmd, "251,%u", brate);
		send_cmd(cmd, &tmpserial);

		delay(100);

		serial->begin(brate);
		serial->listen();
		this->brate = brate;
	}

	const uint8_t rx, tx;
	size_t brate;
	SoftwareSerial *serial;
};

#endif
