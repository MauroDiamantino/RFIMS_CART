################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AntennaPositioner.cpp \
../Basics.cpp \
../Command.cpp \
../CurveAdjuster.cpp \
../DataLogger.cpp \
../FreqValues.cpp \
../FrontEndCalibrator.cpp \
../GPSInterface.cpp \
../RFIDetector.cpp \
../Reply.cpp \
../SpectranConfigurator.cpp \
../SpectranInterface.cpp \
../SweepBuilder.cpp \
../TimeData.cpp \
../gnuplot_i.cpp \
../main.cpp 

OBJS += \
./AntennaPositioner.o \
./Basics.o \
./Command.o \
./CurveAdjuster.o \
./DataLogger.o \
./FreqValues.o \
./FrontEndCalibrator.o \
./GPSInterface.o \
./RFIDetector.o \
./Reply.o \
./SpectranConfigurator.o \
./SpectranInterface.o \
./SweepBuilder.o \
./TimeData.o \
./gnuplot_i.o \
./main.o 

CPP_DEPS += \
./AntennaPositioner.d \
./Basics.d \
./Command.d \
./CurveAdjuster.d \
./DataLogger.d \
./FreqValues.d \
./FrontEndCalibrator.d \
./GPSInterface.d \
./RFIDetector.d \
./Reply.d \
./SpectranConfigurator.d \
./SpectranInterface.d \
./SweepBuilder.d \
./TimeData.d \
./gnuplot_i.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


