// libspeexを利用して、pcmをencodeする動作、ただし1chのみ対応
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/encoder/speexEncoder.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/speex.h>

typedef bool (* encodeCallback)(
		uint8_t *speex,
		size_t length,
		uint64_t pts,
		uint32_t timebase,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num);

typedef struct speexEncoder_t {
	// speexEncoderオブジェクト
	ttLibC_SpeexEncoder *encoder;
	// 使い回すpcmオブジェクト
	ttLibC_PcmS16 *pcm;
	uint64_t pts;
	encodeCallback callback;
} speexEncoder_t;

speexEncoder_t *speexEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t quality) {
	speexEncoder_t *encoder = malloc(sizeof(speexEncoder_t));
	if(encoder == NULL) {
		return NULL;
	}
	encoder->encoder = ttLibC_SpeexEncoder_make(
			sample_rate,
			channel_num,
			quality);
	if(encoder->encoder == NULL) {
		free(encoder);
		return NULL;
	}
	encoder->pcm = NULL;
	encoder->pts = 0;
	encoder->callback = NULL;
	return encoder;
}

static bool speexEncoder_encodeCallback(void *ptr, ttLibC_Speex *speex) {
	speexEncoder_t *encoder = (speexEncoder_t *)ptr;
	// あとはcallbackを実施すればよい。
	if(encoder->callback != NULL) {
		return encoder->callback(
				speex->inherit_super.inherit_super.data,
				speex->inherit_super.inherit_super.buffer_size,
				speex->inherit_super.inherit_super.pts,
				speex->inherit_super.inherit_super.timebase,
				speex->inherit_super.sample_rate,
				speex->inherit_super.sample_num,
				speex->inherit_super.channel_num);
	}
	return true;
}

bool speexEncoder_encode(
		speexEncoder_t *encoder,
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
	return ttLibC_SpeexEncoder_encode(
			encoder->encoder,
			encoder->pcm,
			speexEncoder_encodeCallback,
			encoder);
}

void speexEncoder_close(speexEncoder_t *encoder) {
	if(encoder == NULL) {
		return;
	}
	ttLibC_SpeexEncoder_close(&encoder->encoder);
	ttLibC_PcmS16_close(&encoder->pcm);
	free(encoder);
}
