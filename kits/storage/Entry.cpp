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
 * File: Entry.cpp
 *
 * --------------------------------------------------------------------------*/

#ifndef _WIN32
#define __USE_LARGEFILE64
#define __USE_FILE_OFFSET64
#include <unistd.h>
#else
#include <io.h>
#include <windows.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <support/String.h>

#include "Entry.h"
#include "Directory.h"

// implement in "Path.cpp"
extern status_t path_expound(BString &path, const char *dir, const char *leaf, bool *normalize);
extern status_t path_get_parent(BString &parent, const char *path);


BEntry::BEntry()
		: fName(NULL)
{
}


BEntry::BEntry(const char *dir, const char *leaf, bool traverse)
		: fName(NULL)
{
	SetTo(dir, leaf, traverse);
}


BEntry::BEntry(const BDirectory *dir, const char *leaf, bool traverse)
		: fName(NULL)
{
	SetTo(dir, leaf, traverse);
}


BEntry::BEntry(const char *path, bool traverse)
		: fName(NULL)
{
	SetTo(path, traverse);
}


BEntry::BEntry(const BEntry &entry)
		: fName(NULL)
{
	BEntry::operator=(entry);
}


BEntry::~BEntry()
{
	if (fName != NULL) delete[] fName;
}


status_t
BEntry::SetTo(const char *path, bool traverse)
{
	return SetTo(path, NULL, traverse);
}


status_t
BEntry::SetTo(const char *dir, const char *leaf, bool traverse)
{
	if (dir == NULL) return B_BAD_VALUE;

	BString str;
	if (path_expound(str, dir, leaf, NULL) != B_OK) return B_BAD_VALUE;

	BString parent;
	status_t status = path_get_parent(parent, str.String());
	if (status == B_ENTRY_NOT_FOUND) parent = str;
	else if (status != B_OK) return B_BAD_VALUE;

#ifdef _WIN32
	parent.ReplaceAll("/", "\\");
#endif

	bool parentExists;
#ifndef _WIN32
	struct stat st;
	parentExists = (stat(parent.String(), &st) == 0);
#else
	struct _stat st;
	parentExists = (_stat(parent.String(), &st) == 0);
#endif
	if (!parentExists) return B_ENTRY_NOT_FOUND;

	// TODO: traverse

	char *name = EStrdup(str.String());
	if (name == NULL) return B_NO_MEMORY;

	if (fName != NULL) delete[] fName;
	fName = name;

	return B_OK;
}


status_t
BEntry::SetTo(const BDirectory *dir, const char *leaf, bool traverse)
{
	if (dir == NULL || dir->InitCheck() != B_OK) return B_BAD_VALUE;
	return SetTo(dir->fName, leaf, traverse);
}


void
BEntry::Unset()
{
	if (fName != NULL) delete[] fName;
	fName = NULL;
}


status_t
BEntry::InitCheck() const
{
	if (fName == NULL) return B_NO_INIT;
	return B_OK;
}


bool
BEntry::Exists() const
{
	if (fName == NULL) return false;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();
#endif

#ifndef _WIN32
	struct stat st;
	return(stat(filename, &st) == 0);
#else
	struct _stat st;
	return(_stat(filename, &st) == 0);
#endif
}


bool
BEntry::IsHidden() const
{
	bool retVal = false;

	BPath aPath(fName);
	const char *leaf = aPath.Leaf();
	if (!(leaf == NULL || *leaf != '.')) retVal = true;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");

	if (GetFileAttributes(str.String()) & FILE_ATTRIBUTE_HIDDEN) retVal = true;
#endif

	return retVal;
}


bool
BEntry::IsFile() const
{
	if (fName == NULL) return false;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();

	struct _stat st;
	if (_stat(filename, &st) != 0) return false;
	return((st.st_mode & _S_IFREG) ? true : false);
#else
	struct stat st;
	if (stat(filename, &st) != 0) return false;
	return S_ISREG(st.st_mode);
#endif
}


bool
BEntry::IsDirectory() const
{
	if (fName == NULL) return false;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();

	struct _stat st;
	if (_stat(filename, &st) != 0) return false;
	return((st.st_mode & _S_IFDIR) ? true : false);
#else
	struct stat st;
	if (stat(filename, &st) != 0) return false;
	return S_ISDIR(st.st_mode);
#endif
}


bool
BEntry::IsSymLink() const
{
#ifdef S_ISLNK
	struct stat st;
	if (fName == NULL || lstat(fName, &st) != 0) return false;
	return S_ISLNK(st.st_mode);
#else
	return false;
#endif
}


status_t
BEntry::GetSize(int64 *file_size) const
{
	if (fName == NULL || file_size == NULL) return B_ERROR;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();

	struct _stati64 stat;
	if (_stati64(filename, &stat) != 0) return B_ERROR;
	*file_size = (int64)stat.st_size;
#elif defined(HAVE_STAT64)
	struct stat64 stat;
	if (stat64(filename, &stat) != 0) return B_ERROR;
	*file_size = (int64)stat.st_size;
#else
	struct stat st;
	if (stat(filename, &st) != 0) return B_ERROR;
	*file_size = (int64)st.st_size;
#endif

	return B_OK;
}


