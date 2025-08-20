#include "pch.h"

#include "PSDReader.h"
#include <Magick++.h>


PSDReader::PSDReader() {
}

PSDReader::~PSDReader() {
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
	// Max width / Max height
	MessageBoxW(NULL, (L"MW x MH: " + std::to_wstring(maxWidth) + L" x " + std::to_wstring(maxHeigth)).c_str(), L"Debug", MB_OK);

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
	// MessageBoxW(NULL, (L"SL / BR: " + std::to_wstring(streamLength) + L" / " + std::to_wstring(bytesRead)).c_str(), L"Debug", MB_OK);

	// Load ImageMagick's Image from the buffer
	Magick::Image magickImage;
	Magick::Blob blob = Magick::Blob(buffer.data(), bytesRead);
	
	// Blob size
	// MessageBoxW(NULL, (std::to_wstring(blob.length())).c_str(), L"Debug", MB_OK);

	try {
		magickImage.read(blob);
		// This does not throw an exception, but `magickImage.format()` doesn't work.
		// `magickImage.columns()`, `magickImage.rows()` and `magickImage.thumbnail()` seem to work.
	}
	catch (Magick::Exception&) {
		return E_FAIL;
	}

	/*{
		try {
			Magick::Image image;
			MessageBoxA(NULL, "It works?", "Debug", MB_OK);
			image.read("G:\\test\\test.psd");
			MessageBoxA(NULL, "It works!", "Debug", MB_OK);
		}
		catch (Magick::Exception& ex) {
			MessageBoxA(NULL, ex.what(), "Debug", MB_OK | MB_ICONERROR);
		}
	}
	return S_OK;*/

	// Cols x rows
	// MessageBoxW(NULL, (L"C x R: " + std::to_wstring(magickImage.columns()) + L" x " + std::to_wstring(magickImage.rows())).c_str(), L"Debug", MB_OK);

	// Convert to Bitmap
	Magick::Geometry geometry = Magick::Geometry(maxWidth, maxHeigth);
	geometry.aspect(false);
	magickImage.thumbnail(geometry);
	magickImage.strip();

	// Export pixels
	const size_t actualWidth = magickImage.columns();
	const size_t actualHeight = magickImage.rows();

	// Actual width x Actual height
	// MessageBoxW(NULL, (L"AW x AH: " + std::to_wstring(actualWidth) + L" x " + std::to_wstring(actualHeight)).c_str(), L"Debug", MB_OK);

	std::vector<unsigned char> pixels(actualWidth * actualHeight * 4);
	try {
		magickImage.write(0, 0, actualWidth, actualHeight, "BGRA", Magick::CharPixel, pixels.data());
	}
	catch (Magick::Exception& ex) {
		MessageBoxA(NULL, ex.what(), "Debug", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// Create 32-bit top-down DIB
	BITMAPINFO bmi = {0};
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

	memcpy(dibPixels, pixels.data(), pixels.size());

	*pdwAlpha = WTSAT_ARGB;
	*phbmp = hBitmap;
	return S_OK;
}