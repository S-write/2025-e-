################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driver/%.o: ../driver/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"E:/TI/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"E:/TI/myvode/diansai_LP_MSPM0G3507_nortos_ticlang/driver" -I"E:/TI/myvode/diansai_LP_MSPM0G3507_nortos_ticlang" -I"E:/TI/myvode/diansai_LP_MSPM0G3507_nortos_ticlang/Debug" -I"E:/TI/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/TI/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -MMD -MP -MF"driver/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


