PTHREAD_LIB=-lpthread
################# UC-5112 Series #############
ifeq ($(ARCH),5112)
    CC=arm-linux-gnueabihf-g++
	STRIP=arm-linux-gnueabihf-strip
	CFLAGS=-O2 -I/usr/local/include 
	##-static
	LIBS=/usr/local/lib/

$(EXEC): $(EXEC).cpp
	$(CC) $(CFLAGS) -o $(EXEC) $(EXEC).cpp $(LIBS)libmxio.a $(PTHREAD_LIB) -lmx_dio_ctl
	$(CC) $(CFLAGS) -o $(EXEC)so $(EXEC).cpp $(LIBS)libmxio.so $(PTHREAD_LIB) -lmx_dio_ctl
endif

################# UC-5112 Series #############
ifeq ($(ARCH),pc)
    CC=g++
	STRIP=-strip
	CFLAGS=-O2 -I/usr/local/include
	LIBS=/usr/local/lib/

$(EXEC): $(EXEC).cpp

	$(CC) $(CFLAGS) -o $(EXEC)pc $(EXEC).cpp $(LIBS)libmxio_x64.so $(PTHREAD_LIB)
endif

.PHONY : clean
clean :
	rm -rf $(EXEC)so $(EXEC)a  $(EXEC).o $(EXEC).gdb *.bak *~ *.*~
