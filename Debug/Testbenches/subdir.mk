################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Testbenches/TestbenchRFIAndSweepCompress.cpp 

OBJS += \
./Testbenches/TestbenchRFIAndSweepCompress.o 

CPP_DEPS += \
./Testbenches/TestbenchRFIAndSweepCompress.d 


# Each subdirectory must supply rules for building sources it contributes
Testbenches/%.o: ../Testbenches/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


