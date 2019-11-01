#pragma once

#ifdef CREATEDLL_EXPORTS  
#define CREATEDLL_API __declspec(dllexport)  
#else  
#define CREATEDLL_API __declspec(dllimport)  
#endif  

extern "C" _declspec(dllexport)//µ¼³ö
void CannyEdgeDetect(unsigned char* yuvIn, int w, int h, unsigned char* yuvOut);