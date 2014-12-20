# vi: noexpandtab shiftwidth=8 softtabstop=8

#
# Load microcontroller-specific definitions
#
include	$(AVR_ROOT)/build/$(MCU).mk
include $(AVR_ROOT)/build/defines.mk

IFLAGS	:=	-DMCU_H='"$(MCU).h"' $(IFLAGS)
IFLAGS	+=	-I. -I$(AVR_ROOT)/include

vpath %.c .

#
# Create directory for compiled objects
#
OBJDIR	=	obj

ifeq (${wildcard $(OBJDIR)},)
	DUMMY := ${shell mkdir $(OBJDIR)}
endif

#
# Derived names
#
OBJ		=	$(addprefix $(OBJDIR)/,$(CFILES:.c=.o))

RESULT_ELF	=	$(OBJDIR)/$(NAME).elf
RESULT_HEX	=	$(OBJDIR)/$(NAME).hex
RESULT_MAP	=	$(OBJDIR)/$(NAME).map

#
# Include uIP application modules
#
ifdef UIP_APPS
	APPDIRS = $(foreach APP, $(UIP_APPS), $(AVR_ROOT)/uip/apps/$(APP))
	include $(foreach APP, $(UIP_APPS), $(AVR_ROOT)/uip/apps/$(APP)/Makefile.$(APP))
	IFLAGS += $(addprefix -I$(AVR_ROOT)/uip/apps/,$(UIP_APPS))
	vpath %.c $(APPDIRS)
	CFILES += $(APP_SOURCES)
endif

#
# Include uIP driver modules
#
ifdef UIP_DRVS
	DRVDIRS = $(foreach DRV, $(UIP_DRVS), $(AVR_ROOT)/uip/drivers/$(DRV))
	include $(foreach DRV, $(UIP_DRVS), $(AVR_ROOT)/uip/drivers/$(DRV)/Makefile.$(DRV))
	IFLAGS += -I$(AVR_ROOT)/uip/drivers/include
	IFLAGS += $(addprefix -I$(AVR_ROOT)/uip/drivers/,$(UIP_DRVS))
	vpath %.c $(DRVDIRS)
	CFILES += $(DRV_SOURCES)
endif

ifdef UIP_DRVS
	#
	# Include uIP base code
	#
	IFLAGS += -I$(AVR_ROOT)/uip/uip
	CFILES += uip.c uip_arp.c uiplib.c psock.c timer.c uip-neighbor.c
	vpath %.c $(AVR_ROOT)/uip/uip
endif

#
# Include any required modules
#
ifdef MODULES
	CFILES += $(MODULES:=.c)
	vpath %.c $(AVR_ROOT)/modules
endif

#
# Add module-cfg.h to the include list (via avr-common.h) if it is present
#
IFLAGS	:=	$(if $(wildcard module-cfg.h),-DMODULE_CFG_H='"module-cfg.h"') $(IFLAGS)

#
# Include IFLAGS in CFLAGS
#
CFLAGS	+=	$(IFLAGS)

#
# Define rules to build intermediate targets
#

$(OBJDIR)/%.o : $(or $(AVR_ROOT),..)/modules/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.c
	$(GCC) -c $(CFLAGS) $< -o $@

%.s : %.c
	$(GCC) -S -fverbose-asm -c $(CFLAGS) $< -o $@

#
# Define rules to build final targets
#
all		:	$(RESULT_HEX)

install		:	$(RESULT_HEX)
	$(AVRDUDE) -U flash:w:$(RESULT_HEX):i

clean		:
	rm -f $(OBJ)
	rm -f $(RESULT_HEX) $(RESULT_ELF) $(RESULT_MAP)

realclean	:	clean
	rmdir $(OBJDIR)
	# rm -f crt$(CHIPCODE).o

.PRECIOUS:      %.elf
%.hex : %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $? $@

.PRECIOUS:      $(OBJ)
$(RESULT_ELF) : $(OBJ) # crt$(CHIPCODE).o 
	$(GCC)  --output=$(RESULT_ELF) $(OBJ) $(LDFLAGS) $(LDFLAGS_EXTRA)

setfuses        :
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

resetfuses      :
	$(AVRDUDE) -U hfuse:w:$(DEFAULT_HFUSE):m -U lfuse:w:$(DEFAULT_LFUSE):m

getfuses        :
	$(AVRDUDE) -U hfuse:r:-:h
	$(AVRDUDE) -U lfuse:r:-:h

id              :
	$(AVRDUDE) -v

setid           :
	if [ "$(ID)" ]; \
        then \
                $(AVRDUDE) -U eeprom:w:$(ID):m; \
        else \
                echo "ID is not set!"; \
        fi

getid		:
	$(AVRDUDE) -U eeprom:r:-:h

geteeprom:
	$(AVRDUDE) -U eeprom:r:-:h

depend:
	makedepend -pobj/ -I/usr/include $(IFLAGS) $(CFILES)
	makedepend -a -I/usr/include $(IFLAGS) $(addprefix $(AVR_ROOT)/modules/,$(MODULES:=.c))
	

#
# Hack to work around bug in avr-ld
#
#crt$(CHIPCODE).o:	$(AVR)/lib/$(INSTRSET)/crt$(CHIPCODE).o
#	cp $? $@
