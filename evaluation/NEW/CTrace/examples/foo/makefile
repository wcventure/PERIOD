#########################################################
# ctrace test makefile 
#########################################################

#########################################################
# macros
#########################################################

BUILD_BASE=../..

VPATH = $(BUILD_BASE)/inc
INC = -I$(BUILD_BASE)/inc

# call "make dbug=1 [target]" for debug targets
dbug=0
ifeq ($(dbug), 1)
	OBJ_PTH = $(BUILD_BASE)/dobj
	INC += -L$(BUILD_BASE)/dlib
	CFLAGS += $(INC) -g 
else
	OBJ_PTH = $(BUILD_BASE)/obj
	INC += -L$(BUILD_BASE)/lib
	CFLAGS += $(INC) -g 
endif

# add libraries you need for linking onto LIBS list.
# libraries will be linked in the order they appear in this list.
# the linker only links what it needs from the library at the point
# it parses it. therefore the order of this list is important.
# if you are getting "fn undefined" messages the order of this list
# may be the reason.
# e.g LIBS = -ladmi -lcom -lpthread
LIBS = -lpthread
SRC = $(wildcard *.c)
OBJ := $(patsubst %.c, %.o, $(SRC))
LINK_SRC := $(wildcard $(BUILD_BASE)/src/*.c)
LINK_OBJ := $(patsubst $(BUILD_BASE)/src/%.c, $(OBJ_PTH)/%.o, $(LINK_SRC))

#########################################################
# pattern rules
#########################################################

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.d : %.c
	@ echo "creating $@ ..."
	@ echo "$(CC) -M $(CFLAGS) $< > $@"
	@ $(CC) -M $(CFLAGS) $< > $@.tmp 
	@ echo "$@ $(OBJ_PTH)/`cat $@.tmp`" > $@
	@ rm -f $@.tmp
	@ echo "re-running make ..."
	@ $(MAKE) --no-print-directory
	
#########################################################
# targets 
#########################################################

all: unit foo 

unit:
	@ $(MAKE) --no-print-directory -C $(BUILD_BASE)/src

foo:  $(OBJ) $(LINK_OBJ)
	$(CC) -o foo $(CFLAGS) $(OBJ) $(LINK_OBJ) $(LIBS) 

# header dependencies
include $(SRC:.c=.d)

.PHONY : clean
clean :
	rm $(OBJ_PTH)/*.o *.o *.d foo 
