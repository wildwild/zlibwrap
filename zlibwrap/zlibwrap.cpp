//------------------------------------------------------------------------------
//
//    Copyright (C) Ralph. All rights reserved.
//
//    File Name:   ZLibWrapLib.cpp
//    Author:      Ralph
//    Create Time: 2010-09-16
//    Description: 
//
//    Version history:
//
//------------------------------------------------------------------------------


#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <wtypes.h>
#include "zlibwrap.h"
#include "strcvt.h"
#include "zip.h"
#include "unzip.h"
#include <atlstr.h>
#include <assert.h>

#define ZIP_GPBF_LANGUAGE_ENCODING_FLAG 0x800

BOOL ZipAddFile(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFilePath, BOOL bUtf8, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    char *strFileNameInZipA = NULL;
    BOOL bMustClose = FALSE;
    HANDLE hMyFile = NULL;
    byte * byBuffer = NULL;
    do 
    {
        int nRs = 0;
        unsigned int attr = 0;
        unsigned int dosDate = 0;
        context->pfn_GetFileAttributes(lpszFilePath, &attr, &dosDate, context);

        zip_fileinfo FileInfo;
        memset(&FileInfo, 0, sizeof(FileInfo));

        FileInfo.dosDate = dosDate;
        FileInfo.external_fa = attr;

        if (bUtf8) {
            strFileNameInZipA = WideStringToUtf8String(lpszFileNameInZip);

            nRs = zipOpenNewFileInZip4(zf, strFileNameInZipA, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9,
                0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0, ZIP_GPBF_LANGUAGE_ENCODING_FLAG);
            if (nRs != ZIP_OK) {
                break;
            }
        } else {
            strFileNameInZipA = WideStringToAnsiString(lpszFileNameInZip);
            nRs = zipOpenNewFileInZip(zf, strFileNameInZipA, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9);
            if (nRs != ZIP_OK) {
                break;
            }
        }

        bMustClose = TRUE;

        if (context->pfn_IsDirectory(lpszFilePath, context)) {
            bRs = TRUE;
            break;
        }

        hMyFile = context->pfn_OpenFile(lpszFilePath, TRUE, context);
        if (NULL==hMyFile || INVALID_HANDLE_VALUE==hMyFile) {
            break;
        }

        const unsigned int BUFFER_SIZE = 8192;
        byBuffer = (BYTE *) malloc(BUFFER_SIZE);
        unsigned int dwRead = 0;

        while ( context->pfn_ReadFile(hMyFile, byBuffer, BUFFER_SIZE, &dwRead, context) ) {
            if (0 == dwRead) {
                break;
            }
            if (zipWriteInFileInZip(zf, byBuffer, dwRead) < 0) {
                break;
            }
        }
        bRs = TRUE;
    } while (FALSE);

    if (byBuffer) {
        free(byBuffer);
    }

    if (NULL!=hMyFile && INVALID_HANDLE_VALUE!=hMyFile) {
        context->pfn_CloseFile(hMyFile, context);
    }

    if (bMustClose) {
        zipCloseFileInZip(zf);
    }

    if (strFileNameInZipA) {
        free(strFileNameInZipA);
    }

    return bRs;
}

BOOL ZipAddFiles(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFiles, BOOL bUtf8, ZipGenericContext *context);

BOOL CDECL ZipAddFilesInternal(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFilePath, LPCWSTR lpszFileName, BOOL bUtf8, BOOL isDir, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    WCHAR * strFileNameInZip = NULL;
    WCHAR * strFilePath = NULL;
    do 
    {
        if (0==wcscmp(lpszFileName, L".") || 0==wcscmp(lpszFileName, L"..")) {
            bRs = TRUE;
            break;
        }

        strFileNameInZip = (WCHAR *) malloc((MAX_PATH*2)*sizeof(WCHAR));
        strFileNameInZip[0] = L'\0';
        wcscat(strFileNameInZip, lpszFileNameInZip);
        wcscat(strFileNameInZip, lpszFileName);

        strFilePath = (WCHAR *) malloc((MAX_PATH*2)*sizeof(WCHAR));
        strFilePath[0] = L'\0';
        wcscat(strFilePath, lpszFilePath);
        wcscat(strFilePath, lpszFileName);

        if (isDir) {
            wcscat(strFileNameInZip, L"/");
            bRs = ZipAddFile(zf, strFileNameInZip, strFilePath, bUtf8, context);
            if (!bRs) {
                break;
            }
            wcscat(strFilePath, L"/*");
            bRs = ZipAddFiles(zf, strFileNameInZip, strFilePath, bUtf8, context);
            if (!bRs) {
                break;
            }
        } else {
            bRs = ZipAddFile(zf, strFileNameInZip, strFilePath, bUtf8, context);
            if (!bRs) {
                break;
            }
        }
    } while (FALSE);

    if (strFileNameInZip) {
        free(strFileNameInZip);
    }
    if (strFilePath) {
        free(strFilePath);
    }

    return bRs;
}

