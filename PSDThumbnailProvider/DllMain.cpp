#include "pch.h"

#include "Common.h"
#include <string>
#include <thumbcache.h>
#include <ShlObj.h>
#include "ClassFactory.cpp"

using namespace std;

HMODULE g_hModule = nullptr;
long  g_cRefDll = 0;

// DllMain
LSTATUS setRegistryKeyValue(const HKEY hkey, const wstring& key, const wstring& value, const wstring& name)
{
    HKEY reg;
    LSTATUS ls = RegCreateKeyExW(hkey,
        key.data(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE | KEY_WOW64_64KEY,
        NULL,
        &reg,
        NULL);

    if (ls != ERROR_SUCCESS) return ls;

    ls = RegSetKeyValueW(reg, NULL, name.empty() ? NULL : name.data(), REG_SZ, value.data(),
        ((DWORD)value.length()+1) * sizeof(WCHAR));
    
    RegCloseKey(reg);

    return ls;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // DLL loaded into Explorer (or regsvr32.exe when registering)
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        g_hModule = nullptr;
        break;
    }
    return TRUE;
}

// COM server & DLL things
PSDTP_API HRESULT STDAPICALLTYPE DllRegisterServer()
{
    WCHAR szModule[MAX_PATH];
    if (!GetModuleFileNameW(g_hModule, szModule, MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());

    setRegistryKeyValue(HKEY_CLASSES_ROOT, L".psd\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}", PSD_SHELL_CLSID, L"");
    setRegistryKeyValue(HKEY_CLASSES_ROOT, "CLSID\\" PSD_SHELL_CLSID "\\InprocServer32", szModule, L"");
    setRegistryKeyValue(HKEY_CLASSES_ROOT, "CLSID\\" PSD_SHELL_CLSID "\\InprocServer32", L"Apartment", L"ThreadingModel");

    /*setRegistryKeyValue(HKEY_CURRENT_USER, L"Software\\Classes\\.psd\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}", PSD_SHELL_CLSID, L"");
    setRegistryKeyValue(HKEY_CURRENT_USER, "Software\\Classes\\CLSID\\" PSD_SHELL_CLSID "\\InprocServer32", szModule, L"");
    setRegistryKeyValue(HKEY_CURRENT_USER, "Software\\Classes\\CLSID\\" PSD_SHELL_CLSID "\\InprocServer32", L"Apartment", L"ThreadingModel");*/

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return S_OK;
}

PSDTP_API HRESULT STDAPICALLTYPE DllUnregisterServer()
{
    wchar_t buff[MAX_PATH];

    swprintf_s(buff, MAX_PATH, L".psd\\shellex\\%ls", PSD_SHELL_CLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, buff);

    swprintf_s(buff, MAX_PATH, L"CLSID\\%ls\\InprocServer32", PSD_SHELL_CLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, buff);

    swprintf_s(buff, MAX_PATH, L"CLSID\\%ls", PSD_SHELL_CLSID);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, buff);

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return S_OK;
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefDll > 0 ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (ppv == NULL)
        return E_INVALIDARG;

    if (!IsEqualCLSID(CLSID_PSDThumbnailProvider, rclsid))
        return CLASS_E_CLASSNOTAVAILABLE;
    
    ClassFactory* pClassFactory = new(std::nothrow) ClassFactory();
    if (!pClassFactory)
        return E_OUTOFMEMORY;

    HRESULT hr = pClassFactory->QueryInterface(riid, ppv);
    pClassFactory->Release();
    return hr;
}