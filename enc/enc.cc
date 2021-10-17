// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <openenclave/enclave.h>

#include "flatbuffers/flatbuffers.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "oeml_t.h"

#include "misc.h"
#include "oeml_model_data.h"
#include "common.h"


const size_t tensor_arena_len = 10485760;
uint8_t tensor_arena[tensor_arena_len] = { 0 };

struct tflite_model {
    const tflite::Model *model;
    tflite::ErrorReporter *error_reporter;
    tflite::AllOpsResolver resolver;
    tflite::MicroInterpreter *interpreter;
};
struct tflite_model tf;


/* Re-implement missing symbols in accordance with OP-TEE */
void *__stack_chk_guard = (void*)0x0000aff;
extern "C" {
void *__memcpy_chk(void *dest, const void *src, size_t len, size_t dstlen)
{
	return memcpy(dest, src, len);
}
}

void viewModel(struct tflite_model *tf);
oe_result_t infer(struct tflite_model *tf, uint8_t *img, size_t w, size_t h, size_t n);
void softmax(float *arr, double *arr_out, size_t size);

int ecall_nn(uint8_t *img, size_t size, size_t w, size_t h, size_t n)
{
    int retval = 0;

    dbg("OEML enclave start");

    tflite::MicroErrorReporter micro_error_reporter;
    tf.error_reporter = &micro_error_reporter;
    tf.model = ::tflite::GetModel(g_oeml_model_data);
    if (tf.model->version() != TFLITE_SCHEMA_VERSION) {
	dbg("error");
        tf.error_reporter->Report("Model provided is schema version %d not equal "
                                  "to supported version %d",
                                  tf.model->version(), TFLITE_SCHEMA_VERSION);
    }
    tf.interpreter = new tflite::MicroInterpreter(tf.model, tf.resolver, tensor_arena, tensor_arena_len, tf.error_reporter);

    dbg("allocating tensor arena of size %d", tensor_arena);
    tf.interpreter->AllocateTensors();

#ifdef DEBUG
    viewModel(&tf);
#endif

    infer(&tf, img, w, h, n);

    return OE_OK;
}

void viewModel(struct tflite_model *tf)
{
    dbg("start");
    dbg("tf=%p", tf);
    dbg("tf->interpreter=%p", tf->interpreter);
    TfLiteTensor *input = tf->interpreter->input(0);
    tf->interpreter->Invoke();
    dbg("input=%p", input);
    TfLiteTensor *output = tf->interpreter->output(0);
    dbg("output=%p", output);

    dbg("Model input:");
    dbg("  arena: %p", tensor_arena);
    dbg("  input: %p", input);
    dbg("  input->dims: %p", input->dims);
    dbg("  input->dims->size: %d", input->dims->size);
    dbg("  dims->data[0]: %d", input->dims->data[0]);
    dbg("  dims->data[1]: %d", input->dims->data[1]);
    dbg("  dims->data[2]: %d", input->dims->data[2]);
    dbg("  dims->data[3]: %d", input->dims->data[3]);
    dbg("  input->type: %d", input->type);
    dbg("Model output:");
    dbg("  dims->size: %d", output->dims->size);
    dbg("  dims->data[0]: %d", output->dims->data[0]);
    dbg("  dims->data[1]: %d", output->dims->data[1]);
}

oe_result_t infer(struct tflite_model *tf, uint8_t *img, size_t w, size_t h, size_t n)
{
    TfLiteTensor *input = tf->interpreter->input(0);
    TfLiteTensor *output = tf->interpreter->output(0);
    TfLiteStatus invoke_status = kTfLiteError;
	int ret = 0;

	size_t img_len = w * h * n;
	if (!img) {
		err("failed to allocate float formated image memory");
		return OE_FAILURE;
	}

	dbg("composing input data of %d float elements", img_len * sizeof(float));
	for (int i = 0; i < img_len; i++)
		input->data.f[i] = (float)img[i];

	dbg("running inference");
    invoke_status = tf->interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
		err("invoke failed");
		return OE_FAILURE;
    }

	const size_t out_dims = 4;
	double out[out_dims];
    for (int i = 0; i < out_dims; i++) {
        dbg("raw-out[%d]: %f", i, output->data.f[i]);
    }
	softmax(output->data.f, out, out_dims);
    for (int i = 0; i < out_dims; i++) {
        dbg("final[%d]: %f", i, out[i]);
    }

	/* Arm */
	if (out[0] > 0.95)
		ocall_class_result(&ret, LOGO_ARM, out[0]);
	/* Microsoft */
	else if (out[1] > 0.95)
		ocall_class_result(&ret, LOGO_MSFT, out[1]);
	/* Non-class */
	else if (out[2] > 0.95)
		ocall_class_result(&ret, LOGO_NONE, out[2]);
	/* Scalys */
	else if (out[3] > 0.95)
		ocall_class_result(&ret, LOGO_SCALYS, out[3]);
	else
		ocall_class_result(&ret, LOGO_NONE, 0);

	return OE_OK;
}

void softmax(float *arr, double *arr_out, size_t size)
{
	double total = 0;

	for (int i = 0; i < size; i++) {
		total += exp((double)arr[i]);
	}
	if (total == 0) {
		return;
	}

	for (int i = 0; i < size; i++) {
		arr_out[i] = exp((double)arr[i]) / total;
	}
}


#define TA_UUID                                            \
    { /* 1f574668-6c89-41b5-b313-4b2d85d63c9d */           \
        0x1f574668, 0x6c89, 0x41b5,                        \
        {                                                  \
            0xb3, 0x13, 0x4d, 0x2d, 0x85, 0xd6, 0x3c, 0x9d \
        }                                                  \
    }

OE_SET_ENCLAVE_OPTEE(
    TA_UUID,                  // UUID
    2 * 1024 * 1024,          // HEAP_SIZE
    16 * 1024,                // STACK_SIZE
    0,                        // FLAGS
    "1.0.0",                  // VERSION
    "oeml")                   // DESCRIPTION
