PROJECT = stove
SOURCES = main.c

MMCU       = atmega88p
PART       = m88p
PROGRAMMER = 2232HIO

CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

CFLAGS  = -mmcu=$(MMCU) -O3 -Wall
LDFLAGS = -mmcu=$(MMCU)

OBJDIR = obj
OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all compile upload clean

all: $(PROJECT).elf $(PROJECT).hex $(PROJECT).lst

compile: $(PROJECT).elf

upload: $(PROJECT).hex
	avrdude -p $(PART) -c $(PROGRAMMER) -e -U flash:w:$(PROJECT).hex

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
