
NN_SRC += $(M0N0_KWS_DIR)/src/nn/src/nn.c

# From CMSIS_5/NN arm_fully_connected_mat_q7_vec_q15
CMSIS_NN_SRC += $(CMSIS_5_DIR)/CMSIS/NN/Source/FullyConnectedFunctions/arm_fully_connected_mat_q7_vec_q15.c
CMSIS_NN_SRC += $(CMSIS_5_DIR)/CMSIS/NN/Source/FullyConnectedFunctions/arm_fully_connected_q7_opt.c
CMSIS_NN_SRC += $(CMSIS_5_DIR)/CMSIS/NN/Source/FullyConnectedFunctions/arm_fully_connected_q7.c
CMSIS_NN_SRC += $(CMSIS_5_DIR)/CMSIS/NN/Source/ActivationFunctions/arm_relu_q7.c
CMSIS_NN_SRC += $(CMSIS_5_DIR)/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.c
CMSIS_NN_SRC += $(CMSIS_5_DIR)/CMSIS/NN/Source/NNSupportFunctions/arm_q7_to_q15_reordered_no_shift.c

NN_SRC += $(CMSIS_NN_SRC)
