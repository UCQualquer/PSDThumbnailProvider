#pragma once

#include <windows.h>
#include <thumbcache.h>
#include <string>

class PSDThumbnailProvider :
    public IThumbnailProvider,
    public IInitializeWithStream
{
public:
    PSDThumbnailProvider();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream* pstream, DWORD grfMode);

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);

protected:
    ~PSDThumbnailProvider();

private:
    ULONG m_ref;
    IStream* m_stream;
};