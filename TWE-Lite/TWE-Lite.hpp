#ifndef TWE_LITE_H_
#define TWE_LITE_H_

#ifdef ARDUINO
	// Arduino
	#ifndef TWE_LITE_USE_HARDWARE_SERIAL
		#include <SoftwareSerial.h>
	#endif
#else
	// others
	#include <cstdio>
	#include <cstring>
	#include <string>
	using std::strlen;
	#ifdef _WIN32
		#error fuck Windows
	#elif defined(RASPBERRY_PI)
		// Raspberry Pi
		#include <wiringPi.h>
		#include <wiringSerial.h>
	#else
		#define NO_MILLIS
		// Linux
		#include <chrono>
		#include <unistd.h>
		#include <fcntl.h>
		#include <termios.h>
	#endif
#endif

class TWE_Lite {
public:
	// コンパイル時定数
	constexpr static long int default_brate		= 115200;
	constexpr static uint8_t  default_buf_size	= 30;
	constexpr static uint16_t MSB				= 0x8000;

	// コンストラクタ
#ifdef ARDUINO
	explicit TWE_Lite(const uint8_t rx, const uint8_t tx, const long int brate=default_brate) : rx(rx), tx(tx), brate(brate) {}
#else
	explicit TWE_Lite(const std::string &devfile, const long int &brate) : devfile(devfile), brate(brate) {}
#endif

	// デストラクタ
	~TWE_Lite(){
		#ifdef ARDUINO
			// 特にやることなし(そもそもデストラクタが呼ばれるべきではない)
		#elif defined(RASPBERRY_PI)
			if(fd != 0){
				serialFlush(fd);
				serialClose(fd);
			}
		#else
			close(fd);
		#endif
	}

	// 定数
#ifdef ARDUINO
	const uint8_t rx, tx;
#else
	const std::string devfile;
#endif
	const long int brate;

protected:
	// 内部送信バッファ
//	uint8_t send_buf_internal[default_buf_size];
public:
	uint8_t recv_buf[default_buf_size];

public:
	// 初期化
	bool init(){
		// プラットフォーム依存の初期化
		#ifdef RASPBERRY_PI
			fd = serialOpen(devfile.c_str(), brate);
			if(fd < 0) return false;
		#elif defined(ARDUINO)
			#ifdef TWE_LITE_USE_HARDWARE_SERIAL
				serial = &Serial;
			#else
				serial = new SoftwareSerial(rx, tx);
			#endif
			serial->begin(brate);
		#else
			fd = open(devfile.c_str(), O_RDWR | O_NOCTTY | O_SYNC /*O_NDELAY*/ /*| O_NONBLOCK*/);
			termios setting;
			memset(&setting, 0, sizeof(setting));

			cfsetispeed(&setting, B115200);
			cfsetospeed(&setting, B115200);

			setting.c_cflag = (setting.c_cflag & ~CSIZE) | CS8;
			setting.c_cflag |= (/*CRTSCTS |*/ CLOCAL | CREAD);
			setting.c_iflag = 0; //IGNPAR;
			setting.c_oflag = 0;
			setting.c_lflag = 0;		// non-canonical, no-echo
			setting.c_cc[VTIME]	= 0;	// no block
			setting.c_cc[VMIN]	= 1;	// 0.1 sec read timeout

			tcflush(fd, TCIOFLUSH);
			tcsetattr(fd, TCSANOW, &setting);
		#endif
		return true;
	}

	// シリアル通信の抽象化
	inline void swrite8(const uint8_t &val) const {
#ifdef ARDUINO
		serial->write(val);
#elif defined(RASPBERRY_PI)
		serialPutchar(fd, val);
		//std::cout << std::hex << (int)val;
#else
		write(fd, &val, 1);
		//std::cout << " " << std::hex << (int)val;
#endif
	}
	inline void swrite16_big(const uint16_t &val) const {
		swrite8(static_cast<uint8_t>(val >> 8));
		swrite8(static_cast<uint8_t>(val & 0xff));
	}
	inline void swrite(const uint8_t *buf, const size_t &size) const {
#ifdef ARDUINO
		serial->write(buf, size);
#else
		for(size_t i=0;i<size;i++)
			swrite8(buf[i]);
#endif
	}

