# Set up the variances between the different build modes

CPP = $(CC) -E
LD  := $(LD) -shared

ifeq ($(BUILD), PROFILE)
  CFLAGS	+= -O2 -pg
  LDFLAGS	+=  -L$(STATIC_LIB) -static
  LIB_EXT	:= a
else
  LDFLAGS	+=  -L$(LIB) -Xlinker -R$(LIB) 
  LIB_EXT	:= so
endif

ifeq ($(BUILD), RELEASE)
  CFLAGS	+= -O2
endif

ifeq ($(BUILD), DEVEL)
  DEBUGMODE	= 1
  CFLAGS	+= -g -Wall -O $(DEBUG_CFLAGS)
  YFLAGS	+= -t
endif

ifeq ($(BUILD),DEBUG)
  CFLAGS	+= -g -Wall $(DEBUG_CFLAGS)
  YFLAGS	+= -t
endif

%.a:
	$(AR) -ru $@ $?
	$(RANLIB) $@

%.so:
	$(LD) $(LDFLAGS) $^ -o $@

%.CPP: %.c
	$(CPP) $(CFLAGS) $(CPPFLAGS) $< > $@
