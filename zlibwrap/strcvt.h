#ifndef __STRCVT_H__ 
#define __STRCVT_H__ 1

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// Convert BSTR string to ANSI string. 
// After using the result, we must free the returned buffer by free(p).
extern char * WideStringToUtf8String(LPCWSTR src);
extern char * WideStringToAnsiString(LPCWSTR src);

// Convert ANSI string to wide char string. 
// After using the result, we must free the returned buffer by free(p).
extern LPWSTR Utf8StringToWideString(const char * src);
extern LPWSTR AnsiStringToWideString(const char * src);

// Convert ANSI string to BSTR string. 
// After using the result, we must free the returned buffer by SysFreeString.
extern HRESULT Utf8StringToBstr(const char * src, BSTR *out);




#ifdef __cplusplus
}
#endif

#endif  // __STRCVT_H__

