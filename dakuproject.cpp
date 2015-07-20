#include "stdafx.h"
#include "frame-pusher.h"

#define VID_WIDTH  640
#define VID_HEIGHT 400
#define VID_DUR    10
#define VID_FRATE  30
#define AUD_SRATE  44100

#define random rand

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Code-based video generating based on frame pullers & pushers\n");
		printf("Usage: %s <output>\n", argv[0]);
		printf("Reads a video and outputs it to another format.\n");
		return 1;
	}
	av_register_all();

	int ret;
	frame_pusher *pusher;
	if ((ret = frame_pusher_open(&pusher, argv[1], AUD_SRATE, VID_FRATE, VID_WIDTH, VID_HEIGHT, 800000)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open output\n");
		return ret;
	}

	uint8_t *picture[4] = { 0 };
	// The buffer size needs to be multiplied by 3 because the format is RGB24
	int linesize[4] = { VID_WIDTH * 3 * sizeof(uint8_t) };
	int bufsize = VID_HEIGHT * linesize[0];
	picture[0] = (uint8_t *)av_malloc(bufsize);
	picture[1] = (uint8_t *)av_malloc(bufsize);

	// Generate frames
	int i, j;
	uint8_t grey;
	float loudness;
	for (i = 0; i < VID_FRATE * VID_DUR; ++i) {
		// Video
		for (j = linesize[0] * i; j < linesize[0] * (i + 1); ++j)
			picture[0][j] = random() % 256;
		memset(picture[0] + linesize[0] * (i + 1), i < 150 ? 255 : 0, linesize[0]);
		grey = (int)((double)i / (double)(VID_FRATE * VID_DUR - 1) * 255.0);
		memset(picture[0] + linesize[0] * (i + 2), grey, bufsize);
		frame_pusher_write_video(pusher, picture, linesize, 1);
		// Audio
		loudness = ((1 + sin((float)i / 75.0 * M_PI)) / 2);
		loudness = loudness * loudness * 2 + 0.2;
		for (j = 0; j < AUD_SRATE / VID_FRATE; ++j)
			frame_pusher_write_audio(pusher,
			(int16_t)(loudness * (float)(random() % 200 - 100)),
			(int16_t)(loudness * (float)(random() % 200 - 100)));
	}

	// Release resources
	frame_pusher_close(pusher);
	return 0;
}
