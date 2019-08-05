#ifndef TWE_LITE_H_
#define TWE_LITE_H_

#ifdef ARDUINO
	// Arduino
	#ifdef TWE_LITE_USE_HARDWARE_SERIAL
		#include <SoftwareSerial.h>
	#endif
#else
	// others
	#include <cstdio>
	#include <string>
	#ifdef _WIN32
		#error fuck Windows
	#elif defined(RASPBERRY_PI)
		// Raspberry Pi
		#include <wiringPi.h>
		#include <wiringSerial.h>
	#endif
#endif

class TWE_Lite {
public:
	// コンパイル時定数
	constexpr static long int default_brate	= 115200;
	constexpr static uint8_t buf_size		= 255;
	constexpr static uint16_t MSB			= 0x8000;

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
#else
	if(fd != 0){
	#ifdef RASPBERRY_PI
		serialFlush(fd);
		serialClose(fd);
	#else
		std::fclose(fd);
	#endif
	}
#endif
	}

	// 定数
#ifdef ARDUINO
	const uint8_t rx, tx;
#else
	const std::string devfile;
#endif
	const long int brate;

	// publicな変数
	uint8_t recv_buf[buf_size];	// あとで消す

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
#endif

		parser.set_buf(recv_buf);

		return true;
	}

	// シリアル通信の抽象化
	inline void swrite8(const uint8_t &val) const {
#ifdef ARDUINO
		serial->write(val);
#elif defined(RASPBERRY_PI)
		serialPutchar(fd, val);
		//std::cout << std::hex << (int)val;
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
		return serial->read();
#elif defined(RASPBERRY_PI)
		return serialGetchar(fd);
#endif
	}

	inline int savail() const {
#ifdef ARDUINO
		return serial->available();
#elif defined(RASPBERRY_PI)
		return serialDataAvail(fd);
#endif
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
	inline bool check_send(){
		const size_t msg_size = recv();
		if(msg_size < 4)
			return false;

		if((recv_buf[0] == 0xDB) && (recv_buf[1] == 0xA1)){	// 応答メッセージか？
			// response_id = recv_buf[2]; // 応答ID
			// Serial.print("id = ");
			// Serial.println(recv_buf[2], DEC);

			if(recv_buf[3] == 0x01){
				return true;			// 送信成功
			}
		}
		return false;
	}

	// 受信する(成功時送信コマンド長を返す)
	const size_t recv(const size_t timeout=0){
		#ifdef ARDUINO
			#ifndef TWE_LITE_USE_HARDWARE_SERIAL
			serial->listen();
			#endif
		#endif

		if(try_recv(timeout))
			return parser.get_cmd_length();
		return 0;
	}

	// 受信成功時trueを返す
	// 受信失敗, タイムアウト時falseを返す
	inline auto try_recv(const size_t &timeout) -> bool {
		const size_t size = savail();
		if(size <= 0) return false;
		const auto start = millis();	// TODO: Arduino,Raspberry Pi以外では使えない
		for(size_t i=0;i<size;i++){
			if(parser.parse8(sread8()))
				return true;
			if((millis() - start) >= timeout)
				break;
		}
		return false;
	}

	// 1byte受け取って受信成功したらtrueを返す
	inline auto try_recv8() -> bool {
		return parser.parse8(sread8());
	}

	// 受信データのパーサ(1byteずつパースする)
	// parse8()がtrueを返したらパース完了(checksumも合っている)
	class Parser {
	public:
		explicit Parser() : s(state::empty), cmd_length(0x0000), checksum(0x00) {}

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
		inline auto get_cmd_length() const -> const uint16_t { return cmd_length; }

		inline void set_buf(uint8_t *buf){
			payload = buf;
		}

		// 1byteずつパースする
		bool parse8(const uint8_t &b){
			parse8_binary(b);
		}

		// 送信コマンドのパース
		inline void parse_cmd(const uint8_t &b){
			static uint8_t from_id = 0x00;
			static uint8_t cmd_type= 0x00;
			static uint8_t response_id = 0x00;
			static uint32_t from_ext_addr = 0x00;
			static uint32_t to_ext_addr = 0x00;
			static uint8_t LQI = 0x00;
			static uint16_t length = 0x00;

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
				//std::cout << std::endl;
				if(b != 0xA0){ // 簡易形式
					cmd_type = b; // コマンド種別
					c = cmd::payload;
					//std::cout << "simple format" << std::endl;
				}else{
					//response_id		= 0x00;
					from_ext_addr	= 0x00;
					to_ext_addr		= 0x00;
					//LQI				= 0x00;
					length			= 0x00;
					addr_pos		= 0x00;
					//length_pos		= 0;
					c = cmd::response_id; // 拡張形式
					//std::cout << "extended format" << std::endl;
				}
				break;
			case cmd::payload:
				//std::cout
				//	<< "payload[" << std::dec << (uint32_t) payload_pos << "] = "
				//	<< std::hex << (uint32_t)b << std::endl;
				payload[payload_pos] = b;
				payload_pos++;
				break;
			case cmd::response_id:
				response_id = b;
				c = cmd::addr;
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
				if(!length_pos) length = b << 8;
				else{
					length += b;
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
				if(cmd_length <= buf_size){
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
					return true;
				}else
					error();
				break;
			}
			return false;
		}
	private:
		cmd c;
		state s, e;
		uint16_t cmd_length; // 送信コマンドの長さ
		uint8_t checksum;
		uint16_t cmd_pos;
		uint8_t *payload;

		inline void error(){
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
#else
	int fd = 0;
#endif
};

#endif
