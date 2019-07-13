AVRDUDE_CONF = /etc/avrdude.conf

ifeq ($(BOARD_TAG),nano)
	BOARD_SUB := atmega328old # 互換品なのでブートローダが古い
endif

include /usr/share/arduino/Arduino.mk