	inline uint8_t sread8(){
#ifdef ARDUINO
		int c = serial->read();
		if(c < 0){
			sread_error = true;
			return 0x00;
		}else
			sread_error = false;
		uint8_t b = static_cast<uint8_t>(c);
//		if(b == 0xA5)
//			Serial.println("");
//		Serial.print(b, HEX);
//		Serial.write(" ");
		return b;
#elif defined(RASPBERRY_PI)
		return serialGetchar(fd);
#else
		char c;
		if(read(fd, &c, 1) == 1){
			sread_error = false;
			return c;
		}else{
			sread_error = true;
			return '\0';
		}
#endif
	}

	inline int savail() const {
#ifdef ARDUINO
		return serial->available();
#elif defined(RASPBERRY_PI)
		return serialDataAvail(fd);
#else
		return 1;
#endif
	}

	template<typename T>
	void send_simple(const uint8_t id, const uint8_t type, const T &data){
		send_buf_simple(id, type, &data, sizeof(T));
	};

	void send_simple(const uint8_t id, const uint8_t type, const char *str){
		send_buf_simple(id, type, str, strlen(str));
	}

	template<typename T>
	void send_extend(const uint8_t id, const uint8_t response_id, const T &data){
		send_buf_extend(id, response_id, &data, sizeof(T));
	};

	void send_extend(const uint8_t id, const uint8_t response_id, const char *str){
		send_buf_extend(id, response_id, str, strlen(str));
	}

	// 任意のバッファを簡易形式で送信する(typeは0x80未満任意)
	void send_buf_simple(const uint8_t id, const uint8_t type, const void *buf, const size_t &size) const {
		uint8_t header[] = { id, type };
		do_send(header, 2, static_cast<const uint8_t*>(buf), size);
	}

	// 任意のバッファを拡張形式で送信する(拡張アドレス,オプション列はとりあえず無視)
	void send_buf_extend(const uint8_t id, const uint8_t response_id, const void *buf, const size_t &size) const {
		uint8_t header[] = { id, 0xA0, response_id, 0xFF };
		do_send(header, 4, static_cast<const uint8_t*>(buf), size);
	}

	// 送信コマンドにヘッダとチェックサムをつけて送信する(とりあえずバイナリ形式のみ)
	inline void do_send(const uint8_t *header, const size_t &header_size, const uint8_t *buf, const size_t &size) const {
		// チェックサム計算
		uint8_t checksum = 0x00;
		for(size_t i=0;i<header_size;i++)
			checksum = checksum ^ header[i];
		for(size_t i=0;i<size;i++)
			checksum = checksum ^ buf[i];

		// 送信コマンドサイズ
		uint16_t cmd_size = static_cast<uint16_t>(header_size) + static_cast<uint16_t>(size);

		// 送信
		swrite8(0xA5);	// バイナリ形式ヘッダ
		swrite8(0x5A);
		swrite16_big(MSB + cmd_size);
		swrite(header, header_size);
		swrite(buf, size);
		swrite8(checksum);
	}

	// 応答メッセージのチェック
	inline int check_send(){
		if(recv(100) != 1)
			return -1;

		if(!is_response())			// 応答メッセージか？
			return -1;

		if(recv_buf[0] == 0x01)		// 送信成功
			return 1;

		return 0;
	}

	template<typename T>
	const T* get_data(const size_t start=0) const {
		if((parser.get_length() - start) != sizeof(T))
			return nullptr;
		return reinterpret_cast<const T*>(static_cast<const uint8_t*>(recv_buf) + start);
	}

	template<typename T>
	const size_t recv(T &data, const size_t timeout=100){
		parser.set_buf(reinterpret_cast<uint8_t*>(&data), sizeof(T));
		if(try_recv(timeout))
			return parser.get_length();
		return 0;
	}

	// 受信する(成功時受信バイト数を返す)
	const size_t recv(const size_t timeout=100){
		#ifdef ARDUINO
			#ifndef TWE_LITE_USE_HARDWARE_SERIAL
			serial->listen();
			#endif
		#endif

		parser.set_buf(recv_buf, default_buf_size);

		if(try_recv(timeout))
			return parser.get_length();
		return 0;
	}

	inline const bool is_simple()  const { return parser.is_simple();}
	inline const bool is_extended()const { return parser.is_extended();}
	inline const bool is_response()const { return parser.is_response();}

	inline const uint8_t from_id()    const { return parser.get_from_id(); }
	inline const uint8_t cmd_type()   const { return parser.get_cmd_type();}
	inline const uint8_t response_id()const { return parser.get_response_id();}
	inline const uint8_t LQI()        const { return parser.get_LQI(); }

