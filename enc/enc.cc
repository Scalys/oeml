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

	size_t out_dims = 3;
	double out[3];
    for (int i = 0; i < 3; i++) {
        dbg("raw-out[%d]: %f", i, output->data.f[i]);
    }
	softmax(output->data.f, out, out_dims);
    for (int i = 0; i < 3; i++) {
        dbg("final[%d]: %f", i, out[i]);
    }

	if (out[0] > 0.9)
		ocall_class_result(&ret, 1, out[0]);
	else if (out[1] > 0.9)
		ocall_class_result(&ret, 2, out[1]);
	else if (out[2] > 0.9)
		ocall_class_result(&ret, 3, out[2]);
	else
		ocall_class_result(&ret, 0, 0);

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
    { /* 5d286b7e-ff68-4b4b-b7b8-05f55dbfd0c7 */           \
        0x5d286b7e, 0xff68, 0x4b4b,                        \
        {                                                  \
            0xb7, 0xb8, 0x05, 0xf5, 0x5d, 0xbf, 0xd0, 0xc7 \
        }                                                  \
    }

OE_SET_ENCLAVE_OPTEE(
    TA_UUID,                  // UUID
    2 * 1024 * 1024,          // HEAP_SIZE
    16 * 1024,                // STACK_SIZE
    0,                        // FLAGS
    "1.0.0",                  // VERSION
    "EnclaveFibonacci")       // DESCRIPTION
