################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../applications/programs/mpi_angular_projection_matching/mpi_angular_projection_matching_main.o 

CPP_SRCS += \
../applications/programs/mpi_angular_projection_matching/mpi_angular_projection_matching_main.cpp 

OBJS += \
./applications/programs/mpi_angular_projection_matching/mpi_angular_projection_matching_main.o 

CPP_DEPS += \
./applications/programs/mpi_angular_projection_matching/mpi_angular_projection_matching_main.d 


# Each subdirectory must supply rules for building sources it contributes
applications/programs/mpi_angular_projection_matching/%.o: ../applications/programs/mpi_angular_projection_matching/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


