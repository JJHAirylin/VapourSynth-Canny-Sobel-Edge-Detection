//////////////////////////////////////////
// This file contains a simple filter
// skeleton you can use to get started.
// With no changes it simply passes
// frames through.

#include <stdlib.h>
#include "VapourSynth.h"
#include "VSHelper.h"


void GaussianBlur(unsigned char* src, unsigned char* dst, int height, int width) {
	const int kernel_single_tab[7] = { 1, 6, 15, 20, 15, 6, 1 }; 	// scale by 4096, row add up to 64
	int* horizontal = new int[width];

	unsigned char* row0 = src;
	unsigned char* row1 = src + width;
	unsigned char* row2 = src + 2 * width;
	unsigned char* row3 = src + 3 * width;
	unsigned char* row4 = src + 4 * width;
	unsigned char* row5 = src + 5 * width;
	unsigned char* row6 = src + 6 * width;

	// image too small to obtain 
	if (height < 7 || width < 7) {
		dst = src;
		return;
	}
	// normal size image: >= 7
	for (int i = 3; i < height - 3; i++) {
		for (int j = 0; j < width; j++) {
			horizontal[j] = row0[j] + row6[j] + 6 * (row1[j] + row5[j]) + 15 * (row2[j] + row4[j]) + 20 * row3[j];
		}
		for (int k = 0; k < width - 6; k++) {
			dst[i * width + k + 3] = (horizontal[k] + 6 * horizontal[k + 1] + 15 * horizontal[k + 2] + 20 * horizontal[k + 3]
				+ 15 * horizontal[k + 4] + 6 * horizontal[k + 5] + horizontal[k + 6]) >>12; //除以4096（2^12)归一化
		}
		row0 = row1;
		row1 = row2;
		row2 = row3;
		row3 = row4;
		row4 = row5;
		row5 = row6;
		row6 += width;
	}
	delete[] horizontal;

} // End of GaussianBlur


typedef struct {
	VSNodeRef *node;
	const VSVideoInfo *vi;
} FilterData;

static void VS_CC filterInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	FilterData *d = (FilterData *)* instanceData;
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC filterGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {

	FilterData *d = (FilterData *)* instanceData;

	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		//用户添加的code
		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		const VSFormat *fi = d->vi->format;
		int height = vsapi->getFrameHeight(src, 0);
		int width = vsapi->getFrameWidth(src, 0);
		VSFrameRef *dst = vsapi->newVideoFrame(fi, width, height, src, core);
		int plane;
		for (plane = 0; plane < fi->numPlanes; plane++)
		//if(plane == 0)
		{
			const uint8_t *srcp = vsapi->getReadPtr(src, plane);
			//int src_stride = vsapi->getStride(src, plane);
			uint8_t *dstp = vsapi->getWritePtr(dst, plane);
			//int dst_stride = vsapi->getStride(dst, plane); // note that if a frame has the same dimensions and format, the stride is guaranteed to be the same. int dst_stride = src_stride would be fine too in this filter.
			// Since planes may be subsampled you have to query the height of them individually
			int h = vsapi->getFrameHeight(src, plane);
			//int y;
			int w = vsapi->getFrameWidth(src, plane);
			//int x;

			GaussianBlur((unsigned char*)srcp, dstp, h, w);
		}

		// Release the source frame
		vsapi->freeFrame(src);
		// Loop over all the planes
		return dst;
	}

	return 0;
}

static void VS_CC filterFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
	FilterData *d = (FilterData *)instanceData;
	vsapi->freeNode(d->node);
	free(d);
}

static void VS_CC filterCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	FilterData d;
	FilterData *data;

	d.node = vsapi->propGetNode(in, "clip", 0, 0);
	d.vi = vsapi->getVideoInfo(d.node);

	data = (FilterData *)malloc(sizeof(d));
	*data = d;

	vsapi->createFilter(in, out, "Filter", filterInit, filterGetFrame, filterFree, fmParallel, 0, data, core);
}

//////////////////////////////////////////
// Init

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
	configFunc("com.example.blur", "blur", "VapourSynth Filter Blur", VAPOURSYNTH_API_VERSION, 1, plugin);
	registerFunc("GaussianBlur", "clip:clip;", filterCreate, 0, plugin);
}
