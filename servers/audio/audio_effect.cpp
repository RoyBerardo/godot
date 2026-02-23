/**************************************************************************/
/*  audio_effect.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "audio_effect.h"

void AudioEffectInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	GDVIRTUAL_CALL(_process, p_src_frames, p_dst_frames, p_frame_count);
}
bool AudioEffectInstance::process_silence() const {
	bool ret = false;
	GDVIRTUAL_CALL(_process_silence, ret);
	return ret;
}

void AudioEffectInstance::_bind_methods() {
	GDVIRTUAL_BIND(_process, "src_buffer", "dst_buffer", "frame_count");
	GDVIRTUAL_BIND(_process_silence);
}

////

Ref<AudioEffectInstance> AudioEffect::instantiate() {
	Ref<AudioEffectInstance> ret;
	GDVIRTUAL_CALL(_instantiate, ret);
	return ret;
}
void AudioEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_mono", "samples"), &AudioEffect::process_mono);
	ClassDB::bind_method(D_METHOD("process_stereo", "samples"), &AudioEffect::process_stereo);
	ClassDB::bind_method(D_METHOD("process_mono_bytes", "samples"), &AudioEffect::process_mono_bytes);
	GDVIRTUAL_BIND(_instantiate);
}

PackedFloat32Array AudioEffect::process_mono(const PackedFloat32Array &p_samples) {
	int todo = p_samples.size();
	PackedFloat32Array ret;
	ret.resize(p_samples.size());

	AudioFrame mix_buffer[MIX_BUFFER_SIZE];
	AudioFrame temp_buffer[MIX_BUFFER_SIZE];
	Ref<AudioEffectInstance> instance = instantiate();

	while (todo) {
		int to_mix = MIN(todo, MIX_BUFFER_SIZE);
		for (int i = 0; i < to_mix; i++) {
			int64_t idx = i + (p_samples.size() - todo);
			mix_buffer[i] = Vector2(p_samples[idx], p_samples[idx]);
		}

		instance->process(mix_buffer, temp_buffer, p_samples.size());

		for (int i = 0; i < to_mix; i++) {
			int64_t idx = i + (p_samples.size() - todo);
			ret.set(idx, temp_buffer[i].left);
		}

		todo -= to_mix;
	}

	return ret;
}

PackedVector2Array AudioEffect::process_stereo(const PackedVector2Array &p_samples) {
	int todo = p_samples.size();
	PackedVector2Array ret;
	ret.resize(p_samples.size());

	AudioFrame mix_buffer[MIX_BUFFER_SIZE];
	AudioFrame temp_buffer[MIX_BUFFER_SIZE];
	Ref<AudioEffectInstance> instance = instantiate();

	while (todo) {
		int to_mix = MIN(todo, MIX_BUFFER_SIZE);
		for (int i = 0; i < to_mix; i++) {
			int64_t idx = i + (p_samples.size() - todo);
			mix_buffer[i] = AudioFrame(p_samples[idx]);
		}

		instance->process(mix_buffer, temp_buffer, p_samples.size());

		for (int i = 0; i < to_mix; i++) {
			int64_t idx = i + (p_samples.size() - todo);
			ret.set(idx, temp_buffer[i]);
		}

		todo -= to_mix;
	}

	return ret;
}

PackedByteArray AudioEffect::process_mono_bytes(const PackedByteArray &p_samples) {
	int todo = p_samples.size();
	PackedByteArray ret;
	ret.resize(p_samples.size());

	Ref<AudioEffectInstance> instance = instantiate();

	while (todo) {
		int to_mix = MIN(todo, MIX_BUFFER_SIZE);

		int offset = p_samples.size() - todo;
		for (int i = 0; i < to_mix; i++) {
			int idx = i + offset;
			mix_buffer[i] = Vector2(p_samples[idx], p_samples[idx]) / 255.0;
		}
		
		instance->process(mix_buffer, temp_buffer, to_mix);

		for (int i = 0; i < to_mix; i++) {
			ret.set(i + offset, temp_buffer[i].left * 255.0);
		}

		todo -= to_mix;
	}
	return ret;
}

AudioEffect::AudioEffect() {
}