	inline const uint16_t get_length() const { return parser.get_length(); }

	inline const uint32_t from_ext_addr() const { return parser.get_from_ext_addr();}
	inline const uint32_t my_ext_addr()   const { return parser.get_my_ext_addr();}

	inline void clear_buf() { parser.clear_buf(); }

	// 受信成功時trueを返す
	// 受信失敗, タイムアウト時falseを返す
	inline auto try_recv(const size_t &timeout) -> bool {
		const auto start =
			#ifdef NO_MILLIS
				std::chrono::system_clock::now();
			#else
				millis();
			#endif
		while(true){
			//if(savail() == 0) break;
			const uint8_t b = sread8();
			if(sread_error) break;;
			if(parser.parse8(b))
				return true;

			#ifdef NO_MILLIS
				const auto end = std::chrono::system_clock::now();
				const double e = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
				const size_t elapsed = static_cast<size_t>(e);
			#else
				const auto elapsed = millis() - start;
			#endif
			if(elapsed >= timeout)
				break;
		}
		return false;
	}

	// 1byte受け取って受信成功したらtrueを返す
	inline auto try_recv8() -> bool {
		uint8_t b = sread8();
		if(sread_error) return false;
		return parser.parse8(b);
	}

	// 受信データのパーサ(1byteずつパースする)
	// parse8()がtrueを返したらパース完了(checksumも合っている)
	class Parser {
	public:
		explicit Parser() : s(state::empty), cmd_length(0x0000), checksum(0x00), payload(nullptr) {}

		// 送信コマンドの処理状態
		enum class cmd : uint8_t {
			from_id,		// 送信元論理ID
			flag,			// 0x00~0x80なら簡易形式(コマンド種別), 0xA0なら拡張形式
			payload,		// 実際の受信データ
			// 以下は拡張形式のみ
			response_id,	// 応答ID
			addr,			// 送信元・先拡張アドレス
			LQI,			// 通信品質
			length,			// データ領域長
		};

		// バイナリ形式の処理状態
		enum class state : uint8_t {
			empty,			// 受信待ち
			header,			// ヘッダ(0xA5, 0x5A)
			length,			// 送信コマンド長(上位8bit)
			length2,		// 送信コマンド長(下位8bit)
			cmd,			// 送信コマンド
			checksum,		// checksum
		};

		inline auto get_state() const -> state { return s; }
		inline auto get_error() const -> state { return e; }

		inline auto is_simple() const -> bool { return flag_simple; }
		inline auto is_extended() const -> bool { return !flag_simple; }
		inline auto is_response() const -> bool { return flag_response; }

		inline auto get_cmd_length() const -> const uint16_t { return cmd_length; }
		inline auto get_length() const -> uint16_t { return payload_length; }
		inline auto get_from_id() const -> uint8_t { return from_id; }
		inline auto get_cmd_type() const -> uint8_t{ return cmd_type;}
		inline auto get_response_id() const -> uint8_t { return response_id; }
		inline auto get_from_ext_addr() const -> uint32_t { return from_ext_addr; }
		inline auto get_my_ext_addr() const -> uint32_t { return to_ext_addr; }
		inline auto get_LQI() const -> uint8_t { return LQI; }

		inline void set_buf(uint8_t *buf, const size_t &size){
			payload = buf;
			payload_bufsize = size;
		}

		inline void clear_buf(){
			payload_length = 0;
		}

		// 1byteずつパースする
		bool parse8(const uint8_t &b){
			return parse8_binary(b);
		}

