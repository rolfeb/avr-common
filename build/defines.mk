# vi: noexpandtab shiftwidth=8 softtabstop=8

# language options
LANG	=	-funsigned-char \
		-unsigned-bitfields \
		-fpack-struct \
		-fshort-enums \
		-fno-strict-aliasing \
		-std=gnu99

# compile options
GCC	=	avr-gcc
CC	=	$(GCC)
OBJCOPY	=	avr-objcopy
SIZE   	=	avr-size
AVR	=	/usr/local/avr
# DEBUG	=	-gdwarf-2
DEBUG	=	-g
OPT	=	-Os
WARN	=	-Wall -Wstrict-prototypes
IFLAGS	=	-I$(AVR)/include
MATHLIB	=	-L$(AVR)/lib/$(INSTRSET) -lm

# compile rules
#ASFLAGS	=	-Wa,-adhlns=$(notdir $(<:.c=.lst))

CFLAGS	=	\
		-mmcu=$(MCU) -DF_CPU=$(CPU_FREQ)UL \
		$(LANG) $(DEBUG) $(OPT) $(WARN) \
		$(ASFLAGS)

LDFLAGS	=	\
		-mmcu=$(MCU) -DF_CPU=$(CPU_FREQ)UL \
		$(LANG) $(DEBUG) $(OPT) $(WARN) \
		-Wl,-Map=$(@:.elf=.map) -Wl,--cref \
		-L$(AVR)/lib/$(INSTRSET) \
		-L$(AVR)/lib \
		$(MATHLIB)


LDFLAGS_EXTPRINTF	=	-Wl,-u,vfprintf -lprintf_flt -lm
LDFLAGS_MINPRINTF	=	-Wl,-u,vfprintf -lprintf_min

# programmer rules
#PROGTYPE=	avr910
#PORT	=	/dev/ttyS0
#CHIPCODE=	m8

PROGTYPE_ISP	=	avrispmkII
PORT_ISP	=	usb:0000B0010839

PROGTYPE_JTAG	=	jtag3
PORT_JTAG	=	usb:J30200008940
SCK_JTAG	=	-B 10

PROGTYPE_JTAGPDI =	jtag3pdi
PORT_JTAGPDI	=	usb:J30200008940
SCK_JTAGPDI	=	-B 10

AVRDUDE		=	sudo avrdude -c $(PROGTYPE_ISP) -p $(CHIPCODE) -P $(PORT_ISP) $(AVRDUDE_EXTRA)
AVRDUDE_JTAG	=	sudo /usr/local/bin/avrdude $(SCK_JTAG) -c $(PROGTYPE_JTAG) -p $(CHIPCODE) -P $(PORT_JTAG) $(AVRDUDE_EXTRA)

AVRDUDE_JTAGPDI	=	sudo /usr/local/bin/avrdude $(SCK_JTAGPDI) -c $(PROGTYPE_JTAGPDI) -p $(CHIPCODE) -P $(PORT_JTAGPDI) $(AVRDUDE_EXTRA)
