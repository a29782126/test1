PTHREAD_LIB=-lpthread
################# UC-8100 Series #############
ifeq ($(ARCH),5112)
    CC=arm-linux-gnueabihf-g++
	STRIP=arm-linux-gnueabihf-strip
	CFLAGS=-O2 -I/usr/local/include
	## -static
	LIBS=/usr/local/lib/

$(EXEC): $(EXEC).cpp
	$(CC) $(CFLAGS) -o $(EXEC) $(EXEC).cpp $(LIBS)libmxio.a $(PTHREAD_LIB)
endif

################# UC-8100 Series #############
ifeq ($(ARCH),pc)
    CC=g++
	STRIP=-strip
	CFLAGS=-O2 -I/usr/local/include 
	##-static
	LIBS=/usr/local/lib/

$(EXEC): $(EXEC).cpp
	$(CC) $(CFLAGS) -o $(EXEC)a $(EXEC).cpp $(LIBS)libmxio_x64.a $(PTHREAD_LIB)
endif

.PHONY : clean
clean :
	rm -rf $(EXEC)so $(EXEC)a  $(EXEC).o $(EXEC).gdb *.bak *~ *.*~
