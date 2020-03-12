# mfcc

#########################
# Standard FFT functions
#########################
# 16b fft overflows by 30KB, for larger processors with more ROM you can directly use
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/CommonTables/arm_const_structs.c
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/CommonTables/arm_common_tables.c
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/TransformFunctions/arm_cfft_q15.c
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/TransformFunctions/arm_rfft_init_q15.c
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/TransformFunctions/arm_rfft_q15.c
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix4_q15.c
# MFCC_ASM_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S

# optimized point-wise multiplication
CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/BasicMathFunctions/arm_mult_q15.c
CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/SupportFunctions/arm_q15_to_q31.c
CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/SupportFunctions/arm_fill_q31.c

MFCC_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/mfcc.c
CUSTOM_MFCC_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_arm_dot_prod_q31.c
CUSTOM_MFCC_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_math.c
#########################
# Standard CMSIS functions
#########################
# CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/BasicMathFunctions/arm_dot_prod_q31.c
CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/BasicMathFunctions/arm_shift_q31.c
CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_q31.c
CMSIS_DSP_SRC += $(CMSIS_5_DIR)/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_squared_q31.c
#########################
# Custom CMSIS functions
# Custom objects from CMSIS 5.0.3
#########################
CMSIS_DSP_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_arm_cfft_q31.c
CMSIS_DSP_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_arm_rfft_init_q31.c
CMSIS_DSP_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_arm_rfft_q31.c
CMSIS_DSP_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_arm_cfft_radix4_q31.c
MFCC_ASM_SRC += $(M0N0_KWS_DIR)/src/mfcc/src/custom_arm_bitreversal2.S

MFCC_SRC += $(CUSTOM_MFCC_SRC)




MFCC_SRC += $(CMSIS_DSP_SRC)
