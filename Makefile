CC=avr-gcc
CPU=atxmega128a1
CFLAGS=-c -g -O3 -Wall -mmcu=$(CPU)
#LDFLAGS=--cref -mmcu=$(CPU)
LDFLAGS= -mmcu=$(CPU)

SRC = src
BUILD = build
EXE = project

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(subst .c,.o,$(subst $(SRC),$(BUILD), $(SRCS)))

all:	init $(EXE).hex

$(BUILD)/%.o : $(SRC)/%.c $(SRC)/%.h
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/$(EXE).elf: $(BUILD)/$(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.hex : $(BUILD)/%.elf
	avr-objcopy -O ihex $< $@

init:
	mkdir -p $(BUILD)

clean:
	rm -f $(SRC)/*~
	rm -rf $(BUILD)/*
	rm -f *.hex

flash: $(EXE).hex
	avrdude -c avrisp2 -p x128a1 -U flash:w:$(EXE).hex
