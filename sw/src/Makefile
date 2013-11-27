PROJECT = stove
SOURCES = owi.c main.c

MMCU       = atmega88p
PART       = m88p
PROGRAMMER = ARMUSBTINYH
PROG_OPTIONS = -C ~/.avrduderc

F_CPU	= 1000000

CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

CFLAGS  = -mmcu=$(MMCU) -g -O2 -W -Wall -DF_CPU=$(F_CPU)
LDFLAGS = -mmcu=$(MMCU) -g

OBJDIR = obj
OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all compile upload clean

all: $(PROJECT).elf $(PROJECT).hex $(PROJECT).lst

compile: $(PROJECT).elf

upload: $(PROJECT).hex
	avrdude -p $(PART) -c $(PROGRAMMER) $(PROG_OPTIONS) -e -U flash:w:$(PROJECT).hex

clean:
	rm -rf $(OBJECTS) obj/ $(PROJECT).hex $(PROJECT).elf $(PROJECT).lst

$(OBJDIR):
	@mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	@echo Compiling $< ...
	@$(CC) $(CFLAGS) -c  $< -o $@

$(PROJECT).elf: $(OBJECTS)
	 @echo Linking $@ ...
	 @$(CC) $(LDFLAGS) $(OBJECTS) -o $(PROJECT).elf

$(PROJECT).hex: $(PROJECT).elf
	@echo Creating $@ ...
	@$(OBJCOPY) -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.lst: %.elf
	@echo Creating $@
	$(OBJDUMP) -d -S $< >$@
