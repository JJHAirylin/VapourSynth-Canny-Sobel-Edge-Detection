# ReadMe

##### 实现canny边缘检测功能

`ExternalDll`：引用 opencv 生成外部dll，将`ExternalDll.dll`放在系统目录`C:\Windows\System32\dllfolder`下。相关代码`CannyEdgeDetect.cpp`，`ExternalDll.h`

`ExternalDllTest.dll`：放在 `VapourSynth\plugins64` 路径下调用canny边缘检测功能。相关代码`CannyFilter.cpp`

`filter-3.vpy`：逐帧处理脚本代码