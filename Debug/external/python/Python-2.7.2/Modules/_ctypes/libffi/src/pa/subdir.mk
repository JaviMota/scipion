################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/ffi.c 

S_UPPER_SRCS += \
../external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/hpux32.S \
../external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/linux.S 

OBJS += \
./external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/ffi.o \
./external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/hpux32.o \
./external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/linux.o 

C_DEPS += \
./external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/ffi.d 


# Each subdirectory must supply rules for building sources it contributes
external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/%.o: ../external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/%.o: ../external/python/Python-2.7.2/Modules/_ctypes/libffi/src/pa/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	as  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


