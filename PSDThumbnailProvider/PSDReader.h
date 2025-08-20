#pragma once

#include <windows.h>
#include <thumbcache.h>

class PSDReader
{
public:
    PSDReader();
    ~PSDReader();

    HRESULT CreateThumbnailBitmap(IStream* pstream, UINT maxWidth, UINT maxHeigth, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
};