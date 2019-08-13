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
		empty = 0,
		GLL,
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

		static char buf[10];
		static size_t count;

		static int status = 0;

		//float lat, lng, time;
		static long lat_h = 0, lat_l = 0;
		static long lng_h = 0, lng_l = 0;
		static long time_h, time_l;

		static bool is_valid = false;

		if(status != 0)
			goto loop;

		while(read() != '$');

		if(read() != 'G' || read() != 'P')
			return false;

		status = 1;

loop:
		for(;;){
			for(count=0;;count++){
				int c = read();
				if(c < 0)
					return false;
				Serial.write((char)c);
				//Serial.write(' ');
				if(c == ',' || c == '.' || c == '*'){
					buf[count] = '\0';
					break;
				}
				buf[count] = static_cast<char>(c);
			}

			switch(status){
			case 1:
				if(strcmp(buf, "GLL") == 0){
					status++;
				}
				break;
			case 2:
				if(count != 0){
					lat_h = atol(buf);
					status++;
				}else{
					lat_h = 0;
					status+=2;
				}
				break;
			case 3:
				if(count != 0){
					lat_l = atol(buf);
				}else{
					lat_l = 0;
				}
				status++;
				break;
			case 4:
				if(strcmp(buf, "N") == 0){
					//Serial.println("north");
				}
				status++;
				break;
			case 5:
				if(count != 0){
					lng_h = atol(buf);
					status++;
				}else{
					lng_h = 0;
					status+=2;
				}
				break;
			case 6:
				if(count != 0){
					lng_l = atol(buf);
				}else{
					lng_l = 0;
				}
				status++;
				break;
			case 7:
				if(strcmp(buf, "E") == 0){
					//Serial.println("east");
				}
				status++;
				break;
			case 8:
				if(count != 0){
					time_h = atol(buf);
					status++;
				}else{
					time_h = 0;
					status+=2;
				}
				break;
			case 9:
				if(count != 0){
					time_l = atol(buf);
				}else{
					time_l = 0;
				}
				status++;
				break;
			case 10:
				if(strcmp(buf, "A") == 0){
					Serial.println("valid");
					is_valid = true;
				}else
					is_valid = false;
				status++;
				break;
			case 11:
				if(strcmp(buf, "D") == 0){
					Serial.println("DGPS");
				}
				status++;
				break;
			case 12:
				if(!is_valid){
					status = 0;
					return false;
				}
				Serial.println("");
				Serial.print("lat = ");
				Serial.print(lat_h);
				Serial.print(".");
				Serial.print(lat_l);
				Serial.print(", lng = ");
				Serial.print(lng_h);
				Serial.print(".");
				Serial.print(lng_l);
				Serial.print(", time = ");
				Serial.print(time_h / 10000 + 9);
				Serial.print("h");
				Serial.print((time_h / 100) % 100);
				Serial.print("m");
				Serial.print(time_h % 100);
				Serial.print("s.");
				Serial.println(time_l);

				status = 0;
				return true;
				break;
			}
		}

		return false;
	}

	void parse_data(const char *buf){
		static NMEA type = NMEA::empty;
		if(type == NMEA::empty){
			if(strcmp(buf, "GLL") == 0)
				type = NMEA::GLL;
			return;
		}

		//Serial.print("parse_data: ");
		//Serial.println(buf);

		switch(type){
		case NMEA::GLL:
			if(parse_gll(buf))
				type = NMEA::empty;
			break;
		default:
			Serial.println("error: unknown NMEA type");
			break;
		}
	}

	bool parse_gll(const char *buf){
		// 3539.6473,N,13921.9736,E,092218.600,A,A
		static int lat_int=0, lat_dec=0;
		static int lng_int=0, lng_dec=0;
		static long time_int=0;
		static int time_dec=0;
		static bool north=true, east=true;
		static bool ok=false, dgps=false;
		static uint8_t status = 0;

		switch(status){
		case 0:
			if(buf[0] == '\0'){
				lat_int = 0;
				status++;
			}else
				lat_int = atoi(buf);
			break;
		case 1:
			lat_dec = atoi(buf);
			break;
		case 2:
			if(buf[0] == 'N')
				north = true;
			else
				north = false;
			break;
		case 3:
			if(buf[0] == '\0'){
				lng_int = 0;
				status++;
			}else
				lng_int = atoi(buf);
			break;
		case 4:
			lng_dec = atoi(buf);
			break;
		case 5:
			if(buf[0] == 'E')
				east = true;
			else
				east = false;
			break;
		case 6:
			if(buf[0] == '\0'){
				time_int = 0;
				status++;
			}else{
				if(buf[0] == '0')
					time_int = atol(buf+1);
				else
					time_int = atol(buf);
			}
			break;
		case 7:
			time_dec = atoi(buf);
			break;
		case 8:
			if(buf[0] == 'A')
				ok = true;
			else
				ok = false;
			break;
		case 9:
			if(buf[0] == 'D')
				dgps = true;
			else if(buf[0] == 'A')
				dgps = false;
			else{
				dgps = false;
				ok = false;
			}
			break;
		}
		status++;
		if(status == 10){
			Serial.println("GLL read finished");
//			if(ok){
				Serial.print("lat: ");
				if(north) Serial.print("N");
				else Serial.print("S");
				Serial.print(lat_int);
				Serial.print(".");
				Serial.print(lat_dec);
				Serial.print(", lng: ");
				if(east) Serial.print("E");
				else Serial.print("W");
				Serial.print(lng_int);
				Serial.print(".");
				Serial.print(lng_dec);
				Serial.print(", time: ");
				Serial.print(time_int);
				Serial.print(".");
				Serial.println(time_dec);
				return true;
			}
//		}

		if(status >= 10)
			status = 0;
		return false;
	}

	bool parse8(const char c){
		static char buf[10];
		static uint8_t header_count = 0;
		static uint8_t count = 0;

		switch(c){
		case '$':		// start
			header_count = 0;
			count = 0;
			break;
		case 'G':
			if(header_count == 0)
				header_count++;
			else{
				buf[count] = c;
				count++;
			}
			break;
		case 'P':
			if(header_count == 1)
				header_count++;
			else{
				buf[count] = c;
				count++;
			}
			break;
		case ',':
		case '.':
		case '*':
			buf[count] = '\0';
			count = 0;
			parse_data(buf);
			break;
		case '\n':
			return true;
			break;
		default:
			buf[count] = c;
			count++;
			break;
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
