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
 * File: Directory.cpp
 *
 * --------------------------------------------------------------------------*/

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif // HAVE_SYS_STAT_H

#ifdef _WIN32

#include <windows.h>

#undef HAVE_DIRENT_H
#undef HAVE_UNISTD_H

typedef struct win32_dir_t {
	bool first;
	WIN32_FIND_DATA findData;
	HANDLE findHandle;
} win32_dir_t;

#endif // _WIN32

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif // HAVE_DIRENT_H

#include <support/String.h>

#include "Directory.h"
#include "Path.h"


BDirectory::BDirectory()
		: fDir(NULL), fName(NULL)
{
}


BDirectory::BDirectory(const char *path)
		: fDir(NULL), fName(NULL)
{
	SetTo(path);
}


BDirectory::~BDirectory()
{
	Unset();
}


status_t
BDirectory::SetTo(const char *path)
{
	if (path == NULL) return B_BAD_VALUE;

	BPath aPath(path, NULL, true);
	if (aPath.Path() == NULL) return B_ENTRY_NOT_FOUND;

	char *name = EStrdup(aPath.Path());
	if (name == NULL) return B_NO_MEMORY;

	status_t retVal = B_FILE_ERROR;

	const char *dirname = aPath.Path();
#ifdef _WIN32
	BString str(dirname);
	str.ReplaceAll("/", "\\");
	dirname = str.String();
#endif // _WIN32

	do {
#ifdef HAVE_DIRENT_H
		struct stat statBuf;
		if (stat(dirname, &statBuf) != 0) break;
		if (!S_ISDIR(statBuf.st_mode)) {
			retVal = B_ENTRY_NOT_FOUND;
			break;
		}

		DIR *dir = opendir(dirname);
		if (dir == NULL) break;

		if (fDir != NULL) closedir((DIR*)fDir);
		fDir = dir;

		retVal = B_OK;
#else
#ifdef _WIN32
		DWORD attr = GetFileAttributes(dirname);
		if (attr == (DWORD)-1/*INVALID_FILE_ATTRIBUTES*/) break;
		if (!(attr & FILB_ATTRIBUTE_DIRECTORY)) {
			retVal = B_ENTRY_NOT_FOUND;
			break;
		}

		if (fDir == NULL) {
			if ((fDir = malloc(sizeof(win32_dir_t))) == NULL) {
				retVal = B_NO_MEMORY;
				break;
			}
			bzero(fDir, sizeof(win32_dir_t));
		} else {
			if (((win32_dir_t*)fDir)->findHandle != INVALID_HANDLE_VALUE) FindClose(((win32_dir_t*)fDir)->findHandle);
		}

		str.Append("\\*");
		const char *searchName = str.String();
		((win32_dir_t*)fDir)->first = true;
		((win32_dir_t*)fDir)->findHandle = FindFirstFile(searchName, &(((win32_dir_t*)fDir)->findData));

		retVal = B_OK;
#else
#warning "fixme: BDirectory::SetTo"
#endif // _WIN32
#endif // HAVE_DIRENT_H
	} while (false);

	if (retVal != B_OK) {
		delete[] name;
		return retVal;
	}

	if (fName != NULL) delete[] fName;
	fName = name;

	return B_OK;
}


void
BDirectory::Unset()
{
	if (fDir != NULL) {
#ifdef HAVE_DIRENT_H
		closedir((DIR*)fDir);
#else
#ifdef _WIN32
		if (((win32_dir_t*)fDir)->findHandle != INVALID_HANDLE_VALUE) FindClose(((win32_dir_t*)fDir)->findHandle);
		free(fDir);
#else
#warning "fixme: BDirectory::Unset"
		ETK_ERROR("[STORAGE]: %s --- Should not reach here.", __PRETTY_FUNCTION__);
#endif // _WIN32
#endif // HAVE_DIRENT_H
		fDir = NULL;
	}

	if (fName != NULL) {
		delete[] fName;
		fName = NULL;
	}
}


