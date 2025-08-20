#pragma once

#include <wtypes.h>


#ifdef PSDTHUMBNAILPROVIDER_EXPORTS
#define PSDTP_API EXTERN_C __declspec(dllexport)
#else
#define PSDTP_API __declspec(dllimport)
#endif

#define LONG_PATH 32767
#define PSD_SHELL_CLSID L"{D723B06D-F1DC-47D7-B66E-2AD9C615366F}"

static constexpr GUID CLSID_PSDThumbnailProvider =
{ 0xd723b06d, 0xf1dc, 0x47d7, {0xb6, 0x6e, 0x2a, 0xd9, 0xc6, 0x15, 0x36, 0x6f} };