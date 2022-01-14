#########################################################
# ctrace lib makefile 
#########################################################

#########################################################
# macros 
#########################################################

# call "make dbug=1 [target]" for debug target
dbug=0
ifeq ($(dbug), 1)
	obj_dir = dobj
	lib_dir = dlib
else
	obj_dir = obj
	lib_dir = lib
endif


OBJS := $(obj_dir)/*.o


#########################################################
# targets 
#########################################################

$(lib_dir)/libctrace.a : $(OBJS) 
	ar rus $@ $?

$(OBJS): 
	@ # make ctrace 
	@ pass=`pwd` ; \
	echo "" ; echo "making unit $$pass/unit/$@..." ; \
	$(MAKE) --no-print-directory -C src 

clean :
	rm -f $(lib_dir)/*.a 
	$(MAKE) --no-print-directory -C src clean
