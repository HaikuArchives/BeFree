/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: File.cpp
 *
 * --------------------------------------------------------------------------*/

#ifndef _WIN32
#define __USE_LARGEFILE64
#define __USE_FILE_OFFSET64
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif // _WIN32

#include <support/String.h>

#include "Path.h"
#include "File.h"

extern status_t path_expound(BString &path, const char *dir, const char *leaf, bool *normalize);

#ifndef _WIN32
inline int file_openmode_to_flags(uint32 open_mode)
{
	int flags;

	if (open_mode &B_READ_WRITE) flags = O_RDWR;
	else if (open_mode &B_WRITE_ONLY) flags = O_WRONLY;
	else flags = O_RDONLY;

	if (open_mode &B_CREATE_FILE) {
		flags |= O_CREAT;
		if (open_mode &B_FAIL_IF_EXISTS) flags |= O_EXCL;
	}

	if (open_mode &B_ERASE_FILE) flags |= O_TRUNC;
	if (open_mode &B_OPEN_AT_END) flags |= O_APPEND;

#ifdef O_LARGEFILE
	flags |= O_LARGEFILE;
#endif

	return flags;
}
#else
inline DWORD file_openmode_to_creation_disposition(uint32 open_mode)
{
	if (open_mode &B_CREATE_FILE) {
		if (open_mode &B_FAIL_IF_EXISTS) return CREATE_NEW;
		if (open_mode &B_ERASE_FILE) return CREATE_ALWAYS;
		return OPEN_ALWAYS;
	}

	if (open_mode &B_ERASE_FILE) return TRUNCATE_EXISTING;
	return OPEN_EXISTING;
}
#endif


#ifndef _WIN32
inline mode_t file_access_mode_to_mode_t(uint32 access_mode)
{
	mode_t mode = 0;

	if (access_mode &B_USER_READ) mode |= S_IRUSR;
	if (access_mode &B_USER_WRITE) mode |= S_IWUSR;
	if (access_mode &B_USER_EXEC) mode |= S_IXUSR;

	if (access_mode &B_GROUP_READ) mode |= S_IRGRP;
	if (access_mode &B_GROUP_WRITE) mode |= S_IWGRP;
	if (access_mode &B_GROUP_EXEC) mode |= S_IXGRP;

	if (access_mode &B_OTHERS_READ) mode |= S_IROTH;
	if (access_mode &B_OTHERS_WRITE) mode |= S_IWOTH;
	if (access_mode &B_OTHERS_EXEC) mode |= S_IXOTH;

	return mode;
}
#endif


BFile::BFile()
		: fFD(NULL), fMode(0)
{
}


BFile::BFile(const char *path, uint32 open_mode, uint32 access_mode)
		: fFD(NULL), fMode(0)
{
	SetTo(path, open_mode, access_mode);
}


BFile::BFile(const BEntry *entry, uint32 open_mode, uint32 access_mode)
		: fFD(NULL), fMode(0)
{
	SetTo(entry, open_mode, access_mode);
}


BFile::BFile(const BDirectory *dir, const char *leaf, uint32 open_mode, uint32 access_mode)
		: fFD(NULL), fMode(0)
{
	SetTo(dir, leaf, open_mode, access_mode);
}


BFile::BFile(const BFile &from)
		: fFD(NULL), fMode(0)
{
	operator=(from);
}


BFile::~BFile()
{
	if (fFD != NULL) {
#ifndef _WIN32
		close(*((int*)fFD));
		free(fFD);
#else
		CloseHandle((HANDLE)fFD);
#endif
	}
}


status_t
BFile::InitCheck() const
{
	return(fFD == NULL ?B_NO_INIT :B_OK);
}


