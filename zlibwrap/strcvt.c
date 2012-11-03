#include <windows.h>

#include "strcvt.h"


//=========================================================================
// WideStringToUtf8String
// 
// Convert BSTR string to ANSI string. 
// After using the result, we must free the returned buffer by free(p).
// 
// usage:
//      char * p = WideStringToUtf8String(src);
//      free(p);
//=========================================================================
char * WideStringToUtf8String(LPCWSTR src)
{
    char * pBuff = NULL;
    size_t nLen = 0;
    int nAnsi = 0;
    int nAnsi2 = 0;
    do 
    {
        if (NULL == src) {
            break;
        }
        nLen = wcslen(src);
        nAnsi = WideCharToMultiByte(CP_UTF8, 0, src, nLen+1, NULL, 0, NULL, NULL);
        if (nAnsi <= 0) {
            break;
        }
        pBuff = (char *) malloc(nAnsi*sizeof(char));
        if (NULL == pBuff) {
            break;
        }
        nAnsi2 = WideCharToMultiByte(CP_UTF8, 0, src, nLen+1, pBuff, nAnsi, NULL, NULL);
        if (nAnsi2 != nAnsi) {
            free(pBuff);
            pBuff = NULL;
        }
    } while (FALSE);
    return pBuff;
}

char * WideStringToAnsiString(LPCWSTR src)
{
    char * pBuff = NULL;
    size_t nLen = 0;
    int nAnsi = 0;
    int nAnsi2 = 0;
    do 
    {
        if (NULL == src) {
            break;
        }
        nLen = wcslen(src);
        nAnsi = WideCharToMultiByte(CP_ACP, 0, src, nLen+1, NULL, 0, NULL, NULL);
        if (nAnsi <= 0) {
            break;
        }
        pBuff = (char *) malloc(nAnsi*sizeof(char));
        if (NULL == pBuff) {
            break;
        }
        nAnsi2 = WideCharToMultiByte(CP_ACP, 0, src, nLen+1, pBuff, nAnsi, NULL, NULL);
        if (nAnsi2 != nAnsi) {
            free(pBuff);
            pBuff = NULL;
        }
    } while (FALSE);
    return pBuff;
}


// Convert ANSI string to wide char string. 
// After using the result, we must free the returned buffer by free(p).
LPWSTR Utf8StringToWideString(const char * src)
{
    LPWSTR pBuff = NULL;
    size_t nLen = strlen(src);
    int nWide = 0;
    nWide = MultiByteToWideChar(CP_UTF8, 0, src, nLen+1, NULL, 0);
    if (nWide > 0) {
        pBuff = (LPWSTR) malloc(nWide*sizeof(WCHAR));
        if (pBuff) {
            int nWide2 = MultiByteToWideChar(CP_UTF8, 0, src, nLen+1, pBuff, nWide);
            if (nWide == nWide2) {
                goto __EXIT_POINT__;
            }
            free(pBuff);
        }
    }
__EXIT_POINT__:
    return pBuff;
}

LPWSTR AnsiStringToWideString(const char * src)
{
    LPWSTR pBuff = NULL;
    size_t nLen = strlen(src);
    int nWide = 0;
    nWide = MultiByteToWideChar(CP_ACP, 0, src, nLen+1, NULL, 0);
    if (nWide > 0) {
        pBuff = (LPWSTR) malloc(nWide*sizeof(WCHAR));
        if (pBuff) {
            int nWide2 = MultiByteToWideChar(CP_ACP, 0, src, nLen+1, pBuff, nWide);
            if (nWide == nWide2) {
                goto __EXIT_POINT__;
            }
            free(pBuff);
        }
    }
__EXIT_POINT__:
    return pBuff;
}


// Convert ANSI string to BSTR string. 
// After using the result, we must free the returned buffer by SysFreeString.
HRESULT Utf8StringToBstr(const char * src, BSTR *out)
{
    HRESULT hr = E_FAIL;
    do 
    {
        LPWSTR pTmp = NULL;
        if (NULL==src || NULL==out) {
            break;
        }
        pTmp = Utf8StringToWideString(src);
        if (NULL == pTmp) {
            break;
        }
        *out = SysAllocString(pTmp);
        free(pTmp);
        hr = S_OK;
    } while (FALSE);
    return hr;
}
