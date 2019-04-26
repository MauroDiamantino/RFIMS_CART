################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Command.cpp \
../CurveAdjuster.cpp \
../DataLogger.cpp \
../FloatComparison.cpp \
../FreqValueSet.cpp \
../FrontEndCalibrator.cpp \
../Reply.cpp \
../SpectranConfigurator.cpp \
../SpectranInterface.cpp \
../SweepBuilder.cpp \
../TestbenchCalibration.cpp \
../gnuplot_i.cpp 

OBJS += \
./Command.o \
./CurveAdjuster.o \
./DataLogger.o \
./FloatComparison.o \
./FreqValueSet.o \
./FrontEndCalibrator.o \
./Reply.o \
./SpectranConfigurator.o \
./SpectranInterface.o \
./SweepBuilder.o \
./TestbenchCalibration.o \
./gnuplot_i.o 

CPP_DEPS += \
./Command.d \
./CurveAdjuster.d \
./DataLogger.d \
./FloatComparison.d \
./FreqValueSet.d \
./FrontEndCalibrator.d \
./Reply.d \
./SpectranConfigurator.d \
./SpectranInterface.d \
./SweepBuilder.d \
./TestbenchCalibration.d \
./gnuplot_i.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


