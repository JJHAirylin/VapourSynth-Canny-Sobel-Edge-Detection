# README

使用VapourSynth对视频进行非线性编辑

## Software

- Python 3.7
- VapourSynth R45
- VapourSynth Editor (vsedit.exe)
- vspipe.exe 将 \VapourSynth\core64添加至环境变量
- ffmpeg or x264
- VS2015

## Plug-in Example

使用滤镜实现转置

1. 将`Plug-in Example\ffms2.dll`插件拷贝到安装目录下的`VapourSynth\plugins64`文件夹内。

2. 利用vsedit.exe 编写脚本：将视频源 (input.m2ts) 读入，并使用VapourSynth 的std 内建函数
   Transpose 对其进行转置处理。脚本代码：

   ```
   import vapoursynth as vs
   core = vs.get_core()
   
   clip = core.ffms2.Source(source='input.m2ts')
   clip = core.std.Transpose(clip)
   
   clip.set_output()
   ```

3. 选择任意一种指令将码流pipe 到下游编码器编码。

   ```
   vspipe.exe --y4m script.vpy -| ffmpeg.exe -i pipe: trans.mp4
   
   vspipe --y4m scritpt.vpy - | ffmpeg -i pipe: trans.mkv
   
   vspipe --y4m script.vpy - | x264 --demuxer y4m - --output encoded.h264
   ```

## Coding your own plug-in

VapourSynth 的核心库是由C++编写的，因此新插件的开发也使用C/C++。VapourSynth 对视频的处理是大多数是逐帧进行的，本实验编写的插件都是以Vapoursynth 安装目录 `\sdk\examples` 目录下的`invert_example.c` 为模板，使用visual studio 2015 编译动态库dll。

1. 将`VapourSynth/sdk/include/vapoursynth`中的头文件`VapourSynth.h`和`VSHelper.h`放在待编译的`invert_example.c`（或其他cpp文件）同一目录下，在VS2015中编译生成dll

2. 将生成好的插件拷贝到`VapourSynth\plugins64`目录下即可调用。插件需要留出供VapourSynth调用的API，具体实现参见`invert_example.c`文件的最下方，使用`core.invert.Filter(clip)` 即可调用。

## Results

`Filter_Canny`: 实现canny算子的实时视频边缘检测

`Filter_Sobel`: 实现Sobel算子的实时视频边缘检测，可以选择是否先进行高斯滤波

`result video`: 效果演示视频