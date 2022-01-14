#########################################################
# ctrace makefile 
#########################################################

#########################################################
# macros 
#########################################################

BUILD_BASE=..

VPATH = $(BUILD_BASE)/inc
INC = -I$(BUILD_BASE)/inc 


# call "make dbug=1 [target]" for debug targets 
dbug=0
ifeq ($(dbug), 1)
	OBJ_PTH = $(BUILD_BASE)/dobj
	CFLAGS += $(INC) -g
else
	OBJ_PTH = $(BUILD_BASE)/obj
	CFLAGS += $(INC) 
endif

SRC = $(wildcard *.c)
OBJ := $(patsubst %.c, $(OBJ_PTH)/%.o, $(SRC))

#########################################################
# pattern rules
#########################################################

$(OBJ_PTH)/%.o : %.c
	$(CC) $(CFLAGS) -c $<
	mv *.o $(OBJ_PTH)/

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

all : $(OBJ)


# header dependencies
include $(SRC:.c=.d)

.PHONY : clean
clean :
	rm $(OBJ_PTH)/*.o *.d
