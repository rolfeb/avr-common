# vi: noexpandtab shiftwidth=8 softtabstop=8

OBJ	=	$(notdir $(CFILES:.c=.o))

%.o : $(or $(AVR_ROOT),..)/modules/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

%.o : %.c
	$(GCC) -c $(CFLAGS) $< -o $@

%.s : %.c
	$(GCC) -S -fverbose-asm -c $(CFLAGS) $< -o $@

.PRECIOUS:	%.elf
%.hex : %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

.PRECIOUS:	$(OBJ)
$(NAME).elf : $(OBJ)
	$(GCC) --output=$(NAME).elf $^ $(LDFLAGS)

avr-features.o	:	avr-features.h

.PHONY:	setfuses getfuses

setfuses	:
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

resetfuses	:
	$(AVRDUDE) -U hfuse:w:$(DEFAULT_HFUSE):m -U lfuse:w:$(DEFAULT_LFUSE):m

getfuses	:
	$(AVRDUDE) -U hfuse:r:-:h 
	$(AVRDUDE) -U lfuse:r:-:h

id		:
	$(AVRDUDE) -v

setid		:
	if [ "$(ID)" ]; \
	then \
		$(AVRDUDE) -U eeprom:w:$(ID):m; \
	else \
		echo "ID is not set!"; \
	fi

getid		:
	$(AVRDUDE) -U eeprom:r:-:h

make-debug	:
	@echo "NAME = $(NAME)"
	@echo "CFILES = $(CFILES)"
	@echo "OBJ = $(OBJ)"
	@echo "CFLAGS = $(CFLAGS)"
