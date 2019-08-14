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
		float_t time;
		float_t latitude, longitude;
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

	void set_output(NMEA nmea, const uint8_t freq=1, const bool send_flag=true){
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
		if(read_num == 0){
			if(strcmp(buf, "GLL") == 0)
				type = NMEA::GLL;
			return;
		}

		//Serial.print("parse_data: ");
		//Serial.println(buf);

		switch(type){
		case NMEA::GLL:
			if(parse_gll(buf))
				read_num = 0;
			break;
		default:
			//Serial.println("error: unknown NMEA type");
			break;
		}
	}

	bool parse_gll(const char *buf){
		// 3539.6473,N,13921.9736,E,092218.600,A,A
		static bool ok=false, dgps=false;

		auto& lat = data.latitude;
		auto& lng = data.longitude;
		auto& time= data.time;

		switch(read_num){
		case 1:
			if(buf[0] == '\0'){
				lat.int_part = lat.dec_part = 0;
				read_num++;
			}else{
				int tmp = atoi(buf);
				if(tmp < 0) lat.int_part = 0;
				else lat.int_part = static_cast<uint32_t>(tmp);
			}
			break;
		case 2:
			{
				int tmp = atoi(buf);
				if(tmp < 0 || static_cast<unsigned int>(tmp) > static_cast<unsigned int>(UINT16_MAX)) lat.dec_part = 0;
				else lat.dec_part = static_cast<uint16_t>(tmp);
			}
			break;
		case 3:
			if(buf[0] == 'N')
				north = true;
			else
				north = false;
			break;
		case 4:
			if(buf[0] == '\0'){
				lng.int_part = lng.dec_part = 0;
				read_num++;
			}else{
				int tmp = atoi(buf);
				if(tmp < 0) lng.int_part = 0;
				else lng.int_part = static_cast<uint32_t>(tmp);
			}
			break;
		case 5:
			{
				int tmp = atoi(buf);
				if(tmp < 0 || static_cast<unsigned int>(tmp) > static_cast<unsigned int>(UINT16_MAX)) lng.dec_part = 0;
				else lng.dec_part = static_cast<uint16_t>(tmp);
			}
			break;
		case 6:
			if(buf[0] == 'E')
				east = true;
			else
				east = false;
			break;
		case 7:
			if(buf[0] == '\0'){
				time.int_part = time.dec_part = 0;
				read_num++;
			}else{
				long tmp;
				if(buf[0] == '0')
					tmp = atol(buf+1);
				else
					tmp = atol(buf);

				unsigned long t2 = tmp;

				if(tmp < 0 || t2 > static_cast<unsigned long>(UINT32_MAX)) time.int_part = 0;
				else time.int_part = static_cast<uint32_t>(t2);
			}
			break;
		case 8:
			{
				int tmp = atoi(buf);
				if(tmp < 0 || static_cast<unsigned int>(tmp) > static_cast<unsigned int>(UINT16_MAX)) time.dec_part = 0;
				else time.dec_part = static_cast<uint16_t>(tmp);
			}
			break;
		case 9:
			data.valid = (buf[0] == 'A');
			break;
		case 10:
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
		if(read_num >= 10){
			if(ok){
				read_num = 0;
				return true;
			}
		}

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
		case '.':
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
