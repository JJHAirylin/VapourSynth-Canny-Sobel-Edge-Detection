//////////////////////////////////////////
// This file contains a simple filter
// skeleton you can use to get started.
// With no changes it simply passes
// frames through.

#include <stdlib.h>
#include "VapourSynth.h"
#include "VSHelper.h"


//sobel算子
void Sobel(unsigned char *pIn, int width, int height, unsigned char *pOut)
{

	//每行像素所占字节数，输出图像与输入图像相同
	//int lineByte = (width + 3) / 4 * 4;
	//申请输出图像缓冲区
	//pOut = new unsigned char[height];
	//循环变量，图像的坐标
	int i, j;
	//中间变量
	int x, y, t;
	//Sobel算子
	for (i = 1; i<height - 1; i++)
	{
		for (j = 1; j<width - 1; j++)
		{
			//x方向梯度
			x = pIn[(i - 1)*width + j + 1]
				+ 2 * pIn[i*width + j + 1]
				+ pIn[(i + 1)*width + j + 1]
				- pIn[(i - 1)*width + j - 1]
				- 2 * pIn[i*width + j - 1]
				- pIn[i*width + 1+ j - 1];

			//y方向梯度
			y = pIn[(i - 1)*width + j - 1]
				+ 2 * pIn[i*width - 1+ j]
				+ pIn [(i - 1)*width + j + 1]
				- pIn[(i + 1)*width + j - 1]
				- 2 * pIn[i*width + 1+ j]
				- pIn[(i + 1)*width + j + 1];

			t = abs(x) + abs(y) + 0.5;
			if (t>80)
			{
				pOut[ i * width + j]= 255;
			}
			else
			{
				pOut[ i * width + j]= 0;
			}
		}
	}
	for (j = 0; j<width; j++)
	{
		pOut[(height - 1) * width + j] = 0;//补齐最后一行
		pOut[j] = 0;//补齐第一行
	}
	for (i = 0; i<height; i++)
	{
		pOut[i * width] = 0;//补齐第一列
		pOut[(i+1) * width - 1] = 0;//补齐最后一列
	}
}
//end of sobel

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
		for (plane = 0; plane < fi->numPlanes; plane++) {
			const uint8_t *srcp = vsapi->getReadPtr(src, plane);
			int src_stride = vsapi->getStride(src, plane);
			uint8_t *dstp = vsapi->getWritePtr(dst, plane);
			int dst_stride = vsapi->getStride(dst, plane); // note that if a frame has the same dimensions and format, the stride is guaranteed to be the same. int dst_stride = src_stride would be fine too in this filter.
			// Since planes may be subsampled you have to query the height of them individually
			int h = vsapi->getFrameHeight(src, plane);
			int y;
			int w = vsapi->getFrameWidth(src, plane);
			int x;

			if(plane == 0)Sobel((unsigned char *)srcp, w, h, dstp);
			else{
				for (y = 0; y < h; y++) {
					for (x = 0; x < w; x++) {
						dstp[x] = 127;
					}
					dstp += dst_stride;
					srcp += src_stride;
				}
			}
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

	data = (FilterData *) malloc(sizeof(d));
	*data = d;

	vsapi->createFilter(in, out, "Filter", filterInit, filterGetFrame, filterFree, fmParallel, 0, data, core);
}

//////////////////////////////////////////
// Init

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
	configFunc("com.example.filter", "filter", "VapourSynth Filter Skeleton", VAPOURSYNTH_API_VERSION, 1, plugin);
	registerFunc("Sobel", "clip:clip;", filterCreate, 0, plugin);
}
