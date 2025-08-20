#include "pch.h"

#include "PSDThumbnailProvider.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

extern long g_cRefDll;

class ClassFactory : public IClassFactory
{
private:
    ULONG m_ref = 1;

protected:
    ~ClassFactory() {
        InterlockedDecrement(&g_cRefDll);
    }

public:
    ClassFactory() {
        InterlockedIncrement(&g_cRefDll);
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(ClassFactory, IClassFactory),
            { 0 }
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&m_ref);
    }
    IFACEMETHODIMP_(ULONG) Release() {
        const ULONG ref = InterlockedDecrement(&m_ref);
        if (ref == 0)
            delete this;

        return ref;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
    {
        if (pUnkOuter == nullptr)
        {
            PSDThumbnailProvider* thp = new (std::nothrow) PSDThumbnailProvider();
            if (!thp)
                return E_OUTOFMEMORY;
            
            HRESULT hr = thp->QueryInterface(riid, ppv);
            thp->Release();
            return hr;
        }
        return CLASS_E_NOAGGREGATION;
    }

    IFACEMETHODIMP LockServer(BOOL fLock) {
        if (fLock)
            InterlockedIncrement(&g_cRefDll);
        else
            InterlockedDecrement(&g_cRefDll);
        return S_OK;
    }
};