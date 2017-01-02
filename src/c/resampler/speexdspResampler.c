#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttLibC/resampler/speexdspResampler.h>
#include <ttLibC/frame/audio/pcms16.h>

typedef struct speexdspResampler_t{
	// resamplerオブジェクト
	ttLibC_SpeexdspResampler *resampler;
	// 使い回しするpcmオブジェクト
	ttLibC_PcmS16 *pcm;
	ttLibC_PcmS16 *resampled_pcm;
} speexdspResampler_t;

typedef bool (* resampleCallback)(uint8_t *pcm, size_t length);

speexdspResampler_t *speexdspResampler_make(
		uint32_t channel_num,
		uint32_t input_sample_rate,
		uint32_t output_sample_rate,
		uint32_t quality) {
	speexdspResampler_t *resampler = malloc(sizeof(speexdspResampler_t));
	if(resampler == NULL) {
		return NULL;
	}
	resampler->resampler = ttLibC_SpeexdspResampler_make(
			channel_num,
			input_sample_rate,
			output_sample_rate,
			quality);
	if(resampler->resampler == NULL) {
		free(resampler);
		return NULL;
	}
	resampler->pcm = NULL;
	resampler->resampled_pcm = NULL;
	return resampler;
}

bool speexdspResampler_resample(
		speexdspResampler_t *resampler,
		uint8_t *pcm,
		size_t pcm_size,
		resampleCallback callback) {
	if(resampler == NULL) {
		return false;
	}
	if(pcm == NULL || pcm_size == 0) {
		return true;
	}
	// リサンプル動作を実行して、応答をcallbackで返す。
	// pcmオブジェクトを作らなければならない。
	// float32がデフォルトで利用しているだが、これをここでs16にするとちょっとオブジェクトがもったいないので
	// js側でこの変換はやろうと思う。
	ttLibC_PcmS16 *pcms16 = ttLibC_PcmS16_make(
			resampler->pcm,
			PcmS16Type_littleEndian,
			resampler->resampler->input_sample_rate,
			pcm_size / resampler->resampler->channel_num / 2,
			resampler->resampler->channel_num,
			pcm,
			pcm_size,
			pcm,
			pcm_size,
			NULL,
			0,
			true,
			0,
			resampler->resampler->input_sample_rate); // prevObjectが必要
	if(pcms16 == NULL) {
		puts("pcmできてない");
		return false;
	}
	resampler->pcm = pcms16;
	ttLibC_PcmS16 *resampled_pcm = ttLibC_SpeexdspResampler_resample(
			resampler->resampler,
			resampler->resampled_pcm,
			resampler->pcm);
	if(resampled_pcm == NULL) {
		puts("リサンプルできてない。");
		return false;
	}
	resampler->resampled_pcm = resampled_pcm;
	if(callback != NULL) {
		if(!callback(
				resampled_pcm->inherit_super.inherit_super.data,
				resampled_pcm->inherit_super.inherit_super.buffer_size)) {
			return false;
		}
	}
	return true;
}

void speexdspResampler_close(speexdspResampler_t *resampler) {
	if(resampler == NULL) {
		return;
	}
	ttLibC_SpeexdspResampler_close(&resampler->resampler);
	ttLibC_PcmS16_close(&resampler->pcm);
	ttLibC_PcmS16_close(&resampler->resampled_pcm);
	free(resampler);
}
