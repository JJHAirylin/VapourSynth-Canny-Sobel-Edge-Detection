#include <stdlib.h>
#include <Windows.h>
#include "VapourSynth.h"
#include "VSHelper.h"

typedef struct {
	VSNodeRef *node;
	const VSVideoInfo *vi;
} FilterData;

static void VS_CC filterInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
	FilterData *d = (FilterData *)* instanceData;
	vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC filterGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {

	//�û���ӵ�code
	//Ҫ�������ⲿdll�еĺ���,�ȶ���һ��ָ��̬�⺯��ָ�������
	typedef void(*pCannyEdgeDetect)(unsigned char* yuvIn, int w, int h, unsigned char* yuvOut);

	FilterData *d = (FilterData *)* instanceData;

	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {

		const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);
		const VSFormat *fi = d->vi->format;
		int h = vsapi->getFrameHeight(src, 0);
		int w = vsapi->getFrameWidth(src, 0);
		int bufLen = w*h * 3 / 2;
		unsigned char* input = new unsigned char[bufLen]; //�����yuv��ʽ֡ͼ������
		unsigned char* output = new unsigned char[bufLen]; //�����yuv��ʽ֡ͼ������
		
		VSFrameRef *dst = vsapi->newVideoFrame(fi, w, h, src, core);
		
		int k;
		//��y��u��v����ƽ���ȡyuv����
		//plane=0
		const uint8_t *srcp = vsapi->getReadPtr(src, 0);
		for(k=0; k<w*h; k++) {input[k] = srcp[k];}
		//plane=1
		srcp = vsapi->getReadPtr(src, 1);
		for(k=0; k<w*h/4; k++) {input[w*h+k] = srcp[k];}
		//plane=2
		srcp = vsapi->getReadPtr(src, 2);
		for(k=0; k<w*h/4; k++) {input[w*h*5/4+k] = srcp[k];}

		//���ض�̬���ӿ�dll�ļ���ʹ�þ���·����
		HINSTANCE hDLL = LoadLibraryEx("C:/Windows/System32/dllfolder/ExternalDll.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH); 
		pCannyEdgeDetect Canny;
		Canny = pCannyEdgeDetect(GetProcAddress(hDLL, "CannyEdgeDetect")); //��ȡdll�еĺ���
		if ((hDLL != NULL) && (Canny != NULL)) {
			Canny(input, w, h, output);
		}//��ȡ�����ɹ����������˾�����������ֱ�����ԭ��Ƶ
		else memcpy(output, input, bufLen * sizeof(unsigned char));

		//��y��u��v����ƽ�����yuv����
		//plane=0
		uint8_t *dstp = vsapi->getWritePtr(dst, 0);
		for (k = 0; k < w*h; k++) { dstp[k] = output[k]; }
		//plane=1
		dstp = vsapi->getWritePtr(dst, 1);
		for (k = 0; k < w*h / 4; k++) { dstp[k] = 127; }
		//plane=2
		dstp = vsapi->getWritePtr(dst, 2);
		for (k = 0; k < w*h / 4; k++) { dstp[k] = 127; }


		// Release the source frame
		vsapi->freeFrame(src);
		FreeLibrary(hDLL);
		delete[] output;
		delete[] input;
		
		// A reference is consumed when it is returned, so saving the dst reference somewhere
		// and reusing it is not allowed.
		return dst;

	}//end of CannyFilter
	
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

	vsapi->createFilter(in, out, "Canny", filterInit, filterGetFrame, filterFree, fmParallel, 0, data, core);
}

//////////////////////////////////////////
// Init

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
	configFunc("com.example.edge", "edge", "VapourSynth Filter Example", VAPOURSYNTH_API_VERSION, 1, plugin);
	registerFunc("Canny", "clip:clip;", filterCreate, 0, plugin);
}