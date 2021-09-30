set(TENSORFLOW_LITE_DIR ${CMAKE_SOURCE_DIR}/libs/tensorflow)
message(":::: TENSORFLOW_LITE_DIR ${TENSORFLOW_LITE_DIR}")

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${TENSORFLOW_LITE_DIR}")
  message(FATAL_ERROR "Tensorflow-lite submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

set (TENSORFLOW_LITE_INC
    ${TENSORFLOW_LITE_DIR}/lite/
    ${TENSORFLOW_LITE_DIR}/lite/flatbuffers/include/flatbuffers
    ${TENSORFLOW_LITE_DIR}/lite/fixedpoint/fixedpoint
    ${TENSORFLOW_LITE_DIR}/lite/fixedpoint
    ${TENSORFLOW_LITE_DIR}/lite/kernels/internal/reference/integer_ops
    ${TENSORFLOW_LITE_DIR}/lite/kernels/internal/reference
    ${TENSORFLOW_LITE_DIR}/lite/kernels/internal/optimized
    ${TENSORFLOW_LITE_DIR}/lite/kernels/internal
    ${TENSORFLOW_LITE_DIR}/lite/kernels
    ${TENSORFLOW_LITE_DIR}/lite/experimental/micro/kernels
    ${TENSORFLOW_LITE_DIR}/lite/experimental/micro/testing
    ${TENSORFLOW_LITE_DIR}/lite/experimental/micro
    ${TENSORFLOW_LITE_DIR}/lite/experimental
    ${TENSORFLOW_LITE_DIR}/lite/core
    ${TENSORFLOW_LITE_DIR}/lite/c
    ${TENSORFLOW_LITE_DIR}/lite/schema
    ${TENSORFLOW_LITE_DIR}/lite
)

include_directories(
    ${TENSORFLOW_LITE_INC}
)

aux_source_directory(${TENSORFLOW_LITE_DIR}/lite/flatbuffers/src FLATBUFFERS_SRC)

#message("::: USE_CORTEX_NN: ${USE_CORTEX_NN}")
#if (USE_CORTEX_NN)
#message("::: USE_CORTEX_NN-0 ${USE_CORTEX_NN}")
#  set(TENSORFLOW_LITE_SRC
#    ${TENSORFLOW_LITE_DIR}/lite/experimental/micro/kernels/cmsis-nn/depthwise_conv.cc
#  )
#else()
#message("::: USE_CORTEX_NN-1 ${USE_CORTEX_NN}")
#  set(TENSORFLOW_LITE_SRC
#    ${TENSORFLOW_LITE_DIR}/lite/experimental/micro/kernels/portable_optimized/depthwise_conv.cc
#  )
#endif()

# Get all source files from the Src directory
set(TENSORFLOW_LITE_SRC
    ${TENSORFLOW_LITE_SRC}
    ${TENSORFLOW_LITE_DIR}/lite/c/common.c
    ${TENSORFLOW_LITE_DIR}/lite/core/api/op_resolver.cc
    ${TENSORFLOW_LITE_DIR}/lite/core/api/flatbuffer_conversions.cc
    ${TENSORFLOW_LITE_DIR}/lite/core/api/error_reporter.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/debug_log.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_error_reporter.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_string.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_interpreter.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_allocator.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/memory_helpers.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/simple_memory_allocator.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/flatbuffer_utils.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/memory_planner/greedy_memory_planner.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_graph.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/all_ops_resolver.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_resource_variable.cc
    ${TENSORFLOW_LITE_DIR}/lite/micro/micro_utils.cc

    ${TENSORFLOW_LITE_DIR}/lite/kernels/kernel_util.cc

${TENSORFLOW_LITE_DIR}/lite/micro/kernels/activations.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/activations_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/add.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/add_n.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/arg_min_max.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/assign_variable.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/batch_to_space_nd.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/call_once.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/cast.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/ceil.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/circular_buffer.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/circular_buffer_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/circular_buffer_flexbuffers_generated_data.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/comparisons.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/concatenation.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/conv.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/conv_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/cumsum.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/depth_to_space.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/depthwise_conv.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/depthwise_conv_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/dequantize.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/detection_postprocess.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/detection_postprocess_flexbuffers_generated_data.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/elementwise.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/elu.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/ethosu.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/exp.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/expand_dims.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/fill.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/floor.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/floor_div.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/floor_mod.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/fully_connected.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/fully_connected_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/gather.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/gather_nd.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/hard_swish.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/hard_swish_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/if.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/kernel_runner.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/kernel_util.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/l2_pool_2d.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/l2norm.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/leaky_relu.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/leaky_relu_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/log_softmax.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/logical.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/logical_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/logistic.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/logistic_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/maximum_minimum.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/mul.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/neg.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/pack.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/pad.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/pooling.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/pooling_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/prelu.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/quantize.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/quantize_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/read_variable.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/reduce.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/reshape.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/resize_bilinear.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/resize_nearest_neighbor.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/round.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/shape.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/softmax.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/softmax_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/space_to_batch_nd.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/space_to_depth.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/split.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/split_v.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/squeeze.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/strided_slice.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/sub.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/svdf.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/svdf_common.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/tanh.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/transpose.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/transpose_conv.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/unpack.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/var_handle.cc
${TENSORFLOW_LITE_DIR}/lite/micro/kernels/zeros_like.cc



    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/unpack.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/var_handle.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/transpose_conv.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/conv.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/fully_connected.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/softmax.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/svdf.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/elementwise.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/add.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/add_n.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/arg_min_max.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/assign_variable.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/pooling.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/ethosu.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/comparisons.cc
    #    ${TENSORFLOW_LITE_DIR}/lite/micro/kernels/elu.cc
    ${TENSORFLOW_LITE_DIR}/lite/schema/schema_utils.cc
    ${FLATBUFFERS_SRC}
)

add_library(Tensorflow_lite_micro STATIC ${TENSORFLOW_LITE_SRC})

set_target_properties(Tensorflow_lite_micro PROPERTIES LINKER_LANGUAGE CXX)

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} Tensorflow_lite_micro)