BOOL ZipAddFiles(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFiles, BOOL bUtf8, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    bRs = context->pfn_EnumDirectory(zf, lpszFileNameInZip, lpszFiles, bUtf8, context);
    return bRs;
}


BOOL CDECL _IsDirectory(LPCWSTR szFile, ZipGenericContext *context)
{
    DWORD dwFileAttr = GetFileAttributesW(szFile);
    if (dwFileAttr == INVALID_FILE_ATTRIBUTES) {
        return FALSE;
    }

    return ((dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0) ? TRUE : FALSE;
}

BOOL CDECL _GetFileAttributes(LPCWSTR szFile, unsigned int *attributes, unsigned int *dosDate, ZipGenericContext *context)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL bRs = FALSE;
    do 
    {
        if (NULL==szFile) {
            break;
        }
        if (attributes) {
            *attributes = 0;
        }
        if (dosDate) {
            *dosDate = 0;
        }

        int nRs = 0;
        DWORD dwFileAttr = GetFileAttributes(szFile);
        if (dwFileAttr == INVALID_FILE_ATTRIBUTES) {
            break;
        }
        DWORD dwOpenAttr = ((dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0) ? FILE_FLAG_BACKUP_SEMANTICS : 0;
        hFile = CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwOpenAttr, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            break;
        }

        FILETIME ftUTC, ftLocal;

        GetFileTime(hFile, NULL, NULL, &ftUTC);
        FileTimeToLocalFileTime(&ftUTC, &ftLocal);

        WORD wDate, wTime;
        FileTimeToDosDateTime(&ftLocal, &wDate, &wTime);

        if (dosDate) {
            *dosDate = (unsigned int)((((DWORD)wDate) << 16) | (DWORD)wTime);
        }
        if (attributes) {
            *attributes = dwFileAttr;
        }

        bRs = TRUE;
    } while (FALSE);

    if (NULL!=hFile && INVALID_HANDLE_VALUE!=hFile) {
        CloseHandle(hFile);
    }

    return bRs;
}

BOOL CDECL _SetFileDateTime(HANDLE hFile, unsigned int dosDate, ZipGenericContext *context)
{
    FILETIME ftLocal, ftUTC;

    DosDateTimeToFileTime((WORD)(dosDate>>16), (WORD)dosDate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftUTC);
    SetFileTime(hFile, &ftUTC, &ftUTC, &ftUTC);
    return TRUE;
}

