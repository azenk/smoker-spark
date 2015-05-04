

firmware.bin: smoker-spark.ino AD7194.cpp AD7194.h thermocouple.cpp thermocouple.h mypwm.cpp mypwm.h pid-control.cpp pid-control.h
	echo "#define BUILDTIME `date +%s`" > buildtime.h
	spark compile smoker-spark.ino AD7194.cpp AD7194.h thermocouple.cpp thermocouple.h mypwm.cpp mypwm.h pid-control.cpp pid-control.h buildtime.h firmware.bin 

install: firmware.bin
	spark flash $(CORE_ID) firmware.bin

clean:
	rm -f firmware.bin
