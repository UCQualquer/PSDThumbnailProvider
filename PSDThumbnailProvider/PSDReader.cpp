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

HRESULT PSDReader::CreateThumbnailBitmap(
	IStream* pstream,
	UINT maxWidth, UINT maxHeigth,
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
	std::cout << "Max dimensions: " << maxWidth << "x" << maxHeigth << std::endl;

	// Get stream length
	size_t streamLength;
	HRESULT hr = getStreamLength(pstream, &streamLength);
	if (FAILED(hr)) return hr;

	// Read from stream until end
	std::vector<unsigned char> buffer(streamLength);
	ULONG bytesRead = 0;
	while (bytesRead < streamLength) {
		ULONG chunk = 0;
		HRESULT hr = pstream->Read(
			buffer.data() + bytesRead,
			streamLength - bytesRead,
			&chunk);
		if (FAILED(hr) || chunk == 0) break;
		bytesRead += chunk;
	}

	if (bytesRead != streamLength) {
		return E_FAIL; // Truncated is bad
	}

	// Stream length / bytes read
	std::cout << "Stream length: " << streamLength << " |Bytes read: " << bytesRead << std::endl;

	// Load ImageMagick's Image from the buffer
	Magick::Image magickImage;
	Magick::Geometry geometry = Magick::Geometry(maxWidth, maxHeigth);
	geometry.aspect(false);
	Magick::Blob blob = Magick::Blob(buffer.data(), bytesRead);
	
	std::cout << "Ellapsed reading stream to blob [ms]: " << since(begin).count() << std::endl;
	begin = std::chrono::steady_clock::now();

	// Blob size
	std::cout << "Blob size: " << blob.length() << std::endl;

	try {
		magickImage.read(blob, geometry); // Giving geometry here seems to not change a thing
	}
	catch (Magick::Exception&) {
		return E_FAIL;
	}

	std::cout << "Ellapsed reading from blob [ms]: " << since(begin).count() << std::endl;
	begin = std::chrono::steady_clock::now();
	std::cout << "Depth: " << magickImage.depth() << std::endl;
	std::cout << "Format: " << magickImage.format() << std::endl;

	// Original dimensions
	std::cout << "Original dimensions: " << magickImage.columns() << "x" << magickImage.rows() << std::endl;

	// Make thumbnail-sized & write to bit map
	magickImage.thumbnail(geometry);
	magickImage.strip();

	// Export pixels
	const size_t actualWidth = magickImage.columns();
	const size_t actualHeight = magickImage.rows();

	// Actual width x Actual height
	std::cout << "New dimensions: " << magickImage.columns() << "x" << magickImage.rows() << std::endl;

	// Create 32-bit top-down DIB
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = (LONG)actualWidth;
	bmi.bmiHeader.biHeight = -(LONG)actualHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* dibPixels = nullptr;
	HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &dibPixels, NULL, 0);

	if (!hBitmap)
		return E_FAIL;

	try {
		magickImage.write(0, 0, actualWidth, actualHeight, "BGRA", Magick::CharPixel, dibPixels);
	}
	catch (Magick::Exception& ex) {
		std::cerr << "Exception when writing bit map: " << ex.what() << std::endl;
		MessageBoxA(NULL, ex.what(), "Debug", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	std::cout << "Ellapsed writting [ms]: " << since(begin).count() << std::endl;

	*pdwAlpha = WTSAT_ARGB;
	*phbmp = hBitmap;
	return S_OK;
}