status_t
BFile::SetTo(const char *path, uint32 open_mode, uint32 access_mode)
{
	if (path == NULL || *path == 0) return B_BAD_VALUE;

	BString strPath;
	path_expound(strPath, path, NULL, NULL);
	if (strPath.Length() <= 0) return B_BAD_VALUE;

#ifndef _WIN32
	int newFD = open(strPath.String(), file_openmode_to_flags(open_mode), file_access_mode_to_mode_t(access_mode));
	if (newFD == -1) return B_FILE_ERROR;
	if (fFD != NULL) {
		close(*((int*)fFD));
	} else if ((fFD = malloc(sizeof(int))) == NULL) {
		close(newFD);
		return B_NO_MEMORY;
	}
	*((int*)fFD) = newFD;
#else
	strPath.ReplaceAll("/", "\\");
	HANDLE newFD = CreateFile(strPath.String(),
	                          (open_mode &B_READ_WRITE) ? (GENERIC_WRITE | GENERIC_READ) :
	                          (open_mode &B_WRITE_ONLY ? GENERIC_WRITE : GENERIC_READ),
	                          FILE_SHARB_READ | FILE_SHARB_WRITE,
	                          NULL,
	                          file_openmode_to_creation_disposition(open_mode),
	                          FILE_ATTRIBUTB_NORMAL,
	                          NULL);
	if (newFD == INVALID_HANDLE_VALUE) return B_FILE_ERROR;
	if (fFD != NULL) CloseHandle((HANDLE)fFD);
	fFD = (void*)newFD;
	if (open_mode &B_OPEN_AT_END) SetFilePointer(newFD, 0, NULL, FILE_END);
#endif

	fMode = open_mode;

	return B_OK;
}


status_t
BFile::SetTo(const BEntry *entry, uint32 open_mode, uint32 access_mode)
{
	if (entry == NULL) return B_BAD_VALUE;

	BPath path;
	if (entry->GetPath(&path) != B_OK) return B_BAD_VALUE;

	return SetTo(path.Path(), open_mode, access_mode);
}


status_t
BFile::SetTo(const BDirectory *dir, const char *leaf, uint32 open_mode, uint32 access_mode)
{
	if (dir == NULL || leaf == NULL) return B_BAD_VALUE;

	BEntry entry;
	if (dir->GetEntry(&entry) != B_OK) return B_BAD_VALUE;

	BPath path;
	if (entry.GetPath(&path) != B_OK) return B_BAD_VALUE;

	if (path.Append(leaf, false) != B_OK) return B_BAD_VALUE;

	return SetTo(path.Path(), open_mode, access_mode);
}


void
BFile::Unset()
{
	if (fFD != NULL) {
#ifndef _WIN32
		close(*((int*)fFD));
		free(fFD);
#else
		CloseHandle((HANDLE)fFD);
#endif
	}

	fFD = NULL;
}


bool
BFile::IsReadable() const
{
	return(fFD == NULL ? false : true);
}


bool
BFile::IsWritable() const
{
	if (fFD == NULL) return false;
	return((fMode & (B_WRITE_ONLY |B_READ_WRITE)) ? true : false);
}


ssize_t
BFile::Read(void *buffer, size_t size)
{
	if (!IsReadable() || buffer == NULL) return -1;
#ifndef _WIN32
	return read(*((int*)fFD), buffer, size);
#else
	DWORD nRead = (DWORD)size;
	if (ReadFile((HANDLE)fFD, buffer, nRead, &nRead, NULL) == 0) return -1;
	return((ssize_t)nRead);
#endif
}


ssize_t
BFile::ReadAt(int64 pos, void *buffer, size_t size)
{
	if (!IsReadable() || buffer == NULL) return -1;
	int64 savePosition = Position();
	if (Seek(pos, B_SEEK_SET) <B_INT64_CONSTANT(0)) return -1;
	ssize_t retVal = Read(buffer, size);
	Seek(savePosition, B_SEEK_SET);
	return retVal;
}


ssize_t
BFile::Write(const void *buffer, size_t size)
{
	if (!IsWritable() || buffer == NULL) return -1;
#ifndef _WIN32
	return write(*((int*)fFD), buffer, size);
#else
	DWORD nWrote = (DWORD)size;
	if (WriteFile((HANDLE)fFD, buffer, nWrote, &nWrote, NULL) == 0) return -1;
	return((ssize_t)nWrote);
#endif
}


ssize_t
BFile::WriteAt(int64 pos, const void *buffer, size_t size)
{
	if (!IsWritable() || buffer == NULL) return -1;
	int64 savePosition = Position();
	if (Seek(pos, B_SEEK_SET) <B_INT64_CONSTANT(0)) return -1;
	ssize_t retVal = Write(buffer, size);
	Seek(savePosition, B_SEEK_SET);
	return retVal;
}


