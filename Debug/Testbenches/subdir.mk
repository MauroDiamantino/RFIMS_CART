################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Testbenches/TestbenchReply.cpp 

OBJS += \
./Testbenches/TestbenchReply.o 

CPP_DEPS += \
./Testbenches/TestbenchReply.d 


# Each subdirectory must supply rules for building sources it contributes
Testbenches/%.o: ../Testbenches/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


