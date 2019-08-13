#ifndef GPS_H_
#define GPS_H_

#ifndef GPS_USE_HARDWARE_SERIAL
	#include <SoftwareSerial.h>
	#define GPS_USE_SOFTWARE_SERIAL
	#warning SoftwareSerial does not work after change baudrate. please use default baudrate(9600)
#endif

// NMEA documentation: https://www.sparkfun.com/datasheets/GPS/Modules/PMTK_Protocol.pdf

class GPS {
public:
	static constexpr size_t default_brate = 9600;

#ifdef GPS_USE_SOFTWARE_SERIAL
	explicit GPS(const uint8_t rx, const uint8_t tx, const size_t brate=default_brate) : serial(nullptr), rx(rx), tx(tx), brate(brate) {}
#else
	explicit GPS(const size_t brate=default_brate) : serial(nullptr), brate(brate) {}
#endif

private:
#ifdef GPS_USE_SOFTWARE_SERIAL
	SoftwareSerial *serial;
	const uint8_t rx, tx;
#else
	HardwareSerial *serial;
#endif

	const size_t brate;

	uint8_t nmea_setting[17];

public:
	enum class NMEA : uint8_t {
		GLL	= 0,
		RMC,
		VTG,
		GGA,
		GSA,
		GSV,
		GRS,
		GST,
		MALM,
		MEPH,
		MDGP,
		MDBG,
	};

	// 初期化
	void init(){
		#ifdef GPS_USE_HARDWARE_SERIAL
			serial = &Serial;
		#else
			serial = new SoftwareSerial(rx, tx);
		#endif

		serial->begin(default_brate);

		// default_brateで使う
		if(brate == default_brate)
			return;

		// ボードレート変更コマンド送信
		change_brate(brate);

		// 新しいボードレートでアクセス
		serial->end();
		serial->begin(brate);
	}

	bool parse(){
		// $GPGLL,3539.6473,N,13921.9736,E,092218.600,A,A*56

		char buf[10];
		size_t count;

		int status = 0;

		//float lat, lng, time;
		long lat_h = 0, lat_l = 0;
		long lng_h = 0, lng_l = 0;
		long time_h, time_l;

		while(read() != '$');

		if(read() != 'G' || read() != 'P')
			return false;

		for(;;){
			for(count=0;;count++){
				int c = read();
				if(c < 0)
					return false;
				//Serial.write((char)c);
				//Serial.write(' ');
				if(c == ',' || c == '.' || c == '*'){
					buf[count] = '\0';
					break;
				}
				buf[count] = static_cast<char>(c);
			}

			switch(status){
			case 0:
				if(strcmp(buf, "GLL") == 0){
					status++;
				}
				break;
			case 1:
				if(count != 0){
					lat_h = atol(buf);
					status++;
				}else{
					lat_h = 0;
					status+=2;
				}
				break;
			case 2:
				if(count != 0){
					lat_l = atol(buf);
				}else{
					lat_l = 0;
				}
				status++;
				break;
			case 3:
				if(strcmp(buf, "N") == 0){
					Serial.println("north");
				}
				status++;
				break;
			case 4:
				if(count != 0){
					lng_h = atol(buf);
					status++;
				}else{
					lng_h = 0;
					status+=2;
				}
				break;
			case 5:
				if(count != 0){
					lng_l = atol(buf);
				}else{
					lng_l = 0;
				}
				status++;
				break;
			case 6:
				if(strcmp(buf, "E") == 0){
					Serial.println("east");
				}
				status++;
				break;
			case 7:
				if(count != 0){
					time_h = atol(buf);
					status++;
				}else{
					time_h = 0;
					status+=2;
				}
				break;
			case 8:
				if(count != 0){
					time_l = atol(buf);
				}else{
					time_l = 0;
				}
				status++;
				break;
			case 9:
				if(strcmp(buf, "A") == 0){
					Serial.println("valid");
				}
				status++;
				break;
			case 10:
				if(strcmp(buf, "D") == 0){
					Serial.println("DGPS");
				}
				status++;
				break;
			case 11:
				Serial.print("lat = ");
				Serial.print(lat_h);
				Serial.print(".");
				Serial.println(lat_l);
				Serial.print("lng = ");
				Serial.print(lng_h);
				Serial.print(".");
				Serial.println(lng_l);
				Serial.print("time = ");
				Serial.print(time_h);
				Serial.print(".");
				Serial.println(time_l);
				return true;
				break;
			}
		}

		return false;
	}

	void hot_start(){		send_cmd("101"); }
	void warm_start(){		send_cmd("102"); }
	void cold_start(){		send_cmd("103"); }
	void full_cold_start(){	send_cmd("104"); }

	void set_interval(const size_t ms){
		char cmd[20];
		sprintf(cmd, "220,%u", ms);
		send_cmd(cmd);
	}

	inline auto read() const -> int {
		#ifdef GPS_USE_SOFTWARE_SERIAL
			serial->listen();
		#endif
		return serial->read();
	}
	inline auto available() const -> int {
		return serial->available();
	}
//private:
	// ボードレート変更コマンドを送信する
	void change_brate(const size_t &brate){
		char cmd[20];
		sprintf(cmd, "251,%u", brate);
		send_cmd(cmd);
	}

	// コマンドを送信する
	void send_cmd(const char *cmd, decltype(serial) s=nullptr){
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
	}
};

#endif
