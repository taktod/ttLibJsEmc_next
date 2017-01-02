// libopusを利用して、opusをdecodeする動作
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/decoder/opusDecoder.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/opus.h>

typedef bool (* decodeCallback)(
		uint8_t *pcm,
		size_t length);

typedef struct opusDecoder_t {
	// opusDecoderオブジェクト
	ttLibC_OpusDecoder *decoder;
	// 使い回しするopusオブジェクト
	ttLibC_Opus *opus;
	decodeCallback callback;
} opusDecoder_t;

opusDecoder_t *opusDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num) {
	opusDecoder_t *decoder = malloc(sizeof(opusDecoder_t));
	if(decoder == NULL) {
		return NULL;
	}
	decoder->decoder = ttLibC_OpusDecoder_make(
			sample_rate,
			channel_num);
	if(decoder->decoder == NULL) {
		free(decoder);
		return NULL;
	}
	decoder->opus = NULL;
	decoder->callback = NULL;
	return decoder;
}

static bool opusDecoder_decodeCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	opusDecoder_t *decoder = (opusDecoder_t *)ptr;
	if(decoder->callback != NULL) {
		return decoder->callback(
			pcm->inherit_super.inherit_super.data,
			pcm->inherit_super.inherit_super.buffer_size);
	}
	return true;
}

bool opusDecoder_decode(
		opusDecoder_t *decoder,
		uint8_t *opus_data,
		size_t opus_size,
		uint64_t pts, // ここ・・・64bitで処理しようとするとまずいことになるわけか・・・
		uint32_t timebase,
		decodeCallback callback) {
	if(decoder == NULL) {
		puts("decoderがない");
		return false;
	}
	if(opus_data == NULL || opus_size == 0) {
		return true;
	}
	ttLibC_Opus *opus = ttLibC_Opus_makeFrame(
			decoder->opus,
			opus_data,
			opus_size,
			pts,
			timebase);
	if(opus == NULL) {
		puts("opusのフレーム復元できてない");
		return false;
	}
	decoder->opus = opus;
	decoder->callback = callback;
	return ttLibC_OpusDecoder_decode(
			decoder->decoder,
			decoder->opus,
			opusDecoder_decodeCallback,
			decoder);
}

void opusDecoder_close(opusDecoder_t *decoder) {
	if(decoder == NULL) {
		return;
	}
	ttLibC_OpusDecoder_close(&decoder->decoder);
	ttLibC_Opus_close(&decoder->opus);
	free(decoder);
}
