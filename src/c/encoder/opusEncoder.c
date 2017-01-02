// libopusを利用して、pcmをencodeする動作
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/encoder/opusEncoder.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/opus.h>

typedef bool (* encodeCallback)(
		uint8_t *opus,
		size_t length,
		uint64_t pts,
		uint32_t timebase,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num);

typedef struct opusEncoder_t {
	// opusEncoderオブジェクト
	ttLibC_OpusEncoder *encoder;
	// 使い回しするpcmオブジェクト
	ttLibC_PcmS16 *pcm;
	uint64_t pts;
	encodeCallback callback;
} opusEncoder_t;

opusEncoder_t *opusEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t unit_sample_num) {
	opusEncoder_t *encoder = malloc(sizeof(opusEncoder_t));
	if(encoder == NULL) {
		return NULL;
	}
	encoder->encoder = ttLibC_OpusEncoder_make(
			sample_rate,
			channel_num,
			unit_sample_num);

	if(encoder->encoder == NULL) {
		free(encoder);
		return NULL;
	}
	encoder->pcm = NULL;
	encoder->pts = 0;
	encoder->callback = NULL;
	return encoder;
}

static bool opusEncoder_encodeCallback(void *ptr, ttLibC_Opus *opus) {
	opusEncoder_t *encoder = (opusEncoder_t *)ptr;
	// あとはcallbackを実施すればよい。
	if(encoder->callback != NULL) {
		return encoder->callback(
				opus->inherit_super.inherit_super.data,
				opus->inherit_super.inherit_super.buffer_size,
				opus->inherit_super.inherit_super.pts,
				opus->inherit_super.inherit_super.timebase,
				opus->inherit_super.sample_rate,
				opus->inherit_super.sample_num,
				opus->inherit_super.channel_num);
	}
	return true;
}

bool opusEncoder_encode(
		opusEncoder_t *encoder,
		uint8_t *pcm,
		size_t pcm_size,
		encodeCallback callback) {
	if(encoder == NULL) {
		return false;
	}
	if(pcm == NULL || pcm_size == 0) {
		return true;
	}
	ttLibC_PcmS16 *pcms16 = ttLibC_PcmS16_make(
			encoder->pcm,
			PcmS16Type_littleEndian,
			encoder->encoder->sample_rate,
			pcm_size / encoder->encoder->channel_num / 2,
			encoder->encoder->channel_num,
			pcm,
			pcm_size,
			pcm,
			pcm_size,
			NULL,
			0,
			true,
			encoder->pts,
			encoder->encoder->sample_rate);
	if(pcms16 == NULL) {
		puts("pcm作成失敗");
		return false;
	}
	encoder->pts += pcms16->inherit_super.sample_num;
	encoder->pcm = pcms16;
	encoder->callback = callback;
	// データの準備ができたので、encodeを実施する。
	return ttLibC_OpusEncoder_encode(
			encoder->encoder,
			encoder->pcm,
			opusEncoder_encodeCallback,
			encoder);
}

void opusEncoder_close(opusEncoder_t *encoder) {
	if(encoder == NULL) {
		return;
	}
	ttLibC_OpusEncoder_close(&encoder->encoder);
	ttLibC_PcmS16_close(&encoder->pcm);
	free(encoder);
}
