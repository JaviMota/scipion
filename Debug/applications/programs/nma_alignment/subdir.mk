################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../applications/programs/nma_alignment/nma_alignment_main.o 

CPP_SRCS += \
../applications/programs/nma_alignment/nma_alignment_main.cpp 

OBJS += \
./applications/programs/nma_alignment/nma_alignment_main.o 

CPP_DEPS += \
./applications/programs/nma_alignment/nma_alignment_main.d 


# Each subdirectory must supply rules for building sources it contributes
applications/programs/nma_alignment/%.o: ../applications/programs/nma_alignment/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


