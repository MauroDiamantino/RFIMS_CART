################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CurveAdjuster.cpp \
../DataLogger.cpp \
../FreqValueSet.cpp \
../FrontEndCalibrator.cpp \
../TestbenchCalibration_2.cpp \
../gnuplot_i.cpp 

OBJS += \
./CurveAdjuster.o \
./DataLogger.o \
./FreqValueSet.o \
./FrontEndCalibrator.o \
./TestbenchCalibration_2.o \
./gnuplot_i.o 

CPP_DEPS += \
./CurveAdjuster.d \
./DataLogger.d \
./FreqValueSet.d \
./FrontEndCalibrator.d \
./TestbenchCalibration_2.d \
./gnuplot_i.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