		// 送信コマンドのパース
		inline void parse_cmd(const uint8_t &b){
			static uint8_t addr_pos = 0;
			static bool length_pos = 0;
			static uint8_t payload_pos = 0;

			switch(c){
			case cmd::from_id:
				payload_pos = 0;
				from_id = b;
				c = cmd::flag;
				break;
			case cmd::flag:
				flag_response = false;
				if(b == 0xA0){				// 拡張形式
					flag_simple = false;
					from_ext_addr	= 0x00;
					to_ext_addr		= 0x00;
					addr_pos		= 0x00;
					c = cmd::response_id;
				}else if(b == 0xA1){		// 応答メッセージ
					flag_response = true;
					payload_length = 1;
					c = cmd::response_id;
				}else{						// 簡易形式
					flag_simple = true;
					cmd_type = b;			// コマンド種別
					payload_length = cmd_length - 2;
					c = cmd::payload;
				}
				break;
			case cmd::payload:
				//std::cout
				//	<< "payload[" << std::dec << (uint32_t) payload_pos << "] = "
				//	<< std::hex << (uint32_t)b << std::endl;
				// Serial.print("payload[");
				// Serial.print(payload_pos, DEC);
				// Serial.print("] = ");
				// Serial.println(b, HEX);

				if(payload != nullptr && payload_pos < payload_bufsize)
					payload[payload_pos] = b;
				payload_pos++;
				break;
			case cmd::response_id:
				response_id = b;
				c = cmd::addr;
				if(flag_response)	// 応答メッセージ
					c = cmd::payload;
				break;
			case cmd::addr:
				// 0 1 2 3  4 5 6 7
				// 3 2 1 0
				// 24 16 8 0
				if(addr_pos < 4){	// 送信元拡張アドレス
					from_ext_addr += b << ((3-addr_pos)*8);
				}else{				// 送信先拡張アドレス
					to_ext_addr += b << ((7-addr_pos)*8);
				}
				if(addr_pos == 7){
					addr_pos = 0;
					c = cmd::LQI;
				}else addr_pos++;
				break;
			case cmd::LQI:
				LQI = b;
				//std::cout << "LQI = " << std::dec << (uint32_t)LQI << std::endl;
				c = cmd::length;
				break;
			case cmd::length:
				if(!length_pos) payload_length = b << 8;
				else{
					payload_length += b;
					//std::cout << "length = " << std::dec << length << std::endl;
					c = cmd::payload;
				}
				length_pos = !length_pos;
				break;
			}
		}

		// バイナリ形式のパース
		inline bool parse8_binary(const uint8_t &b){
			switch(s){
			case state::empty:
				e = state::empty;
				cmd_pos = 0;
				if(b == 0xA5)
					s = state::header;
				break;
			case state::header:
				if(b == 0x5A)
					s = state::length;
				else
					error();
				break;
			case state::length:
				if(b & 0x80){
					cmd_length = (b & 0x7F) << 8;
					s = state::length2;
				}else{
					// short length mode(?)
					error();
				}
				break;
			case state::length2:
				cmd_length += b;
				if(cmd_length <= default_buf_size){
					s = state::cmd;
					c = cmd::from_id;
				}else
					error();
				break;
			case state::cmd:
				parse_cmd(b);
				checksum ^= b;
				if(cmd_pos == cmd_length-1)
					s = state::checksum;
				cmd_pos++;
				break;
			case state::checksum:
				if(b == checksum){
					s = state::empty;
					cmd_pos = 0;
					checksum = 0x00;
					// Serial.println("checksum ok!!!!!!!!!!!");
					return true;
				}else{
					// Serial.println("fuck!!!!!!!!!!!!!!!!!!!!!!!!!!");
					error();
				}
				break;
			}
			return false;
		}
	private:
		cmd c;
		state s, e;
		uint16_t cmd_length; // 送信コマンドの長さ
		uint8_t  checksum;
		uint16_t cmd_pos;

		bool     flag_simple;		// 簡易形式かどうか
		bool     flag_response;		// 応答メッセージかどうか
		uint8_t  from_id;			// 送信元論理ID
		uint8_t  cmd_type;			// コマンド種別(簡易形式のみ)
		uint8_t  response_id;		// 送信元で指定した応答ID(拡張形式のみ)
		uint32_t from_ext_addr;		// 送信元拡張アドレス(拡張形式のみ)
		uint32_t to_ext_addr;		// 送信先拡張アドレス(拡張形式のみ)．論理IDで指定時は0xFFFFFFFF
		uint8_t	 LQI;				// 通信品質
		uint16_t payload_length;	// 実際の受信データ長

		uint8_t *payload;
		size_t payload_bufsize = 0;

		inline void error(){
			// Serial.println("error");
			e = s;
			s = state::empty;
		}
	} parser;

private:
#ifdef ARDUINO
	#ifdef TWE_LITE_USE_HARDWARE_SERIAL
	HardwareSerial *serial = nullptr;
	#else
	SoftwareSerial *serial = nullptr;
	#endif
//#elif defined(RASPBERRY_PI)
#else
	int fd = 0;
#endif
	bool sread_error = false;
};

#endif
