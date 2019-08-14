#include <SD.h>

#define SD_CS_PIN	10
#define TEST_FILE_NAME	"testfile.txt"
#define TEST_FILE_SIZE	(1024UL*512UL)		// 1024 block file

uint8_t buf[512];
File myFile;

void setup() {
	Serial.begin(38400);
	while(!Serial);			// wait PC

	if(!SD.begin(SD_CS_PIN)) {
		Serial.println("SD.begin failed!");
		return;
	}
	Serial.println("begin");

	for (size_t n = 1; n <= 512; n *= 2) {
		SD.remove(TEST_FILE_NAME);
		myFile = SD.open(TEST_FILE_NAME, FILE_WRITE);
		if (!myFile) {
			Serial.println("open failed");
			return;
		}

		// Write file data
		uint32_t us = micros();
		for (uint32_t i = 0; i < TEST_FILE_SIZE; i += n) {
			if (n != myFile.write(buf, n)) {
				Serial.println("Write failed");
				return;
			}
		}

		us = micros() - us;
		myFile.close();
		Serial.print("buffer size (bytes): ");
		Serial.print(n);
		Serial.print(", time (sec): ");
		Serial.print(0.000001*us);
		Serial.print(", rate (KB/sec): ");
		Serial.println(TEST_FILE_SIZE / (0.001 * us));
	}
}

void loop() {
	// nothing happens after setup
}
