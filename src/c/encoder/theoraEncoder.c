// libtheoraを利用して、theoraをencodeする動作
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/frame/video/yuv420.h>
#include <ttLibC/frame/video/theora.h>
#include <ttLibC/encoder/theoraEncoder.h>

typedef bool (* encodeCallback)(
		uint8_t *theora,
		size_t length);

typedef struct theoraEncoder_t {
	ttLibC_TheoraEncoder *encoder;
	ttLibC_Yuv420 *yuv;
	encodeCallback callback;
} theoraEncoder_t;

theoraEncoder_t *theoraEncoder_make(
		uint32_t width,
		uint32_t height) {
	theoraEncoder_t *encoder = malloc(sizeof(theoraEncoder_t));
	if(encoder == NULL) {
		return NULL;
	}
	encoder->encoder = ttLibC_TheoraEncoder_make(width, height);
	if(encoder->encoder == NULL) {
		free(encoder);
		return NULL;
	}
	encoder->yuv = NULL;
	return encoder;
}

static bool theoraEncoder_encodeCallback(void *ptr, ttLibC_Theora *theora) {
	// theoraのデータ
	theoraEncoder_t *encoder = (theoraEncoder_t *)ptr;
	if(encoder->callback != NULL) {
		if(!encoder->callback(
				theora->inherit_super.inherit_super.data,
				theora->inherit_super.inherit_super.buffer_size)) {
			return false;
		}
	}
	return true;
}

bool theoraEncoder_encode(
		theoraEncoder_t *encoder,
		uint8_t *y,
		size_t y_size,
		uint8_t *u,
		size_t u_size,
		uint8_t *v,
		size_t v_size,
		encodeCallback callback) {
	if(encoder == NULL) {
		return false;
	}
	if(y == NULL || y_size == 0
	|| u == NULL || u_size == 0
	|| v == NULL || v_size == 0) {
		return true;
	}
	// ttLibC_Yuvデータを作らなければいけない。
	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_make(
			encoder->yuv,
			Yuv420Type_planar,
			encoder->encoder->width,
			encoder->encoder->height,
			NULL,
			0,
			y,
			encoder->encoder->width,
			u,
			encoder->encoder->width >> 1,
			v,
			encoder->encoder->width >> 1,
			true,
			0,
			1000);
	if(yuv == NULL) {
		return false;
	}
	encoder->yuv = yuv;
	encoder->callback = callback;
	return ttLibC_TheoraEncoder_encode(encoder->encoder, encoder->yuv, theoraEncoder_encodeCallback, encoder);
}

void theoraEncoder_close(theoraEncoder_t *encoder) {
	if(encoder == NULL) {
		return;
	}
	ttLibC_TheoraEncoder_close(&encoder->encoder);
	ttLibC_Yuv420_close(&encoder->yuv);
	free(encoder);
}
