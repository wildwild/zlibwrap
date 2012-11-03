//------------------------------------------------------------------------------
//
//    Copyright (C) Ralph. All rights reserved.
//
//    File Name:   ZLibWrapLib.h
//    Author:      Ralph
//    Create Time: 2010-09-14
//    Description: 
//
//    Version history:
//
//------------------------------------------------------------------------------

#ifndef __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__
#define __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__


#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Description: Compress files to a ZIP file.
// Parameter: lpszSourceFiles Source files, supporting wildcard.
// Parameter: lpszDestFile The ZIP file path.
// Parameter: bUtf8 If using UTF-8 to encode the file name.
// Return Value: TRUE/FALSE.
//------------------------------------------------------------------------------
BOOL ZipCompress(LPCWSTR lpszSourceFiles, LPCWSTR lpszDestFile, BOOL bUtf8);

//------------------------------------------------------------------------------
// Description: Extract files from a ZIP file.
// Parameter: lpszSourceFile Source ZIP file.
// Parameter: lpszDestFolder The folder to output files. The parent of the
//                           specified folder MUST exist.
// Return Value: TRUE/FALSE.
//------------------------------------------------------------------------------
BOOL ZipExtract(LPCWSTR lpszSourceFile, LPCWSTR lpszDestFolder);


typedef struct ZipGenericContext ZipGenericContext;
typedef BOOL (CDECL *PFN_IsDirectory)(LPCWSTR szFile, ZipGenericContext *context);
typedef BOOL (CDECL *PFN_GetFileAttributes)(LPCWSTR szFile, unsigned int *attributes, unsigned int *dosDate, ZipGenericContext *context);
typedef BOOL (CDECL *PFN_SetFileDateTime)(HANDLE hFile, unsigned int dosDate, ZipGenericContext *context);
typedef HANDLE (CDECL *PFN_OpenFile)(LPCWSTR szFile, BOOL OpenForRead, ZipGenericContext *context);
typedef BOOL (CDECL *PFN_ReadFile)(HANDLE hFile, void *buff, unsigned int numberOfBytesToRead, unsigned int *numberOfBytesRead, ZipGenericContext *context);
typedef BOOL (CDECL *PFN_WriteFile)(HANDLE hFile, void *buff, unsigned int nNumberOfBytesToWrite, unsigned int *lpNumberOfBytesWritten, ZipGenericContext *context);
typedef void (CDECL *PFN_CloseFile)(HANDLE hFile, ZipGenericContext *context);

typedef void * zipFile;
typedef BOOL (CDECL *PFN_EnumDirectory)(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFiles, BOOL bUtf8, ZipGenericContext *context);

typedef BOOL (CDECL *PFN_CreateDirectory)(LPCWSTR lpszDir, ZipGenericContext *context);

typedef struct ZipGenericContext {
    PFN_IsDirectory pfn_IsDirectory;
    PFN_GetFileAttributes pfn_GetFileAttributes;
    PFN_SetFileDateTime pfn_SetFileDateTime;
    PFN_OpenFile pfn_OpenFile;
    PFN_ReadFile pfn_ReadFile;
    PFN_WriteFile pfn_WriteFile;
    PFN_CloseFile pfn_CloseFile;
    PFN_EnumDirectory pfn_EnumDirectory;
    PFN_CreateDirectory pfn_CreateDirectory;
    void * parameter;
} ZipGenericContext;

BOOL ZipCompressRaw(LPCWSTR lpszSourceFiles, LPCWSTR lpszDestFile, BOOL bUtf8, ZipGenericContext *context);
BOOL ZipExtractRaw(LPCWSTR lpszSourceFile, LPCWSTR lpszDestFolder, ZipGenericContext *context);
BOOL ZipAddFilesInternal(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFilePath, LPCWSTR lpszFileName, BOOL bUtf8, BOOL isDir, ZipGenericContext *context);

typedef BOOL (CDECL *PFN_DumpFileContent)(void *data, int len, void*pv);
BOOL ZipExtractSpecifiedFile(LPCWSTR zipFile, LPCWSTR fileInZip, PFN_DumpFileContent callback, void *pv);

#ifdef __cplusplus
}
#endif

#endif // #ifndef __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__