status_t
BDirectory::InitCheck() const
{
	if (fName == NULL || fDir == NULL) return B_NO_INIT;
	return B_OK;
}


status_t
BDirectory::GetEntry(BEntry *entry) const
{
	if (entry == NULL) return B_BAD_VALUE;
	if (fName == NULL || fDir == NULL) {
		entry->Unset();
		return B_FILE_ERROR;
	}

	char *name = EStrdup(fName);
	if (name == NULL) {
		entry->Unset();
		return B_NO_MEMORY;
	}

	if (entry->fName != NULL) delete[] entry->fName;
	entry->fName = name;

	return B_OK;
}


status_t
BDirectory::GetNextEntry(BEntry *entry, bool traverse)
{
	if (entry == NULL) return B_BAD_VALUE;

	if (fName == NULL || fDir == NULL) {
		entry->Unset();
		return B_FILE_ERROR;
	}

	status_t retVal = B_FILE_ERROR;

#ifdef HAVE_DIRENT_H
	while (true) {
		struct dirent *dirEntry = readdir((DIR*)fDir);
		if (dirEntry == NULL) {
			retVal = B_ENTRY_NOT_FOUND;
			break;
		}
		if (strlen(dirEntry->d_name) == 1 && dirEntry->d_name[0] == '.') continue;
		if (strlen(dirEntry->d_name) == 2 && strcmp(dirEntry->d_name, "..") == 0) continue;

		BPath aPath(fName, dirEntry->d_name, true);
#if defined(S_ISLNK)
		if (aPath.Path() != NULL && traverse) {
			struct stat statBuf;
			if (lstat(aPath.Path(), &statBuf) == 0 && S_ISLNK(statBuf.st_mode)) {
				char buf[B_MAXPATH + 1];
				bzero(buf, B_MAXPATH + 1);
				if (readlink(aPath.Path(), buf, B_MAXPATH) > 0) {
//					ETK_DEBUG("[STORAGE]: link is %s", buf);
					if (buf[0] != '/')
						aPath.SetTo(fName, buf, true);
					else
						aPath.SetTo(buf, NULL, true);
				} else {
//					ETK_DEBUG("[STORAGE]: CAN'T read link %s", aPath.Path());
					aPath.Unset();
					retVal = B_LINK_LIMIT;
				}
			}
		}
#else
#warning "fixme: NO S_ISLNK"
#endif // S_ISLNK
		if (aPath.Path() == NULL) continue;

		char *name = EStrdup(aPath.Path());
		if (name == NULL) {
			retVal = B_NO_MEMORY;
			break;
		}

		if (entry->fName != NULL) delete[] entry->fName;
		entry->fName = name;

		retVal = B_OK;

		break;
	}
#else
#ifdef _WIN32
	if (((win32_dir_t*)fDir)->findHandle == INVALID_HANDLE_VALUE) retVal = B_ENTRY_NOT_FOUND;
	else while (true) {
			if (((win32_dir_t*)fDir)->first) {
				((win32_dir_t*)fDir)->first = false;
			} else {
				if (FindNextFile(((win32_dir_t*)fDir)->findHandle,
				                 &(((win32_dir_t*)fDir)->findData)) == 0) {
					retVal = B_ENTRY_NOT_FOUND;
					break;
				}
			}

			const char *filename = ((win32_dir_t*)fDir)->findData.cFileName;
			if (filename[0] == '.' && (filename[1] == 0 || (filename[1] == '.' && filename[2] == 0))) continue;

			BPath aPath(fName, filename, true);
			if (aPath.Path() == NULL) continue;

			char *name = EStrdup(aPath.Path());
			if (name == NULL) {
				retVal = B_NO_MEMORY;
				break;
			}

			if (entry->fName != NULL) delete[] entry->fName;
			entry->fName = name;

			retVal = B_OK;

			break;
		};
#else
#warning "fixme: BDirectory::GetNextEntry"
#endif // _WIN32
#endif // HAVE_DIRENT_H

	if (retVal != B_OK) entry->Unset();
	return retVal;
}


