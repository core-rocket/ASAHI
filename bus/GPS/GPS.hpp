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

	uint64_t nmea_setting = 0;

	uint8_t read_num = 0;

public:
	bool north, east;
	struct float_t {
		uint32_t int_part;
		uint16_t dec_part;
	};

	struct data_t {
		bool valid;
		bool dgps;
		float_t time;
		float_t latitude, longitude;
		bool is_meter, is_meter_geo;
		float_t altitude, altitude_geo;
	} data;

	enum NMEA {
		GLL = 0b000000000001,
		RMC = 0b000000000010,
		VTG = 0b000000000100,
		GGA = 0b000000001000,
		GSA = 0b000000010000,
		GSV = 0b000000100000,
		GRS = 0b000001000000,
		GST = 0b000010000000,
		MALM= 0b000100000000,
		MEPH= 0b001000000000,
		MDGP= 0b010000000000,
		MDBG= 0b100000000000,
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

	void set_output(int nmea, const uint8_t freq=1, const bool send_flag=true){
		if(send_flag){
			char buf[3 + 2*17 + 1];
			buf[0] = '3';
			buf[1] = '1';
			buf[2] = '4';
			for(size_t i=0;i<17;i++){
				buf[3+i*2] = ',';
				if(nmea & (1 << i))
					buf[3+i*2+1] = '0' + freq;
				else
					buf[3+i*2+1] = '0';
			}
			buf[3+2*17] = '\0';
			Serial.println(buf);
			send_cmd(buf);
		}
	}

	bool parse(){
		while(true){
			int c = read();
			if(c<0) break;
			//Serial.print((char)c);
			if(parse8(static_cast<char>(c)))
				return true;
		}
		return false;
	}

	void parse_data(const char *buf){
		static NMEA type;

		#define CHECK_TYPE(t) if(strcmp(buf, #t) == 0){type=t;return;}

		if(read_num == 0){
			if(strcmp(buf, "GLL") == 0){
				type = GLL;
				return;
			}
			CHECK_TYPE(RMC);
			CHECK_TYPE(VTG);
			CHECK_TYPE(GGA);
			CHECK_TYPE(GSA);
			CHECK_TYPE(GSV);
			CHECK_TYPE(GRS);
			CHECK_TYPE(GST);
			CHECK_TYPE(MALM);
			CHECK_TYPE(MEPH);
			CHECK_TYPE(MDGP);
			CHECK_TYPE(MDBG);
			return;
		}
		#undef CHECK_TYPE

		//Serial.print("parse_data(");
		//Serial.print(read_num);
		//Serial.print("): ");
		//Serial.println(buf);

		if(buf[0] == '\0'){
			return;
		}

		#define CASE(t) \
		case t: \
			if(parse_ ## t (buf)) \
				{read_num = 0;} \
			break;

		switch(type){
		CASE(GLL);
		CASE(GGA);
		default:
			//Serial.println("error: unknown NMEA type");
			break;
		}

		#undef CASE
	}

	bool parse_GLL(const char *buf){
		// 3539.6473,N,13921.9736,E,092218.600,A,A
		static bool ok=false, dgps=false;

		//Serial.print("read_num = ");
		//Serial.println(read_num);
		switch(read_num){
		case 1:
			read_float(buf, data.latitude);
			break;
		case 2:
			north = (buf[0] == 'N');
			break;
		case 3:
			read_float(buf, data.longitude);
			break;
		case 4:
			east = (buf[0] == 'E');
			break;
		case 5:
			read_float(buf, data.time);
			break;
		case 6:
			data.valid = (buf[0] == 'A');
			break;
		case 7:
			if(buf[0] == 'D')
				dgps = true;
			else if(buf[0] == 'A')
				dgps = false;
			else{
				dgps = false;
				data.valid = false;
			}
			break;
		}
		if(read_num >= 7){
			if(ok){
				read_num = 0;
				return true;
			}
		}

		return false;
	}

	bool parse_GGA(const char *buf){
		switch(read_num){
			case 1:
				read_float(buf, data.time);
				break;
			case 2:
				read_float(buf, data.latitude);
				break;
			case 3:
				north = (buf[0] == 'N');
				break;
			case 4:
				read_float(buf, data.longitude);
				break;
			case 5:
				east = (buf[0] == 'E');
				break;
			case 6:
				data.valid = (buf[0] != '0');
				data.dgps  = (buf[0] == '2');
				break;
			case 7:
				//Serial.print("sat num: ");
				//Serial.println(buf);
				break;
			case 8:
				// DOP
				//Serial.print("DOP: ");
				//Serial.println(buf);
				break;
			case 9:
				//Serial.print("altitude: ");
				//Serial.println(buf);
				read_float(buf, data.altitude);
				break;
			case 10:
				// unit
				//Serial.print("unit: ");
				//Serial.println(buf);
				data.is_meter = (buf[0] == 'M');
				break;
			case 11:
				//Serial.print("altitude2: ");
				//Serial.println(buf);
				read_float(buf, data.altitude_geo);
				break;
			case 12:
				//Serial.print("unit2: ");
				//Serial.println(buf);
				data.is_meter = (buf[0] == 'M');
				break;
			case 13:
				//Serial.print("DGPS_time: ");
				//Serial.println(buf);
				break;
			case 14:
				//Serial.print("DGPS id: ");
				//Serial.println(buf);
				break;
		}

		if(read_num >= 14){
			read_num = 0;
		}

		return false;
	}

	void read_float(const char *buf, float_t &f){
		unsigned long tmp;
		size_t start = 0;
		size_t end = 0;
		if(buf[0] == '0' && buf[1] != '.')
			start++;
		for(end=start;;end++){
			if(*(buf+end) == '.') break;
			if(*(buf+end) == '\0') return;
		}
		end--;
		const char *endp = (buf+end);
		tmp = strtoul(buf+start, const_cast<char**>(&endp), 10);
		if(tmp > static_cast<unsigned long>(UINT32_MAX))
			f.int_part = 0;
		else
			f.int_part = static_cast<uint32_t>(tmp);
		end += 2;
		tmp = strtoul(buf+end, nullptr, 10);
		if(tmp > static_cast<unsigned long>(UINT16_MAX))
			f.dec_part = 0;
		else
			f.dec_part = static_cast<uint16_t>(tmp);
	}

	bool parse8(const char c){
		static char buf[15];
		static uint8_t header_count = 0;
		static uint8_t count = 0;

		switch(c){
		case '$':		// start
			header_count = 0;
			count = 0;
			read_num = 0;
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
		case '*':
			buf[count] = '\0';
			count = 0;
			parse_data(buf);
			read_num++;
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
