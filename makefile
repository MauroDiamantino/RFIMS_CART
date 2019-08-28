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

LDLIBS = -L/usr/local/lib -lftd2xx -lboost_filesystem -lboost_system -lboost_timer -lnmea -lwiringPi -lpthread #For Raspberry Pi boards
#LDLIBS = -L/usr/local/lib -lftd2xx -lboost_filesystem -lboost_system -lboost_timer -lnmea -lpthread #For non-Raspberry boards

#######################FILES###########################
HEADER_NAMES = AntennaPositioning.h TopLevel.h Basics.h Spectran.h SweepProcessing.h gnuplot_i.hpp

SRC_NAMES = AntennaPositioner.cpp Command.cpp CurveAdjuster.cpp DataLogger.cpp Basics.cpp FreqValues.cpp\
FrontEndCalibrator.cpp gnuplot_i.cpp GPSInterface.cpp Reply.cpp RFIDetector.cpp\
SpectranConfigurator.cpp SpectranInterface.cpp SweepBuilder.cpp TimeData.cpp\
main.cpp

TARGET = bin/rfims-cart

SOURCES = $(addprefix src/, $(SRC_NAMES))
OBJECTS = $(addprefix obj/, $(subst .cpp,.o,$(SRC_NAMES)))
HEADERS = $(addprefix src/, $(HEADER_NAMES))

###################MAKE INSTRUCTIONS#####################
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@mkdir -p bin/
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

obj/main.o: src/main.cpp $(HEADERS)
	@mkdir -p obj/
	$(CXX) $(CPPFLAGS) -o obj/main.o -c src/main.cpp

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

clean:
	@echo "Cleaning..."
	rm -f -r obj/ bin/

copy-files:
	@echo "Copying the program binary, the scripts and the program data files..."
	cp -f $(TARGET) /usr/local/bin
	cp -f scripts/client.py /usr/local
	cp -f -r data/RFIMS/ /home/pi/
	cp -f data/99-aaronia-spectran.rules /etc/udev/rules.d
