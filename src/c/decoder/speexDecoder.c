// libspeexを利用して、speexをdecodeする動作
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/decoder/speexDecoder.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/speex.h>

typedef bool (* decodeCallback)(
		uint8_t *pcm,
		size_t length);

typedef struct speexDecoder_t {
	ttLibC_SpeexDecoder *decoder;
	ttLibC_Speex *speex;
	decodeCallback callback;
} speexDecoder_t ;

speexDecoder_t *speexDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num) {
	speexDecoder_t *decoder = malloc(sizeof(speexDecoder_t));
	if(decoder == NULL) {
		return NULL;
	}
	decoder->decoder = ttLibC_SpeexDecoder_make(
			sample_rate,
			channel_num);
	if(decoder->decoder == NULL) {
		free(decoder);
		return NULL;
	}
	decoder->speex = NULL;
	decoder->callback = NULL;
	return decoder;
}

static bool speexDecoder_decodeCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	speexDecoder_t *decoder = (speexDecoder_t *)ptr;
	if(decoder->callback != NULL) {
		return decoder->callback(
				pcm->inherit_super.inherit_super.data,
				pcm->inherit_super.inherit_super.buffer_size);
	}
	return true;
}

bool speexDecoder_decode(
		speexDecoder_t *decoder,
		uint8_t *speex_data,
		size_t speex_size,
		uint64_t pts,
		uint32_t timebase,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		decodeCallback callback) {
	if(decoder == NULL) {
		puts("decoderがない");
		return false;
	}
	if(speex_data == NULL || speex_size == 0) {
		return true;
	}
	ttLibC_Speex *speex = ttLibC_Speex_make(
			decoder->speex,
			SpeexType_frame,
			sample_rate,
			sample_num,
			channel_num,
			speex_data,
			speex_size,
			true,
			pts,
			timebase);
	if(speex == NULL) {
		puts("failed to make speex object.");
		return false;
	}
	decoder->speex = speex;
	decoder->callback = callback;
	return ttLibC_SpeexDecoder_decode(
			decoder->decoder,
			decoder->speex,
			speexDecoder_decodeCallback,
			decoder);
}

void speexDecoder_close(speexDecoder_t *decoder) {
	if(decoder == NULL) {
		return;
	}
	ttLibC_SpeexDecoder_close(&decoder->decoder);
	ttLibC_Speex_close(&decoder->speex);
	free(decoder);
}