HANDLE CDECL _OpenFile(LPCWSTR szFile, BOOL OpenForRead, ZipGenericContext *context)
{
    if (OpenForRead) {
        return CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    } else {
        return CreateFileW(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    }
}

BOOL CDECL _ReadFile(HANDLE hFile, void *buff, unsigned int numberOfBytesToRead, unsigned int *numberOfBytesRead, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    DWORD dwTmp = 0;
    bRs = ReadFile(hFile, buff, numberOfBytesToRead, &dwTmp, NULL);
    if (numberOfBytesRead) {
        *numberOfBytesRead = (unsigned int) dwTmp;
    }
    return bRs;
}

BOOL CDECL _WriteFile(HANDLE hFile, void *buff, unsigned int nNumberOfBytesToWrite, unsigned int *lpNumberOfBytesWritten, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    DWORD dwTmp = 0;
    bRs = WriteFile(hFile, buff, nNumberOfBytesToWrite, &dwTmp, NULL);
    if (lpNumberOfBytesWritten) {
        *lpNumberOfBytesWritten = (unsigned int) dwTmp;
    }
    return bRs;
}

void CDECL _CloseFile(HANDLE hFile, ZipGenericContext *context) {
    CloseHandle(hFile);
}

BOOL CDECL _EnumDirectory(zipFile zf, LPCWSTR lpszFileNameInZip, LPCWSTR lpszFiles, BOOL bUtf8, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    do 
    {
        WIN32_FIND_DATAW wfd = { 0 };
        hFind = FindFirstFileW(lpszFiles, &wfd);
        if (hFind == INVALID_HANDLE_VALUE) {
            break;
        }

        CStringW strFilePath = lpszFiles;
        int nPos = strFilePath.ReverseFind(L'/');

        if (nPos != -1) {
            strFilePath = strFilePath.Left(nPos + 1);
        } else {
            strFilePath.Empty();
        }

        do 
        {
            CStringW strFileName = wfd.cFileName;
            BOOL bIsDir = ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ? TRUE : FALSE;

            bRs = ZipAddFilesInternal(zf, lpszFileNameInZip, strFilePath, strFileName, bUtf8, bIsDir, context);
            if (FALSE == bRs) {
                break;
            }
        } while (FindNextFile(hFind, &wfd));
    } while (FALSE);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }
    return bRs;
}

