#include <SoftwareSerial.h>

#define BRATE	38400

SoftwareSerial TWE(4, 3); // RX, TX

void setup(){
	Serial.begin(BRATE);
	TWE.begin(BRATE);
}

void send(uint8_t id, char *str){
	// header=A55A size=8005 id=78 cmd_type=00 11 22 33 78

	uint8_t header[] = {0xA5, 0x5A};
	uint8_t cmd_type = 0x01;
	uint8_t *send_data = str;
//	{0xca, 0xfe, 0xbe, 0xaf};
	//{'h', 'o', 'g', 'e', '\0'};
//	{0x11, 0x22, 0x33, 0x44};
	uint8_t len = strlen(send_data);
	constexpr uint16_t MSB = 0x8000;
	uint16_t size = 0x8000 + len + 2;

	// 送信コマンド: id, cmd_type, send_data
	TWE.write(header[0]);
	TWE.write(header[1]);
	TWE.write(size >> 8);
	TWE.write(size & 0xff);
	TWE.write(id);
	TWE.write(cmd_type);
	for(int i=0;i<len;i++)
		TWE.write(send_data[i]);
	uint8_t checksum = id ^ cmd_type;
	for(int i=0;i<len;i++)
		checksum = checksum ^ send_data[i];
	TWE.write(checksum);
}



void loop(){
//	static uint8_t i=0x00;
	uint8_t i = 0x01;
	Serial.print("send[");
	Serial.print(i);
	send(i, "hoge");
	Serial.println("]   done");
	while(TWE.available())
		Serial.print(TWE.read(), HEX);
	delay(1000);
	i++;
}
