#ifndef AVI_H
#define AVI_H
#include "iso.h"
typedef struct { unsigned video,audio,width,height,rate,scale,codec,format,hz,channels,frames,audio_blocks,video_bytes,audio_bytes; } AviInfo;
int avi_scan(IsoFile f,AviInfo *a);
#endif