BOOL ZipExtractCurrentFile(unzFile uf, LPCWSTR lpszDestFolder, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    BOOL CurrentFileOpened = FALSE;
    char * szFilePathA = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BYTE * byBuffer = NULL;
    BOOL bFailed = FALSE;
    WCHAR * strFileName = NULL;
    WCHAR * strDestPath = NULL;
    do 
    {
        unz_file_info64 FileInfo;
        szFilePathA = (char *) malloc(MAX_PATH*4);
        if (NULL == szFilePathA) {
            break;
        }

        if (unzGetCurrentFileInfo64(uf, &FileInfo, 
            szFilePathA, MAX_PATH*4, NULL, 0, NULL, 0) != UNZ_OK)
        {
            break;
        }

        if (unzOpenCurrentFile(uf) != UNZ_OK) {
            break;
        }

        CurrentFileOpened = TRUE;

        strDestPath = (WCHAR *) malloc(MAX_PATH*4);
        wcscpy(strDestPath, lpszDestFolder);

        if ((FileInfo.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0) {
            strFileName = Utf8StringToWideString(szFilePathA);
        } else {
            strFileName = AnsiStringToWideString(szFilePathA);
        }

        int nLength = wcslen(strFileName);
        LPWSTR lpszCurrentFile = strFileName;

        for (int i = 0; i <= nLength; ++i)
        {
            if (strFileName[i] == _T('\0')) {
                wcscat(strDestPath, lpszCurrentFile);
                break;
            }

            if (strFileName[i] == '/') {
                strFileName[i] = '\0';

                wcscat(strDestPath, lpszCurrentFile);
                wcscat(strDestPath, L"/");

                context->pfn_CreateDirectory(strDestPath, context);

                lpszCurrentFile = strFileName + i + 1;
            }
        }

        if (lpszCurrentFile[0] == _T('\0')) {
            bRs = TRUE;
            break;
        }

        hFile = context->pfn_OpenFile(strDestPath, FALSE, context);

        if (hFile == INVALID_HANDLE_VALUE || NULL==hFile) {
            break;
        }

        const DWORD BUFFER_SIZE = 8192;
        byBuffer = (BYTE *) malloc(BUFFER_SIZE);

        while (true)
        {
            int nSize = unzReadCurrentFile(uf, byBuffer, BUFFER_SIZE);

            if (nSize < 0) {
                // error occurred.
                bFailed = TRUE;
                break;
            } else if (nSize == 0) {
                break;
            } else {
                unsigned int dwWritten = 0;
                BOOL bRs = context->pfn_WriteFile(hFile, byBuffer, (unsigned int)nSize, &dwWritten, context);
                if (!bRs || dwWritten != (DWORD)nSize) {
                    bFailed = TRUE;
                    break;
                }
            }
        }

        if (bFailed) {
            break;
        }

        context->pfn_SetFileDateTime(hFile, FileInfo.dosDate, context);

        bRs = TRUE;
    } while (FALSE);

    if (strDestPath) {
        free(strDestPath);
    }

    if (strFileName) {
        free(strFileName);
    }

    if (byBuffer) {
        free(byBuffer);
    }

    if (NULL!=hFile && INVALID_HANDLE_VALUE!=hFile) {
        context->pfn_CloseFile(hFile, context);
    }

    if (CurrentFileOpened) {
        unzCloseCurrentFile(uf);
    }

    if (szFilePathA) {
        free(szFilePathA);
    }

    return bRs;
}

void ConvertPathToUnixStyleW(LPWSTR lpszPath)
{
    if (lpszPath) {
        while(*lpszPath) {
            if (*lpszPath == L'\\') {
                *lpszPath = L'/';
            }
            ++lpszPath;
        }
    }
}

void ConvertPathToUnixStyleA(LPSTR lpszPath)
{
    if (lpszPath) {
        while(*lpszPath) {
            if (*lpszPath == '\\') {
                *lpszPath = '/';
            }
            ++lpszPath;
        }
    }
}

BOOL ZipCompressRaw(LPCWSTR lpszSourceFiles0, LPCWSTR lpszDestFile, BOOL bUtf8, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    zipFile zf = NULL;
    char * strDestFile = NULL;
    WCHAR * lpszSourceFiles = NULL;
    do 
    {
        if (NULL == lpszSourceFiles0) {
            break;
        }
        lpszSourceFiles = (WCHAR *) calloc(wcslen(lpszSourceFiles0)+1, sizeof(WCHAR));
        if (NULL == lpszSourceFiles) {
            break;
        }
        wcscpy(lpszSourceFiles, lpszSourceFiles0);
        ConvertPathToUnixStyleW(lpszSourceFiles);

        strDestFile = WideStringToAnsiString(lpszDestFile);

        zf = zipOpen64(strDestFile, 0);
        if (zf == NULL) {
            break;
        }

        if (!ZipAddFiles(zf, L"", lpszSourceFiles, bUtf8, context)) {
            break;
        }
        bRs = TRUE;
    } while (FALSE);

    if (lpszSourceFiles) {
        free(lpszSourceFiles);
    }

    if (strDestFile) {
        free(strDestFile);
    }

    if (zf) {
        zipClose(zf, (const char *)NULL);
    }

    return bRs;
}

BOOL ZipExtractRaw(LPCWSTR lpszSourceFile, LPCWSTR lpszDestFolder0, ZipGenericContext *context)
{
    BOOL bRs = FALSE;
    char * strSourceFileA = NULL;
    unzFile uf = NULL;
    WCHAR * lpszDestFolder = NULL;
    do 
    {
        strSourceFileA = WideStringToAnsiString(lpszSourceFile);

        if (NULL == lpszDestFolder0) {
            break;
        }
        lpszDestFolder = (WCHAR *) calloc(wcslen(lpszDestFolder0)+1, sizeof(WCHAR));
        wcscpy(lpszDestFolder, lpszDestFolder0);
        ConvertPathToUnixStyleW(lpszDestFolder);

        uf = unzOpen64(strSourceFileA);
        if (uf == NULL) {
            break;
        }

        unz_global_info64 gi;
        if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK) {
            break;
        }

        CStringW strDestFolder = lpszDestFolder;

        context->pfn_CreateDirectory(lpszDestFolder, context);

        if (!strDestFolder.IsEmpty() && strDestFolder[strDestFolder.GetLength() - 1] != _T('/'))
        {
            strDestFolder += _T("/");
        }

        for (int i = 0; i < gi.number_entry; ++i) {
            if (!ZipExtractCurrentFile(uf, strDestFolder, context)) {
                return FALSE;
            }

            if (i < gi.number_entry - 1) {
                if (unzGoToNextFile(uf) != UNZ_OK) {
                    return FALSE;
                }
            }
        }
        bRs = TRUE;
    } while (FALSE);

    if (uf) {
        unzClose(uf);
    }

    if (lpszDestFolder) {
        free(lpszDestFolder);
    }

    if (strSourceFileA) {
        free(strSourceFileA);
    }

    return bRs;
}

