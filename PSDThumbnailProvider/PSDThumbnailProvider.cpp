#include "pch.h"

#include <thumbcache.h>
#include <Shlwapi.h>
#include "PSDThumbnailProvider.h"
#include "PSDReader.h"
#include <Magick++.h>

using namespace std;

extern long g_cRefDll;
extern HMODULE g_hModule;
bool g_magickInitialized = false;

PSDThumbnailProvider::PSDThumbnailProvider() {
    InterlockedIncrement(&g_cRefDll);

    this->m_ref = 1;
    this->m_stream = nullptr;

    if (!g_magickInitialized) {
        char dllPath[MAX_PATH];
        GetModuleFileNameA(g_hModule, dllPath, MAX_PATH);
        // MessageBoxA(NULL, dllPath, "Debug", MB_OK);
        Magick::InitializeMagick(dllPath);
        g_magickInitialized = true;
    }
}
PSDThumbnailProvider::~PSDThumbnailProvider() {
    InterlockedDecrement(&g_cRefDll);
    if (this->m_stream)
        this->m_stream->Release();
}

IFACEMETHODIMP PSDThumbnailProvider::QueryInterface(REFIID riid, void** ppvObject) {
    static const QITAB qit[] =
    {
        QITABENT(PSDThumbnailProvider, IThumbnailProvider),
        QITABENT(PSDThumbnailProvider, IInitializeWithStream),
        {0}
    };
    return QISearch(this, qit, riid, ppvObject);
}
IFACEMETHODIMP_(ULONG) PSDThumbnailProvider::AddRef() {
    return InterlockedIncrement(&m_ref);
}
IFACEMETHODIMP_(ULONG) PSDThumbnailProvider::Release() {
    const ULONG ref = InterlockedDecrement(&m_ref);
    if (ref == 0)
        delete this;

    return ref;
}

// IInitializeWithStream
IFACEMETHODIMP PSDThumbnailProvider::Initialize(IStream* pstream, DWORD grfMode)
{
    if (!pstream)
        return E_INVALIDARG;
    if (this->m_stream)
        return E_UNEXPECTED;

    HRESULT hr = pstream->QueryInterface(&m_stream);
    return hr;
}

// IThumbnailProvider
IFACEMETHODIMP PSDThumbnailProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
    PSDReader psdReader;
    HRESULT hr = psdReader.CreateThumbnailBitmap(
        this->m_stream, cx, cx, phbmp, pdwAlpha);
    return hr;

    /*MessageBoxW(NULL, to_wstring(cx).data(), L"Debug", MB_OK);

    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP hBmp = CreateCompatibleBitmap(GetDC(nullptr), cx, cx);
    SelectObject(hdc, hBmp);
    HBRUSH hBrush = CreateSolidBrush(RGB(123, 123, 255));
    RECT rect = { 0, 0, (LONG)cx, (LONG)cx };

    FillRect(hdc, &rect, hBrush);
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, 5, 5, L"HELLO", 5);
    DeleteObject(hBrush);
    DeleteDC(hdc);

    *phbmp = hBmp;
    *pdwAlpha = WTSAT_RGB;
    return S_OK;*/
}