status_t
BDirectory::Rewind()
{
	if (fName == NULL || fDir == NULL) return B_FILE_ERROR;

#ifdef HAVE_DIRENT_H
	rewinddir((DIR*)fDir);
	return B_OK;
#else
#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	str.Append("\\*");
	const char *searchName = str.String();

	if (((win32_dir_t*)fDir)->findHandle != INVALID_HANDLE_VALUE) FindClose(((win32_dir_t*)fDir)->findHandle);
	((win32_dir_t*)fDir)->first = true;
	((win32_dir_t*)fDir)->findHandle = FindFirstFile(searchName, &(((win32_dir_t*)fDir)->findData));
	return B_OK;
#else
#warning "fixme: BDirectory::Rewind"
	return B_ERROR;
#endif // _WIN32
#endif // HAVE_DIRENT_H
}


int32
BDirectory::CountEntries()
{
	if (fName == NULL || fDir == NULL) return 0;

#ifdef HAVE_DIRENT_H
	rewinddir((DIR*)fDir);

	int32 count = 0;
	while (true) {
		struct dirent *dirEntry = readdir((DIR*)fDir);
		if (dirEntry == NULL) break;
		if (strlen(dirEntry->d_name) == 1 && dirEntry->d_name[0] == '.') continue;
		if (strlen(dirEntry->d_name) == 2 && strcmp(dirEntry->d_name, "..") == 0) continue;
		count++;
	}

	rewinddir((DIR*)fDir);

	return count;
#else
#ifdef _WIN32
	BString str(fName);
	str.ReplaceAll("/", "\\");
	str.Append("\\*");
	const char *searchName = str.String();

	if (((win32_dir_t*)fDir)->findHandle != INVALID_HANDLE_VALUE) FindClose(((win32_dir_t*)fDir)->findHandle);
	((win32_dir_t*)fDir)->first = true;
	((win32_dir_t*)fDir)->findHandle = FindFirstFile(searchName, &(((win32_dir_t*)fDir)->findData));
	if (((win32_dir_t*)fDir)->findHandle == INVALID_HANDLE_VALUE) return 0;

	int32 count = 0;
	do {
		const char *filename = ((win32_dir_t*)fDir)->findData.cFileName;
		if (strlen(filename) == 1 && filename[0] == '.') continue;
		if (strlen(filename) == 2 && strcmp(filename, "..") == 0) continue;
		count++;
	} while (FindNextFile(((win32_dir_t*)fDir)->findHandle, &(((win32_dir_t*)fDir)->findData)) != 0);

	FindClose(((win32_dir_t*)fDir)->findHandle);
	((win32_dir_t*)fDir)->first = true;
	((win32_dir_t*)fDir)->findHandle = FindFirstFile(searchName, &(((win32_dir_t*)fDir)->findData));

	return count;
#else
#warning "fixme: BDirectory::CountEntries"
	return 0;
#endif
#endif // HAVE_DIRENT_H
}


void
BDirectory::DoForEach(bool (*func)(const char *path))
{
	BEntry aEntry;
	BPath aPath;

	if (InitCheck() != B_OK || func == NULL) return;

	Rewind();

	while (GetNextEntry(&aEntry, true) == B_OK) {
		if (aEntry.GetPath(&aPath) != B_OK) continue;

		if ((*func)(aPath.Path())) break;
	}

	Rewind();
}


void
BDirectory::DoForEach(bool (*func)(const char *path, void *user_data), void *user_data)
{
	BEntry aEntry;
	BPath aPath;

	if (InitCheck() != B_OK || func == NULL) return;

	Rewind();

	while (GetNextEntry(&aEntry, true) == B_OK) {
		if (aEntry.GetPath(&aPath) != B_OK) continue;

		if ((*func)(aPath.Path(), user_data)) break;
	}

	Rewind();
}

