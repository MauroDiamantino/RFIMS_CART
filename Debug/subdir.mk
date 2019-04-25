################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Command.cpp \
../FreqValueSet.cpp \
../Reply.cpp \
../SpectranConfigurator.cpp \
../SpectranInterface.cpp \
../SweepBuilder.cpp \
../TestbenchSweepCapturing.cpp 

OBJS += \
./Command.o \
./FreqValueSet.o \
./Reply.o \
./SpectranConfigurator.o \
./SpectranInterface.o \
./SweepBuilder.o \
./TestbenchSweepCapturing.o 

CPP_DEPS += \
./Command.d \
./FreqValueSet.d \
./Reply.d \
./SpectranConfigurator.d \
./SpectranInterface.d \
./SweepBuilder.d \
./TestbenchSweepCapturing.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