status_t
BEntry::GetModificationTime(bigtime_t *time) const
{
	if (fName == NULL || time == NULL) return B_ERROR;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();

	struct _stat stat;
	if (_stat(filename, &stat) != 0) return B_ERROR;
	*time = B_INT64_CONSTANT(1000000) * (bigtime_t)stat.st_mtime;
#else
	struct stat st;
	if (stat(filename, &st) != 0) return B_ERROR;
	*time = B_INT64_CONSTANT(1000000) * (bigtime_t)st.st_mtime;
#endif

	return B_OK;
}


status_t
BEntry::GetCreationTime(bigtime_t *time) const
{
	if (fName == NULL || time == NULL) return B_ERROR;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();

	struct _stat stat;
	if (_stat(filename, &stat) != 0) return B_ERROR;
	*time = B_INT64_CONSTANT(1000000) * (bigtime_t)stat.st_ctime;
#else
	struct stat st;
	if (stat(filename, &st) != 0) return B_ERROR;
	*time = B_INT64_CONSTANT(1000000) * (bigtime_t)st.st_ctime;
#endif

	return B_OK;
}


status_t
BEntry::GetAccessTime(bigtime_t *time) const
{
	if (fName == NULL || time == NULL) return B_ERROR;

	const char *filename = (const char*)fName;

#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	filename = str.String();

	struct _stat stat;
	if (_stat(filename, &stat) != 0) return B_ERROR;
	*time = B_INT64_CONSTANT(1000000) * (bigtime_t)stat.st_atime;
#else
	struct stat st;
	if (stat(filename, &st) != 0) return B_ERROR;
	*time = B_INT64_CONSTANT(1000000) * (bigtime_t)st.st_atime;
#endif

	return B_OK;
}


const char*
BEntry::Name() const
{
	if (fName == NULL) return NULL;

	size_t nameLen = strlen(fName);

#ifdef _WIN32
	if (nameLen <= 3) return NULL;
#else
	if (nameLen == 1 && *fName == '/') return NULL;
#endif

	const char *tmp = fName + nameLen - 1;
	for (; nameLen > 0; nameLen--, tmp--) {
		if (*tmp == '/') return(tmp + 1);
	}

	return NULL;
}


status_t
BEntry::GetName(char *buffer, size_t bufferSize) const
{
	const char *name = Name();
	if (name == NULL) return B_ERROR;

	if (buffer == NULL && bufferSize == 0) return B_BAD_VALUE;
	if (bufferSize < strlen(name) + 1) return B_NAME_TOO_LONG;
	memcpy(buffer, name, strlen(name) + 1);
	return B_OK;
}


const char*
BEntry::Path() const
{
	return fName;
}


status_t
BEntry::GetPath(BPath *path) const
{
	if (path == NULL) return B_BAD_VALUE;
	if (fName == NULL) {
		path->Unset();
		return B_NO_INIT;
	}

	return path->SetTo(fName, NULL, false);
}


status_t
BEntry::GetParent(BEntry *entry) const
{
	if (entry == NULL) return B_BAD_VALUE;
	if (fName == NULL) return B_NO_INIT;

	BString str;
	status_t status = path_get_parent(str, fName);
	if (status != B_OK) return status;

	return entry->SetTo(str.String(), false);
}


status_t
BEntry::GetParent(BPath *path) const
{
	if (path == NULL) return B_BAD_VALUE;
	if (fName == NULL) return B_NO_INIT;

	BString str;
	status_t status = path_get_parent(str, fName);
	if (status != B_OK) return status;

	return path->SetTo(str.String(), NULL, false);
}


status_t
BEntry::GetParent(BDirectory *dir) const
{
	if (dir == NULL) return B_BAD_VALUE;
	if (fName == NULL) return B_NO_INIT;

	BString str;
	status_t status = path_get_parent(str, fName);
	if (status != B_OK) return status;

	return dir->SetTo(str.String());
}


bool
BEntry::operator==(const BEntry &entry) const
{
	if (fName == NULL && entry.fName == NULL) return true;
	if (fName == NULL || entry.fName == NULL) return false;
	return(!(strlen(fName) != strlen(entry.fName) || strcmp(fName, entry.fName) != 0));
}


bool
BEntry::operator!=(const BEntry &entry) const
{
	if (fName == NULL && entry.fName == NULL) return false;
	if (fName == NULL || entry.fName == NULL) return true;
	if (strlen(fName) != strlen(entry.fName)) return true;
	return(strcmp(fName, entry.fName) != 0);
}


BEntry&
BEntry::operator=(const BEntry &entry)
{
	if (fName != NULL) delete[] fName;
	fName = EStrdup(entry.fName);
	return *this;
}

