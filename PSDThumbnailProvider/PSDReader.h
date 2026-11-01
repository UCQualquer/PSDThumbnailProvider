#pragma once

#include <windows.h>
#include <thumbcache.h>

class PSDReader
{
public:
    PSDReader();
    ~PSDReader();

    HRESULT CreateBitmap(IStream* pstream, UINT maxWidth, UINT maxHeight, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
};