################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Sources/Command.cpp \
../Sources/Reply.cpp \
../Sources/SpectranInterface.cpp \
../Sources/TestbenchSpectranInterface.cpp 

OBJS += \
./Sources/Command.o \
./Sources/Reply.o \
./Sources/SpectranInterface.o \
./Sources/TestbenchSpectranInterface.o 

CPP_DEPS += \
./Sources/Command.d \
./Sources/Reply.d \
./Sources/SpectranInterface.d \
./Sources/TestbenchSpectranInterface.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