BOOL CDECL _CreateDirectory(LPCWSTR lpszDir, ZipGenericContext *context)
{
    return CreateDirectoryW(lpszDir, NULL);
}

void InitContext(ZipGenericContext * context)
{
    memset(context, 0, sizeof(ZipGenericContext));
    context->pfn_IsDirectory = _IsDirectory;
    context->pfn_GetFileAttributes = _GetFileAttributes;
    context->pfn_SetFileDateTime = _SetFileDateTime;
    context->pfn_OpenFile = _OpenFile;
    context->pfn_ReadFile = _ReadFile;
    context->pfn_WriteFile = _WriteFile;
    context->pfn_CloseFile = _CloseFile;
    context->pfn_EnumDirectory = _EnumDirectory;
    context->pfn_CreateDirectory = _CreateDirectory;
}

BOOL ZipCompress(LPCWSTR lpszSourceFiles, LPCWSTR lpszDestFile, BOOL bUtf8)
{
    ZipGenericContext context;
    InitContext(&context);
    return ZipCompressRaw(lpszSourceFiles, lpszDestFile, bUtf8, &context);
}

BOOL ZipExtract(LPCWSTR lpszSourceFile, LPCWSTR lpszDestFolder)
{
    ZipGenericContext context;
    InitContext(&context);
    return ZipExtractRaw(lpszSourceFile, lpszDestFolder, &context);
}


#define WRITEBUFFERSIZE (0x500000) // 5Mb buffer

BOOL ZipExtractSpecifiedFile(LPCWSTR zipFile, LPCWSTR fileInZip, PFN_DumpFileContent callback, void *pv)
{
    BOOL bRs = FALSE;
    int err = UNZ_OK;                 // error status
    uInt size_buf = WRITEBUFFERSIZE;  // byte size of buffer to store raw csv data
    void* buf = NULL;                 // the buffer  
    char filename_inzip[256];         // for unzGetCurrentFileInfo
    unz_file_info file_info;          // for unzGetCurrentFileInfo 
    char * strSourceFileA = NULL;
    char * fileInZipA = NULL;
    BOOL isFailed = FALSE;
    BOOL mustCloseFile = FALSE;
    unzFile uf = NULL;
    do 
    {
        if (NULL==zipFile || NULL==fileInZip || NULL==callback) {
            break;
        }

        strSourceFileA = WideStringToAnsiString(zipFile);
        fileInZipA = WideStringToAnsiString(fileInZip);

        ConvertPathToUnixStyleA(strSourceFileA);
        ConvertPathToUnixStyleA(fileInZipA);

        uf = unzOpen(strSourceFileA); // open zip-file stream
        if (uf==NULL) {
            break;
        } // file is open

        if ( unzLocateFile(uf,fileInZipA,1) ) { // try to locate file inside zip
            // second argument of unzLocateFile: 1 = case sensitive, 0 = case-insensitive
            break;
        } // file inside zip found

        if (unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0)) {
            break;
        } // obtained the necessary details about file inside zip

        err = unzOpenCurrentFilePassword(uf, NULL); // Open the file inside the zip (password = NULL)
        if (err!=UNZ_OK) {
            break;
        } // file inside the zip is open

        mustCloseFile = TRUE;

        buf = (void*)malloc(size_buf); // setup buffer
        if (buf==NULL) {
            break;
        } // buffer ready

        // Copy contents of the file inside the zip to the buffer
        do {
            err = unzReadCurrentFile(uf,buf,size_buf);
            if (err<0) {
                isFailed = TRUE;
                break;
            } else if (0 == err) {
                break;
            }
            if (FALSE == callback(buf, err, pv)) {
                break;
            }
        } while (err>0);

        if (isFailed) {
            break;
        }
        bRs = TRUE;
    } while (FALSE);

    if (mustCloseFile) {
        err = unzCloseCurrentFile (uf);  // close the zip-file
    }

    if (uf) {
        unzClose(uf);
    }

    if (fileInZipA) {
        free(fileInZipA);
    }

    if (strSourceFileA) {
        free(strSourceFileA);
    }

    if (buf) {
        free(buf); // free up buffer memory
    }
    return bRs;
}
