#include "pch.h"
#include "PSDReader.h"
#include <Magick++.h>

#include <chrono> // DEBUG
#include <iostream>
#include <thread>


PSDReader::PSDReader() {
}

PSDReader::~PSDReader() {
}

template <
	class result_t = std::chrono::milliseconds,
	class clock_t = std::chrono::steady_clock,
	class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
	return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

HRESULT getStreamLength(IStream* pstream, size_t* pLength) {
	STATSTG stat;
	HRESULT hr = pstream->Stat(&stat, STATFLAG_DEFAULT);
	*pLength = stat.cbSize.QuadPart;

	return hr;
}

HRESULT PSDReader::CreateBitmap(
	IStream* pstream,
	UINT maxWidth, UINT maxHeight,
	HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
	auto begin = std::chrono::steady_clock::now();

	// Create console for debugging
	/*AllocConsole();
	FILE* pConsole;
	freopen_s(&pConsole, "CONOUT$", "w", stdout);
	freopen_s(&pConsole, "CONOUT$", "w", stderr);
	freopen_s(&pConsole, "CONIN$", "r", stdin);*/

	// Max width / Max height
	std::cout << "Max dimensions: " << maxWidth << "x" << maxHeight << std::endl;

	begin = std::chrono::steady_clock::now();
	// Get stream length
	size_t psdLength;
	HRESULT hr = getStreamLength(pstream, &psdLength);
	if (FAILED(hr)) return hr;

	// Read from stream until end
	std::vector<unsigned char> psdBytes(psdLength);
	ULONG bytesRead = 0;
	while (bytesRead < psdLength) {
		ULONG chunk = 0;
		HRESULT hr = pstream->Read(
			psdBytes.data() + bytesRead,
			psdLength - bytesRead,
			&chunk);
		if (FAILED(hr) || chunk == 0) break;
		bytesRead += chunk;
	}

	if (bytesRead != psdLength) {
		return E_FAIL; // Truncated is bad
	}
	std::cout << "Ellapsed until after stream read: " << since(begin).count() << std::endl;

	// Create image
	Magick::Geometry geometry(maxWidth, maxHeight);
	geometry.greater(true);
	geometry.aspect(false);
	Magick::Blob psdBlob(psdBytes.data(), psdLength);
	std::cout << "Ellapsed until after blob created: " << since(begin).count() << std::endl;
	Magick::Image image;
	image.scene(0);
	image.subImage(0);
	image.subRange(1);
	try {
		image.read(psdBlob, geometry, "PSD");
	}
	catch (Magick::Exception& ex) {
		MessageBoxA(NULL, ex.what(), "Debug", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	std::cout << "Ellapsed until after image created: " << since(begin).count() << std::endl;;

	// Create bitmap
	size_t width = image.columns();
	size_t height = image.rows();
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = static_cast<LONG>(width);
	bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = nullptr;
	HDC hdc = GetDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdc);

	if (!hBitmap || !pBits)
		return E_FAIL;

	try {
		image.write(0, 0, width, height, "BGRA", Magick::CharPixel, pBits);
	}
	catch (Magick::Exception& ex) {
		MessageBoxA(NULL, ex.what(), "Debug", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	*pdwAlpha = WTSAT_ARGB;
	*phbmp = hBitmap;
	return S_OK;
}
