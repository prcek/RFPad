

TARGET=main
SRC=main.c prompt.c uart.c rf.c print.c scan.c ee.c utils.c io.c timer.c server.c client.c

MCU=atmega328p
AVRDUDE_PROGRAMMER = arduino
AVRDUDE_BAUD = 57600
AVR_BAUD = 115200


MACHINE=$(shell uname -s)
#ifeq ($(MACHINE),"Linux")
AVRDUDE_PORT=/dev/ttyUSB0
AVRDUDE_PORT2=/dev/ttyUSB1
STTY=stty -F $(AVRDUDE_PORT)
STTY2=stty -F $(AVRDUDE_PORT2)
#else ifeq ($(MACHINE),"Darwin") 
#AVRDUDE_PORT=/dev/tty.usbserial-FTTG8GP4
#AVRDUDE_PORT=/dev/tty.usbserial-FTT7KTEW
#STTY=stty -f $(AVRDUDE_PORT)
#else 
#
#endif




#AVRDUDE_PORT = /dev/ttyUSB0
#AVRDUDE_PORT = /dev/tty.usbserial-FTTG8GP4


#STTY = stty -F $(AVRDUDE_PORT)
#STTY = stty -f $(AVRDUDE_PORT)

BUILD_TIME=`date`

CFLAGS= -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=$(MCU)    -DF_CPU=16000000UL  -DBUILD_TIME="$(BUILD_TIME)"
#-MMD -MP -MF"main.d" -MT"main.d"

all: elf hex 

hex: $(TARGET).hex
elf: $(TARGET).elf
OBJ = $(SRC:.c=.o)
PREP=$(SRC:.c=.prep)
GENS=$(SRC:.c=.s)

.SUFFIXES: .elf .hex .eep .lss .sym .prep .s .asm

.elf.hex:
	avr-objdump -h  $<	
	avr-objcopy -O ihex -R .eeprom $< $@

$(TARGET).elf: $(OBJ)
	avr-gcc -mmcu=atmega168 $(OBJ) -o $@ 
	#avr-gcc -Wl,-Map,$(TARGET).map -mmcu=atmega168 $(OBJ) -o $@ 

.c.o:
	avr-gcc $(CFLAGS) -c $< -o $@ 

.c.prep:
	avr-gcc $(CFLAGS) -E $< -o $@ 

.c.s:
	avr-gcc $(CFLAGS) -S $< -o $@ 

clean:
	rm -rf $(OBJ) $(TARGET).hex  $(TARGET).elf $(TARGET).map $(TARGET).d $(PREP) $(GENS)


install:	$(TARGET).hex
	$(STTY) hupcl
	avrdude -v -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b $(AVRDUDE_BAUD) -D -U flash:w:$(TARGET).hex:a

install2:	$(TARGET).hex
	$(STTY2) hupcl
	avrdude -v -p $(MCU) -P $(AVRDUDE_PORT2) -c $(AVRDUDE_PROGRAMMER) -b $(AVRDUDE_BAUD) -D -U flash:w:$(TARGET).hex:a

reset:
	$(STTY) hupcl

reset2:
	$(STTY2) hupcl

attach:
	screen $(AVRDUDE_PORT) $(AVR_BAUD)
attach2:
	screen $(AVRDUDE_PORT2) $(AVR_BAUD)

read_eeprom:
	$(STTY) hupcl
	avrdude -v -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b $(AVRDUDE_BAUD) -D -U eeprom:r:eeprom_dump.hex:i
	

