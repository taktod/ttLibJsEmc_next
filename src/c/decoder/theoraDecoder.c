// libtheoraをつかってtheoraをdecodeする動作
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/frame/video/theora.h>
#include <ttLibC/decoder/theoraDecoder.h>

typedef bool (* decodeCallback)(
		uint8_t *y,
		size_t   y_size,
		uint32_t y_stride,
		uint8_t *u,
		size_t   u_size,
		uint32_t u_stride,
		uint8_t *v,
		size_t   v_size,
		uint32_t v_stride);

typedef struct theoraDecoder_t {
	ttLibC_TheoraDecoder *decoder;
	ttLibC_Theora *theora;
	decodeCallback callback;
} theoraDecoder_t;

theoraDecoder_t *theoraDecoder_make() {
	theoraDecoder_t *decoder = malloc(sizeof(theoraDecoder_t));
	if(decoder == NULL) {
		return NULL;
	}
	decoder->decoder = ttLibC_TheoraDecoder_make();
	if(decoder->decoder == NULL) {
		free(decoder);
		return NULL;
	}
	decoder->theora = NULL;
	return decoder;
}

static bool theoraDecoder_decodeCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	theoraDecoder_t *decoder = (theoraDecoder_t *)ptr;
	if(decoder->callback != NULL) {
		if(!decoder->callback(
				yuv->y_data,
				yuv->y_stride * yuv->inherit_super.height,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride * yuv->inherit_super.height / 2,
				yuv->u_stride,
				yuv->v_data,
				yuv->u_stride * yuv->inherit_super.height / 2,
				yuv->v_stride)) {
			return false;
		}
	}
	return true;
}

bool theoraDecoder_decode(
		theoraDecoder_t *decoder,
		uint8_t *theora_buffer,
		size_t theora_size,
		decodeCallback callback) {
	if(decoder == NULL) {
		return false;
	}
	if(theora_buffer == NULL || theora_size == 0) {
		return true;
	}
	ttLibC_Theora *theora = ttLibC_Theora_getFrame(
			decoder->theora,
			theora_buffer,
			theora_size,
			true,
			0,
			1000);
	if(theora == NULL) {
		return false;
	}
	decoder->theora = theora;
	decoder->callback = callback;
	return ttLibC_TheoraDecoder_decode(
			decoder->decoder,
			decoder->theora,
			theoraDecoder_decodeCallback,
			decoder);
}

void theoraDecoder_close(
		theoraDecoder_t *decoder) {
	if(decoder == NULL) {
		return;
	}
	ttLibC_TheoraDecoder_close(&decoder->decoder);
	ttLibC_Theora_close(&decoder->theora);
	free(decoder);
}