int64
BFile::Seek(int64 position, uint32 seek_mode)
{
	if (fFD == NULL || (seek_mode == B_SEEK_SET && position <B_INT64_CONSTANT(0))) return B_INT64_CONSTANT(-1);

#ifndef _WIN32
	int whence = SEEK_SET;
	if (seek_mode == B_SEEK_CUR) whence = SEEK_CUR;
	else if (seek_mode == B_SEEK_END) whence = SEEK_END;

	off_t pos = (off_t)-1;
	if (sizeof(off_t) > 4 || pos < (int64)B_MAXUINT32) pos = lseek(*((int*)fFD), (off_t)position, whence);
	if (pos == (off_t)-1) return B_INT64_CONSTANT(-1);
	return (int64)pos;
#else
	DWORD whence = FILE_BEGIN;
	if (seek_mode == B_SEEK_CUR) whence = FILE_CURRENT;
	else if (seek_mode == B_SEEK_END) whence = FILE_END;

	LARGB_INTEGER li;
	li.QuadPart = position;
	li.LowPart = SetFilePointer((HANDLE)fFD, li.LowPart, &li.HighPart, whence);
	if (li.LowPart == (DWORD)-1/*INVALID_SET_FILB_POINTER*/) return B_INT64_CONSTANT(-1);
	return li.QuadPart;
#endif
}


int64
BFile::Position() const
{
	if (fFD == NULL) return B_INT64_CONSTANT(-1);

#ifndef _WIN32
	off_t pos = lseek(*((int*)fFD), 0, SEEK_CUR);
	if (pos == (off_t)-1) return B_INT64_CONSTANT(-1);
	return (int64)pos;
#else
	LARGB_INTEGER li;
	li.HighPart = 0;
	li.LowPart = SetFilePointer((HANDLE)fFD, 0, &li.HighPart, FILE_CURRENT);
	if (li.LowPart == (DWORD)-1/*INVALID_SET_FILB_POINTER*/) return B_INT64_CONSTANT(-1);
	return li.QuadPart;
#endif
}


status_t
BFile::SetSize(int64 size)
{
	if (fFD == NULL || size <B_INT64_CONSTANT(0) || size == B_MAXINT64) return B_BAD_VALUE;
#ifndef _WIN32
	int status = -1;
	if (sizeof(off_t) > 4 || size < (int64)B_MAXUINT32) status = ftruncate(*((int*)fFD), (off_t)size);
	if (status != 0) return B_FILE_ERROR;

	lseek(*((int*)fFD), 0, SEEK_SET);
	return B_OK;
#else
	int64 oldPos = Position();
	if (Seek(size, B_SEEK_SET) <B_INT64_CONSTANT(0) || SetEndOfFile((HANDLE)fFD) == 0) {
		Seek(oldPos, B_SEEK_SET);
		return B_FILE_ERROR;
	}

	Seek(0, B_SEEK_SET);
	return B_OK;
#endif
}


BFile&
BFile::operator=(const BFile &from)
{
#ifndef _WIN32
	int newFD = (from.fFD == NULL ? -1 : dup(*((int*)from.fFD)));
	if (newFD == -1) {
		if (fFD != NULL) {
			close(*((int*)fFD));
			free(fFD);
			fFD = NULL;
		}
	} else {
		if (fFD != NULL) {
			close(*((int*)fFD));
			*((int*)fFD) = newFD;
		} else if ((fFD = malloc(sizeof(int))) == NULL) {
			close(newFD);
		} else {
			*((int*)fFD) = newFD;
		}
	}
#else
	if (fFD != NULL) {
		CloseHandle((HANDLE)fFD);
		fFD = NULL;
	}

	HANDLE newfFD = NULL;
	if (from.fFD) DuplicateHandle(GetCurrentProcess(), (HANDLE)from.fFD,
		                              GetCurrentProcess(), &newfFD,
		                              0, FALSE, DUPLICATE_SAME_ACCESS);
	if (newfFD) fFD = (void*)newfFD;
#endif

	fMode = from.fMode;

	return *this;
}

