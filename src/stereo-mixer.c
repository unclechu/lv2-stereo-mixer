/*

Plugin Mixer
LV2 plugin. Pan, wide, pan gain compensation.

License: GPLv3 (https://github.com/unclechu/lv2-stereo-mixer/blob/master/LICENSE)
Author: Viacheslav Lotsmanov (unclechu)

*/

#ifdef DEBUG
#include <stdio.h>
#endif

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"

#define URI "https://github.com/unclechu/lv2-stereo-mixer"

/** Define a macro for converting a gain in dB to a coefficient */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

double samplerate;

typedef enum {
	input_l = 0,
	input_r = 1,
	output_l = 2,
	output_r = 3,
	gain_in_l = 4,
	gain_in_r = 5,
	gain_in_both = 6,
	wide = 7,
	wide_law = 8,
	pan = 9,
	pan_law = 10,
	pan_gain_compensation = 11,
	gain_out_l = 12,
	gain_out_r = 13,
	gain_out_both = 14
} PortIndex;

typedef struct {
	const float* input_l;
	const float* input_r;
	float* output_l;
	float* output_r;
	const float* gain_in_l;
	const float* gain_in_r;
	const float* gain_in_both;
	const float* wide;
	const float* wide_law;
	const float* pan;
	const float* pan_law;
	const float* pan_gain_compensation;
	const float* gain_out_l;
	const float* gain_out_r;
	const float* gain_out_both;
} Plugin;

static LV2_Handle instantiate (
	const LV2_Descriptor *descriptor,
	double rate,
	const char *bundle_path,
	const LV2_Feature* const* features
) {
	Plugin *plugin = (Plugin *)malloc(sizeof(Plugin));

	samplerate = rate;

#ifdef DEBUG
	printf("Sample rate: %d\n", (int)samplerate);
#endif

	return (LV2_Handle)plugin;
}

static void connect_port ( // {{{1
	LV2_Handle instance,
	uint32_t port,
	void* data
) {
	Plugin *plugin = (Plugin *)instance;

	switch((PortIndex)port) {
	case input_l:
		plugin->input_l = (const float *)data;
		break;
	case input_r:
		plugin->input_r = (const float *)data;
		break;
	case output_l:
		plugin->output_l = (float *)data;
		break;
	case output_r:
		plugin->output_r = (float *)data;
		break;
	case gain_in_l:
		plugin->gain_in_l = (const float *)data;
		break;
	case gain_in_r:
		plugin->gain_in_r = (const float *)data;
		break;
	case gain_in_both:
		plugin->gain_in_both = (const float *)data;
		break;
	case wide:
		plugin->wide = (const float *)data;
		break;
	case wide_law:
		plugin->wide_law = (const float *)data;
		break;
	case pan:
		plugin->pan = (const float *)data;
		break;
	case pan_law:
		plugin->pan_law = (const float *)data;
		break;
	case pan_gain_compensation:
		plugin->pan_gain_compensation = (const float *)data;
		break;
	case gain_out_l:
		plugin->gain_out_l = (const float *)data;
		break;
	case gain_out_r:
		plugin->gain_out_r = (const float *)data;
		break;
	case gain_out_both:
		plugin->gain_out_both = (const float *)data;
		break;
	}
} // connect_port() }}}1

static void run ( // {{{1
	LV2_Handle instance,
	uint32_t n_samples
) {
	const Plugin *plugin = (const Plugin *)instance;

	const float* const input_l = plugin->input_l;
	const float* const input_r = plugin->input_r;
	float* const output_l = plugin->output_l;
	float* const output_r = plugin->output_r;
	const float gain_in_l = *(plugin->gain_in_l);
	const float gain_in_r = *(plugin->gain_in_r);
	const float gain_in_both = *(plugin->gain_in_both);
	const float wide = *(plugin->wide);
	const float wide_law = *(plugin->wide_law);
	const float pan = *(plugin->pan);
	const float pan_law = *(plugin->pan_law);
	const float pan_gain_compensation = *(plugin->pan_gain_compensation);
	const float gain_out_l = *(plugin->gain_out_l);
	const float gain_out_r = *(plugin->gain_out_r);
	const float gain_out_both = *(plugin->gain_out_both);

	inline float gainPrepare(const float gain) {
		if (gain > -90) return DB_CO(gain); else return 0;
	}

	float gain_in_l_val = gainPrepare(gain_in_l);
	float gain_in_r_val = gainPrepare(gain_in_r);
	float gain_in_both_val = gainPrepare(gain_in_both);
	float gain_out_l_val = gainPrepare(gain_out_l);
	float gain_out_r_val = gainPrepare(gain_out_r);
	float gain_out_both_val = gainPrepare(gain_out_both);

	gain_in_l_val *= gain_in_both_val;
	gain_in_r_val *= gain_in_both_val;
	gain_out_l_val *= gain_out_both_val;
	gain_out_r_val *= gain_out_both_val;

	float wide_val = wide;
	if (wide_val < 0) wide_val = 0;
	if (wide_val > 100) wide_val = 100;
	wide_val = wide_val / 100;

	float wide_law_val = gainPrepare(wide_law);

	float pan_law_val = gainPrepare(pan_law);

	float pan_val = pan;
	if (pan_val < -100) pan_val = -100;
	if (pan_val > 100) pan_val = 100;

	inline float out(const float ch1, const float ch2, const float gain_out) {
		return ( (ch1) + (ch2 * (1-wide_val)) )
			* ( ((1-wide_law_val) * wide_val) + wide_law_val )
			* (gain_out);
	}

	inline float coefPan(const uint8_t side) {
		float a = pan_val + 100; // 0..200
		if (side == 0) { a = 200 - a; } // invert for left
		float c = a / 100;
		float result;

		result = pan_law_val;

		if (a < 100) {
			result *= c;
		} else if (a > 100) {
			result += ((1-pan_law_val) * (c-1) / 1);
		}

		if (pan_gain_compensation == 1) result *= (1 - pan_law_val) + 1;

		return result;
	}

	uint32_t i;
	for (i=0; i<n_samples; i++) {
		output_l[i] = out(input_l[i] * gain_in_l_val, input_r[i] * gain_in_r_val, gain_out_l_val) * coefPan(0);
		output_r[i] = out(input_r[i] * gain_in_r_val, input_l[i] * gain_in_l_val, gain_out_r_val) * coefPan(1);
	}
} // run() }}}1

// destroy
static void cleanup ( LV2_Handle instance ) {
	free( instance );
}

static const LV2_Descriptor descriptor = {
	URI,
	instantiate,
	connect_port,
	NULL,
	run,
	NULL,
	cleanup
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor ( uint32_t index ) {
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
