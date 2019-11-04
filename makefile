##########################################RFIMS-CART SOFTWARE MAKEFILE###################################

####################COMPILER###########################
CXX = g++

###############COMPILER AND LINKER FLAGS###############
#DEBUG
#CPPFLAGS = -std=c++11 -O0 -g3 -Wall -fmessage-length=0 -I/usr/local/include
#LDFLAGS = -g3

#RELEASE
CPPFLAGS = -std=c++11 -O3 -g0 -Wall -fmessage-length=0 -I/usr/local/include
LDFLAGS = -g0

#LDLIBS = -L/usr/local/lib -lftd2xx -lboost_filesystem -lboost_system -lboost_timer -lnmea -lwiringPi -lpthread #For Raspberry Pi boards
LDLIBS = -L/usr/local/lib -lftd2xx -lboost_filesystem -lboost_system -lboost_timer -lnmea -lpthread #For non-Raspberry boards

#######################FILES###########################
HEADER_NAMES = AntennaPositioning.h TopLevel.h Basics.h Spectran.h SweepProcessing.h gnuplot_i.hpp

SRC_NAMES = AntennaPositioner.cpp Command.cpp CurveAdjuster.cpp DataLogger.cpp Basics.cpp FreqValues.cpp\
FrontEndCalibrator.cpp gnuplot_i.cpp GPSInterface.cpp Reply.cpp RFIDetector.cpp\
SpectranConfigurator.cpp SpectranInterface.cpp SweepBuilder.cpp TimeData.cpp TopLevel.cpp
#main.cpp

MAIN_TARGET = bin/rfims-cart

SOURCES = $(addprefix src/, $(SRC_NAMES))
OBJECTS = $(addprefix obj/, $(subst .cpp,.o,$(SRC_NAMES)))
HEADERS = $(addprefix src/, $(HEADER_NAMES))

###################MAKE INSTRUCTIONS#####################

all: $(MAIN_TARGET)

tests: bin/test-gps bin/test-spectran

$(MAIN_TARGET): $(OBJECTS) obj/main.o
	@echo "Linking..."
	@mkdir -p bin/
	$(CXX) $(LDFLAGS) -o $(MAIN_TARGET) $(OBJECTS) obj/main.o $(LDLIBS)

bin/test-gps: $(OBJECTS) obj/TestbenchGPSLogger.o
	@echo "Linking test-gps..."
	@mkdir -p bin/
	$(CXX) $(LDFLAGS) -o bin/test-gps $(OBJECTS) obj/TestbenchGPSLogger.o $(LDLIBS)

bin/test-spectran: $(OBJECTS) obj/TestbenchSpectran.o
	@echo "Linking test-spectran..."
	@mkdir -p bin/
	$(CXX) $(LDFLAGS) -o bin/test-spectran $(OBJECTS) obj/TestbenchSpectran.o $(LDLIBS)

obj/main.o: src/main.cpp $(HEADERS)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/main.o -c src/main.cpp

obj/TestbenchGPSLogger.o: test/TestbenchGPSLogger.cpp $(HEADERS)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/TestbenchGPSLogger.o -c test/TestbenchGPSLogger.cpp

obj/TestbenchSpectran.o: test/TestbenchSpectran.cpp $(HEADERS)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/TestbenchSpectran.o -c test/TestbenchSpectran.cpp

obj/AntennaPositioner.o: $(addprefix src/, AntennaPositioner.cpp Basics.h AntennaPositioning.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/AntennaPositioner.o -c src/AntennaPositioner.cpp

obj/Command.o: $(addprefix src/, Command.cpp Basics.h Spectran.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/Command.o -c src/Command.cpp

obj/CurveAdjuster.o: $(addprefix src/, CurveAdjuster.cpp Basics.h SweepProcessing.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/CurveAdjuster.o -c src/CurveAdjuster.cpp

obj/DataLogger.o: $(addprefix src/, DataLogger.cpp Basics.h SweepProcessing.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/DataLogger.o -c src/DataLogger.cpp

obj/Basics.o: $(addprefix src/, Basics.cpp Basics.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/Basics.o -c src/Basics.cpp

obj/FreqValues.o: $(addprefix src/, FreqValues.cpp Basics.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/FreqValues.o -c src/FreqValues.cpp

obj/FrontEndCalibrator.o: $(addprefix src/, FrontEndCalibrator.cpp Basics.h SweepProcessing.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/FrontEndCalibrator.o -c src/FrontEndCalibrator.cpp

obj/gnuplot_i.o: $(addprefix src/, gnuplot_i.cpp Basics.h gnuplot_i.hpp)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/gnuplot_i.o -c src/gnuplot_i.cpp

obj/GPSInterface.o: $(addprefix src/, GPSInterface.cpp Basics.h AntennaPositioning.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/GPSInterface.o -c src/GPSInterface.cpp

obj/Reply.o: $(addprefix src/, Reply.cpp Basics.h Spectran.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/Reply.o -c src/Reply.cpp

obj/RFIDetector.o: $(addprefix src/, RFIDetector.cpp SweepProcessing.h Basics.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/RFIDetector.o -c src/RFIDetector.cpp

obj/SpectranConfigurator.o: $(addprefix src/, SpectranConfigurator.cpp Basics.h Spectran.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/SpectranConfigurator.o -c src/SpectranConfigurator.cpp

obj/SpectranInterface.o: $(addprefix src/, SpectranInterface.cpp Basics.h Spectran.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/SpectranInterface.o -c src/SpectranInterface.cpp

obj/SweepBuilder.o: $(addprefix src/, SweepBuilder.cpp Basics.h Spectran.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/SweepBuilder.o -c src/SweepBuilder.cpp

obj/TimeData.o: $(addprefix src/, TimeData.cpp Basics.h)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/TimeData.o -c src/TimeData.cpp

obj/TopLevel.o: src/TopLevel.cpp src/TopLevel.h
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/TopLevel.o -c src/TopLevel.cpp

clean:
	@echo "Cleaning..."
	rm -f -r obj/ bin/

copy-files:
	@echo "Copying the program binary, the scripts and the program data files..."
	cp -f $(MAIN_TARGET) /usr/local/bin
	cp -f scripts/client.py /usr/local
	cp -f -r data/RFIMS-CART/ /home/pi/
	cp -f data/99-aaronia-spectran.rules /etc/udev/rules.d
	mkdir -p /home/pi/.config/autostart
	cp -f scripts/rfims.desktop /home/pi/.config/autostart
