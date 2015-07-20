#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>


extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavutil/opt.h>
}

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")