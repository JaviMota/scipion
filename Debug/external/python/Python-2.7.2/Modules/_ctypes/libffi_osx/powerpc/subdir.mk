################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-ffi_darwin.c 

S_UPPER_SRCS += \
../external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-darwin.S \
../external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-darwin_closure.S \
../external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc64-darwin_closure.S 

OBJS += \
./external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-darwin.o \
./external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-darwin_closure.o \
./external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-ffi_darwin.o \
./external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc64-darwin_closure.o 

C_DEPS += \
./external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/ppc-ffi_darwin.d 


# Each subdirectory must supply rules for building sources it contributes
external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/%.o: ../external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	as  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/%.o: ../external/python/Python-2.7.2/Modules/_ctypes/libffi_osx/powerpc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


